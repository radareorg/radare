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

#include "main.h"

static struct list_head ranges;
static int ranges_is_init = 0;

int ranges_init()
{
	if (ranges_is_init)
		return 1;
	INIT_LIST_HEAD(&ranges);
	return 0;
}

void ranges_free()
{
	struct list_head *pos;
	struct range_t *h;
	list_for_each(pos, &ranges) {
		h = list_entry(pos, struct range_t, list);
		free(h);
	}
	INIT_LIST_HEAD(&ranges);
}

static int ranges_get_flags()
{
	int flags = 0;
// TODO: not yet implemented
#if 0
	flags |= config_get("range.traces")?RANGE_TRACES:0;
	flags |= config_get("range.graphs")?RANGE_GRAPHS:0;
	flags |= config_get("range.functions")?RANGE_FUNCTIONS:0;
#endif
	return flags;
}

static void ranges_abs(u64 *f, u64 *t)
{
	u64 tmp;
	if (*f>*t) {
		tmp = *f;
		*f = *t;
		*t = tmp;
	}
}

#if 0
    update to      new one     update from   update from/to  ignore

   |______|        |___|           |_____|      |____|      |_______|  range_t
+     |______|   +      |__|   + |___|      + |_________|  +  |__|     from/to
  ------------   -----------   -----------  -------------  -----------
=  |_________|   = |___||__|   = |_______|  = |_________|   |_______|  result
#endif

/* TODO: we need a merging operation over ranges here */
int ranges_add(struct list_head *rang, u64 from, u64 to)
{
	struct range_t *r;
	struct list_head *pos;
	int add = 1;

	if (rang == NULL)
		rang = &ranges;

	ranges_abs(&from, &to);

	list_for_each(pos, rang) {
		r = list_entry(pos, struct range_t, list);
		if (r->from == from && r->to==to) {
			add = 0;
		} else
		if (r->from<=from && r->from <= to && r->to>=from && r->to <= to) {
			r->to = to;
			add = 0;
		} else
		if (r->from>=from && r->from<=to && r->to>=from && r->to >= to) {
			r->from = from;
			add = 0;
		} else
		if (r->from<=from && r->from<=to && r->to>=from && r->to >= to) {
			/* ignore */
			add = 0;
		} else
		if (r->from>=from && r->from<=to && r->to>=from && r->to <= to) {
			r->from = from;
			r->to = to;
			add = 0;
		}
	}

	if (add) {
		r = (struct range_t *)malloc(sizeof(struct range_t));
		r->from = from;
		r->to = to;
		list_add_tail(&(r->list), rang);
	}
}

#if 0
    update to      new one     update from      delete        split

   |______|        |___|           |_____|      |____|       |________|  range_t
-     |______|   -      |__|   - |___|      - |_________|  -    |__|     from/to
  ------------   -----------   -----------  -------------  ------------
=  |__|          =     ||      =     |___|  =                |__|  |__|   result
#endif

/* TODO: is this wurking properly ? */
int ranges_sub(struct list_head *rang, u64 from, u64 to)
{
	struct range_t *r;
	struct list_head *pos;
	u64 f = 0, t = 0;

	if (rang == NULL)
		rang = &ranges;

	ranges_abs(&from, &to);

	__reloop:
	list_for_each(pos, rang) {
		r = list_entry(pos, struct range_t, list);
		/* update to */
		if (r->from<from && r->from < to && r->to>from && r->to < to) {
			r->to = from;
		} else
		/* new one */
		if (r->from<from && r->from < to && r->to>from && r->to < to) {
			ranges_add(rang, f, t);
			goto __reloop;
			// TODO: reinit foreach loop
		}
		/* update from */
		if (r->from>from && r->from<to && r->to>from && r->to > to) {
			r->from = to;
		} else
		/* delete */
		if (r->from>from && r->from<to && r->to>from && r->to < to) {
			/* delete */
			list_del(&(r->list));
			goto __reloop;
		}
		/* split */
		if (r->from<from && r->from<to && r->to>from && r->to > to) {
			t = r->to;
			r->to = from;
			ranges_add(rang, to, t);
		//	eprintf("split\n");
			goto __reloop;
		}
	}
	return 0;
}

int ranges_cmd(const char *arg)
{
	int flags;
	u64 from, to;
	char *ptr;
	char *a = alloca(strlen(arg));

	strcpy(a, arg+1);

	switch(*arg) {
	case '?':
		eprintf("Usage: ar[b] [args]\n");
		eprintf("ar                ; show all ranges\n");
		eprintf("ar%%               ; show ranges in a visual percentage\n");
		eprintf("ar*               ; show ranges in radare commands\n");
		eprintf("arb [[from] [to]] ; boolean ranges against\n");
		eprintf("arb%% [[from] [to]]; boolean ranges against in visual pcent\n");
		eprintf("ar+ [from] [to]   ; add new range\n");
		eprintf("ar- [from] [to]   ; drop range\n");
		eprintf("ar-*              ; reset range tables\n");
		eprintf(" ; range.from     ; default boolean from address\n");
		eprintf(" ; range.to       ; default boolean to address\n");
#if 0
		eprintf(" ; range.traces   ; (true) join trace information (at?)\n");
		eprintf(" ; range.graphs   ; (true) join graph information (g?)\n");
		eprintf(" ; range.functions; (true) join functions information (CF)\n");
#endif
		eprintf(" ; e range.       ; show range config vars\n");
		break;
	case '+':
		if (*a==' ') a=a+1;
		ptr = strchr(a, ' ');
		if (ptr) {
			ptr[0]='\0';
			from = get_math(a);
			to = get_math(ptr+1);
			ranges_add(NULL, from, to);
		} else {
			eprintf("Usage: ar+ [from] [to]\n");
		}
		break;
	case '%':
		eprintf("TODO\n");
		break;
	case '-':
		if (arg[1]=='*') {
			ranges_free();
		} else {
			ptr = strchr(a, ' ');
			if (ptr) {
				ptr[0]='\0';
				from = get_math(a);
				to = get_math(ptr+1);
				ranges_sub(NULL, from, to);
			} else {
				eprintf("Usage: ar- [from] [to]\n");
			}
		}
		break;
	case 'b': // bolean
		ptr = strchr(a, ' ');
		if (ptr) {
			ptr[0]='\0';
			from = get_math(a);
			to = get_math(ptr+1);
		} else {
			from = config_get_i("range.from");
			to = config_get_i("range.to");
		}
		ranges_boolean(from, to, ranges_get_flags());
		break;
	case '*':
		ranges_list(1);
		break;
	default:
		ranges_list(0);
		break;
	}
}

int ranges_list(int rad)
{
	u64 total = 0;
	struct list_head *pos;
	list_for_each(pos, &ranges) {
		struct range_t *r = list_entry(pos, struct range_t, list);
		if (rad) cons_printf("ra+ 0x%08llx 0x%08llx\n", r->from, r->to);
		else cons_printf("0x%08llx 0x%08llx ; %lld\n", r->from, r->to, r->to-r->from);
		total += (r->to-r->from);
	}
	eprintf("Total bytes: %lld\n", total);
	return 0;
}

int ranges_boolean(u64 from, u64 to, int flags)
{
	eprintf("; ranges from 0x%08llx to 0x%08llx (flags=%d)\n",
		from, to, flags);
	// TODO
	// show ranges
	// show total bytes
	// show total booleaned bytes
	return 0;
}
