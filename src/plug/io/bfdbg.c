/*
 * Copyright (C) 2008
 *       pancake <@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <main.h>
#include <plugin.h>

/* VM */

struct bfvm_cpu_t {
	u64 eip;
	u64 esp;
	int ptr;
	int trace;
	u64 base;
	u8 *mem;
	u32 size;
	u64 screen;
	int screen_idx;
	int screen_size;
	u8 *screen_buf;
	u64 input;
	int input_idx;
	int input_size;
	u8 *input_buf;
	int circular; /* circular memory */
} bfvm_cpu;

static u8 bfvm_op()
{
	u8 buf[4];
	if (!radare_read_at(bfvm_cpu.eip, buf, 4))
		return 0xff;
	return buf[0];
}

static int bfvm_in_trap()
{
	switch(bfvm_op()) {
	case 0x00:
	case 0xff:
		return 1;
	}
	return 0;
}

static int bfvm_init(u32 size, int circular)
{
	memset(&bfvm_cpu,'\0', sizeof(struct bfvm_cpu_t));

	/* data */
	bfvm_cpu.mem = (u8 *)malloc(size);
	if (bfvm_cpu.mem == NULL)
		return 0;
	if (size>65536)
		bfvm_cpu.base = 0xd0000000;
	else bfvm_cpu.base = 0xd0000;
	memset(bfvm_cpu.mem,'\0',size);

	/* setup */
	bfvm_cpu.circular = circular;
	bfvm_cpu.eip = 0; // look forward nops
	bfvm_cpu.size = size;

	/* screen */
	bfvm_cpu.screen = 0x50000;
	bfvm_cpu.screen_size = 4096;
	bfvm_cpu.screen_buf = (u8*)malloc( bfvm_cpu.screen_size );
	memset(bfvm_cpu.screen_buf, '\0', bfvm_cpu.screen_size);

	/* input */
	bfvm_cpu.input = 0x10000;
	bfvm_cpu.input_size = 4096;
	bfvm_cpu.input_buf = (u8*)malloc( bfvm_cpu.input_size );
	memset(bfvm_cpu.input_buf, '\0', bfvm_cpu.input_size);
	bfvm_cpu.esp = bfvm_cpu.base;
	eprintf("BFVM INIT!\n");

	return 1;
}

static struct bfvm_cpu_t *bfvm_destroy(struct bfvm_cpu_t *bfvm)
{
	free(bfvm_cpu.mem);
	bfvm_cpu.mem = 0;

	free(bfvm_cpu.screen_buf);
	bfvm_cpu.screen_buf = 0;

	free(bfvm);
	return NULL;
}

static u8 *bfvm_get_ptr_at(u64 at)
{
	if (at >= bfvm_cpu.base)
		at-=bfvm_cpu.base;

	if (at<0) {
		if (bfvm_cpu.circular)
			at = bfvm_cpu.size-2;
		else at=0;
	} else
	if (at >= bfvm_cpu.size) {
		if (bfvm_cpu.circular)
			at = 0;
		else at = bfvm_cpu.size-1;
	}
	if (at<0)
		at = 0;
	return bfvm_cpu.mem+at;
}

static u8 *bfvm_get_ptr()
{
//return bfvm_cpu.mem;
	return bfvm_get_ptr_at(bfvm_cpu.ptr);
}

static u8 bfvm_get()
{
	u8 *ptr = bfvm_get_ptr();
	if (ptr != NULL)
		return ptr[0];
	return 0;
}

static void bfvm_inc()
{
	u8 *mem = bfvm_get_ptr();
	if (mem != NULL)
		mem[0]++;
}

static void bfvm_dec()
{
	u8 *mem = bfvm_get_ptr();
	if (mem != NULL)
		mem[0]--;
}

static int bfvm_reg_set(const char *str)
{
	char *ptr = strchr(str, ' ');
	if (ptr == NULL)
		return 0;
	if (strstr(str, "eip")) {
		bfvm_cpu.eip = get_math(ptr+1);
	} else
	if (strstr(str, "esp")) {
		bfvm_cpu.esp = get_math(ptr+1);
	} else
	if (strstr(str, "ptr")) {
		bfvm_cpu.ptr = get_math(ptr+1);
	}
	return 1;
}

