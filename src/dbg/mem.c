/*
 * Copyright (C) 2007, 2008, 2009
 *       th0rpe <nopcode.org>
 *       pancake <youterm.com>
 *
 * radare is part of the radare project
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

#include "libps2fd.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#if __UNIX__
#include <sys/mman.h>
#endif
#include "../main.h"
#include "../utils.h"
#include "../list.h"
#include "mem.h"
#include "string.h"
#include "debug.h"


addr_t dealloc_page(addr_t addr)
{
	struct list_head *pos;

	list_for_each_prev(pos, &ps.map_mem) {

		MAP_MEM *mm = list_entry(pos, MAP_MEM, list);

		if(mm->addr == addr) {
		/* printf("delete addr: mm->addr: 0%x mm->size: %i\n",
			 mm->addr, mm->size);
		*/
			arch_dealloc_page(mm->addr, mm->size);
			list_del(&mm->list);
			if(mm->tag)
				free((void*)mm->tag);
			free(mm);

			ps.mem_sz -= mm->size;
			return addr;
		}
	}

	return 0;
}

addr_t mmap_tagged_page(const char *file, addr_t addr, addr_t size)
{
	int rsize = size;
	int fd;
	addr_t retaddr;

	// TODO: close fds on close!!!
//	fd = open(file, 0);
#include <syscall.h>
	fd = arch_syscall(ps.pid, SYS_open, file, 0); //O_RDWR);
eprintf("FILE=(%s)\n", file);
	if (fd == -1) {
		fprintf(stderr, "Cannot open %s\n", file);
		return 0;
	}
eprintf("fd=%d\n", fd);

	retaddr = arch_mmap(fd, addr, rsize);
eprintf("new addr = 0x%08llx\n", retaddr);

	if(retaddr == (u64)-1) {
		fprintf(stderr, "host_mmap:error\n");
		close(fd);
		return 0;
	}
	MAP_MEM *mm = (MAP_MEM * )malloc(sizeof(MAP_MEM));

	if(mm == NULL) {
		close(fd);
		return 0;
	}

	// XXX u64 for 64 bits!
	mm->addr = (addr_t) retaddr;
	mm->size = rsize;

	/* tag for map region */
	mm->tag = strdup(file);

	list_add_tail(&(mm->list), &(ps.map_mem));

	ps.mem_sz += rsize;

	// XXX
	return 0;
}

addr_t alloc_tagged_page(const char *tag, unsigned long size)
{
	unsigned long rsize;
	addr_t addr = arch_alloc_page(size, &rsize);

	if(addr != (addr_t)-1) {
		MAP_MEM *mm = (MAP_MEM * )malloc(sizeof(MAP_MEM));

		if(mm == NULL)
			return 0;

		mm->addr = addr;
		mm->size = rsize;

		/* tag for map region */
		if(tag)
			mm->tag = strdup(tag);
		else
			mm->tag = NULL;

		list_add_tail(&(mm->list), &(ps.map_mem));

		ps.mem_sz += rsize;
	}

	return addr;
}

u64 alloc_page(int size) 
{
	return alloc_tagged_page(NULL, size);
}

void add_regmap(MAP_REG *mr)
{
	unsigned long entry_alig = ps.entrypoint & ~(PAGE_SIZE-1);
	
	if(entry_alig >= mr->ini && entry_alig <= mr->end) {
		mr->flags |= FLAG_USERCODE;
		ps.bin_usrcode = mr->bin;

	} else if(ps.bin_usrcode && mr->bin && !strcmp(ps.bin_usrcode, mr->bin)) {
		mr->flags |= FLAG_USERCODE;

	} else if (mr->perms & REGION_EXEC) {
		mr->flags |= FLAG_SYSCODE;
	}
	mr->perms_orig = mr->perms;

	list_add_tail(&(mr->list), &(ps.map_reg));
    	ps.map_regs_sz++;
}

void dealloc_all()
{
   struct list_head *p, *aux;
   
   p = (&ps.map_mem)->next;

   while(p && p != &(ps.map_mem)) {
	MAP_MEM *mm = list_entry(p, MAP_MEM, list);

	aux = p->next;

	/* printf("all delete mm->addr: 0%x mm->size: %i\n",
		 mm->addr, mm->size); */
	arch_dealloc_page(mm->addr, mm->size);
	list_del(&(mm->list));
	if(mm->tag)
		free(mm->tag);
	free(mm);

	p = aux;
    }

    ps.mem_sz = 0;
}

