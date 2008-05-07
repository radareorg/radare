#include "widget.h"
#include "node.h"
#include "edge.h"

void load_graph_at()
{
	printf("Punkt!\n");
}

void core_load_graph_at(void *ptr, const char *str)
{
	printf("Loading at %s\n", str);
}

void mygrava_bp_at(void *ptr, const char *str)
{
}

void mygrava_bp_rm_at(void *ptr, const char *str)
{
}


void core_load_graph_at_label(void *ptr, const char *str)
{
	printf("Loading at %s\n", str);
}


int main(int argc, char **argv)
{
	GravaNode *node, *node2, *node3, *node4;
	GravaEdge *edge;
	GravaWidget *grava;
	GtkWindow *w;
	char *_tmp0 = "label";
	char *_tmp1 = "mov eax, ebx\nmov ebx,ecx\n inc ecx";
	
	//gtk_init(&argc, &argv);
	gtk_init(NULL, NULL);

	grava = grava_widget_new();

	node = grava_node_new();
	grava_node_set(node, "label", "foo:");
	grava_node_set(node, "color", "black");
	grava_node_set(node, "body", "mov eax, 33\nxor ebx, ebx");
	grava_graph_add_node(grava->graph, node);

	node2 = grava_node_new();
	grava_node_set(node2, "label", "bar:");
	grava_node_set(node2, "color", "white");
	grava_node_set(node2, "body", "mov eax, 33\npush 0x80484800\ncall 0x8049320\nxor ebx, ebx");
	grava_graph_add_node(grava->graph, node2);

	edge = grava_edge_with(grava_edge_new(), node, node2);
	grava_edge_set(edge, "color", "red");
	grava_graph_add_edge(grava->graph, edge);

	node3 = grava_node_new();
	grava_node_set(node3, "label", "cow:");
	grava_node_set(node3, "color", "blue");
	grava_node_set(node3, "body", "xor eax, eax\nxor ebx, ebx");
	grava_graph_add_node(grava->graph, node3);

	grava_node_add_call(node3, 0x8048000);
	grava_node_add_call(node3, 0x8048320);
	grava_node_add_call(node3, 0x8048120);

	edge = grava_edge_with(grava_edge_new(), node, node3);
	grava_edge_set(edge, "color", "red");
	grava_graph_add_edge(grava->graph, edge);

	node4 = grava_node_new();
	grava_node_set(node4, "label", "miau:");
	grava_node_set(node4, "color", "green");
	grava_node_set(node4, "body", "xor eax, eax\nxor ebx, ebx");
	grava_graph_add_node(grava->graph, node4);

	edge = grava_edge_with(grava_edge_new(), node4, node3);
	grava_edge_set(edge, "color", "blue");
	grava_graph_add_edge(grava->graph, edge);

	edge = grava_edge_with(grava_edge_new(), node2, node3);
	grava_edge_set(edge, "color", "blue");
	grava_graph_add_edge(grava->graph, edge);

	grava_graph_update(grava->graph);

	/* window and so */
	w = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_resize(w, 600,400);
	g_signal_connect (w, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	gtk_container_add(GTK_CONTAINER(w), grava_widget_get_widget(grava));	

	g_signal_connect_object (grava, "load-graph-at", ((GCallback) load_graph_at), grava, 0);

	gtk_widget_show_all(GTK_WIDGET(w));

	gtk_main();

	return 0;
}
