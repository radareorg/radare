/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
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

#include "r_io.h"
#include "r_io_section.h"
#include "list.h"

// XXX
#define cons_printf printf

static struct list_head sections;

void r_io_section_set(u64 from, u64 to, u64 vaddr, u64 paddr, const char *comment)
{
	struct list_head *pos;
	list_for_each(pos, &sections) {
		struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (s->from == from) {
			if (to != -1)
				s->to = to;
			if (vaddr != -1)
				s->vaddr = vaddr;
			if (paddr != -1)
				s->paddr = paddr;
			if (comment)
				strncpy(s->comment, comment, 254);
		}
	}
}

void r_io_section_add(u64 from, u64 to, u64 vaddr, u64 paddr, const char *comment)
{
	struct r_io_section_t *s = (struct r_io_section_t *)malloc(sizeof(struct r_io_section_t));
	s->from = from;
	s->to = to;
	s->vaddr = vaddr;
	s->paddr = paddr;
	if (comment)
		strncpy(s->comment, comment, 254);
	else s->comment[0]='\0';
	list_add(&(s->list), &sections);
}

struct r_io_section_t *r_io_section_get_i(int idx)
{
	int i = 0;
	struct list_head *pos;
	list_for_each_prev(pos, &sections) {
		struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (i == idx)
			return s;
		i++;
	}
	return NULL;
}

int r_io_section_rm(int idx)
{
	struct r_io_section_t *s = r_io_section_get_i(idx);
	if (s != NULL) {
		list_del((&s->list));
		free(s);
		return 1;
	}
	return 0;
}

// TODO: implement as callback
void r_io_section_list(u64 addr, int rad)
{
	int i = 0;
	//char buf[128];
	struct list_head *pos;
	list_for_each_prev(pos, &sections) {
		struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (rad) {
			cons_printf("S 0x%08llx 0x%08llx %s @ 0x%08llx\n",
				s->to-s->from, s->vaddr, s->comment, s->from);
			cons_printf("Sd 0x%08llx @ 0x%08llx\n", s->paddr, s->from);
		} else {
			cons_printf("%02d %c 0x%08llx - 0x%08llx bs=0x%08llx sz=0x%08llx phy=0x%08llx %s",
				i, (addr>=s->from && addr <=s->to)?'*':'.',
				s->from, s->to, s->vaddr, (u64)((s->to)-(s->from)), s->paddr, s->comment);
			

// TODO: IMPLEMENT AS CALLBACK
//			if (string_flag_offset(buf, s->from))
//				cons_printf(" ; %s", buf);
#if 0
			ol = r_io_section_overlaps(s);
			if (ol != -1)
				cons_printf(" ; Overlaps with %d", ol);
#endif
			cons_printf("\n");
		}
		i++;
	}
}

void r_io_section_list_visual(u64 seek, u64 len)
{
	u64 min = -1;
	u64 max = -1;
	u64 mul;
	int j, i;
	struct list_head *pos;
	int width = 78; //config.width-30;

	list_for_each(pos, &sections) {
		struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (min == -1 || s->from < min)
			min = s->from;
		if (max == -1 || s->to > max)
			max = s->to;
	}

	mul = (max-min) / width;
	if (min != -1 && mul != 0) {
		i = 0;
		list_for_each_prev(pos, &sections) {
			struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
			cons_printf("%02d  0x%08llx |", i, s->from);
			for(j=0;j<width;j++) {
				if ((j*mul)+min >= s->from && (j*mul)+min <=s->to)
					cons_printf("#");
				else
					cons_printf("-");
			}
			cons_printf("| 0x%08llx\n", s->to);
			i++;
		}
		/* current seek */
		if (i>0 && len != 0) {
			cons_printf("=>  0x%08llx |", seek);
			for(j=0;j<width;j++) {
				if ((j*mul)+min >= seek && (j*mul)+min <= seek+len)
					cons_printf("#");
				else
					cons_printf("-");
			}
			cons_printf("| 0x%08llx\n", seek+len);
		}
	}
}

struct r_io_section_t *r_io_section_get(u64 addr)
{
	struct list_head *pos;
	list_for_each(pos, &sections) {
		struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (addr >= s->from && addr <= s->to)
			return s;
	}
	return NULL;
}

u64 r_io_section_get_paddr(u64 addr)
{
	struct r_io_section_t *s = r_io_section_get(addr);
	if (s != NULL)
		return s->paddr;
	return -1;
}

u64 r_io_section_get_vaddr(u64 addr)
{
	struct r_io_section_t *s = r_io_section_get(addr);
	if (s != NULL)
		return s->vaddr;
	return -1;
}

int r_io_section_overlaps(struct r_io_section_t *s)
{
	int i = 0;
	struct list_head *pos;
	list_for_each_prev(pos, &sections) {
		struct r_io_section_t *s2 = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (s != s2) {
			if (s->from >= s2->from) {
				if (s2->to < s->from)
					return i;
			} else {
				if (s->to < s2->from)
					return i;
			}
		}
		i++;
	}
	return -1;
}

// seek 
u64 last_align = 0;
u64 r_io_section_align(u64 addr, u64 vaddr, u64 paddr)
{
	struct list_head *pos;
	if (addr == last_align)
		return last_align;

	list_for_each_prev(pos, &sections) {
		struct r_io_section_t *s = (struct r_io_section_t *)list_entry(pos, struct r_io_section_t, list);
		if (addr >= s->from && addr <= s->to) {
#if 0
			saltem a 0x24324
			comença a 0x11000; (adreça vaddr)
			equival a la 0x400 (real de disc)
#endif
			return ( addr - s->vaddr + s->paddr ); 
		}
	}
	last_align = addr-vaddr+paddr;
	//printf("? 0x%llx-0x%llx+0x%llx\n", addr, vaddr, paddr);
	return last_align;
}

void r_io_section_init(int foo)
{
	INIT_LIST_HEAD(&(sections));
}