void print_status_alloc()
{
	struct list_head *pos;

	printf("Alloc map: \n"
	       "  total allocated: %i bytes\n", ps.mem_sz);

	if(ps.mem_sz)
		printf("\n");

	list_for_each_prev(pos, &ps.map_mem) {
		MAP_MEM *mm = list_entry(pos, MAP_MEM, list);

		printf(" * address: 0x%08llx size: %i bytes", mm->addr, (unsigned int)mm->size);
		if(mm->tag) 
			printf(" tag: %s\n", mm->tag);
		else
			printf("\n");
	}
}

void free_regmaps(int rest)
{
	struct list_head *p, *aux;

	p = (&ps.map_reg)->next;

	while(p && p != &(ps.map_reg)) {
		MAP_REG *mr = list_entry(p, MAP_REG, list);

		aux = p->next;
		list_del(&(mr->list));
		/* restore region permissions */
		if(rest)
			rest_region(mr);
/* oops 
		if(mr->bin)
			free(mr->bin);
			*/

		free(mr);
		p = aux;
	}

	ps.map_regs_sz = 0;
}

int radare_get_region(u64 *from, u64 *to)
{
	struct list_head *pos;

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG *mr = list_entry(pos, MAP_REG, list);
		if (config.seek >mr->ini && config.seek < mr->end) {
			*from = mr->ini;
			*to = mr->end;
			return 1;
		}
	}
	return 0;
}

void print_maps_regions(int rad, int two)
{
	int i;
	struct list_head *pos;
	char name[128];
	char perms[5];
	u64 from=0, to=0;

	if (rad)
		cons_printf("fs maps\n");

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG *mr = list_entry(pos, MAP_REG, list);

		if (rad) {
			if(mr->bin) {
				strcpy(name, mr->bin);
				for(i=0;mr->bin[i];i++) {
					int ch = mr->bin[i];
					if (!is_printable(ch)|| ch=='['||ch==']'|| ch=='/'||ch=='.'||ch=='-'||ch=='+') {
						ch = '_';
					}
					name[i] = ch;
				}
				name[i]='\0';
				cons_printf("f map.%s @ 0x%08llx\n", name, (u64)mr->ini);
				cons_printf("f map.%s_end @ 0x%08llx\n", name, (u64)mr->end);
				
				// TODO: control limits..needs >= comparisions
				//cons_printf("? $zoom.from >= 0x%08llx && ?? e zoom.from = 0x%08llx\n", mr->ini);
				if (mr->flags & FLAG_USERCODE) {
					if (from == 0 || mr->ini < from)
						from = mr->ini;
					if (to == 0 || mr->ini > to)
						to = mr->end;
				}
			}
		} else {
			perms[0] = (mr->perms & REGION_READ)?   'r' : '-';
			perms[1] = (mr->perms & REGION_WRITE)?  'w' : '-';
			perms[2] = (mr->perms & REGION_EXEC)?   'x' : '-';
			perms[3] = (mr->flags & FLAG_USERCODE)? 'u' : '-';
			perms[4] = '\0';

			if ((!two) || (two && ((config.seek>mr->ini) && (config.seek < mr->end))))
			cons_printf("0x%.8llx %c 0x%.8llx %s 0x%.8llx %s\n",
				(u64) mr->ini, 
				((config.seek>=mr->ini) && (config.seek <= mr->end))?'*':'-',
				(u64) mr->end, perms,
				(u64) mr->size, mr->bin? mr->bin : "");
		}
	}

	if (rad)
		cons_printf(
		"e zoom.from = 0x%08llx\n"
		"e zoom.to = 0x%08llx\n", from, to);
}

void page_restore()
{
	struct list_head *pos;
	char *buf;
	FILE *fd;

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG *mr = list_entry(pos, MAP_REG, list);

		if ( mr->perms & REGION_WRITE ) {
			printf("Restoring from %08llx-%08llX.dump  ; 0x%.8x  %s\n",
				 mr->ini, (long long)mr->end,
				 (unsigned int)mr->size, mr->bin);

			buf = (char*)malloc(mr->size);
			sprintf(buf, "%08llX-%08llX.dump", mr->ini, mr->end);
			fd = fopen(buf, "rb");
			if (fd == NULL) {
				printf("Oops. cannot open\n");
				continue;
			}
			fread(buf, mr->size, 1, fd);
			debug_write_at(ps.tid, buf, mr->size, mr->ini);
#if __UNIX__
			msync((void *)(long)mr->ini, (size_t)mr->size, MS_SYNC);
#endif
			fclose(fd);
			free(buf);
		}
	}
}

