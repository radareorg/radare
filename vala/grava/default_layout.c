/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007  pancake <youterm.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "default_layout.h"
#include "edge.h"

struct _GravaDefaultLayoutPrivate {
	GravaGraph* graph;
};
#define GRAVA_DEFAULT_LAYOUT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayoutPrivate))
enum  {
	GRAVA_DEFAULT_LAYOUT_DUMMY_PROPERTY
};
static void grava_default_layout_treenodes (GravaDefaultLayout* self, GravaNode* n);
static void grava_default_layout_real_run (GravaLayout* base, GravaGraph* graph);
static gpointer grava_default_layout_parent_class = NULL;
static void grava_default_layout_dispose (GObject * obj);


#line 27 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
static void grava_default_layout_treenodes (GravaDefaultLayout* self, GravaNode* n) {
	gint ox;
	GSList* nodes;
#line 27 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_if_fail (GRAVA_IS_DEFAULT_LAYOUT (self));
#line 27 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_if_fail (n == NULL || GRAVA_IS_NODE (n));
#line 29 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	ox = 0;
#line 30 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	nodes = grava_graph_outer_nodes (self->priv->graph, n);
	{
		GSList* node_collection;
		GSList* node_it;
#line 31 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		node_collection = nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 31 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			node = node_it->data;
			{
#line 32 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				node->y = n->y + n->h + self->y_offset;
#line 33 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				node->x = node->x + (n->w + ox);
#line 34 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				ox = ox + (50);
#line 35 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				grava_default_layout_treenodes (self, node);
			}
		}
	}
	(nodes == NULL ? NULL : (nodes = (g_slist_foreach (nodes, ((GFunc) g_object_unref), NULL), g_slist_free (nodes), NULL)));
}


#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
void grava_default_layout_walkChild (GravaDefaultLayout* self, GravaNode* node, gint level) {
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_if_fail (GRAVA_IS_DEFAULT_LAYOUT (self));
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_if_fail (node == NULL || GRAVA_IS_NODE (node));
#line 41 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	if (level < 1) {
#line 41 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		return;
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		edge_collection = self->priv->graph->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			edge = edge_it->data;
			{
#line 43 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (edge->orig == node) {
#line 44 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					edge->dest->y = edge->orig->y + edge->orig->h + self->y_offset;
#line 45 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					grava_default_layout_walkChild (self, edge->dest, (level = level - 1));
				}
			}
		}
	}
}


#line 50 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
GravaNode* grava_default_layout_get_parent (GravaDefaultLayout* self, GravaNode* node) {
#line 50 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_val_if_fail (GRAVA_IS_DEFAULT_LAYOUT (self), NULL);
#line 50 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_val_if_fail (node == NULL || GRAVA_IS_NODE (node), NULL);
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 52 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		edge_collection = self->priv->graph->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 52 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			edge = edge_it->data;
			{
#line 53 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (edge->dest == node) {
#line 54 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					return edge->orig;
				}
			}
		}
	}
#line 56 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	return NULL;
}


#line 59 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
static void grava_default_layout_real_run (GravaLayout* base, GravaGraph* graph) {
	GravaDefaultLayout * self;
	gboolean overlaps;
	GSList* nodz;
	self = GRAVA_DEFAULT_LAYOUT (base);
#line 59 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	g_return_if_fail (graph == NULL || GRAVA_IS_GRAPH (graph));
#line 61 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	self->priv->graph = graph;
	{
		GSList* node_collection;
		GSList* node_it;
#line 65 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		node_collection = graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 65 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			node = node_it->data;
			{
				/*//return; // HAHAHA
				 reset all node positions*/
#line 66 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (grava_graph_selected == NULL) {
#line 67 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					grava_graph_selected = node;
				}
#line 68 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (!node->visible) {
#line 68 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					continue;
				}
				/* reset node positions*/
#line 70 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				node->x = 50;
#line 71 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				node->y = 50;
#line 72 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				grava_node_fit (node);
			}
		}
	}
#line 75 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	grava_default_layout_walkChild (self, grava_graph_selected, 5);
	{
		GSList* node_collection;
		GSList* node_it;
#line 77 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		node_collection = graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 77 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			node = node_it->data;
			{
				/*walkChild(graph.selected); //*/
#line 78 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				grava_default_layout_walkChild (self, node, 5);
			}
		}
	}
	{
		GSList* node_collection;
		GSList* node_it;
#line 81 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		node_collection = graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 81 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			node = node_it->data;
			{
				GravaNode* parent;
#line 82 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (!node->visible) {
#line 82 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					continue;
				}
#line 84 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				parent = grava_default_layout_get_parent (self, node);
#line 85 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (parent != NULL) {
#line 86 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					node->x = parent->x;
				}
			}
		}
	}
