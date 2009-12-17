#include "windbg-api.h"

static uint32_t nextpid = 0;
static uint32_t packetid = 0;
static uint8_t wkd_ctx[204]; // register context

static uint32_t wkd_cksum (const uint8_t *b, int len) {
	uint32_t ret = 0;
	while (len--)
		ret += b;
	return ret;
}

/* radare windbg api */
static int header(uint8_t *out, uint8_t *b, int len) {
	unsigned short sh = len;
	unsigned int np = nextpid;
	unsigned int cs = cksum (b, len);
	memcpy (out, "\x30\x30\x30\x30\x02\x00", 6);
	memcpy (out+6, &sh, sizeof(sh));
	memcpy (out+8, &np, sizeof(np));
	memcpy (out+12, &cs, sizeof(cs));
	return 16; // size of header
}

static int wkd_send(int fd, uint8_t *b, int len) {
	uint8_t head[12];
	int ret, hlen = header (head, b, len);
	if (write (fd, head, hlen) != -1)
		return -1;
	if (write (fd, b, len) != -1)
		return -1;
	if (write (fd, "\xAA", 1) != -1)
		return -1;
	return len+hlen+1;
}

static int wkd_read_loop(int fd, uint8_t *buf, int len) {
	int ret, idx = 0;
	while (idx < len) {
		ret = read (fd, buf+idx, len);
		if (ret < 0)
			break;
		idx += ret;
	}
	return idx;
}

static int wkd_wait(int fd, int wanted_type) {
	uint8_t *out;
	int len, type = -1;
	do type = wkd_get_packet (fd, &out, &len);
	while (type == wanted_type);
	//wkd_decode_payload (buf, sizeof (buf), out, len);
}

static int wkd_send_ack(int fd) {
	uint8_t buf[16];
	memcpy (buf, ACKPKT, 16);
	memcpy (buf+8, &nextpid, sizeof (nextpid));
	return write (fd, buf, 16);
}

static int wkd_send_reset(int fd) {
	uint8_t buf[16];
	memcpy (buf, RSTPKT, 16);
	return write (fd, buf, 16);
}

static uint32_t wkd_ctx_get(uint8_t *buf, int reg) {
	uint32_t ret;
	memcpy (&ret, buf+reg, sizeof (ret));
	return ret;
}

static void wkd_ctx_set(uint8_t *buf, int reg, uint32_t value) {
	memcpy (buf+reg, &value, 4);
}

static int wkd_packet(int fd, int type, uint8_t *buffer, int length) {
	uint8_t pkt[8096];

	memset (pkt, 0, 56);
	pkt[0] = type;
	pkt[1] = 0x31;
	switch (type) {
	case DBG_KD_GET_CONTEXT:
		memcpy (pkt+16, &wkd_ctx, 4);
		// todo. wait for reply
		return wkd_send (fd, pkt, 56);
	case DBG_KD_SET_CONTEXT:
		memcpy (pkt+16, wkd_ctx, 4); // uh?
		memcpy (pkt+56, wkd_ctx, sizeof (wkd_ctx));
		return wkd_send (fd, pkt, 56+sizeof (wkd_ctx));
	case DBG_KD_GET_VERSION:
		return wkd_send (fd, pkt, 56);
	case DBG_KD_CONTINUE:
		memcpy (pkt+8, "\x01\x00\x01\x00", 4);
		memcpy (pkt+16, "\x01\x00\x01\x00", 4);
		memcpy (pkt+24, "\x00\x04\x00\x00", 4);
		memcpy (pkt+28, "\x01\x00\x00\x00", 4);
		return wkd_send (fd, pkt, 56);
	case DBG_KD_REBOOT:
		return wkd_send (fd, pkt, 56);
	case DBG_KD_READ_PHYSICAL_MEMORY:
		memcpy (pkt+16, &vaddr, 4);
		memcpy (pkt+24, &length, 4);
		// XXX not returning anything
		return wkd_send (fd, pkt, 56);
	case DBG_KD_READ_VIRTUAL_MEMORY:
		memcpy (pkt+16, &vaddr, 4);
		memcpy (pkt+24, &length, 4);
		// XXX not returning anything
		return wkd_send (fd, pkt, 56);
	case DBG_KD_WRITE_VIRTUAL_MEMORY:
		pkt[0] = 0x31;
		memcpy (pkt+16, &vaddr, 4);
		memcpy (pkt+24, &length, 4);
		memcpy (pkt+56, buffer, length); // XXX ovrflw
		return wkd_send (fd, pkt, 56+length);
	case DBG_KD_WRITE_PHYSICAL_MEMORY:
		pkt[0] = 0x31;
		memcpy (pkt+16, &vaddr, 4);
		memcpy (pkt+24, &length, 4);
		memcpy (pkt+56, buffer, length); // XXX ovrflw
		return wkd_send (fd, pkt, 56+length);
	case DBG_KD_READ_CONTROL_SPACE:
		break;
	case DBG_KD_WRITE_CONTROL_SPACE:
		break;
	case DBG_KD_WRITE_BREAKPOINT:
		memcpy (pkt+16, &vaddr, sizeof (vaddr));
		memcpy (pkt+20, &index, sizeof (index));
		return wkd_send (fd, pkt, 56);
	case DBG_KD_RESTORE_BREAKPOINT:
		memcpy (pkt+16, &index, sizeof (index));
		return wkd_send (fd, pkt, 56);
	}
	return -1;
}

