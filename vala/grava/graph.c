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

#include "graph.h"
#include "renderer.h"
#include "default_layout.h"

enum  {
	GRAVA_GRAPH_DUMMY_PROPERTY
};
static GObject * grava_graph_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_graph_parent_class = NULL;
static void grava_graph_dispose (GObject * obj);


#line 40 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_select_next (GravaGraph* self) {
#line 40 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
	{
		GSList* node_collection;
		GSList* node_it;
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			node = node_it->data;
			{
#line 43 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (grava_graph_selected == NULL) {
#line 44 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					grava_graph_selected = node;
#line 45 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					break;
				}
#line 47 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (grava_graph_selected == node) {
#line 48 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					grava_graph_selected = NULL;
				}
			}
		}
	}
}


#line 52 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
gboolean grava_graph_is_selected (GravaNode* n) {
#line 52 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (n == NULL || GRAVA_IS_NODE (n), FALSE);
#line 54 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	return n == grava_graph_selected;
}


#line 57 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_update (GravaGraph* self) {
#line 57 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
#line 59 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	grava_layout_run (self->layout, self);
}


#line 62 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_add_node (GravaGraph* self, GravaNode* n) {
	GravaNode* _tmp0;
#line 62 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
#line 62 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (n == NULL || GRAVA_IS_NODE (n));
#line 64 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	grava_node_fit (n);
#line 65 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	_tmp0 = NULL;
#line 65 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	self->nodes = g_slist_append (self->nodes, (_tmp0 = n, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
}


#line 68 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_add_edge (GravaGraph* self, GravaEdge* e) {
	GravaEdge* _tmp0;
#line 68 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
#line 68 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (e == NULL || GRAVA_IS_EDGE (e));
#line 70 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	_tmp0 = NULL;
#line 70 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	self->edges = g_slist_append (self->edges, (_tmp0 = e, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
}


#line 73 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
GSList* grava_graph_unlinked_nodes (GravaGraph* self) {
	GSList* ret;
#line 73 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (GRAVA_IS_GRAPH (self), NULL);
#line 75 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	ret = NULL;
	{
		GSList* node_collection;
		GSList* node_it;
#line 76 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 76 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			node = node_it->data;
			{
				gboolean found;
#line 77 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				found = FALSE;
				{
					GSList* edge_collection;
					GSList* edge_it;
#line 78 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					edge_collection = self->edges;
					for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
						GravaEdge* edge;
#line 78 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
						edge = edge_it->data;
						{
#line 79 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
							if (node == edge->orig || node == edge->dest) {
#line 80 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
								found = TRUE;
#line 81 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
								break;
							}
						}
					}
				}
#line 84 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (!found) {
					GravaNode* _tmp0;
#line 85 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					_tmp0 = NULL;
#line 85 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					ret = g_slist_append (ret, (_tmp0 = node, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
				}
			}
		}
	}
#line 87 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	return ret;
}


#line 90 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
GSList* grava_graph_outer_nodes (GravaGraph* self, GravaNode* n) {
	GSList* ret;
#line 90 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (GRAVA_IS_GRAPH (self), NULL);
#line 90 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (n == NULL || GRAVA_IS_NODE (n), NULL);
#line 92 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 93 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 93 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			edge = edge_it->data;
			{
#line 94 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (edge->visible && (edge->orig == n)) {
					GravaNode* _tmp0;
#line 95 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					_tmp0 = NULL;
#line 95 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					ret = g_slist_append (ret, (_tmp0 = edge->dest, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
				}
			}
		}
	}
#line 97 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	return ret;
}


#line 100 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
GSList* grava_graph_inner_nodes (GravaGraph* self, GravaNode* n) {
	GSList* ret;
#line 100 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (GRAVA_IS_GRAPH (self), NULL);
#line 100 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (n == NULL || GRAVA_IS_NODE (n), NULL);
#line 102 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 103 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 103 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			edge = edge_it->data;
			{
#line 104 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (edge->visible && (edge->dest == n)) {
					GravaNode* _tmp0;
#line 105 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					_tmp0 = NULL;
#line 105 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					ret = g_slist_append (ret, (_tmp0 = edge->orig, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
				}
			}
		}
	}
#line 107 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	return ret;
}


#line 110 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
GravaNode* grava_graph_click (GravaGraph* self, double x, double y) {
	double z;
#line 110 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (GRAVA_IS_GRAPH (self), NULL);
#line 112 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	z = self->zoom;
	{
		GSList* node_collection;
		GSList* node_it;
#line 113 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 113 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			node = node_it->data;
			{
#line 114 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (x >= node->x * z && x <= node->x * z + node->w * z && y >= node->y * z && y <= node->y * z + node->h * z) {
#line 116 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					return node;
				}
			}
		}
	}
#line 118 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	return NULL;
}


#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
gboolean grava_graph_overlaps (GravaGraph* self, GravaNode* n) {
#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (GRAVA_IS_GRAPH (self), FALSE);
#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_val_if_fail (n == NULL || GRAVA_IS_NODE (n), FALSE);
	{
		GSList* node_collection;
		GSList* node_it;
#line 123 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 123 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			node = node_it->data;
			{
#line 124 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (node != n && grava_node_overlaps (node, n)) {
#line 125 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					return TRUE;
				}
			}
		}
	}