/* screen and input */
static void bfvm_peek()
{
	int idx = bfvm_cpu.input_idx;
	u8 *ptr = bfvm_get_ptr();

	if (idx >= bfvm_cpu.input_size)
		idx = 0;

	if (ptr) {
		*ptr = bfvm_cpu.input_buf[idx];
		bfvm_cpu.input_idx = idx+1;
	}

}

static void bfvm_poke()
{
	int idx = bfvm_cpu.screen_idx;
	bfvm_cpu.screen_buf[idx] = bfvm_get();
	bfvm_cpu.screen_idx = idx+1;
}

int bfvm_trace_op(u8 op)
{
	u8 g;
	switch(op) {
	case '\0':
		cons_printf(" ; trap (%02x)\n", op);
	case '.':
	case ',':
	case '+':
	case '-':
	case '>':
	case '<':
		cons_printf("%c", op);
		break;
	case '[':
	case ']':
		g = bfvm_get();
		cons_printf("%c  ; [ptr] = %d\n", op, g);
		if (g!= 0)
			cons_printf("[");
		break;
	}
}

#define T if (bfvm_cpu.trace)
/* debug */
static int bfvm_step(int over)
{
	u8 *buf;
	u8 op = bfvm_op();
	u8 op2;

	do {
		T bfvm_trace_op(op);
		switch(op) {
		case '\0':
			/* trap */
			return;
		case '.':
			buf = bfvm_get_ptr();
			bfvm_poke();
			break;
		case ',':
			bfvm_peek();
			/* TODO read */
			break;
		case '+':
			bfvm_inc();
			break;
		case '-':
			bfvm_dec();
			break;
		case '>':
			bfvm_cpu.ptr++;
			break;
		case '<':
			bfvm_cpu.ptr--;
			break;
		case '[':
			break;
		case ']':
			if (bfvm_get() != 0) {
				do {
					bfvm_cpu.eip--;
					/* control underflow */
					if (bfvm_cpu.eip<0) {
						bfvm_cpu.eip = 0;
						break;
					}
				} while(bfvm_op()!='[');
			}
			break;
		default:
			break;
		}
		bfvm_cpu.eip++;
		op2 = bfvm_op();
	} while(over && op == op2);
}

static int bfvm_contsc()
{
	radare_controlc();
	while(!config.interrupted) {
		bfvm_step(0);
		if (bfvm_in_trap()) {
			eprintf("Trap instruction at 0x%08llx\n", bfvm_cpu.eip);
			break;
		}
		switch(bfvm_op()) {
		case ',':
			eprintf("contsc: read from input trap\n");
			config.interrupted=1;
			continue;
		case '.':
			eprintf("contsc: print to screen trap\n");
			config.interrupted=1;
			continue;
		}
	}
	radare_controlc_end();
}

static int bfvm_cont(u64 until)
{
	radare_controlc();
	while(!config.interrupted && bfvm_cpu.eip != until) {
		bfvm_step(0);
		if (bfvm_in_trap()) {
			eprintf("Trap instruction at 0x%08llx\n", bfvm_cpu.eip);
			break;
		}
	}
	radare_controlc_end();
}

static int bfvm_trace(u64 until)
{
	bfvm_cpu.trace=1;
	bfvm_cont(until);
	bfvm_cpu.trace=0;
}

static void bfvm_show_regs(int rad)
{
	if (rad) {
		cons_printf("fs regs\n");
		cons_printf("f eip @ 0x%08llx\n", (u64)bfvm_cpu.eip);
		cons_printf("f esp @ 0x%08llx\n", (u64)bfvm_cpu.esp);
		cons_printf("f ptr @ 0x%08llx\n", (u64)bfvm_cpu.ptr+bfvm_cpu.base);
		cons_printf("fs *\n");
	} else {
		u8 ch = bfvm_get();
		cons_printf("  eip  0x%08llx     esp  0x%08llx\n",
			(u64)bfvm_cpu.eip, (u64)bfvm_cpu.esp);
		cons_printf("  ptr  0x%08x     [ptr]  %d = 0x%02x '%c'\n",
			(u32)bfvm_cpu.ptr, ch, ch, is_printable(ch)?ch:' ');
	}
}

