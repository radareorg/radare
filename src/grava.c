/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef VALA
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "main.h"
#include "widget.h"
#include "node.h"
#include "edge.h"

#if _MAEMO_
//#include <hildon/hildon.h>
//needs to link against
//HildonProgram *p = NULL;
#endif

struct mygrava_window {
	GtkWidget *vbox, *hbox;
	GtkWindow *w;
	GtkWidget *entry;
	GtkWidget *back;
	GtkWidget *go;
	GtkWidget *fix;
	GtkWidget *bnew;
	GtkWidget *close;
	GravaWidget *grava;
	GtkWidget *text;
};

static struct mygrava_window *last_window = NULL;
static int new_window = 0;
static int n_windows = 0;

struct static_nodes {
	char *command;
	struct list_head list;
};

struct list_head static_nodes;// = NULL;

void grava_program_graph(struct program_t *prg, struct mygrava_window *);
static void core_load_graph_entry(void *widget, gpointer obj); //GtkWidget *obj);
static void core_load_node_entry(void *widget, gpointer obj); //GtkWidget *obj);
static struct program_t *prg = NULL; // last program code analysis

void core_load_graph_at(void *obj, const char *str)
{
	//GravaWidget *widget = obj;
	u64 off = get_offset(str);
#if 0
	char *follow = config_get("asm.follow");	
	if (follow&&follow[0]) {
		u64 addr = get_offset(follow);
		if (addr && (addr < off) || ((off+config.block_size)<addr))
			off = addr; //radare_seek(addr, SEEK_SET);
	}
#endif
	monitors_run();
	eprintf("Loading graph... (%s)\n", str);
	radare_seek(off, SEEK_SET);
	//gtk_widget_destroy(w);
	prg = code_analyze(config.baddr + config.seek, config_get_i("graph.depth"));
	list_add_tail(&prg->list, &config.rdbs);
	new_window = 1;
	
	if (last_window) {
		new_window = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(last_window));
	} 
	grava_program_graph(prg, last_window);
}

void mygrava_bp_at(void *unk, const char *str)
{
	char buf[1024];
	u64 off = get_offset(str);
	sprintf(buf, "!bp %s", str);
	radare_cmd(buf, 0);
	eprintf("Breakpoint at (%08llx) (%s) added.\n", off, str);
}

void mygrava_bp_rm_at(void *unk, const char *str)
{
	char buf[1024];
	u64 off = get_offset(str);
	sprintf(buf, "!bp -%s", str);
	radare_cmd(buf, 0);
	eprintf("Breakpoint at (%08llx) (%s) removed.\n", off, str);
}


static void mygrava_close(void *widget, gpointer obj)
{
	struct mygrava_window *w = obj;
	gtk_widget_destroy(GTK_WIDGET(w->w));
	if (--n_windows<0)
		n_windows = 0;
printf("N_WINDOWS: %d\n", n_windows);
	if (n_windows<1)
		gtk_main_quit();
}

static void mygrava_close2(void *widget, void *foo, void *obj) //GtkWidget *obj)
{ mygrava_close(widget, obj); }

static void mygrava_new_window(void *widget, gpointer obj)//GtkWidget *obj)
{
	new_window = 1;
	printf("mygrava_new -> %p\n", obj);
	core_load_graph_entry(widget, obj);
	new_window = 0;
	n_windows++;
}
static void mygrava_new_window2(void *widget, void *foo, void *obj) //GtkWidget *obj)
{ mygrava_new_window(widget, obj); }

static void mygrava_back(void *widget, GtkWidget *obj)
{
	radare_cmd("undo", 0);
	core_load_graph_entry(widget,obj);
}
static void mygrava_back2(void *widget, void *foo, void *obj) //GtkWidget *obj)
{ mygrava_back(widget, obj); }

static void core_load_node_entry(void *widget, void *obj) //GtkWidget *obj)
{
	const char *str;
	struct static_nodes *snode;
	struct mygrava_window *w = obj;

	if (w) {
		str =  gtk_entry_get_text(GTK_ENTRY(w->entry));
		last_window = w;
	} else {
		eprintf("NullObj\n");
		return;
	}

	printf("new node with : %s\n", str);

	snode = malloc(sizeof(struct static_nodes));
	snode->command = strdup(str);
	list_add_tail(&(snode->list), &(static_nodes));

	gtk_entry_set_text(GTK_ENTRY(w->entry),"");

	core_load_graph_entry(widget,obj);

}