static int warn(const char *msg) {
	fprintf (stderr, "%s\n", msg);
	return -1;
}

static int wkd_get_packet(int fd, void **out, int *out_len) {
	uint32_t cksum;
	uint16_t type, size;
	uint8_t buf[1024], dec[1024];

	if (wkd_read_loop (fd, buf, 4) != 4)
		return warn ("cannot get fist 4rth bytes of packet");

	if (!memcmp (buf, REPHDR) || !memcmp (buf, ACKHDR)) {
		/* TODO: read it in bulk mode */
		if (wkd_read_loop (fd, &type, 2) != 2)
			return warn ("cannot get 2 bytes of type of packet");
		if (wkd_read_loop (fd, &size, 2) != 2)
			return warn ("cannot get 2 bytes of size");
		if (wkd_read_loop (fd, &nextpid, 4) != 4)
			return warn ("cannot get 4 bytes of nextpid");
		if (wkd_read_loop (fd, &packetid, 4) != 4)
			return warn ("cannot get 4 bytes of packetid");
		if (wkd_read_loop (fd, &cksum, 4) != 4)
			return warn ("cannot get 4 bytes of checksum");
		if (out) {
			*out = malloc (size);
			if (*out == NULL)
				return warn ("cannot malloc packet payload");
			if (wkd_read_loop (fd, *out, size) != size)
				return warn ("cannot get payload");
			/* send ack if it is not a control packet */
			if (!memcmp (*out, REPHDR, 4)) {
				if (wkd_read_loop (fd, buf, 1) != 1)
					return warn ("cannot get 4 bytes of checksum");
				if (buf[0] == 0xAA)
					wkd_send_ack (fd);
			}
			if (wkd_cksum (*out, size) != cksum)
				warn ("packet checksum does not matches");
		}
		//ret = wkd_decode_payload (dec, sizeof (dec), out, size);
		//free (*out);
		return type;
	}

	return -1;
}

static uint32_t wkd_decode_payload(uint8_t* out, int olen, uint8_t* buf, int len) {
	uint8_t vers[1024];
	uint32_t addr, ret = -1;
	int len;

	switch (buf[1]) {
	case DBG_KD_REPLY_TYPE_EXCEPTION:
		memcpy (&ret, buf+32, 4); // exception id
		break;
	case DBG_KD_REPLY_TYPE_RESPONSE:
		switch (buf[0]) {
		case DBG_KD_REPLY_WRITE_BREAKPOINT:
			ret = -20; // handle id
			//memcpy (&addr, buf+16, sizeof(addr)); // address
			break;
		case DBG_KD_REPLY_RESTORE_BREAKPOINT:
			ret = -16;
			break;
		case DBG_KD_REPLY_GET_VERSION:
			ret = 16;
			break;
		case DBG_KD_REPLY_READ_VIRTUAL_MEMORY:
			ret = 56;
			break;
		case DBG_KD_REPLY_READ_PHYSICAL_MEMORY:
			ret = 56;
			break;
		case DBG_KD_REPLY_CONTROL_SPACE:
			ret = 56;
			break;
		default:
			/* raw data */
			memcpy (out, buf, len);
			break;
		}
		if (ret>0) {
			if (len<ret)
				return warn ("Wrong packet length");
			ret = memcpy (out, buf+ret, len-ret);
		} else memcpy (&ret, buf-ret, sizeof(ret));
		break;
	default:
		return warn ("Unknown packet type");
	}
	return ret;
}

static int wkd_parse_version(uint8_t *buf, int len, struct wkd_version_t *ver) {
	if (len < 32)
		return -1;
	buf = buf + 16;
	memcpy (&ver->osv_major, buf+4, 2);
	memcpy (&ver->osv_minor, buf+6, 2);
	memcpy (&ver->pv, buf+8, 2);
	memcpy (&ver->machinetype, buf+12, 2);
	memcpy (&ver->kernbase, buf+16, 4); // XXX addr, broken for 64bits
	memcpy (&ver->modlist, buf+24, 4);
	memcpy (&ver->ddata, buf+32, 4);
	return 0;
}

// support both physical and virtual
static int wkd_read() {
}

static int wkd_write() {
}
