/*
 * Copyright (C) 2007, 2008
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
#include <stdlib.h>
#if __UNIX__
#include <sys/mman.h>
#endif
#include "../main.h"
#include "../utils.h"
#include "../list.h"
#include "mem.h"
#include "string.h"

void *dealloc_page(void *addr)
{
	struct list_head *pos;

	list_for_each_prev(pos, &ps.map_mem) {

		MAP_MEM	*mm = (MAP_MEM *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_MEM));

		if(mm->addr == addr) {
		/* printf("delete addr: mm->addr: 0%x mm->size: %i\n",
			 mm->addr, mm->size);
		*/
			arch_dealloc_page(mm->addr, mm->size);
			list_del(&mm->list);
			if(mm->tag)
				free(mm->tag);
			free(mm);

			ps.mem_sz -= mm->size;
			return addr;
		}
	}

	return 0;
}

void *mmap_tagged_page(char *file, u64 addr, u64 size)
{
	int rsize = size;
	int fd;

	// TODO: close fds on close!!!
	fd = open(file,0);
	if (fd == -1) {
		fprintf(stderr, "Cannot open %s\n", file);
		return 0;
	}

	if (rsize <1) {
		rsize = lseek(fd, SEEK_END, 0);
		lseek(fd, SEEK_SET, 0);
	}

	addr = (int)arch_mmap(fd, addr, rsize);

	if(addr == (u64)-1) {
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
	mm->addr = (unsigned long) addr;
	mm->size = rsize;

	/* tag for map region */
	mm->tag = strdup(file);

	list_add_tail(&(mm->list), &(ps.map_mem));

	ps.mem_sz += rsize;

	// XXX
	return (void *)addr;
}

void *alloc_tagged_page(char *tag, int size)
{
	int rsize;
	void *addr = (void *)arch_alloc_page(size, &rsize);

	if(addr != (void *)-1) {
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

inline void *alloc_page(int size) 
{
	return (void *)alloc_tagged_page(NULL, size);
}

inline void add_regmap(MAP_REG *mr)
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

void *dealloc_all()
{
   struct list_head *p, *aux;
   int i;
   
   p = (&ps.map_mem)->next;

   while(p && p != &(ps.map_mem))
    {
	MAP_MEM	*mm = (MAP_MEM *)((char *)p + \
			sizeof(struct list_head) - \
			sizeof(MAP_MEM));

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
		MAP_MEM	*mm = (MAP_MEM *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_MEM));

		printf(" * address: 0x%x size: %i bytes", mm->addr, mm->size);
		if(mm->tag) 
			printf(" tag: %s\n", mm->tag);
		else
			printf("\n");
	}
}

void free_regmaps(int rest)
{
	struct list_head *p, *aux;
	int i;

	p = (&ps.map_reg)->next;

	while(p && p != &(ps.map_reg)) {
		MAP_REG	*mr = (MAP_REG *)((char *)p + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

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

void print_maps_regions(int rad)
{
	int i,ch;
	struct list_head *pos;
	char name[128];
	char perms[5];

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

		if (rad) {
			if(mr->bin) {
				strcpy(name, mr->bin);
				for(i=0;mr->bin[i];i++) {
					int ch = mr->bin[i];
					if (!is_printable(ch)||ch=='/'||ch=='.'||ch=='-') {
						ch = '_';
					}
					name[i] = ch;
				}
				name[i]='\0';
				cons_printf("f section_%s @ 0x%08llx\n", name,
						(unsigned long long)mr->ini);
			}
		} else {
			perms[0] = (mr->perms & REGION_READ)?  'r' : '-';
			perms[1] = (mr->perms & REGION_WRITE)? 'w' : '-';
			perms[2] = (mr->perms & REGION_EXEC)?  'x' : '-';
			perms[3] = (mr->flags & FLAG_USERCODE)? 'u' : '-';
			perms[4] = 0;

			cons_printf("0x%.8llx - 0x%.8llx %s 0x%.8llx %s\n",
				 (unsigned long long)mr->ini, (unsigned long long)mr->end, perms,
				(unsigned long long)mr->size, mr->bin? mr->bin : "");
		}
	}
}

static int dump_num = 0;
static char dumpdir[128];

void page_restore(const char *dir)
{
	struct list_head *pos;
	char *buf;
	FILE *fd;

	if (!dir||strchr(dir, '+')) {
		dump_num--;
		if (dump_num<0) {
			eprintf("No dumps to restore from. Sorry\n");
			return;
		}
		sprintf(dumpdir, "dump%d", dump_num);
		dir = &dumpdir;
	} else
	if (dir&&*dir) {
		dir = dir + 1;
		if (strchr(dir,'/')) {
			eprintf("No '/' permitted here.\n");
			return;
		}
	}
	if ( chdir(dir) == -1 ) {
		eprintf("Cannot chdir to '%s'\n", dir);
		return;
	}

	printf("Restore directory: %s\n", dir);
	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

		if ( mr->perms & REGION_WRITE ) {
			printf("Restoring from %08X-%08X.dump  ; 0x%.8x  %s\n",
				 (long long)mr->ini, (long long)mr->end,
				 (long long) mr->size, (long long)mr->bin);

			buf = (char*)malloc(mr->size);
			sprintf(buf, "%08X-%08X.dump", mr->ini, mr->end);
			fd = fopen(buf, "rb");
			if (fd == NULL) {
				printf("Oops. cannot open\n");
				continue;
			}
			fread(buf, mr->size, 1, fd);
			debug_write_at(ps.tid, buf, mr->size, mr->ini);
#if __UNIX__
			msync((long int*)mr->ini, mr->size, MS_SYNC);
#endif
			fclose(fd);
			free(buf);
		}
	}
	arch_restore_registers();
	if (dir&&*dir)
		chdir("..");
}

void page_dumper(const char *dir)
{
	struct list_head *pos;
	char *buf;
	FILE *fd;

	if (!dir||strchr(dir, '+')) {
		sprintf(dumpdir, "dump%d", dump_num);
		dir = &dumpdir;
		dump_num++;
	} else
	if (dir&&*dir) {
		dir = dir + 1;
		if (strchr(dir,'/')) {
			eprintf("No '/' permitted here.\n");
			return;
		}
	}
#if __WINDOWS__ && !__CYGWIN__
	mkdir(dir);
#else
	mkdir(dir, 0755);
#endif
	if ( chdir(dir) == -1) {
		eprintf("No '/' permitted here.\n");
		return;
	}
	printf("Dump directory: %s\n", dir);

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

		if ( mr->perms & REGION_WRITE || mr->flags & FLAG_USERCODE ) {
			printf("Dumping %08llX-%08llX.dump  ; 0x%.8x  %s\n",
				 (long long) mr->ini,  (long long) mr->end,
				 (long long) mr->size, (long long)mr->bin);

			buf = (char*)malloc(mr->size);
			sprintf(buf, "%08llX-%08llX.dump", (long long)mr->ini, (long long)mr->end);
			fd = fopen(buf, "wb");
			if (fd == NULL) {
				eprintf("Oops. cannot open\n");
				continue;
			}
#if __UNIX__
			msync((long int*) mr->ini, mr->size, MS_SYNC);
#endif
			debug_read_at(ps.tid, buf, mr->size, mr->ini);
			fwrite(buf,mr->size, 1, fd);
			fclose(fd);
			free(buf);
		}
	}
	arch_dump_registers();

	if (dir&&*dir)
		chdir("..");
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
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
				sizeof(struct list_head) - \
				sizeof(MAP_REG));

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
