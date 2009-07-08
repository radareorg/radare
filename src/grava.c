/*
 * Copyright (C) 2007, 2008, 2009
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

#include "main.h"

struct ugraph_node_t {
	ut64 from;
	ut64 size;
	char cmd[128];
	void *ptr;
	struct list_head list;
};

struct ugraph_edge_t {
	ut64 from;
	ut64 to;
	struct list_head list;
};

struct ugraph_t {
	struct list_head nodes;
	struct list_head edges;
};

static struct ugraph_t ug;

char *ugraph_get_str(ut64 addr, int size, int dot);

int graph_get_bb_size(struct program_t *prg, ut64 addr)
{
	struct block_t *b0;
	struct list_head *head;
	int size = 0;
	list_for_each_prev(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);
		if (addr == b0->addr)
			return b0->n_bytes;
	}
	return size;
}

void graph_viz(struct program_t *prg, int body)
{
	struct block_t *b0;
	struct list_head *head;
	int bs = 0;
	int withweight = config_get_i("graph.weight");
	int withtraces = config_get_i("graph.traces");

	cons_printf("digraph code {\n");
	//cons_printf("\tsize=\"6,6\";\n");
	cons_printf("\tgraph [bgcolor=%s];\n", config_get("graph.bgcolor"));
	cons_printf("\tnode [color=lightgray, style=filled shape=box fontname=\"Courier\" fontsize=\"8\"];\n");
	list_for_each_prev(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);
		if (withweight) {
			if (b0->tnext)
				cons_printf("\t\"0x%08llx\" -> \"0x%08llx\" [color=\"green\" weight=\"%d\"];\n",
						b0->addr, b0->tnext, (int)ABS(b0->tnext-b0->addr));
			if (b0->fnext)
				cons_printf("\t\"0x%08llx\" -> \"0x%08llx\" [color=\"red\" weight=\"%d\"];\n",
						b0->addr, b0->fnext, (int)ABS(b0->fnext-b0->addr));
		} else {
			if (b0->tnext)
				cons_printf("\t\"0x%08llx\" -> \"0x%08llx\" [color=\"green\"];\n", b0->addr, b0->tnext);
			if (b0->fnext)
				cons_printf("\t\"0x%08llx\" -> \"0x%08llx\" [color=\"red\"];\n", b0->addr, b0->fnext);
		}
		if (!b0->tnext && !b0->fnext)
			cons_printf("\t\"0x%08llx\";\n", b0->addr);
		if (body) {
			char *str = ugraph_get_str(b0->addr, b0->n_bytes, 1);
			if (str) {
				cons_printf(" \"0x%08llx\" [label=\"%s\"]\n", b0->addr, str);
				free(str);
				if (b0->fnext) {
					if ((bs = graph_get_bb_size(prg, b0->fnext))) {
						str = ugraph_get_str(b0->fnext, bs, 1);
						if (str) {
							cons_printf(" \"0x%08llx\" [fillcolor=\"%s\" color=\"red\" label=\"%s\"]\n",
								b0->fnext, (withtraces&&trace_times(b0->fnext))?"yellow":"gray", str);
								//(unsigned int)(ABS(b0->fnext-b0->addr)), str);
							free(str);
						}
					}
				}
				if (b0->tnext) {
					if ((bs = graph_get_bb_size(prg, b0->tnext))) {
						str = ugraph_get_str(b0->tnext, bs, 1);
						if (str) {
							cons_printf(" \"0x%08llx\" [fillcolor=\"%s\" color=\"red\" label=\"%s\"]\n",
								b0->tnext, (withtraces&&trace_times(b0->fnext))?"yellow":"white", str);
							free(str);
						}
					}
				}
			}
		}
	}
	cons_printf("}\n");
}

static int graphuser = 0;
static int graphuserinit = 0;

void graph_set_user(int b)
{
	graphuser = b;
}

#if HAVE_GUI
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "widget.h"
#include "node.h"
#include "edge.h"
#include "default_layout.h"

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
	GtkWidget *zoomin;
	GtkWidget *zoomout;
	GravaWidget *grava;
	GtkWidget *text;
};

static struct mygrava_window *last_window = NULL;
static int new_window = 0;
static int gtk_is_init = 0;
static int n_windows = 0;

struct static_nodes {
	char *command;
	struct list_head list;
};

struct list_head static_nodes;// = NULL;

struct mygrava_window *mygrava_get_widget(struct program_t *prg, int new);
void grava_program_graph(struct program_t *prg, struct mygrava_window *);
void do_grava_analysis(struct program_t *prg, struct mygrava_window *win);
static void core_load_graph_entry(void *widget, gpointer obj); //GtkWidget *obj);
static void core_load_node_entry(void *widget, gpointer obj); //GtkWidget *obj);
static struct program_t *prg = NULL; // last program code analysis

void core_load_graph_at(void *obj, const char *str)
{
	//GravaWidget *widget = obj;
	ut64 off;

	monitors_run();
	if (graphuser) {
		eprintf("Loading user graph... (%s)\n", str);
		//grava_graph_reset(win->grava->graph);
		//do_grava_analysis(prg, win);
	} else {
		off = get_math(str);
		eprintf("Loading graph... (%s)\n", str);
		radare_seek(off, SEEK_SET);
		prg = code_analyze(config.vaddr + config.seek, (int)config_get_i("graph.depth"));
		list_add_tail(&prg->list, &config.rdbs);
	}

	new_window = 1;
	if (last_window) {
		new_window = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(last_window));
	} 
	grava_program_graph(prg, last_window);
}

void mygrava_bp_at(void *unk, const char *str)
{
	char buf[1024];
	ut64 off = get_offset(str);
	sprintf(buf, "!bp %s", str);
	radare_cmd(buf, 0);
	eprintf("Breakpoint at (%08llx) (%s) added.\n", off, str);
	core_load_graph_at(NULL, "here");
}

void mygrava_bp_rm_at(void *unk, const char *str)
{
	char buf[1024];
	ut64 off = get_offset(str);
	sprintf(buf, "!bp -%s", str);
	radare_cmd(buf, 0);
	eprintf("Breakpoint at (%08llx) (%s) removed.\n", off, str);
	core_load_graph_at(NULL, "here");
}

static void mygrava_close(void *widget, gpointer obj)
{
	struct mygrava_window *w = obj;
	gtk_widget_destroy(GTK_WIDGET(w->w));
	if (--n_windows<0)
		n_windows = 0;
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
static void mygrava_back2(void *widget, void *foo, void *obj)
{ mygrava_back(widget, obj); }

static void core_load_node_entry(void *widget, void *obj)
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
	if (str == NULL || str[0]=='\0')
		return;

	snode = malloc(sizeof(struct static_nodes));
	snode->command = strdup(str);
	list_add_tail(&(snode->list), &(static_nodes));

	gtk_entry_set_text(GTK_ENTRY(w->entry),"");

	core_load_graph_entry(widget,obj);
}

static void core_load_graph_entry(void *widget, void *obj)
{
	const char *str;
	char *buf, *ptr;
	struct program_t *prg;
	struct mygrava_window *w = obj;
	ut64 off;

	if (w) {
		str = gtk_entry_get_text(GTK_ENTRY(w->entry));
		last_window = w;
	} else {
		eprintf("NullObj\n");
		return;
	}

	off = get_offset(str);
	//grava_graph_reset(last_window->grava->graph);
//	ptr = config_get("graph.layout");
//	if (ptr && !strcmp(ptr, "graphviz"))
//	else
	grava_default_layout_reset_layout(last_window->grava->graph->layout);

	eprintf("Loading graph... (%s) 0x%llx\n", str, off);
	if (off == 0 && str[0]!='0') {
		/* run command */
		if (config.debug)
			radare_cmd(".!regs*", 0);
		//radare_cmd(str, 0);
		//buf = radare_cmd_str(str);
		//cons_flush();
		radare_cmd_raw(str, 0);
		ptr = (char *)config_get("scr.seek");
		if (ptr&&ptr[0]) {
			ut64 off = get_math(ptr);
			if (off != 0)
				radare_seek(off, SEEK_SET);
		} 
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
	//gtk_widget_destroy(w);
	radare_seek(off, SEEK_SET);
	prg = code_analyze(config.vaddr + config.seek, (int)config_get_i("graph.depth"));
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
	fs ^= 1;
}