#line 90 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	overlaps = FALSE;
#line 91 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
	nodz = NULL;
	{
		GSList* n_collection;
		GSList* n_it;
#line 92 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		n_collection = graph->nodes;
		for (n_it = n_collection; n_it != NULL; n_it = n_it->next) {
			GravaNode* n;
#line 92 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			n = n_it->data;
			{
				double totalx;
				gint total;
				GSList* _tmp0;
				GravaNode* _tmp1;
				GSList* _tmp3;
#line 93 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				totalx = 0;
#line 94 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				total = 0;
				_tmp0 = NULL;
#line 95 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				nodz = (_tmp0 = NULL, (nodz == NULL ? NULL : (nodz = (g_slist_foreach (nodz, ((GFunc) g_object_unref), NULL), g_slist_free (nodz), NULL))), _tmp0);
#line 96 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				_tmp1 = NULL;
#line 96 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				nodz = g_slist_append (nodz, (_tmp1 = n, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))));
				{
					GSList* n2_collection;
					GSList* n2_it;
#line 97 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					n2_collection = graph->nodes;
					for (n2_it = n2_collection; n2_it != NULL; n2_it = n2_it->next) {
						GravaNode* n2;
#line 97 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
						n2 = n2_it->data;
						{
#line 98 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
							if (n != n2 && grava_node_overlaps (n, n2)) {
								GravaNode* _tmp2;
#line 99 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
								totalx = totalx + (n2->w + self->x_offset);
#line 100 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
								n->x = n->x + (n->w + n2->w + self->x_offset);
#line 101 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
								_tmp2 = NULL;
#line 101 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
								nodz = g_slist_append (nodz, (_tmp2 = n2, (_tmp2 == NULL ? NULL : g_object_ref (_tmp2))));
							}
						}
					}
				}
#line 104 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				totalx = totalx / (2);
				{
					GSList* n2_collection;
					GSList* n2_it;
#line 105 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					n2_collection = nodz;
					for (n2_it = n2_collection; n2_it != NULL; n2_it = n2_it->next) {
						GravaNode* n2;
#line 105 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
						n2 = n2_it->data;
						{
#line 106 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
							if (n != n2 && grava_node_overlaps (n, n2)) {
#line 107 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
								totalx = totalx + (n2->w);
#line 108 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
								n->x = n->x - ((total));
							}
						}
					}
				}
				_tmp3 = NULL;
#line 111 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				nodz = (_tmp3 = NULL, (nodz == NULL ? NULL : (nodz = (g_slist_foreach (nodz, ((GFunc) g_object_unref), NULL), g_slist_free (nodz), NULL))), _tmp3);
			}
		}
	}
	{
		GSList* node_collection;
		GSList* node_it;
#line 114 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		node_collection = graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 114 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			node = node_it->data;
			{
#line 115 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (!node->visible) {
#line 115 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					continue;
				}
#line 117 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (grava_graph_overlaps (graph, node)) {
#line 118 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					node->x = node->x + (node->w);
				}
			}
		}
	}
	{
		GSList* node_collection;
		GSList* node_it;
#line 120 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
		node_collection = graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 120 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
			node = node_it->data;
			{
				GravaNode* parent;
#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (!node->visible) {
#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					continue;
				}
#line 123 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				parent = grava_default_layout_get_parent (self, node);
#line 124 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
				if (parent != NULL) {
#line 125 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
					node->x = parent->x;
				}
			}
		}
	}
	(nodz == NULL ? NULL : (nodz = (g_slist_foreach (nodz, ((GFunc) g_object_unref), NULL), g_slist_free (nodz), NULL)));
}


#line 21 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/default_layout.vala"
GravaDefaultLayout* grava_default_layout_new (void) {
	GravaDefaultLayout * self;
	self = g_object_newv (GRAVA_TYPE_DEFAULT_LAYOUT, 0, NULL);
	return self;
}


static void grava_default_layout_class_init (GravaDefaultLayoutClass * klass) {
	grava_default_layout_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (GravaDefaultLayoutPrivate));
	G_OBJECT_CLASS (klass)->dispose = grava_default_layout_dispose;
	GRAVA_LAYOUT_CLASS (klass)->run = grava_default_layout_real_run;
}


static void grava_default_layout_init (GravaDefaultLayout * self) {
	self->priv = GRAVA_DEFAULT_LAYOUT_GET_PRIVATE (self);
	self->y_offset = 100;
	self->x_offset = 50;
}


static void grava_default_layout_dispose (GObject * obj) {
	GravaDefaultLayout * self;
	self = GRAVA_DEFAULT_LAYOUT (obj);
	G_OBJECT_CLASS (grava_default_layout_parent_class)->dispose (obj);
}


GType grava_default_layout_get_type (void) {
	static GType grava_default_layout_type_id = 0;
	if (G_UNLIKELY (grava_default_layout_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaDefaultLayoutClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_default_layout_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaDefaultLayout), 0, (GInstanceInitFunc) grava_default_layout_init };
		grava_default_layout_type_id = g_type_register_static (GRAVA_TYPE_LAYOUT, "GravaDefaultLayout", &g_define_type_info, 0);
	}
	return grava_default_layout_type_id;
}