void page_dumper(const char *dir)
{
	struct list_head *pos;
	char *buf;
	FILE *fd;

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

		if ( mr->perms & REGION_WRITE || mr->flags & FLAG_USERCODE ) {
			printf("Dumping %08llX-%08llX.dump  ; 0x%.8x  %s\n",
				 mr->ini,  mr->end,
				 (unsigned int)mr->size, mr->bin);

			buf = (char*)malloc(mr->size);
			sprintf(buf, "%08llX-%08llX.dump", (long long)mr->ini, (long long)mr->end);
			fd = fopen(buf, "wb");
			if (fd == NULL) {
				eprintf("Oops. cannot open\n");
				continue;
			}
#if __UNIX__
			msync((void *)(unsigned long)mr->ini, (size_t)mr->size, MS_SYNC);
#endif
			debug_read_at(ps.tid, buf, mr->size, mr->ini);
			fwrite(buf,mr->size, 1, fd);
			fclose(fd);
			free(buf);
		}
	}
}

unsigned int prot2reg_perm(unsigned int prot_perms)
{
	unsigned reg_perms = 0;

	if(prot_perms & PROT_READ)
		reg_perms |= REGION_READ;

	if(prot_perms & PROT_WRITE)
		reg_perms |= REGION_WRITE;

	if(prot_perms & PROT_EXEC)
		reg_perms |= REGION_EXEC;

	return reg_perms;
}

unsigned int reg2prot_perm(unsigned int reg_perms)
{
	unsigned prot_perms = 0;

	if(reg_perms & REGION_READ)
		prot_perms |= PROT_READ;

	if(reg_perms & REGION_WRITE)
		prot_perms |= PROT_WRITE;

	if(reg_perms & REGION_EXEC)
		prot_perms |= PROT_EXEC;

	return prot_perms;
}

void up_regions(int usrcode)
{
	struct list_head *pos;
	int ret;

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG *mr = list_entry(pos, MAP_REG, list);

		if(mr->flags & FLAG_NOPERM)
			continue;

		if((mr->flags & FLAG_USERCODE) == usrcode) {
			/* restore permissions */
			if(mr->perms != mr->perms_orig) {
				ret = arch_mprotect(mr->ini, mr->size,
						reg2prot_perm(mr->perms_orig));
				if(ret >= 0)
					mr->perms = mr->perms_orig;
			}

		} else {
			/* none permissions at region  */
			ret = arch_mprotect(mr->ini, mr->size, PROT_READ);
			if(ret >= 0)
				mr->perms = REGION_READ;
		}
	}
}

void rest_region(MAP_REG *mr)
{
	/* restore permissions */
	if(mr->perms != mr->perms_orig) {
		if(arch_mprotect(mr->ini, mr->size,
			reg2prot_perm(mr->perms_orig)) >= 0)
			mr->perms = mr->perms_orig;
	}
}

#if 0
void rest_all_regions()
{
	struct list_head *pos;

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

		/* restore permissions */
		if(mr->perms != mr->perms_orig) {
			if(arch_mprotect(mr->ini, mr->size,
				reg2prot_perm(mr->perms_orig)) >= 0)
				mr->perms = mr->perms_orig;
		}
	}
}
#endif

int debug_imap(char *args)
{
	int fd = -1;
	int ret = -1;

	if (!ps.opened) {
		eprintf(":imap No program loaded.\n");
		return 1;
	}

	if(!args) {
		eprintf(":imap No insert input stream.\n");
		return 1;
	}

	if (!strncmp("file://", args + 1, 7)) {
		struct stat inf;
		addr_t addr;
		char *pos;
		char buf[4096];
		char *filename = args + 8;
		int len;

		if((fd = open(filename, O_RDONLY)) < 0) {
			perror(":map open");
			goto err_map;
		}

		if(fstat(fd, &inf) == -1) {
			perror(":map fstat");
			goto err_map;
		}

		if(inf.st_size > MAX_MAP_SIZE) {
			eprintf(":map file too long\n");
			goto err_map;
		}

		addr = alloc_tagged_page(args + 1, inf.st_size);
		if(addr == (addr_t)-1) {
			eprintf(":imap memory size %i failed\n", inf.st_size);
			goto err_map;
		}

		pos = (char *)(long)addr;
		while((len = read(fd, buf, 4096)) > 4096) {
			debug_write_at(ps.tid, buf, 4096, (long)pos);
			pos += 4096;
		}

		if(len > 0)
			debug_write_at(ps.tid, buf, len, (long)pos);

		eprintf("file %s mapped at 0x%x\n", filename, addr);
	} else {
		eprintf(":imap Invalid input stream\n");
		goto err_map;
	}

	ret = 0;

err_map:
	if(fd >= 0)
		close(fd);

	return ret;
}

/* memory protection permissions */
struct mp_t {
	u64 addr;
	unsigned int size;
	int perms;
	struct list_head list;
};

static int mp_is_init = 0;

struct list_head mps;