#ifndef GRAVA_WIDGET_ZOOM_FACTOR
#define GRAVA_WIDGET_ZOOM_FACTOR 0.3
#endif

gboolean grava_key_press_cb(GtkWidget * widget, GdkEventKey * event, struct mygrava_window *w)
{
    switch (event->keyval) {
#if _MAEMO_
    case GDK_F6:
  //      hildon_banner_show_information(GTK_WIDGET(window), NULL, "Full screen");
	grava_toggle_fullscreen(w->w);
        return TRUE;
    case GDK_F7:
        //hildon_banner_show_information(GTK_WIDGET(window), NULL, "Increase (zoom in)");
	w->grava->graph->zoom += (GRAVA_WIDGET_ZOOM_FACTOR);
        return TRUE;

    case GDK_F8:
	w->grava->graph->zoom -= (GRAVA_WIDGET_ZOOM_FACTOR);
        //hildon_banner_show_information(GTK_WIDGET(window), NULL, "Decrease (zoom out)");
        return TRUE;
#else
    case GDK_F10:
	grava_toggle_fullscreen(w->w);
        return TRUE;
#endif
    }
	return FALSE;
}

gboolean mygrava_zoomin(void *foo, void *bar, struct mygrava_window *w)
{
	w->grava->graph->zoom += (GRAVA_WIDGET_ZOOM_FACTOR);
	core_load_graph_at(NULL, "here");
	return TRUE;
}

