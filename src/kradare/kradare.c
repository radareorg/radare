#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <linux/stacktrace.h>
//#include <stdlib.h>

#include <linux/smp_lock.h>
#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define KTRACE "kradare"
#define MAX_DUMP 1000
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
#define atolx(str) simple_strtoull(((str != NULL) ? str : ""), NULL, 16)

//Linux tracer
char action;
char cmd[12]; //d0x11223344


static ssize_t write_the_proc(struct file *filp, const char __user * buff, unsigned long len, void *data) {
	int safelen;

	if (len <= 0)
		return 0;

	memset(cmd,0,4);
	action = buff[0];

	if (len > sizeof(cmd)-1)
		safelen = sizeof(cmd)-1;
	else
		safelen = len;
			
	if (copy_from_user(cmd, buff, safelen))
		return -EFAULT;

	return len;
}

static int read_the_proc(char *page, char **start, off_t off, int count, int *eof, void *data) {
	static inline const char *(*kallsyms_lookup)(unsigned long,unsigned long,unsigned long,char **,char *) = (__print_symbol - 302);
	char symname[KSYM_NAME_LEN+1]="";
	char buf[128];
	char buf2[1024];
	char *modname;
	unsigned long symsize, offset;
	char *stack;
	int param = 20;
	int len = 0;
	int i;
	unsigned char *ptr;
	
	switch (cmd[0]) {
	case 's':
		stack = &len;
		i = atoi((char *)&cmd[1]);
		if (i > 0 && i < MAX_DUMP)
			param = i;

		len += sprintf(page, "Stack at 0x%x\n",&len);
		for (i=0; i<param; i++) {
			stack++;
			//module_address_lookup(stack,&symsize, &offset, &modname);
			kallsyms_lookup(*(unsigned long *)stack, &symsize, &offset, &modname, symname);
			//print_ip_sym(stack);
			len = sprintf(page, "%s0x%x: 0x%x\t%s\n",page, stack, *(unsigned long *)stack, symname);
		}
		break;
	case 'r':
		/* read memory */
		sscanf(cmd+1, "0x%08x %d", &offset, &len);
		if (offset<1)
			break;
		if (len <1)
			len = 128;
		ptr = offset;
		buf2[0]='\0';
		for(i=0;i<len;i++) {
			ptr = ptr + 1;
			sprintf(buf, "%02x ", (unsigned char)ptr[0]);
			strcat(buf2, buf);
		}
		len = sprintf(page, "%s", buf2);
		break;
	case 'R':
		/* show registers */
		break;
	case '0':
		param = atoi((char *)&cmd[0]);
		stack = (unsigned long *)&param;
		len = sprintf(page, "reading from 0x%x\n", *(unsigned long *)param);
		kallsyms_lookup(*(unsigned long *)stack, &symsize, &offset, &modname, symname);
		len = sprintf(page, "0x%x: 0x%x \"%s\" %s\n", stack,  *(unsigned long *)stack, (char *)stack, symname);
		break;
	case 'h':
	default:
		len = sprintf(page,
			"--- kradare ---\n"
			"h\tHelp\n"
			"s[num]\tStack Dump\n"
			"r [0xaddr] [len]\tRead memory\n"
			"w [0xaddr] [len]\tWrite hexpairs\n"
			"0x[addr] *(ulong *)addr\ta\n"
			"--------------\n");
	}

	return len;
}

/*
unsigned long locate_kallsyms_lookup(void) {
	char addr[4] = "\xfa\x13\x14\xc0";
	char *p = (char *)__print_symbol;
	int i;

	for (i=0; i<100; i++) {
		p++;
		
		if (*p == 0x0c)
			printk("found");
	}
}*/

static int register_proc (void) {
	struct proc_dir_entry * comm;

	comm = create_proc_entry(KTRACE, 0700, &proc_root);
	if (!comm) 
		return -1;

	comm->read_proc = read_the_proc;
	comm->write_proc = write_the_proc;

	return 0;
}

static void unregister_proc (void) {
	remove_proc_entry(KTRACE, &proc_root);
}

static int __init carga (void) {
	register_proc();

        printk (KERN_INFO "kradare loaded\n");
        return 0;
}

static void __exit descarga (void) {
      	unregister_proc(); 
	printk (KERN_INFO "kradare unloaded\n");
}

module_init (carga);
module_exit (descarga);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("sha0, pancake");