static void core_load_graph_entry(void *widget, void *obj) //GtkWidget *obj)
{
	const char *str;
	char *buf;
	struct program_t *prg;
	struct mygrava_window *w = obj;
	u64 off;

	if (w) {
		str =  gtk_entry_get_text(GTK_ENTRY(w->entry));
		last_window = w;
	} else {
		eprintf("NullObj\n");
		return;
	}

	off = get_offset(str);

	eprintf("Loading graph... (%s) 0x%llx\n", str, off);
	if (off == 0 && str[0]!='0') {
		/* run command */
		if (config.debug)
			radare_cmd(".!regs*", 0);
		//radare_cmd(str, 0);
		//buf = radare_cmd_str(str);
		//cons_flush();
		radare_cmd(str, 0);
		buf = cons_get_buffer();
		if (buf && buf[0]) {
			printf("BUFFER(%s->%s)\n", str, buf);
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(w->text)), buf, -1);
			//gtk_text_view_set_buffer(w->text, 
			//free(buf);
		}
		if (config.debug)
			radare_cmd(".!regs*", 0);
		off = config.seek;
	}
	radare_seek(off, SEEK_SET);
	//gtk_widget_destroy(w);
	prg = code_analyze(config.baddr + config.seek, config_get_i("graph.depth"));
	list_add_tail(&prg->list, &config.rdbs);

	grava_program_graph(prg, w);
}

static void core_load_graph_entry2(void *widget, void *foo, GtkWidget *obj)
{ core_load_graph_entry(widget, obj); }

static void core_load_node_entry2(void *widget, void *foo, GtkWidget *obj)
{ core_load_node_entry(widget, obj); }

void core_load_graph_at_label(void *foo, const char *str)
{
	char buf[256];
	char *ptr;

	strncpy(buf, str, 255);
	ptr = strchr(buf, ':');
	if (ptr) ptr[0]='\0';
	core_load_graph_at(NULL, buf);
}

int first = 1;
static int fs = 0;
void grava_toggle_fullscreen(GtkWindow *w)
{
	if (fs) gtk_window_unfullscreen(w);
	else gtk_window_fullscreen(w);
	fs^=1;
}

#ifndef GRAVA_WIDGET_ZOOM_FACTOR
#define GRAVA_WIDGET_ZOOM_FACTOR 0.3
#endif

gboolean grava_key_press_cb(GtkWidget * widget, GdkEventKey * event, struct mygrava_window *w)
{
    switch (event->keyval) {
    case GDK_F6:
  //      hildon_banner_show_information(GTK_WIDGET(window), NULL, "Full screen");
	grava_toggle_fullscreen(w->w);
        return TRUE;
#if _MAEMO_
    case GDK_F7:
        //hildon_banner_show_information(GTK_WIDGET(window), NULL, "Increase (zoom in)");
	w->grava->graph->zoom += (GRAVA_WIDGET_ZOOM_FACTOR);
        return TRUE;

    case GDK_F8:
	w->grava->graph->zoom -= (GRAVA_WIDGET_ZOOM_FACTOR);
        //hildon_banner_show_information(GTK_WIDGET(window), NULL, "Decrease (zoom out)");
        return TRUE;
#endif
    }
	return FALSE;
}