gboolean mygrava_zoomout(void *foo, void *bar, struct mygrava_window *w)
{
	w->grava->graph->zoom -= (GRAVA_WIDGET_ZOOM_FACTOR);
	core_load_graph_at(NULL, "here");
	return TRUE;
}

void mygrava_run_cmd(void * foo, char *cmd)
{
//printf("running cmd (%s)\n", cmd);
	radare_cmd(cmd, 0);
}

void core_graph_reload(void * foo, char *cmd)
{
//printf("running cmd (%s)\n", cmd);
	radare_cmd(cmd, 0);
}

static int
 cfg_offset,
 cfg_section,
 cfg_bytes,
 cfg_trace,
 cfg_color,
 cfg_verbose,
 cfg_stackptr,
 cfg_lines;

void asm_state_save()
{
	/* store values */
	cfg_stackptr= config_get_i("asm.stackptr");
	cfg_offset = config_get_i("asm.offset");
	cfg_section= config_get_i("asm.section");
	cfg_verbose= config_get_i("asm.verbose");
	cfg_bytes  = config_get_i("asm.bytes");
	cfg_trace  = config_get_i("asm.trace");
	cfg_color  = config_get_i("cfg.color");
	cfg_lines  = config_get_i("asm.lines");

	/* set our own config */
	config_set("asm.profile", "graph");
	config.color = 0;
}

void asm_state_restore()
{
	config_set_i("asm.section", cfg_section);
	config_set_i("cfg.verbose", cfg_verbose);
	config_set_i("scr.color", cfg_color);
	config_set_i("asm.offset", cfg_offset);
	config_set_i("asm.trace", cfg_trace);
	config_set_i("asm.bytes", cfg_bytes);
	config_set_i("asm.lines", cfg_lines);
	config_set_i("asm.stackptr", cfg_stackptr);
}

struct mygrava_window *mygrava_get_widget(struct program_t *prg, int new)
{
	GtkWidget *tw,*tw2;
	char title[256], name[128];
	struct mygrava_window *win;

	win = (struct mygrava_window*)malloc(sizeof(struct mygrava_window));
	memset(win, '\0', sizeof(struct mygrava_window));

	win->grava = grava_widget_new();
	g_signal_connect(win->grava, "load-graph-at", ((GCallback) core_load_graph_at), win);
	g_signal_connect(win->grava, "breakpoint-at", ((GCallback) mygrava_bp_at), win);
	g_signal_connect(win->grava, "run-cmd", ((GCallback) mygrava_run_cmd), win);

	/* TODO: add left action panel */
	//acti = gradare_actions_new();
	//gtk_box_pack_start(GTK_HBOX(hpan), acti, TRUE, TRUE,0);

