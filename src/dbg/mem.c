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

	// TODO: close fds on close!!!
	fd = open(file,0);
	if (fd == -1) {
		fprintf(stderr, "Cannot open %s\n", file);
		return 0;
	}

	if (rsize <1) {
		rsize = lseek(fd, (off_t)0, SEEK_END);
		lseek(fd, (off_t)0, SEEK_SET);
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
	mm->addr = (addr_t) addr;
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

   while(p && p != &(ps.map_mem))
    {

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
	int zoominit=0;
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
					if (!is_printable(ch)|| ch=='['||ch==']'|| ch=='/'||ch=='.'||ch=='-') {
						ch = '_';
					}
					name[i] = ch;
				}
				name[i]='\0';
				cons_printf("f map_%s @ 0x%08llx\n", name, (u64)mr->ini);
				cons_printf("f map_%s_end @ 0x%08llx\n", name, (u64)mr->end);
				
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
			perms[4] = 0;

			if ((!two) || (two && ((config.seek>mr->ini) && (config.seek < mr->end))))
			cons_printf("0x%.8llx %c 0x%.8llx %s 0x%.8llx %s\n",
				(unsigned long long)mr->ini, 
				((config.seek>=mr->ini) && (config.seek <= mr->end))?'*':'-',
				(unsigned long long)mr->end, perms,
				(unsigned long long)mr->size, mr->bin? mr->bin : "");
		}
	}

	if (rad) {
		cons_printf("e zoom.from = 0x%08llx\n", from);
		cons_printf("e zoom.to = 0x%08llx\n", to);
	}
}

static int dump_num = 0;
static char dumpdir[128];

void page_restore(const char *dir)
{
	struct list_head *pos;
	char *buf;
	FILE *fd;

	if (strnull(dir)) {
		dump_num--;
		if (dump_num<0) {
			eprintf("No dumps to restore from. Sorry\n");
			return;
		}
		sprintf(dumpdir, "dump%d", dump_num);
		dir = dumpdir;
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
	arch_restore_registers();
	if (dir&&*dir)
		chdir("..");
}

void page_dumper(const char *dir)
{
	struct list_head *pos;
	char *buf;
	FILE *fd;

	if (strnull(dir)) {
		sprintf(dumpdir, "dump%d", dump_num);
		dir = dumpdir;
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