void grava_program_graph(struct program_t *prg, struct mygrava_window *win)
{
	char cmd[1024];
	static int gtk_is_init = 0;
	char *ptr;
	int i;
	GtkWidget *tw,*tw2;
	struct list_head *head, *head2;
	struct block_t *b0, *b1;
	struct xref_t *c0;
	u64 here = config.seek;
	char title[256], name[128];
	int graph_flagblocks = (int)config_get("graph.flagblocks");

	GravaNode *node, *node2;
	GravaEdge *edge;

	/* create widget */
	if (!gtk_is_init) {
		if ( ! gtk_init_check(NULL, NULL) ) {
			fprintf(stderr, "Oops. Cannot initialize gui\n");
			return;
		}
		gtk_is_init = 1;
		new_window = 1;
		INIT_LIST_HEAD(&static_nodes);
	}

	if (win==NULL || new_window) {
		win = (struct mygrava_window*)malloc(sizeof(struct mygrava_window));
		memset(win, '\0', sizeof(struct mygrava_window));
		last_window = win;

		win->grava  = grava_widget_new();
		g_signal_connect(win->grava, "load-graph-at", ((GCallback) core_load_graph_at), win);
		g_signal_connect(win->grava, "breakpoint-at", ((GCallback) mygrava_bp_at), win);

		/* add window */
		win->w = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if 0 && _MAEMO_
		hildon_program_add_window(p, win->w);
#endif
		g_signal_connect(G_OBJECT(win->w), "key_press_event", G_CALLBACK(grava_key_press_cb), win);
		string_flag_offset(name, config.seek);
		sprintf(title, "code graph: %s (0x%08x) %s", config.file, (unsigned int )config.seek, name);
		gtk_window_set_title(GTK_WINDOW(win->w), title);
		g_signal_connect (win->w, "destroy", G_CALLBACK (mygrava_close), win);

		/* TODO: add more control widgets */
		win->vbox = gtk_vbox_new(FALSE, 1);
		win->entry = gtk_entry_new();

		/* new button */
		win->bnew = gtk_button_new_with_mnemonic("");
		gtk_button_set_image (GTK_BUTTON (win->bnew), gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_BUTTON));

		win->close = gtk_button_new_with_mnemonic("");
		gtk_button_set_image (GTK_BUTTON (win->close), gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON));

		g_signal_connect(win->bnew,"activate",((GCallback) mygrava_new_window), (gpointer)win); 
		g_signal_connect(win->bnew,"button-release-event",((GCallback) mygrava_new_window2), (gpointer)win); 
		g_signal_connect(win->close,"activate",((GCallback) mygrava_close), (gpointer)win); 
		g_signal_connect(win->close,"button-release-event",((GCallback) mygrava_close2), (gpointer)win); 
		//win->back = gtk_button_new_from_stock("gtk-undo"); //go-back");
		//g_signal_connect(win->back,"activate",((GCallback) mygrava_back), win->entry); 
		g_signal_connect(win->bnew,"button-release-event",((GCallback) mygrava_back2), win); 
		win->hbox = gtk_hbox_new(FALSE, 2);
		win->go = gtk_button_new_from_stock("gtk-jump-to");

		/* go button */
		win->go = gtk_button_new_with_mnemonic("");
		gtk_button_set_image (GTK_BUTTON (win->go), gtk_image_new_from_stock ("gtk-media-play", GTK_ICON_SIZE_BUTTON));
		g_signal_connect(win->entry,"activate",((GCallback) core_load_graph_entry), win); 
		g_signal_connect(win->go,"activate",((GCallback) core_load_graph_entry), win); 
		g_signal_connect(win->go,"button-release-event",((GCallback) core_load_graph_entry2), win);

		/* fix button */
		win->fix= gtk_button_new_with_mnemonic("");
		gtk_button_set_image (GTK_BUTTON (win->fix), gtk_image_new_from_stock ("gtk-media-forward", GTK_ICON_SIZE_BUTTON));
		g_signal_connect(win->fix,"activate",((GCallback) core_load_node_entry), win); 
		g_signal_connect(win->fix,"button-release-event",((GCallback) core_load_node_entry2), win);

		//gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->back), FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(win->hbox), win->entry);
		gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->go), FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->fix), FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->bnew), FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->close), FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->hbox), FALSE, FALSE, 2);
		tw = gtk_expander_new("Output buffer:");
		win->text =  gtk_text_view_new ();
		gtk_text_view_set_editable(GTK_TEXT_VIEW(win->text), 0);
//gtk_text_buffer_new(NULL);
		tw2 = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tw2), GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(tw2), win->text);
		gtk_container_add(GTK_CONTAINER(tw), tw2);
		gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(tw), FALSE, FALSE, 2);

		// TODO: Add asm.arch combobox from gradare

		gtk_container_add(GTK_CONTAINER(win->w), win->vbox);
		gtk_container_add(GTK_CONTAINER(win->vbox), grava_widget_get_widget(win->grava));
	} else {
		grava_graph_reset(win->grava->graph);
	}

	/* analyze code */
	config_set("asm.offset", "false");
	if (config_get("graph.offset"))
		config_set("asm.offset", "true");
	else 	config_set("asm.offset", "false");
	config_set("asm.bytes", "false");
	config_set("asm.trace", "false");
	config_set("scr.color", "false");
	config_set("asm.lines", "false");
	config.color = 0;

	/* add static nodes */
	i = 0;
	list_for_each_prev(head, &(static_nodes)) {
		struct static_nodes *s0 = list_entry(head, struct static_nodes, list);
		GravaNode *node = grava_node_new();
		g_object_ref(node);
		grava_node_set(node, "label", s0->command);
		
		ptr =  pipe_command_to_string(s0->command);
		if (ptr)
			grava_node_set(node, "body", ptr);
		grava_graph_add_node(win->grava->graph, node);
		g_object_unref(node);
	}

	/* add nodes */
	i = 0;
	list_for_each_prev(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);

		node = grava_node_new();
		g_object_ref(node);

		/* label */
		// TODO: support for real labelling stuff
		string_flag_offset(cmd, b0->addr);
		cmd[127]='\0'; // XXX ugly string recycle hack
		sprintf(cmd+128, "0x%08lX  %s", b0->addr, cmd);
		if (cmd) {
			if (!graph_flagblocks)
				continue;
			grava_node_set(node, "color", "red");
		}

		// traced nodes are turquoise
		if (trace_times(b0->addr)>0)
			grava_node_set(node, "bgcolor", "yellow");

