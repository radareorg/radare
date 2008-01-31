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
#include "main.h"
#include "widget.h"
#include "node.h"
#include "edge.h"

static GravaWidget *grava;
static GtkWindow *w;
static GtkWidget *entry;

void grava_program_graph(struct program_t *prg);

void core_load_graph_at(void *widget, char *str)
{
	struct program_t *prg;
	off_t off = get_offset(str);
	eprintf("Loading graph... (%s)\n", str);
	radare_seek(off, SEEK_SET);
	//gtk_widget_destroy(w);
	prg = code_analyze(config.baddr + config.seek, config_get_i("graph.depth"));
	list_add_tail(&prg->list, &config.rdbs);
	grava_program_graph(prg);
}

static void core_load_graph_entry(void *widget, GtkWidget *obj)
{
	const char *str = gtk_entry_get_text(obj);
	struct program_t *prg;
	off_t off = get_offset(str);

	eprintf("Loading graph... (%s)\n", str);
	radare_seek(off, SEEK_SET);
	//gtk_widget_destroy(w);
	prg = code_analyze(config.baddr + config.seek, config_get_i("graph.depth"));
	list_add_tail(&prg->list, &config.rdbs);
	grava_program_graph(prg);
}
static void core_load_graph_entry2(void *widget, void *foo, GtkWidget *obj)
{ core_load_graph_entry(widget, obj); }

void core_load_graph_at_label(const char *str)
{
	char buf[256];
	char *ptr;

	strncpy(buf, str, 255);
	ptr = strchr(buf, ':');
	if (ptr) ptr[0]='\0';
	core_load_graph_at(NULL, buf);
}


void grava_program_graph(struct program_t *prg)
{
	char cmd[1024];
	static int gtk_is_init = 0;
	char *ptr;
	int i;
	struct list_head *head, *head2;
	struct block_t *b0, *b1;
	struct xref_t *c0;
	off_t here = config.seek;
	char title[256], name[128];
	int graph_flagblocks = config_get("graph.flagblocks");

	GtkWidget *vbox, *hbox, *go;
	GravaNode *node, *node2;
	GravaEdge *edge;
	/* create widget */
	if (!gtk_is_init)
		gtk_init(NULL,NULL);

	/* add window */
	w = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	string_flag_offset(name, config.seek);
	sprintf(title, "code graph: %s (0x%08x) %s", config.file, (unsigned int )config.seek, name);
	gtk_window_set_title(GTK_WINDOW(w), title);
	g_signal_connect (w, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	/* TODO: add control widgets */

	/* add grava widget */
	grava  = grava_widget_new();
	g_signal_connect_object (grava, "load-graph-at", ((GCallback) core_load_graph_at), grava, 0);
	vbox = gtk_vbox_new(FALSE, 1);

	entry = gtk_entry_new();
	hbox = gtk_hbox_new(FALSE, 2);
	go = gtk_button_new_with_label("Go");
	g_signal_connect_object(entry,"activate",((GCallback) core_load_graph_entry), entry, 0); 
	g_signal_connect_object(go,"activate",((GCallback) core_load_graph_entry), entry, 0); 
	g_signal_connect_object(go,"button-release-event",((GCallback) core_load_graph_entry2), entry, 0); 
	gtk_container_add(GTK_CONTAINER(hbox), entry);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(go), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox), FALSE, FALSE, 2);

	gtk_container_add(GTK_CONTAINER(w), vbox);
	gtk_container_add(GTK_CONTAINER(vbox), grava_widget_get_widget(grava));

	/* analyze code */
	config_set("asm.offset", "false");
	if (config_get("graph.offset"))
		config_set("asm.offset", "true");
	else 	config_set("asm.offset", "false");
	config_set("asm.bytes", "false");
	config_set("scr.color", "false");
	config_set("asm.lines", "false");
	config.color = 0;

	/* add nodes */
	i = 0;
	list_for_each_prev(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);

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

		node = grava_node_new();

		/* add call references for this node */
		// XXX avoid dupped calls
		list_for_each(head2, &(b0->calls)) {
			c0 = list_entry(head2, struct xref_t, list);
			grava_node_add_call(node, c0->addr);
		}

		node->baseaddr = b0->addr;
		grava_node_set(node, "label", cmd+128);

		/* disassemble body */
		sprintf(cmd, "pD %d @ 0x%08lx", b0->n_bytes-1 , (unsigned int)b0->addr);
		config.seek = b0->addr;
		radare_read(0);
		ptr =  pipe_command_to_string(cmd);
		grava_node_set(node, "body", ptr);
		printf("B (0x%08x) (%d) (\n%s)\n", (unsigned int)b0->addr, (unsigned int)b0->n_bytes-1, ptr);
		free(ptr);

		grava_graph_add_node(grava->graph, node);
		b0->data = node;
		i++;
	}

	/* add edges */
	i = 0;
	list_for_each_prev(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);
		node = b0->data;

		printf("A %08x\n", b0->addr);
		if (b0->tnext) {
			list_for_each(head2, &(prg->blocks)) {
				b1 = list_entry(head2, struct block_t, list);
				if (b0->tnext == b1->addr) {
					printf("T %08x\n", b0->tnext);
					node2 = b1->data;
					//if (!gtk_is_init)
					//grava_node_set(node2, "color", "green");
					edge = grava_edge_with(grava_edge_new(), node, node2);
					grava_edge_set(edge, "color", "green");
					edge->jmpcnd = 1; // true
					grava_graph_add_edge(grava->graph, edge);
					break;
				}
			}
		}

		if (b0->fnext) {
			list_for_each(head2, &(prg->blocks)) {
				b1 = list_entry(head2, struct block_t, list);
				if (b0->fnext == b1->addr) {
					printf("F %08x\n", b0->fnext);
					node2 = b1->data;
					//if (!gtk_is_init)
					//grava_node_set(node2, "color", "red");
					edge = grava_edge_with(grava_edge_new(), node, node2);
					grava_edge_set(edge, "color", "red");
					edge->jmpcnd = 0; // false
					grava_graph_add_edge(grava->graph, edge);
					break;
				}
			}
		}
	}
	program_free(prg);
	grava_graph_update(grava->graph);
	gtk_widget_show_all(GTK_WIDGET(w));
	gtk_window_resize(GTK_WINDOW(w), 600,400);
	gtk_main();

	// oops. tihs is not for real!
	config_set("cfg.verbose", "true");
	config_set("scr.color", "true");
	config_set("asm.offset", "true");
	config_set("asm.bytes", "true");
	config_set("asm.lines", "true");
	cons_set_fd(1);
	config.seek = here;
}
#endif