	if (new) {
		/* add window */
		win->w = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	#if 0 && _MAEMO_
		hildon_program_add_window(p, win->w);
	#endif
		g_signal_connect(G_OBJECT(win->w), "key_press_event", G_CALLBACK(grava_key_press_cb), win);
		g_signal_connect (win->w, "destroy", G_CALLBACK (mygrava_close), win);
		string_flag_offset(name, config.seek, -1);
		sprintf(title, "code graph: %s (0x%08x) %s", config.file, (unsigned int )config.seek, name);
		gtk_window_set_title(GTK_WINDOW(win->w), title);
	}

	/* TODO: add more control widgets */
	win->vbox = gtk_vbox_new(FALSE, 1);
	win->entry = gtk_entry_new();

	/* new button */
	win->bnew = gtk_button_new_with_mnemonic("");
	gtk_button_set_image (GTK_BUTTON (win->bnew), gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_BUTTON));

	win->zoomin = gtk_button_new_with_mnemonic("");
	gtk_button_set_image (GTK_BUTTON (win->zoomin), gtk_image_new_from_stock ("gtk-zoom-in", GTK_ICON_SIZE_BUTTON));

	win->zoomout = gtk_button_new_with_mnemonic("");
	gtk_button_set_image (GTK_BUTTON (win->zoomout), gtk_image_new_from_stock ("gtk-zoom-out", GTK_ICON_SIZE_BUTTON));

	win->close = gtk_button_new_with_mnemonic("");
	gtk_button_set_image (GTK_BUTTON (win->close), gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON));

	g_signal_connect(win->zoomin,"activate",((GCallback) mygrava_zoomin), (gpointer)win); 
	g_signal_connect(win->zoomin,"button-release-event",((GCallback) mygrava_zoomin), (gpointer)win); 
	g_signal_connect(win->zoomout,"activate",((GCallback) mygrava_zoomout), (gpointer)win); 
	g_signal_connect(win->zoomout,"button-release-event",((GCallback) mygrava_zoomout), (gpointer)win); 
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
	gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->zoomin), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->zoomout), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->bnew), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->close), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->hbox), FALSE, FALSE, 2);
	tw = gtk_expander_new("Output buffer:");
	win->text =  gtk_text_view_new ();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->text), 0);
//gtk_text_buffer_new(NULL);
	tw2 = gtk_scrolled_window_new(NULL,NULL);
	//gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tw2), GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tw2), GTK_POLICY_NEVER,GTK_POLICY_NEVER);
	gtk_container_add(GTK_CONTAINER(tw2), win->text);
	gtk_container_add(GTK_CONTAINER(tw), tw2);
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(tw), FALSE, FALSE, 2);

	if (new)
		gtk_container_add(GTK_CONTAINER(win->w), win->vbox);
	gtk_container_add(GTK_CONTAINER(win->vbox), grava_widget_get_widget(win->grava));

	if (!graphuser)
		do_grava_analysis(prg, win);

	return win;
}

#if 0
/* new one */
void do_grava_analysis_callgraph(struct program_t *prg, struct mygrava_window *win)
{
	int i;
	char *ptr;
	char cmd[1024];
	struct list_head *head, *head2;
	struct block_t *b0, *b1;
	struct xrefs_t *c0;
	GravaNode *node, *node2;
	GravaEdge *edge;
	/* analyze code */

	asm_state_save();

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
		sprintf(cmd+128, "0x%08llx  %s", b0->addr, cmd);
		grava_node_set(node, "color", "gray");

		// traced nodes are turquoise
		if (trace_times(b0->addr)>0)
			grava_node_set(node, "bgcolor", "beige");
			//grava_node_set(node, "color", "darkgray");

#if 0
		// XXX this makes radare segfault with g_object_unref HUH!
		if (flags_between((ut64)b0->addr,(ut64)(b0->addr + b0->n_bytes))>0)
			grava_node_set(node, "bgcolor", "yellow");
#endif

		/* add call references for this node */
		// XXX avoid dupped calls
		list_for_each(head2, &(b0->calls)) {
			c0 = list_entry(head2, struct xrefs_t, list);
			grava_node_add_call(node, c0->addr);
		}

		node->baseaddr = b0->addr;
		grava_node_set(node, "label", cmd+128);
		grava_node_set_i(node, "offset", b0->addr);

		/* disassemble body */
		//sprintf(cmd, "pD %d @ 0x%08llx", b0->n_bytes +((b0->n_bytes<3)?1:0), b0->addr);
		sprintf(cmd, "pD %d @ 0x%08llx", b0->n_bytes, b0->addr);
		ptr =  pipe_command_to_string(cmd);
		grava_node_set(node, "body", ptr);
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

		printf("A %08llx\n", b0->addr);
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
}
#endif
#endif

