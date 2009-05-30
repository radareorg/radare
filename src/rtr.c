#include "main.h"
#include "rtr.h"

static int rtr_n = 0;
static struct rtr_host_t rtr_host[RTR_MAX_HOSTS];

void rtr_help()
{
	eprintf(" =                  ; List hosts\n"
			" =[fd] cmd          ; Exec cmd in host n\n"
			" =+ [proto://]host  ; Add host (default protocol is rap://)\n"
			" =-[fd]             ; Remove all hosts or host 'fd'\n"
			" ==[fd]             ; Open remote session with host 'fd'\n"
			"NOTE: Last used host comes to be default\n");
}

void rtr_list()
{
	int i;

	for (i = 0; i < RTR_MAX_HOSTS; i++)
		if (rtr_host[i].fd) {
			eprintf("%i - ", rtr_host[i].fd);
			if (rtr_host[i].proto == RTR_PROT_TCP)
				eprintf("tcp://");
			else if (rtr_host[i].proto == RTR_PROT_TCP)
				eprintf("udp://");
			else eprintf("rap://");
			eprintf("%s:%i/%s\n",
					rtr_host[i].host, rtr_host[i].port, rtr_host[i].file);
	}
}
		
void rtr_add(char *input)
{
	char *host, *file, *ptr, buf[1024];
	int proto, port, fd, i;

	/* Parse uri */
	if ((ptr = strstr(input, "tcp://")))
		proto = RTR_PROT_TCP;
	else if ((ptr = strstr(input, "udp://")))
		proto = RTR_PROT_UDP;
	else if ((ptr = strstr(input, "rap://")))
		proto = RTR_PROT_RAP;
	else {
		eprintf("Error: Unknown protocol\n");
		return;
	}
	host = ptr+6;

	if (!(ptr = strchr(host, ':'))) {
		eprintf("Error: Port is not specified\n");
		return;
	}
	ptr[0] = '\0';
	ptr = ptr+1;

	if (!(file = strchr(ptr, '/'))) {
		eprintf("Error: File is not specified\n");
		return;
	}
	file[0] = '\0';
	file = file+1;

	port = get_math(ptr);

	fd = socket_connect(host, port);
	if (fd == -1) {
		eprintf("Error: Cannot connect to '%s' (%d)\n", host, port);
		return;
	}

	eprintf("Connected to: %s at port %d\n", host, port);
	/* send */
	buf[0] = RTR_RAP_OPEN;
	buf[1] = 0;
	buf[2] = (uchar)strlen(file);
	memcpy(buf+3, file, buf[2]);
	write(fd, buf, 3+buf[2]);
	/* read */
	eprintf("waiting... "); fflush(stdout);
	read(fd, buf, 5);
	endian_memcpy((uchar *)&i, (uchar*)buf+1, 4);
	if (buf[0] != (char)(RTR_RAP_OPEN|RTR_RAP_REPLY) || i<= 0) {
		eprintf("Error: Wrong reply\n");
		return;
	}
	eprintf("ok\n");

	for (i = 0; i < RTR_MAX_HOSTS; i++)
		if (!rtr_host[i].fd) {
			rtr_host[i].proto = proto;
			memcpy(rtr_host[i].host, host, 512);
			rtr_host[i].port = port;
			memcpy(rtr_host[i].file, file, 1024);
			rtr_host[i].fd = fd;
			rtr_n = i;
			break;
		}

	rtr_list();
}

void rtr_remove(char *input)
{
	int fd, i;

	if (input[1] >= '0' && input[1] <= '9') {
		fd = get_math(input+1);
		for (i = 0; i < RTR_MAX_HOSTS; i++)
			if (rtr_host[i].fd == fd) {
				socket_close(rtr_host[i].fd);
				rtr_host[i].fd = 0;
				if (rtr_n == i)
					for (rtr_n = 0; !rtr_host[rtr_n].fd && rtr_n < RTR_MAX_HOSTS; rtr_n++);
				break;
		}
	} else {
		for (i = 0; i < RTR_MAX_HOSTS; i++)
			if (rtr_host[i].fd)
				socket_close(rtr_host[i].fd);
		memset(rtr_host, '\0', RTR_MAX_HOSTS * sizeof(struct rtr_host_t));
		rtr_n = 0;
	}
}

void rtr_session(char *input)
{
	eprintf("TODO: Start session\n");
}

void rtr_cmd(char *input)
{
	char bufw[1024], bufr[16];
	char *cmd = NULL, *cmd_output = NULL;
	int i, fd;
	u64 cmd_len;

	if (input[0] >= '0' && input[0] <= '9') {
		fd = get_math(input);
		for (rtr_n = 0; rtr_host[rtr_n].fd != fd && rtr_n < RTR_MAX_HOSTS; rtr_n++);
	}

	if (!rtr_host[rtr_n].fd){
		eprintf("Error: Unknown host\n");
		return;
	}

	cmd = strchr(input, ' ');
	if (!cmd) {
		eprintf("error\n");
		return;
	}
	cmd = cmd + 1;

	/* send */
	bufw[0] = RTR_RAP_CMD;
	i = strlen(cmd);
	endian_memcpy(bufw+1, (uchar*)&i, 4);
	memcpy(bufw+5, cmd, i);
	write(rtr_host[rtr_n].fd, bufw, 5+i);
	/* read */
	read(rtr_host[rtr_n].fd, bufr, 9);
	if (bufr[0] != (char)(RTR_RAP_CMD|RTR_RAP_REPLY)) {
		eprintf("error\n");
		return;
	}
	endian_memcpy((uchar*)&cmd_len, bufr+1, 8);
	if (i == 0)
		return;
	if (i < 0) {
		eprintf("error\n");
		return;
	}
	cmd_output = malloc(cmd_len);
	if (!cmd_output) {
		eprintf("error\n");
		return;
	}
	read(rtr_host[rtr_n].fd, cmd_output, cmd_len);
	cons_printf("%s\n", cmd_output);
	free(cmd_output);
}