static void bfvm_maps(int rad)
{
	if (rad) {
		cons_printf("fs sections\n");
		cons_printf("e cmd.vprompt=px@screen\n");
		cons_printf("f section_code @ 0x%08llx\n", (u64)0LL);
		cons_printf("f section_code_end @ 0x%08llx\n", (u64)config.size);
		cons_printf("f section_data @ 0x%08llx\n", (u64)bfvm_cpu.base);
		cons_printf("f section_data_end @ 0x%08llx\n", (u64)bfvm_cpu.base+bfvm_cpu.size);
		cons_printf("f screen @ 0x%08llx\n", (u64)bfvm_cpu.screen);
		cons_printf("f section_screen @ 0x%08llx\n", (u64)bfvm_cpu.screen);
		cons_printf("f section_screen_end @ 0x%08llx\n", (u64)bfvm_cpu.screen+bfvm_cpu.screen_size);
		cons_printf("f input @ 0x%08llx\n", (u64)bfvm_cpu.input);
		cons_printf("f section_input @ 0x%08llx\n", (u64)bfvm_cpu.input);
		cons_printf("f section_input_end @ 0x%08llx\n", (u64)bfvm_cpu.input+bfvm_cpu.input_size);
		cons_printf("fs *\n");
	} else {
		cons_printf("0x%08llx - 0x%08llx rwxu 0x%08llx .code\n", (u64)0, (u64)config.size, config.size);
		cons_printf("0x%08llx - 0x%08llx rw-- 0x%08llx .data\n", bfvm_cpu.base, bfvm_cpu.base+bfvm_cpu.size, bfvm_cpu.size);
		cons_printf("0x%08llx - 0x%08llx rw-- 0x%08llx .screen\n", bfvm_cpu.screen, bfvm_cpu.screen+bfvm_cpu.screen_size, bfvm_cpu.screen_size);
		cons_printf("0x%08llx - 0x%08llx rw-- 0x%08llx .input\n", bfvm_cpu.input, bfvm_cpu.input+bfvm_cpu.input_size, bfvm_cpu.input_size);
	}
}

/* PLUGIN */
static u64 cur_seek = 0;

static int bfdbg_fd = -1;

int bfdbg_handle_fd(int fd)
{
	return fd == bfdbg_fd;
}

int bfdbg_handle_open(const char *file)
{
	if (!memcmp(file, "bfdbg://", 8))
		return 1;
	return 0;
}

static ssize_t bfdbg_write(int fd, const void *buf, size_t count)
{
	if (cur_seek>=bfvm_cpu.screen && cur_seek<=bfvm_cpu.screen+bfvm_cpu.screen_size) {
		memcpy(bfvm_cpu.screen_buf+cur_seek-bfvm_cpu.screen, buf, count);
		return count;
	}
	if (cur_seek>=bfvm_cpu.input && cur_seek<=bfvm_cpu.input+bfvm_cpu.input_size) {
		memcpy(bfvm_cpu.input_buf+cur_seek-bfvm_cpu.input, buf, count);
		return count;
	}
	if (cur_seek>=bfvm_cpu.base) {
		memcpy(bfvm_cpu.mem+cur_seek-bfvm_cpu.base, buf, count);
		return count;
	}
	// TODO: call real read/write here?!?
        return write(fd, buf, count);
}

static ssize_t bfdbg_read(int fd, void *buf, size_t count)
{
	if (cur_seek>=bfvm_cpu.screen && cur_seek<=bfvm_cpu.screen+bfvm_cpu.screen_size) {
		memcpy(buf, bfvm_cpu.screen_buf, count);
		return count;
	}
	if (cur_seek>=bfvm_cpu.input && cur_seek<=bfvm_cpu.input+bfvm_cpu.input_size) {
		memcpy(buf, bfvm_cpu.input_buf, count);
		return count;
	}
	if (cur_seek>=bfvm_cpu.base) {
		memcpy(buf, bfvm_cpu.mem, count);
		return count;
	}

        return read(fd, buf, count);
}