/* user graph */
void ugraph_reset()
{
	if (graphuserinit) {
		/* TODO : memory leak */
	} 
	INIT_LIST_HEAD(&ug.nodes);
	INIT_LIST_HEAD(&ug.edges);
	graphuserinit = 1;
}

void ugraph_node(ut64 from, ut64 size, const char *cmd)
{
	struct list_head *pos;
	struct ugraph_node_t *n;
	if (!config_get_i("graph.userdup")) {
		list_for_each_prev(pos, &(ug.nodes)) {
			n = list_entry(pos, struct ugraph_node_t, list);
			if (n->from == from)
				return;
		}
	}
	n = MALLOC_STRUCT(struct ugraph_node_t);
	n->from = from;
	n->size = size;
	strncpy(n->cmd, cmd, sizeof(n->cmd));
	if (n->cmd[strlen(n->cmd)-1]=='"')
		n->cmd[strlen(n->cmd)-1]='\0';
	if (!graphuserinit)
		ugraph_reset();
	list_add_tail(&(n->list), &ug.nodes);
}

void ugraph_edge(ut64 from, ut64 to)
{
	struct ugraph_edge_t *e = MALLOC_STRUCT(struct ugraph_edge_t);
	
	e->from = from;
	e->to = to;
	if (!graphuserinit)
		ugraph_reset();
	list_add_tail(&(e->list), &ug.edges);
}

struct ugraph_node_t *ugraph_get(ut64 addr)
{
	struct list_head *head;
	if (!graphuserinit)
		ugraph_reset();
	list_for_each_prev(head, &(ug.nodes)) {
		struct ugraph_node_t *n = list_entry(head, struct ugraph_node_t, list);
		if (n->from == addr)
			return n;
	}
	return NULL;
}

char *ugraph_get_str(ut64 addr, int _size, int dot)
{
	int i,j;
	char *cmdstr, *str = NULL;
	char *cmd = "pd";
	struct ugraph_node_t *n = ugraph_get(addr);
	ut64 from = addr;
	int size = _size;
	if (n) {
		from = n->from;
		size = n->size;
		cmd = n->cmd;
	}
	radare_seek(from, SEEK_SET);
	radare_set_block_size_i(size);
	cmdstr = radare_cmd_str(cmd);
	if (!dot)
		return cmdstr;
	if (cmdstr != NULL)
		str = malloc(strlen(cmdstr)*2);
	if (str != NULL) {
		for(i=j=0;cmdstr[i];i++,j++) {
			switch(cmdstr[i]) {
			case 0x1b: // hackyansistrip
				/* skip ansi chars */
				for(i++;cmdstr[i]&&cmdstr[i]!='m'&&cmdstr[i]!='H'&&cmdstr[i]!='J';i++);
				j--;
				break;
			case '\n':
			case '\r':
				str[j]='\\';
				str[++j]='l';
				break;
			default:
				str[j]=cmdstr[i];
			}
		}
		str[j]='\0';
		free(cmdstr);
	}
	return str;
}

