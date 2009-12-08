/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * libps2fd is part of the radare project
 *
 * libps2fd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libps2fd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libps2fd; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "main.h"
#include "code.h"
#include "utils.h"
#include "print.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if HAVE_GUI
#include <gtk/gtk.h>

static int gtk_is_init = 0;
static GtkWindow *w;

static void hack_close_window(/* TODO : get args */)
{
	gtk_widget_destroy(GTK_WIDGET(w));
	gtk_main_quit();
}

static int radare_hack_call(struct hack_t *h, const char *arg)
{
  if (h->widget != NULL) {
    /* initialize gtk before */
    if (!gtk_is_init) {
      if ( ! gtk_init_check(NULL, NULL) ) {
        D eprintf("Oops. Cannot initialize gui\n");
        return 1;
      }
      gtk_is_init = 1;
    }

    // XXX hacky :(
    h->callback((void *)arg);
    w = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_add(GTK_CONTAINER(w), *h->widget);
    gtk_widget_show_all(GTK_WIDGET(w));
    g_signal_connect (w, "destroy", G_CALLBACK (hack_close_window), w);
    gtk_main();
  } else
    h->callback((void *)arg);

  return 0;
}

#else

static int radare_hack_call(struct hack_t *h, const char *arg)
{
	h->callback(arg);
	return 0;
}
#endif

struct list_head hacks;

int radare_hack_help()
{
	int i=1;
	struct list_head *pos;

	list_for_each(pos, &hacks) {
		struct hack_t *h= list_entry(pos, struct hack_t, list);
		cons_printf("%02d %s\t%s\n", i++, h->name, h->desc);
	}
	return 0;
}

/* fuck off! */
static int hack_nop(char *lold)
{
	struct aop_t aop;
	unsigned char buf[1024];
	int i, len;
	int delta = (config.cursor_mode)?config.cursor:0;
	const char *arch = config_get("asm.arch");

	if (!config_get("file.write"))
		return 0;
	//debug_getregs(ps.tid, &reg);
	//debug_read_at(ps.tid, buf, 16, R_EIP(reg));
	radare_read_at(config.seek+delta, buf, 16);
	len = arch_aop(config.seek+delta, buf, &aop);
	if (!strcmp(arch, "x86")) {
		for(i=0;i<len;i++)
			buf[i]=0x90;
	} else {
		// TODO real multiarch
		for(i=0;i<len;i++)
			buf[i]=0x00;
	}
	//debug_write_at(ps.tid, buf, 16, R_EIP(reg));
	radare_write_at( config.seek+delta, buf, 4);
	return 0;
}

struct hack_t *radare_hack_new(const char *name, const char *desc, int (*callback)())
{
	struct hack_t *hack = (struct hack_t *)malloc(sizeof(struct hack_t));
	hack->name = name?strdup(name):NULL;
	hack->desc = desc?strdup(desc):NULL;
	hack->callback = callback;
	return hack;
}

int radare_hack_init()
{
	static int init = 0;
	struct hack_t *hack;
	if (init) return 0; init=1;
	INIT_LIST_HEAD(&hacks);
	hack = radare_hack_new("no", "nop one opcode", &hack_nop);
	list_add_tail(&(hack->list), &(hacks));
	return 1;
}

void *plugin_get_widget(const char *name)
{
	struct list_head *pos;

	list_for_each(pos, &hacks) {
		struct hack_t *h = list_entry(pos, struct hack_t, list);
		if (!strcmp(name, h->name)) {
			h->callback(""); // to init
			if (h->widget)
				return *h->widget;
			return NULL;
		}
	}

	return NULL;
}

int radare_hack(const char *cmd)
{
	int i=1;
	struct list_head *pos;
	int num = 0;
	char *ptr;
	char *arg;
	char *end;

	if (cmd == NULL)
		return 0;

	ptr = strchr(cmd, ' ');
	if (ptr) {
		ptr = ptr + 1;
		num = atoi(ptr);

		arg = ptr;
		end = strchr(ptr, ' ');
		if (end) {
			end[0]='\0';
			arg = end +1;
		} else
			arg = arg+strlen(arg);
	}

	if ((!num && !ptr) || (strnull(cmd)))
		return radare_hack_help();

	list_for_each(pos, &hacks) {
		struct hack_t *h = list_entry(pos, struct hack_t, list);
		if (num) {
			if  (i==num) {
				radare_hack_call(h,arg);
				return 0;
			}
		} else {
			if (!strcmp(ptr, h->name)) {
				radare_hack_call(h,arg);
				return 0;
			}
		}
	}
	eprintf("Unknown hack\n");
	return 1;
}