// TODO: support to remove ( store old page permissions )
// TODO: remove overlapped memory map changes
int debug_mp(char *str)
{
	struct list_head *i;
	struct mp_t *mp;
	char buf[128];
	char buf2[128];
	char buf3[128];
	char *ptr = buf;
	u64 addr;
	u64 size;
	int perms = 0;

	// TODO: move this to debug_init .. must be reinit when !load is called
	if (!mp_is_init) {
		INIT_LIST_HEAD(&mps);
		mp_is_init = 1;
	}

	if (str[0]=='\0') {
		list_for_each(i, &(mps)) {
			struct mp_t *m = list_entry(i, struct mp_t, list);
			cons_printf("0x%08llx %d %c%c%c\n", m->addr, m->size,
			m->perms&4?'r':'-', m->perms&2?'w':'-', m->perms&1?'x':'-');
		}
		return 0;
	}

	if (strchr(str, '?')) {
		cons_printf("Usage: !mp [rwx] [addr] [size]\n");
		cons_printf("  > !mp       - lists all memory protection changes\n");
		cons_printf("  > !mp --- 0x8048100 4096\n");
		cons_printf("  > !mp rwx 0x8048100 4096\n");
		cons_printf("- addr and size are aligned to memory (-=%%4).\n");
		return 0;
	}

	sscanf(str, "%127s %127s %127s", buf, buf2, buf3);
	addr = get_math(buf2);
	size = get_math(buf3);

	if (size == 0) {
		eprintf("Invalid arguments\n");
		return 1;
	}


	/* PROT_{EXEC, WRITE, READ} from mman.h */
	for(ptr=buf;ptr[0];ptr=ptr+1) {
		switch(ptr[0]) {
		case 'r': perms |= 1; break;
		case 'w': perms |= 2; break;
		case 'x': perms |= 4; break;
		}
	}

	// align to bottom
	addr = addr - (addr%4);
	size = size + (size-(size%4));

	mp = (struct mp_t*)malloc(sizeof(struct mp_t));
	mp->addr  = addr;
	mp->size  = (unsigned int)size;
	mp->perms = perms;
	list_add_tail(&(mp->list), &(mps));
	
	arch_mprotect((addr_t)mp->addr, mp->size, mp->perms);

	return 0;
}


int debug_mmap(char *args)
{
	char *arg;
	char *file = args + 1;
	addr_t addr;
	addr_t size;
#if 0
Dump of assembler code for function mmap:
0xb7ec0110 <mmap+0>:    push   %ebp
0xb7ec0111 <mmap+1>:    push   %ebx
0xb7ec0112 <mmap+2>:    push   %esi
0xb7ec0113 <mmap+3>:    push   %edi
0xb7ec0114 <mmap+4>:    mov    0x14(%esp),%ebx
0xb7ec0118 <mmap+8>:    mov    0x18(%esp),%ecx
0xb7ec011c <mmap+12>:   mov    0x1c(%esp),%edx
0xb7ec0120 <mmap+16>:   mov    0x20(%esp),%esi
0xb7ec0124 <mmap+20>:   mov    0x24(%esp),%edi
0xb7ec0128 <mmap+24>:   mov    0x28(%esp),%ebp
0xb7ec012c <mmap+28>:   test   $0xfff,%ebp
0xb7ec0132 <mmap+34>:   mov    $0xffffffea,%eax
0xb7ec0137 <mmap+39>:   jne    0xb7ec0143 <mmap+51>
0xb7ec0139 <mmap+41>:   shr    $0xc,%ebp
0xb7ec013c <mmap+44>:   mov    $0xc0,%eax
0xb7ec0141 <mmap+49>:   int    $0x80
#endif

	if (!ps.opened) {
		eprintf(":signal No program loaded.\n");
		return 1;
	}

	if(!args)
		return debug_alloc_status();

	if ((arg = strchr(file, ' '))) {
		arg[0]='\0';
		addr = get_math(arg+1);
		size = file_size(file);
		if (strchr(arg+1, ' '))
			eprintf("TODO: optional size not yet implemented\n");
		//signal_set(signum, address);
eprintf("mmap_tagged_page(%s,0x%08llx,%08lld)\n", file, addr, size); 
		mmap_tagged_page(file, addr, size);
	} else {
		eprintf("Usage: !mmap [file] [address] ([size])\n");
		return 1;
	}

	return 0;
}

u64 debug_alloc(char *args)
{
	int sz;
	char *param;
	addr_t addr;

	if(!args)
		return debug_alloc_status();

	param = args + 1;
	if(!strcmp(param, "status")) {
		print_status_alloc();
	} else {
		sz = get_math(param);
		if(sz <= 0) {
			eprintf(":alloc invalid size\n");
			return -1;
		}

		addr = alloc_page(sz);
		if(addr == (addr_t)-1) {
			eprintf(":alloc can not alloc region!\n");
			return -1;
		}
		printf("0x%08x\n", (unsigned int)addr);
	}

	return addr;
}