void ugraph_print_dot(int body)
{
	int i,j;
	struct list_head *head, *pos;
	int withweight = config_get_i("graph.weight");

	if (!graphuserinit)
		ugraph_reset();

	cons_strcat("digraph {\n");
	cons_printf("  graph [bgcolor=%s];\n", config_get("graph.bgcolor"));
	cons_strcat("  node [color=lightgray, style=filled shape=box fontname=\"Courier\" fontsize=\"8\"];\n");

	list_for_each_prev(head, &(ug.edges)) {
		struct ugraph_edge_t *e = list_entry(head, struct ugraph_edge_t, list);
		if (body) {
			char *str = ugraph_get_str(e->from, 32, 1);
			if (str) {
				cons_printf(" \"0x%08llx\" [label=\"%s\"]\n", e->from, str);
				free(str);
			}
			str = ugraph_get_str(e->to, 32, 1);
			if (str) {
				cons_printf(" \"0x%08llx\" [label=\"%s\"]\n", e->to, str);
				free(str);
			}
		}
		if (withweight) {
			cons_printf(" \"0x%08llx\" -> \"0x%08llx\" ;\n", e->from, e->to);
		} else {
			cons_printf(" \"0x%08llx\" -> \"0x%08llx\" [weight=\"%d\"];\n", e->from, e->to, ABS(e->to-e->from));
		}
	}
	cons_printf("}\n");
	cons_flush();
}

#if HAVE_GUI

void load_user_graph(struct program_t *prg, struct mygrava_window *win)
{
	ut64 oseek = config.seek;
	int i;
	char *ptr;
	char cmd[1024];
	struct list_head *head, *head2;
	int set =0;
	GravaNode *node, *node2;
	GravaEdge *edge;
	struct ugraph_node_t *n, *m;
	asm_state_save();

	if (win == NULL) {
		eprintf("No window yet\n");
		return;
	}
	if (win->grava == NULL) {
		eprintf("No window yet\n");
		return;
	}

	if (!graphuserinit)
		ugraph_reset();

	eprintf("Loading user graph...\n");
	/* add static nodes */
	i = 0;
	list_for_each_prev(head, &(ug.nodes)) {
		struct ugraph_node_t *n = list_entry(head, struct ugraph_node_t, list);
		char *cmdstr;
		GravaNode *node = grava_node_new();
		g_object_ref(node);

		sprintf(cmd, "0x%08llx", n->from);
		grava_node_set(node, "label", cmd);

cmdstr = ugraph_get_str(n->from, n->size, 0);
#if 0
		// TODO: execute command
radare_seek(n->from, SEEK_SET);
radare_set_block_size_i((int)(unsigned int)n->size);
		cmdstr = radare_cmd_str(n->cmd);
//eprintf("(%s)=(%s)\n", n->cmd, cmdstr);
eprintf(" node 0x%08llx\n", n->from);
#endif
		grava_node_set(node, "body", cmdstr);
		free(cmdstr);
		grava_graph_add_node(win->grava->graph, node);
		n->ptr = node;
	//	g_object_unref(node);
	}

	/* add edges */
	i = 0;
	list_for_each_prev(head, &(ug.edges)) {
		struct ugraph_edge_t *e = list_entry(head, struct ugraph_edge_t, list);
		n = ugraph_get(e->from);
		m = ugraph_get(e->to);
eprintf(" edge 0x%08llx 0x%08llx\n", e->from, e->to);
		if (!n || !m) {
			eprintf("Oops. cannot found node 0x%08llx for edge 0x%08llx->0x%08llx\n",
				e->from, e->from, e->to);
			continue;
		}
		node = n->ptr;
		node2 = m->ptr;

		edge = grava_edge_with(grava_edge_new(), node, node2);
		grava_edge_set(edge, "color", "green"); // TODO
		edge->jmpcnd = 1; // true
		grava_graph_add_edge(win->grava->graph, edge);
	}
	radare_seek(oseek, SEEK_SET);
}