static int bfdbg_open(const char *pathname, int flags, mode_t mode)
{
	int fd = -1;
	if (bfdbg_handle_open(pathname)) {
		fd = open(pathname+8, flags, mode);
		if (fd != -1) {
			bfvm_init(0xFFFF, 1);
			bfdbg_fd = fd;
		}
	}
	return fd;
}

static int bfdbg_system(const char *cmd)
{
	if (!memcmp(cmd, "info",4)) {
		bfvm_step(0);
	} else
	if (!memcmp(cmd, "help",4)) {
		eprintf("Brainfuck debugger help:\n");
		eprintf("20!step       ; perform 20 steps\n");
		eprintf("!step         ; perform a step\n");
		eprintf("!stepo        ; step over rep instructions\n");
		eprintf("!maps         ; show registers\n");
		eprintf("!reg          ; show registers\n");
		eprintf("!cont [addr]  ; continue until address or ^C\n");
		eprintf("!trace [addr] ; trace code execution\n");
		eprintf("!contsc       ; continue until write or read syscall\n");
		eprintf("!reg eip 3    ; force program counter\n");
		eprintf(".!reg*        ; adquire register information into core\n");
	} else
	if (!memcmp(cmd, "contsc",6)) {
		bfvm_contsc();
	} else
	if (!memcmp(cmd, "cont",4)) {
		bfvm_cont(get_math(cmd+4));
	} else
	if (!memcmp(cmd, "trace",5)) {
		bfvm_trace(get_math(cmd+5));
	} else
	if (!memcmp(cmd, "stepo",5)) {
		bfvm_step(1);
	} else
	if (!memcmp(cmd, "maps",4)) {
		bfvm_maps(cmd[4]=='*');
	} else
	if (!memcmp(cmd, "step",4)) {
		bfvm_step(0);
	} else
	if (!memcmp(cmd, "reg",3)) {
		if (strchr(cmd+4,' ')) {
			bfvm_reg_set(cmd+4);
		} else {
			switch (cmd[3]) {
			case 's':
				switch(cmd[4]) {
				case '*':
					bfvm_show_regs(1);
					break;
				default:
					bfvm_show_regs(0);
					break;
				}
				break;
			case '*':
				bfvm_show_regs(1);
				break;
			default:
			//case ' ':
			//case '\0':
				bfvm_show_regs(0);
				break;
			}
		}
	} else eprintf("Invalid debugger command. Try !help\n");
	return 0;
}

static int bfdbg_close(int fd)
{
	return close(fd);
}

static u64 bfdbg_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		cur_seek = offset;
		break;
	case SEEK_CUR:
		cur_seek = config.seek+offset;
		break;
#if 1
	case SEEK_END:
		//if (cur_seek>bfvm_cpu.base)
		cur_seek = 0xffffffff;
		return cur_seek;
#endif
	}
#if __WINDOWS__ 
	return _lseek(fildes,(long)offset,whence);
#else
#if __linux__
	return lseek64(fildes,(off_t)offset,whence);
#else
	return lseek(fildes,(off_t)offset,whence);
#endif
#endif
}

void bfdbg_plugin_init()
{
	bfvm_init(0xFFFF, 1);
}

plugin_t bfdbg_plugin = {
	.name        = "bfdbg",
	.desc        = "brainfuck debugger",
	.init        = bfdbg_plugin_init,
	.debug       = 1,
	.system      = bfdbg_system,
	.widget      = NULL,
	.handle_fd   = bfdbg_handle_fd,
	.handle_open = bfdbg_handle_open,
	.open        = bfdbg_open,
	.read        = bfdbg_read,
	.write       = bfdbg_write,
	.lseek       = bfdbg_lseek,
	.close       = bfdbg_close
};