#line 127 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	return FALSE;
}


#line 130 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_draw (GravaGraph* self, cairo_t* ctx) {
#line 130 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
	/*ctx.set_operator (Cairo.Operator.SOURCE);
	 XXX THIS FLICKERS! MUST USE DOUBLE BUFFER*/
#line 134 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	cairo_set_source_rgba (ctx, 1, 1, 1, 1);
#line 135 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	cairo_translate (ctx, self->panx, self->pany);
#line 136 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	cairo_scale (ctx, self->zoom, self->zoom);
#line 137 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	cairo_rotate (ctx, self->angle);
#line 139 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	cairo_paint (ctx);
	{
		GSList* node_collection;
		GSList* node_it;
#line 140 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 140 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			node = node_it->data;
			{
#line 141 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (node->visible) {
#line 142 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					grava_renderer_draw_node (ctx, node);
				}
			}
		}
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 144 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 144 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
			edge = edge_it->data;
			{
#line 145 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
				if (edge->visible) {
#line 146 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
					grava_renderer_draw_edge (ctx, edge);
				}
			}
		}
	}
}


/* TODO: double buffering*/
#line 152 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_add (GravaGraph* self, GravaNode* n) {
	GravaNode* _tmp0;
#line 152 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
#line 152 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (n == NULL || GRAVA_IS_NODE (n));
#line 154 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	_tmp0 = NULL;
#line 154 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	self->nodes = g_slist_append (self->nodes, (_tmp0 = n, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
}


#line 157 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
void grava_graph_link (GravaGraph* self, GravaNode* n, GravaNode* n2) {
	GravaEdge* _tmp0;
#line 157 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (GRAVA_IS_GRAPH (self));
#line 157 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (n == NULL || GRAVA_IS_NODE (n));
#line 157 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	g_return_if_fail (n2 == NULL || GRAVA_IS_NODE (n2));
#line 159 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	_tmp0 = NULL;
#line 159 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
	self->edges = g_slist_append (self->edges, grava_edge_with ((_tmp0 = grava_edge_new ()), n, n2));
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
}


#line 22 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
GravaGraph* grava_graph_new (void) {
	GravaGraph * self;
	self = g_object_newv (GRAVA_TYPE_GRAPH, 0, NULL);
	return self;
}


/* constructor */
static GObject * grava_graph_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GravaGraphClass * klass;
	GObjectClass * parent_class;
	GravaGraph * self;
	klass = GRAVA_GRAPH_CLASS (g_type_class_peek (GRAVA_TYPE_GRAPH));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GRAVA_GRAPH (obj);
	{
		GSList* _tmp0;
		GSList* _tmp1;
		GravaLayout* _tmp2;
		_tmp0 = NULL;
#line 35 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		self->nodes = (_tmp0 = NULL, (self->nodes == NULL ? NULL : (self->nodes = (g_slist_foreach (self->nodes, ((GFunc) g_object_unref), NULL), g_slist_free (self->nodes), NULL))), _tmp0);
		_tmp1 = NULL;
#line 36 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		self->edges = (_tmp1 = NULL, (self->edges == NULL ? NULL : (self->edges = (g_slist_foreach (self->edges, ((GFunc) g_object_unref), NULL), g_slist_free (self->edges), NULL))), _tmp1);
		_tmp2 = NULL;
#line 37 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/graph.vala"
		self->layout = (_tmp2 = GRAVA_LAYOUT (grava_default_layout_new ()), (self->layout == NULL ? NULL : (self->layout = (g_object_unref (self->layout), NULL))), _tmp2);
	}
	return obj;
}


static void grava_graph_class_init (GravaGraphClass * klass) {
	grava_graph_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = grava_graph_constructor;
	G_OBJECT_CLASS (klass)->dispose = grava_graph_dispose;
}


static void grava_graph_init (GravaGraph * self) {
	self->zoom = 1;
	self->panx = 0;
	self->pany = 0;
	self->angle = 0;
}


static void grava_graph_dispose (GObject * obj) {
	GravaGraph * self;
	self = GRAVA_GRAPH (obj);
	(self->layout == NULL ? NULL : (self->layout = (g_object_unref (self->layout), NULL)));
	(self->nodes == NULL ? NULL : (self->nodes = (g_slist_foreach (self->nodes, ((GFunc) g_object_unref), NULL), g_slist_free (self->nodes), NULL)));
	(self->edges == NULL ? NULL : (self->edges = (g_slist_foreach (self->edges, ((GFunc) g_object_unref), NULL), g_slist_free (self->edges), NULL)));
	G_OBJECT_CLASS (grava_graph_parent_class)->dispose (obj);
}


GType grava_graph_get_type (void) {
	static GType grava_graph_type_id = 0;
	if (G_UNLIKELY (grava_graph_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaGraphClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_graph_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaGraph), 0, (GInstanceInitFunc) grava_graph_init };
		grava_graph_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaGraph", &g_define_type_info, 0);
	}
	return grava_graph_type_id;
}