#if 0
		// XXX this makes radare segfault with g_object_unref HUH!
		if (flags_between((u64)b0->addr,(u64)(b0->addr + b0->n_bytes))>0)
			grava_node_set(node, "bgcolor", "yellow");
#endif

		/* add call references for this node */
		// XXX avoid dupped calls
		list_for_each(head2, &(b0->calls)) {
			c0 = list_entry(head2, struct xref_t, list);
			grava_node_add_call(node, c0->addr);
		}

		node->baseaddr = (unsigned long)b0->addr;
		grava_node_set(node, "label", cmd+128);

		/* disassemble body */
		sprintf(cmd, "pD %d @ 0x%08lx", b0->n_bytes , b0->addr);
		config.seek = b0->addr;
		radare_read(0);
		ptr =  pipe_command_to_string(cmd);
		//ptr =  radare_cmd_str(cmd); //pipe_command_to_string(cmd);
		grava_node_set(node, "body", ptr);
		//printf("B (0x%08x) (%d) (\n%s)\n", (unsigned int)b0->addr, (unsigned int)b0->n_bytes-1, ptr);
		free(ptr);

		grava_graph_add_node(win->grava->graph, node);
		b0->data = node;
		g_object_unref(node);
		i++;
	}

	/* add edges */
	i = 0;
	list_for_each_prev(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);
		node = b0->data;
		g_object_ref(node);

		printf("A %08lx\n", b0->addr);
		if (b0->tnext) {
			list_for_each(head2, &(prg->blocks)) {
				b1 = list_entry(head2, struct block_t, list);
				if (b0->tnext == b1->addr) {
					printf("T %08llx\n", b0->tnext);
					node2 = b1->data;
					//if (!gtk_is_init)
					//grava_node_set(node2, "color", "green");
					edge = grava_edge_with(grava_edge_new(), node, node2);
					grava_edge_set(edge, "color", "green");
					edge->jmpcnd = 1; // true
					grava_graph_add_edge(win->grava->graph, edge);
					break;
				}
			}
		}

		if (b0->fnext) {
			list_for_each(head2, &(prg->blocks)) {
				b1 = list_entry(head2, struct block_t, list);
				if (b0->fnext == b1->addr) {
					printf("F %08llx\n", b0->fnext);
					node2 = b1->data;
					//if (!gtk_is_init)
					//grava_node_set(node2, "color", "red");
					edge = grava_edge_with(grava_edge_new(), node, node2);
					grava_edge_set(edge, "color", "red");
					edge->jmpcnd = 0; // false
					grava_graph_add_edge(win->grava->graph, edge);
					break;
				}
			}
		}
	}
	program_free(prg);
	g_object_unref(node);

	grava_graph_update(win->grava->graph);

	if (new_window) {
		gtk_widget_show_all(GTK_WIDGET(win->w));
		gtk_window_resize(GTK_WINDOW(win->w), 600,400);
	}

	grava_widget_draw(win->grava);

	if (n_windows)
		return;

	n_windows++;
	new_window = 0;
	gtk_main();
	// oops. tihs is not for real!
	config_set("cfg.verbose", "true");
	config_set("scr.color", "true");
	config_set("asm.offset", "true");
	config_set("asm.trace", "false");
	config_set("asm.bytes", "true");
	config_set("asm.lines", "true");
	cons_set_fd(1);
	config.seek = here;
	gtk_is_init = 0;
	new_window = 0;
}
#endif