void do_grava_analysis(struct program_t *prg, struct mygrava_window *win)
{
	int i;
	char *ptr;
	char cmd[1024];
	int withtraces = config_get_i("graph.traces");
	int graph_flagblocks = (int)config_get("graph.flagblocks");
	struct list_head *head, *head2;
	struct block_t *b0, *b1;
	struct xrefs_t *c0;
	int set =0;
	GravaNode *node, *node2;
	GravaEdge *edge;
	/* analyze code */

	asm_state_save();

	/* add static nodes */
	i = 0;
	list_for_each_prev(head, &(static_nodes)) {
		struct static_nodes *s0 = list_entry(head, struct static_nodes, list);
		GravaNode *node = grava_node_new();
		g_object_ref(node);
		grava_node_set(node, "label", s0->command);
		
		ptr =  pipe_command_to_string(s0->command);
		if (ptr) {
			grava_node_set(node, "body", ptr);
		}
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
		string_flag_offset(cmd, b0->addr, -1);
		cmd[127]='\0'; // XXX ugly string recycle hack
		sprintf(cmd+128, "0x%08llx  %s", b0->addr, cmd);
		if (!graph_flagblocks)
			continue;

		// traced nodes are turquoise
		if (withtraces && trace_times(b0->addr)>0)
			grava_node_set(node, strdup("bgcolor"), strdup("beige"));
			//grava_node_set(node, "color", "darkgray");

#if 0
		// XXX this makes radare segfault with g_object_unref HUH!
		if (flags_between((ut64)b0->addr,(ut64)(b0->addr + b0->n_bytes))>0)
			grava_node_set(node, "bgcolor", "yellow");
#endif

		/* add call references for this node */
		// XXX avoid dupped calls
		list_for_each(head2, &(b0->calls)) {
			c0 = list_entry(head2, struct xrefs_t, list);
			grava_node_add_call(node, c0->addr);
		}

		node->baseaddr = b0->addr;
		grava_node_set(node, "label", cmd+128);
		grava_node_set_i(node, "offset", b0->addr);

		/* disassemble body */

#if 0
		sprintf(cmd, "pD %d @ 0x%08llx", b0->n_bytes, b0->addr);
		ptr =  pipe_command_to_string(cmd);
#else
ptr= ugraph_get_str (b0->addr, b0->n_bytes, 0);
#endif
		//ptr =  radare_cmd_str(cmd); //pipe_command_to_string(cmd);
		grava_node_set(node, "body", ptr);
		if (strstr(ptr, "eip:"))
			grava_node_set(node, "color", "red");
		else grava_node_set(node, strdup("color"), strdup("gray"));
		//printf("cmd_str = %x\n", grava_graph_get(win->grava->graph, "cmd"));
		//grava_graph_set(win->grava->graph, "cmd", "; XXX LOL!!!");
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

		printf("A %08llx\n", b0->addr);
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

	grava_graph_update(win->grava->graph);
}
#endif

#if HAVE_GUI
void grava_program_graph(struct program_t *prg, struct mygrava_window *win)
{
	ut64 here = config.seek;
	//int graph_flagblocks = (int)config_get("graph.flagblocks");

	/* create widget */
	if (!gtk_is_init) {
		if ( ! gtk_init_check(NULL, NULL) ) {
			eprintf("Oops. Cannot initialize gui\n");
			return;
		}
		gtk_is_init = 1;
		new_window = 1;
		INIT_LIST_HEAD(&static_nodes);
	}

	if (win==NULL || new_window) {
		win = mygrava_get_widget(prg, 1);
		last_window = win;
	} else {
		grava_graph_reset(win->grava->graph);
		if (!graphuser)
			do_grava_analysis(prg, win);
	}

	if (graphuser) {
		grava_graph_reset(win->grava->graph);
		load_user_graph(prg, win);
	}

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

	asm_state_restore();
	// oops. tihs is not for real!
	cons_set_fd(1);
	config.seek = here;
	gtk_is_init = 0;
	new_window = 0;
}
#else
void grava_program_graph(struct program_t *prg, void *win)
{
	eprintf("Compiled without gui\n");
}
#endif

void visual_gui()
{
#if HAVE_GUI
	struct mygrava_window *win = NULL;
	GtkWidget *w, *w2;
	char *items;
	const char *name;
	int i, num;

	/* create widget */
	if (!gtk_is_init) {
		if ( ! gtk_init_check(NULL, NULL) ) {
			eprintf("Oops. Cannot initialize gui\n");
			return;
		}
		gtk_is_init = 1;
		new_window = 1;
		INIT_LIST_HEAD(&static_nodes);
	}

	if (win==NULL || new_window) {
		win = (struct mygrava_window*)malloc(sizeof(struct mygrava_window));
		memset(win, '\0', sizeof(struct mygrava_window));
	//	last_window = win;

		win->grava = grava_widget_new();
		g_signal_connect(win->grava, "load-graph-at", ((GCallback) core_load_graph_at), win);
		g_signal_connect(win->grava, "breakpoint-at", ((GCallback) mygrava_bp_at), win);
		g_signal_connect(win->grava, "run-cmd", ((GCallback) mygrava_run_cmd), win);

		/* TODO: add left action panel */
		//acti = gradare_actions_new();
		//gtk_box_pack_start(GTK_HBOX(hpan), acti, TRUE, TRUE,0);
		win->w = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);

		g_signal_connect(G_OBJECT(win->w), "key_press_event", G_CALLBACK(grava_key_press_cb), win);
		g_signal_connect(win->w, "destroy", G_CALLBACK (mygrava_close), win);
		gtk_window_set_title(GTK_WINDOW(win->w), "radare: visual gui");

		/* TODO: add more control widgets */
		// TODO: move hbox, topbox to win-> struct
		win->vbox = gtk_vbox_new(FALSE, 1);
		gtk_container_add(GTK_CONTAINER(win->w), win->vbox);

		/* fill window */
		items = strdup(config_get("gui.top"));
		num = set0word(items);
		for(i=0;i<num;i++) {
			eprintf("gui.top: %s\n", get0word(items, i));
			w2 = plugin_get_widget(get0word(items,i));
			if (w2 != NULL)
				gtk_box_pack_start(GTK_BOX(win->vbox), w2, FALSE, FALSE, 0);
		}
		free((void *)items);
		win->hbox = gtk_hbox_new(FALSE, 3);
		gtk_container_add(GTK_CONTAINER(win->vbox), win->hbox);
		//
		
		{
		GtkWidget *exp = gtk_expander_new("");
		GtkWidget *vbx = gtk_vbox_new(FALSE, 3);

		gtk_container_add(GTK_CONTAINER(exp), GTK_WIDGET(vbx));

		w = gtk_notebook_new();
		gtk_container_add(GTK_CONTAINER(vbx), w);
		gtk_box_pack_start(GTK_BOX(win->hbox), exp, FALSE, FALSE, 2);
		items = strdup(config_get("gui.left"));
		for(i=0;i<num;i++) {
			eprintf("gui.tabs:(%s)\n", get0word(items, i));
			name = get0word(items,i);
			w2 = plugin_get_widget(get0word(items,i));
			if (w2 != NULL)
				gtk_notebook_append_page(GTK_NOTEBOOK(w),w2, gtk_label_new(name));
		}
		free((void *)items);
		}

		//
		items = strdup(config_get("gui.tabs"));
		w = gtk_notebook_new();
		{ /* code graph tab */
			struct mygrava_window *foo;
			struct program_t *prg = code_analyze(config.vaddr + config.seek, (int)config_get_i("graph.depth"));
			list_add_tail(&prg->list, &config.rdbs);
			foo = mygrava_get_widget(prg, 0);
			w2 = foo->vbox;
			gtk_widget_show_all(GTK_WIDGET(win->vbox));
		//	grava_widget_draw(foo->grava);
			gtk_notebook_append_page(GTK_NOTEBOOK(w),w2, gtk_label_new("Graph"));
		}
		num = set0word(items);
		for(i=0;i<num;i++) {
			eprintf("gui.tabs:(%s)\n", get0word(items, i));
			name = get0word(items,i);
			w2 = plugin_get_widget(name);
			if (w2 != NULL)
				gtk_notebook_append_page(GTK_NOTEBOOK(w),w2, gtk_label_new(name));
		}
		gtk_container_add(GTK_CONTAINER(win->hbox), w);
		free((void *)items);

		gtk_widget_show_all(GTK_WIDGET(win->w));
	}

	if (new_window) {
		gtk_widget_show_all(GTK_WIDGET(win->w));
		gtk_window_resize(GTK_WINDOW(win->w), 600,400);
	}

	if (n_windows)
		return;

	n_windows++;
	new_window = 0;

	gtk_main();
	cons_set_fd(1);
	gtk_is_init = 0;
	new_window = 0;
#else
	eprintf("Compiled without GUI\n");
#endif
}
