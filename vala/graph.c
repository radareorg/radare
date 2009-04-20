/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007, 2008  pancake <youterm.com>
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

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <cairo.h>


#define GRAVA_TYPE_GRAPH (grava_graph_get_type ())
#define GRAVA_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_GRAPH, GravaGraph))
#define GRAVA_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_GRAPH, GravaGraphClass))
#define GRAVA_IS_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_GRAPH))
#define GRAVA_IS_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_GRAPH))
#define GRAVA_GRAPH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_GRAPH, GravaGraphClass))

typedef struct _GravaGraph GravaGraph;
typedef struct _GravaGraphClass GravaGraphClass;
typedef struct _GravaGraphPrivate GravaGraphPrivate;

#define GRAVA_TYPE_LAYOUT (grava_layout_get_type ())
#define GRAVA_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_LAYOUT, GravaLayout))
#define GRAVA_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_LAYOUT, GravaLayoutClass))
#define GRAVA_IS_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_LAYOUT))
#define GRAVA_IS_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_LAYOUT))
#define GRAVA_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_LAYOUT, GravaLayoutClass))

typedef struct _GravaLayout GravaLayout;
typedef struct _GravaLayoutClass GravaLayoutClass;

#define GRAVA_TYPE_NODE (grava_node_get_type ())
#define GRAVA_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_NODE, GravaNode))
#define GRAVA_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_NODE, GravaNodeClass))
#define GRAVA_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_NODE))
#define GRAVA_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_NODE))
#define GRAVA_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_NODE, GravaNodeClass))

typedef struct _GravaNode GravaNode;
typedef struct _GravaNodeClass GravaNodeClass;

#define GRAVA_TYPE_EDGE (grava_edge_get_type ())
#define GRAVA_EDGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_EDGE, GravaEdge))
#define GRAVA_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_EDGE, GravaEdgeClass))
#define GRAVA_IS_EDGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_EDGE))
#define GRAVA_IS_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_EDGE))
#define GRAVA_EDGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_EDGE, GravaEdgeClass))

typedef struct _GravaEdge GravaEdge;
typedef struct _GravaEdgeClass GravaEdgeClass;
typedef struct _GravaEdgePrivate GravaEdgePrivate;
typedef struct _GravaNodePrivate GravaNodePrivate;

#define GRAVA_TYPE_DEFAULT_LAYOUT (grava_default_layout_get_type ())
#define GRAVA_DEFAULT_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayout))
#define GRAVA_DEFAULT_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayoutClass))
#define GRAVA_IS_DEFAULT_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_DEFAULT_LAYOUT))
#define GRAVA_IS_DEFAULT_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_DEFAULT_LAYOUT))
#define GRAVA_DEFAULT_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayoutClass))

typedef struct _GravaDefaultLayout GravaDefaultLayout;
typedef struct _GravaDefaultLayoutClass GravaDefaultLayoutClass;

struct _GravaGraph {
	GObject parent_instance;
	GravaGraphPrivate * priv;
	GravaLayout* layout;
	GSList* selhist;
	GSList* nodes;
	GSList* edges;
	GHashTable* data;
	double zoom;
	double panx;
	double pany;
	double angle;
};

struct _GravaGraphClass {
	GObjectClass parent_class;
};

struct _GravaEdge {
	GObject parent_instance;
	GravaEdgePrivate * priv;
	GHashTable* data;
	GravaNode* orig;
	GravaNode* dest;
	gboolean visible;
	gboolean jmpcnd;
};

struct _GravaEdgeClass {
	GObjectClass parent_class;
};

struct _GravaNode {
	GObject parent_instance;
	GravaNodePrivate * priv;
	GHashTable* data;
	guint baseaddr;
	GSList* calls;
	GSList* xrefs;
	gboolean visible;
	gboolean selected;
	gboolean has_body;
	gint shape;
	double x;
	double y;
	double w;
	double h;
};

struct _GravaNodeClass {
	GObjectClass parent_class;
};



GType grava_graph_get_type (void);
GType grava_layout_get_type (void);
GType grava_node_get_type (void);
GType grava_edge_get_type (void);
enum  {
	GRAVA_GRAPH_DUMMY_PROPERTY
};
static void _g_slist_free_g_object_unref (GSList* self);
extern GravaNode* grava_graph_selected;
GravaNode* grava_graph_selected = NULL;
void grava_graph_undo_select (GravaGraph* self);
void grava_layout_reset (GravaLayout* self);
void grava_graph_reset (GravaGraph* self);
void grava_graph_set (GravaGraph* self, const char* key, const char* val);
char* grava_graph_get (GravaGraph* self, const char* key);
void grava_graph_select_next (GravaGraph* self);
char* grava_edge_get (GravaEdge* self, const char* val);
void grava_graph_select_true (GravaGraph* self);
void grava_graph_select_false (GravaGraph* self);
gboolean grava_graph_is_selected (GravaNode* n);
void grava_layout_run (GravaLayout* self, GravaGraph* graph);
void grava_graph_update (GravaGraph* self);
void grava_layout_set_graph (GravaLayout* self, GravaGraph* graph);
void grava_node_fit (GravaNode* self);
void grava_graph_add_node (GravaGraph* self, GravaNode* n);
void grava_graph_add_edge (GravaGraph* self, GravaEdge* e);
GSList* grava_graph_unlinked_nodes (GravaGraph* self);
GSList* grava_graph_outer_nodes (GravaGraph* self, GravaNode* n);
GSList* grava_graph_inner_nodes (GravaGraph* self, GravaNode* n);
GravaNode* grava_graph_click (GravaGraph* self, double x, double y);
gboolean grava_node_overlaps (GravaNode* self, GravaNode* n);
gboolean grava_graph_overlaps (GravaGraph* self, GravaNode* n);
void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge);
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node);
void grava_graph_draw (GravaGraph* self, cairo_t* ctx);
void grava_graph_add (GravaGraph* self, GravaNode* n);
GravaEdge* grava_edge_new (void);
GravaEdge* grava_edge_construct (GType object_type);
GravaEdge* grava_edge_with (GravaEdge* self, GravaNode* a, GravaNode* b);
void grava_graph_link (GravaGraph* self, GravaNode* n, GravaNode* n2);
GravaGraph* grava_graph_new (void);
GravaGraph* grava_graph_construct (GType object_type);
GravaGraph* grava_graph_new (void);
GravaDefaultLayout* grava_default_layout_new (void);
GravaDefaultLayout* grava_default_layout_construct (GType object_type);
GType grava_default_layout_get_type (void);
static void _g_object_unref_gdestroy_notify (void* data);
static GObject * grava_graph_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_graph_parent_class = NULL;
static void grava_graph_finalize (GObject* obj);
static int _vala_strcmp0 (const char * str1, const char * str2);



static void _g_slist_free_g_object_unref (GSList* self) {
	g_slist_foreach (self, (GFunc) g_object_unref, NULL);
	g_slist_free (self);
}


/*s = new ImageSurface.from_png("/tmp/file.png");*/
#line 46 "graph.vala"
void grava_graph_undo_select (GravaGraph* self) {
#line 200 "graph.c"
	guint length;
#line 46 "graph.vala"
	g_return_if_fail (self != NULL);
#line 204 "graph.c"
	length = g_slist_length (self->selhist);
#line 49 "graph.vala"
	grava_graph_selected = (GravaNode*) g_slist_nth_data (self->selhist, length - 1);
#line 50 "graph.vala"
	self->selhist = g_slist_remove (self->selhist, grava_graph_selected);
#line 210 "graph.c"
}


/* TODO: Add boolean argument to reset layout too */
#line 54 "graph.vala"
void grava_graph_reset (GravaGraph* self) {
#line 217 "graph.c"
	GSList* _tmp0_;
	GSList* _tmp1_;
	GSList* _tmp2_;
#line 54 "graph.vala"
	g_return_if_fail (self != NULL);
#line 55 "graph.vala"
	grava_layout_reset (self->layout);
#line 225 "graph.c"
	_tmp0_ = NULL;
#line 56 "graph.vala"
	self->nodes = (_tmp0_ = NULL, (self->nodes == NULL) ? NULL : (self->nodes = (_g_slist_free_g_object_unref (self->nodes), NULL)), _tmp0_);
#line 229 "graph.c"
	_tmp1_ = NULL;
#line 57 "graph.vala"
	self->edges = (_tmp1_ = NULL, (self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL)), _tmp1_);
#line 233 "graph.c"
	_tmp2_ = NULL;
#line 58 "graph.vala"
	self->selhist = (_tmp2_ = NULL, (self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL)), _tmp2_);
#line 237 "graph.c"
}


/*layout = new DefaultLayout();
 add png here */
#line 63 "graph.vala"
void grava_graph_set (GravaGraph* self, const char* key, const char* val) {
#line 245 "graph.c"
	const char* _tmp1_;
	const char* _tmp0_;
#line 63 "graph.vala"
	g_return_if_fail (self != NULL);
#line 63 "graph.vala"
	g_return_if_fail (key != NULL);
#line 63 "graph.vala"
	g_return_if_fail (val != NULL);
#line 65 "graph.vala"
	_tmp1_ = NULL;
#line 65 "graph.vala"
	_tmp0_ = NULL;
#line 65 "graph.vala"
	g_hash_table_insert (self->data, (_tmp0_ = key, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)), (_tmp1_ = val, (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_)));
#line 260 "graph.c"
}


#line 68 "graph.vala"
char* grava_graph_get (GravaGraph* self, const char* key) {
#line 266 "graph.c"
	const char* _tmp0_;
#line 68 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 68 "graph.vala"
	g_return_val_if_fail (key != NULL, NULL);
#line 70 "graph.vala"
	_tmp0_ = NULL;
#line 70 "graph.vala"
	return (_tmp0_ = (const char*) g_hash_table_lookup (self->data, key), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 276 "graph.c"
}


#line 73 "graph.vala"
void grava_graph_select_next (GravaGraph* self) {
#line 73 "graph.vala"
	g_return_if_fail (self != NULL);
#line 284 "graph.c"
	/*
	foreach(weak Node node in nodes) {
	if (Graph.selected == null) {
	selected = node;
	Graph.selected = node;
	selhist.append(selected);
	break;
	}
	if (selected == node) {
	Graph.selected = node;
	selected = null;
	}
	}
	
	if (Graph.selected == null || selected == null)*/
	{
		GSList* node_collection;
		GSList* node_it;
#line 90 "graph.vala"
		node_collection = self->nodes;
#line 305 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 90 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 310 "graph.c"
			{
#line 91 "graph.vala"
				grava_graph_selected = grava_graph_selected = node;
#line 92 "graph.vala"
				break;
#line 316 "graph.c"
			}
		}
	}
}


#line 96 "graph.vala"
void grava_graph_select_true (GravaGraph* self) {
#line 96 "graph.vala"
	g_return_if_fail (self != NULL);
#line 98 "graph.vala"
	if (grava_graph_selected == NULL) {
#line 99 "graph.vala"
		return;
#line 331 "graph.c"
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 101 "graph.vala"
		edge_collection = self->edges;
#line 338 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 101 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 343 "graph.c"
			{
#line 102 "graph.vala"
				if (grava_graph_selected == edge->orig) {
#line 347 "graph.c"
					char* _tmp0_;
					gboolean _tmp1_;
#line 35 "edge.vala"
					_tmp0_ = NULL;
#line 103 "graph.vala"
					if ((_tmp1_ = _vala_strcmp0 (_tmp0_ = grava_edge_get (edge, "color"), "green") == 0, _tmp0_ = (g_free (_tmp0_), NULL), _tmp1_)) {
#line 354 "graph.c"
						GravaNode* _tmp2_;
#line 104 "graph.vala"
						grava_graph_selected = edge->dest;
#line 105 "graph.vala"
						_tmp2_ = NULL;
#line 105 "graph.vala"
						self->selhist = g_slist_append (self->selhist, (_tmp2_ = grava_graph_selected, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)));
#line 106 "graph.vala"
						break;
#line 364 "graph.c"
					}
				}
			}
		}
	}
}


#line 112 "graph.vala"
void grava_graph_select_false (GravaGraph* self) {
#line 112 "graph.vala"
	g_return_if_fail (self != NULL);
#line 114 "graph.vala"
	if (grava_graph_selected == NULL) {
#line 115 "graph.vala"
		return;
#line 381 "graph.c"
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 117 "graph.vala"
		edge_collection = self->edges;
#line 388 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 117 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 393 "graph.c"
			{
#line 118 "graph.vala"
				if (grava_graph_selected == edge->orig) {
#line 397 "graph.c"
					char* _tmp0_;
					gboolean _tmp1_;
#line 35 "edge.vala"
					_tmp0_ = NULL;
#line 119 "graph.vala"
					if ((_tmp1_ = _vala_strcmp0 (_tmp0_ = grava_edge_get (edge, "color"), "red") == 0, _tmp0_ = (g_free (_tmp0_), NULL), _tmp1_)) {
#line 404 "graph.c"
						GravaNode* _tmp2_;
#line 120 "graph.vala"
						grava_graph_selected = edge->dest;
#line 121 "graph.vala"
						_tmp2_ = NULL;
#line 121 "graph.vala"
						self->selhist = g_slist_append (self->selhist, (_tmp2_ = grava_graph_selected, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)));
#line 122 "graph.vala"
						break;
#line 414 "graph.c"
					}
				}
			}
		}
	}
}


#line 128 "graph.vala"
gboolean grava_graph_is_selected (GravaNode* n) {
#line 128 "graph.vala"
	g_return_val_if_fail (n != NULL, FALSE);
#line 130 "graph.vala"
	return n == grava_graph_selected;
#line 429 "graph.c"
}


#line 133 "graph.vala"
void grava_graph_update (GravaGraph* self) {
#line 133 "graph.vala"
	g_return_if_fail (self != NULL);
#line 135 "graph.vala"
	grava_layout_run (self->layout, self);
#line 439 "graph.c"
}


/* esteve modificat perque insereixi a la llista ordenat per baseaddr.
 volia fer servir node.sort, pero no m'ha sortit...*/
#line 140 "graph.vala"
void grava_graph_add_node (GravaGraph* self, GravaNode* n) {
#line 447 "graph.c"
	gint count = {0};
	GravaNode* p;
	gboolean ins = {0};
	guint len;
#line 140 "graph.vala"
	g_return_if_fail (self != NULL);
#line 140 "graph.vala"
	g_return_if_fail (n != NULL);
#line 456 "graph.c"
	p = NULL;
	len = g_slist_length (self->nodes);
#line 147 "graph.vala"
	grava_layout_set_graph (self->layout, self);
#line 148 "graph.vala"
	grava_node_fit (n);
#line 150 "graph.vala"
	ins = FALSE;
#line 151 "graph.vala"
	for (count = 0; count < len; count++) {
#line 467 "graph.c"
		GravaNode* _tmp1_;
		GravaNode* _tmp0_;
		_tmp1_ = NULL;
#line 152 "graph.vala"
		_tmp0_ = NULL;
#line 152 "graph.vala"
		p = (_tmp1_ = (_tmp0_ = (GravaNode*) g_slist_nth_data (self->nodes, (guint) count), (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)), (p == NULL) ? NULL : (p = (g_object_unref (p), NULL)), _tmp1_);
#line 475 "graph.c"
		/*stdout.printf ("adding node base at %x , this is %x\n", p.baseaddr, n.baseaddr );*/
#line 155 "graph.vala"
		if (p->baseaddr >= n->baseaddr) {
#line 479 "graph.c"
			GravaNode* _tmp2_;
#line 156 "graph.vala"
			ins = TRUE;
#line 157 "graph.vala"
			_tmp2_ = NULL;
#line 157 "graph.vala"
			self->nodes = g_slist_insert (self->nodes, (_tmp2_ = n, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)), count);
#line 158 "graph.vala"
			break;
#line 489 "graph.c"
		}
	}
#line 162 "graph.vala"
	if (ins == FALSE) {
#line 494 "graph.c"
		GravaNode* _tmp3_;
#line 163 "graph.vala"
		_tmp3_ = NULL;
#line 163 "graph.vala"
		self->nodes = g_slist_append (self->nodes, (_tmp3_ = n, (_tmp3_ == NULL) ? NULL : g_object_ref (_tmp3_)));
#line 500 "graph.c"
	}
	(p == NULL) ? NULL : (p = (g_object_unref (p), NULL));
}


#line 166 "graph.vala"
void grava_graph_add_edge (GravaGraph* self, GravaEdge* e) {
#line 508 "graph.c"
	GravaEdge* _tmp0_;
#line 166 "graph.vala"
	g_return_if_fail (self != NULL);
#line 166 "graph.vala"
	g_return_if_fail (e != NULL);
#line 168 "graph.vala"
	_tmp0_ = NULL;
#line 168 "graph.vala"
	self->edges = g_slist_append (self->edges, (_tmp0_ = e, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)));
#line 518 "graph.c"
}


#line 171 "graph.vala"
GSList* grava_graph_unlinked_nodes (GravaGraph* self) {
#line 524 "graph.c"
	GSList* ret;
#line 171 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 528 "graph.c"
	ret = NULL;
	{
		GSList* node_collection;
		GSList* node_it;
#line 174 "graph.vala"
		node_collection = self->nodes;
#line 535 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 174 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 540 "graph.c"
			{
				gboolean found;
				found = FALSE;
				{
					GSList* edge_collection;
					GSList* edge_it;
#line 176 "graph.vala"
					edge_collection = self->edges;
#line 549 "graph.c"
					for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
						GravaEdge* edge;
#line 176 "graph.vala"
						edge = (GravaEdge*) edge_it->data;
#line 554 "graph.c"
						{
							gboolean _tmp0_ = {0};
#line 177 "graph.vala"
							if (node == edge->orig) {
#line 177 "graph.vala"
								_tmp0_ = TRUE;
#line 561 "graph.c"
							} else {
#line 177 "graph.vala"
								_tmp0_ = node == edge->dest;
#line 565 "graph.c"
							}
#line 177 "graph.vala"
							if (_tmp0_) {
#line 178 "graph.vala"
								found = TRUE;
#line 179 "graph.vala"
								break;
#line 573 "graph.c"
							}
						}
					}
				}
#line 182 "graph.vala"
				if (!found) {
#line 580 "graph.c"
					GravaNode* _tmp1_;
#line 183 "graph.vala"
					_tmp1_ = NULL;
#line 183 "graph.vala"
					ret = g_slist_append (ret, (_tmp1_ = node, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)));
#line 586 "graph.c"
				}
			}
		}
	}
#line 185 "graph.vala"
	return ret;
#line 593 "graph.c"
}


#line 188 "graph.vala"
GSList* grava_graph_outer_nodes (GravaGraph* self, GravaNode* n) {
#line 599 "graph.c"
	GSList* ret;
#line 188 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 188 "graph.vala"
	g_return_val_if_fail (n != NULL, NULL);
#line 605 "graph.c"
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 191 "graph.vala"
		edge_collection = self->edges;
#line 612 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 191 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 617 "graph.c"
			{
				gboolean _tmp0_ = {0};
#line 192 "graph.vala"
				if (edge->visible) {
#line 192 "graph.vala"
					_tmp0_ = edge->orig == n;
#line 624 "graph.c"
				} else {
#line 192 "graph.vala"
					_tmp0_ = FALSE;
#line 628 "graph.c"
				}
#line 192 "graph.vala"
				if (_tmp0_) {
#line 632 "graph.c"
					GravaNode* _tmp1_;
#line 193 "graph.vala"
					_tmp1_ = NULL;
#line 193 "graph.vala"
					ret = g_slist_append (ret, (_tmp1_ = edge->dest, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)));
#line 638 "graph.c"
				}
			}
		}
	}
#line 195 "graph.vala"
	return ret;
#line 645 "graph.c"
}


#line 198 "graph.vala"
GSList* grava_graph_inner_nodes (GravaGraph* self, GravaNode* n) {
#line 651 "graph.c"
	GSList* ret;
#line 198 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 198 "graph.vala"
	g_return_val_if_fail (n != NULL, NULL);
#line 657 "graph.c"
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 201 "graph.vala"
		edge_collection = self->edges;
#line 664 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 201 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 669 "graph.c"
			{
				gboolean _tmp0_ = {0};
#line 202 "graph.vala"
				if (edge->visible) {
#line 202 "graph.vala"
					_tmp0_ = edge->dest == n;
#line 676 "graph.c"
				} else {
#line 202 "graph.vala"
					_tmp0_ = FALSE;
#line 680 "graph.c"
				}
#line 202 "graph.vala"
				if (_tmp0_) {
#line 684 "graph.c"
					GravaNode* _tmp1_;
#line 203 "graph.vala"
					_tmp1_ = NULL;
#line 203 "graph.vala"
					ret = g_slist_append (ret, (_tmp1_ = edge->orig, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)));
#line 690 "graph.c"
				}
			}
		}
	}
#line 205 "graph.vala"
	return ret;
#line 697 "graph.c"
}


#line 208 "graph.vala"
GravaNode* grava_graph_click (GravaGraph* self, double x, double y) {
#line 703 "graph.c"
	double z;
#line 208 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 707 "graph.c"
	z = self->zoom;
	{
		GSList* node_collection;
		GSList* node_it;
#line 211 "graph.vala"
		node_collection = self->nodes;
#line 714 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 211 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 719 "graph.c"
			{
				gboolean _tmp0_ = {0};
				gboolean _tmp1_ = {0};
				gboolean _tmp2_ = {0};
#line 212 "graph.vala"
				if (x >= (node->x * z)) {
#line 212 "graph.vala"
					_tmp2_ = x <= ((node->x * z) + (node->w * z));
#line 728 "graph.c"
				} else {
#line 212 "graph.vala"
					_tmp2_ = FALSE;
#line 732 "graph.c"
				}
#line 212 "graph.vala"
				if (_tmp2_) {
#line 213 "graph.vala"
					_tmp1_ = y >= (node->y * z);
#line 738 "graph.c"
				} else {
#line 212 "graph.vala"
					_tmp1_ = FALSE;
#line 742 "graph.c"
				}
#line 212 "graph.vala"
				if (_tmp1_) {
#line 213 "graph.vala"
					_tmp0_ = y <= ((node->y * z) + (node->h * z));
#line 748 "graph.c"
				} else {
#line 212 "graph.vala"
					_tmp0_ = FALSE;
#line 752 "graph.c"
				}
#line 212 "graph.vala"
				if (_tmp0_) {
#line 214 "graph.vala"
					return node;
#line 758 "graph.c"
				}
			}
		}
	}
#line 216 "graph.vala"
	return NULL;
#line 765 "graph.c"
}


#line 219 "graph.vala"
gboolean grava_graph_overlaps (GravaGraph* self, GravaNode* n) {
#line 219 "graph.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 219 "graph.vala"
	g_return_val_if_fail (n != NULL, FALSE);
#line 775 "graph.c"
	{
		GSList* node_collection;
		GSList* node_it;
#line 221 "graph.vala"
		node_collection = self->nodes;
#line 781 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 221 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 786 "graph.c"
			{
				gboolean _tmp0_ = {0};
#line 222 "graph.vala"
				if (node != n) {
#line 222 "graph.vala"
					_tmp0_ = grava_node_overlaps (node, n);
#line 793 "graph.c"
				} else {
#line 222 "graph.vala"
					_tmp0_ = FALSE;
#line 797 "graph.c"
				}
#line 222 "graph.vala"
				if (_tmp0_) {
#line 223 "graph.vala"
					return TRUE;
#line 803 "graph.c"
				}
			}
		}
	}
#line 225 "graph.vala"
	return FALSE;
#line 810 "graph.c"
}


#line 228 "graph.vala"
void grava_graph_draw (GravaGraph* self, cairo_t* ctx) {
#line 228 "graph.vala"
	g_return_if_fail (self != NULL);
#line 228 "graph.vala"
	g_return_if_fail (ctx != NULL);
#line 820 "graph.c"
	/*ctx.set_operator (Cairo.Operator.SOURCE);
	 XXX THIS FLICKERS! MUST USE DOUBLE BUFFER*/
#line 232 "graph.vala"
	if (ctx == NULL) {
#line 233 "graph.vala"
		return;
#line 827 "graph.c"
	}
#line 235 "graph.vala"
	cairo_set_source_rgba (ctx, (double) 1, (double) 1, (double) 1, (double) 1);
#line 236 "graph.vala"
	cairo_translate (ctx, self->panx, self->pany);
#line 237 "graph.vala"
	cairo_scale (ctx, self->zoom, self->zoom);
#line 238 "graph.vala"
	cairo_rotate (ctx, self->angle);
#line 837 "graph.c"
	/* blank screen */
#line 241 "graph.vala"
	cairo_paint (ctx);
#line 841 "graph.c"
	/* draw bg picture
	ctx.save();
	ctx.set_source_surface(s, panx, pany);
	ctx.paint();
	ctx.restore ();
	*/
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 250 "graph.vala"
		edge_collection = self->edges;
#line 853 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 250 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 858 "graph.c"
			{
#line 251 "graph.vala"
				if (edge->visible) {
#line 252 "graph.vala"
					grava_renderer_draw_edge (ctx, edge);
#line 864 "graph.c"
				}
			}
		}
	}
	{
		GSList* node_collection;
		GSList* node_it;
#line 254 "graph.vala"
		node_collection = self->nodes;
#line 874 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 254 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 879 "graph.c"
			{
#line 255 "graph.vala"
				if (node->visible) {
#line 256 "graph.vala"
					grava_renderer_draw_node (ctx, node);
#line 885 "graph.c"
				}
			}
		}
	}
}


/* TODO: double buffering*/
#line 262 "graph.vala"
void grava_graph_add (GravaGraph* self, GravaNode* n) {
#line 896 "graph.c"
	GravaNode* _tmp0_;
#line 262 "graph.vala"
	g_return_if_fail (self != NULL);
#line 262 "graph.vala"
	g_return_if_fail (n != NULL);
#line 264 "graph.vala"
	_tmp0_ = NULL;
#line 264 "graph.vala"
	self->nodes = g_slist_append (self->nodes, (_tmp0_ = n, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)));
#line 906 "graph.c"
}


#line 267 "graph.vala"
void grava_graph_link (GravaGraph* self, GravaNode* n, GravaNode* n2) {
#line 912 "graph.c"
	GravaEdge* _tmp0_;
#line 267 "graph.vala"
	g_return_if_fail (self != NULL);
#line 267 "graph.vala"
	g_return_if_fail (n != NULL);
#line 267 "graph.vala"
	g_return_if_fail (n2 != NULL);
#line 920 "graph.c"
	_tmp0_ = NULL;
#line 269 "graph.vala"
	self->edges = g_slist_append (self->edges, grava_edge_with (_tmp0_ = grava_edge_new (), n, n2));
#line 924 "graph.c"
	(_tmp0_ == NULL) ? NULL : (_tmp0_ = (g_object_unref (_tmp0_), NULL));
}


#line 22 "graph.vala"
GravaGraph* grava_graph_construct (GType object_type) {
#line 931 "graph.c"
	GravaGraph * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 22 "graph.vala"
GravaGraph* grava_graph_new (void) {
#line 22 "graph.vala"
	return grava_graph_construct (GRAVA_TYPE_GRAPH);
#line 942 "graph.c"
}


#line 182 "gobject-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
#line 948 "graph.c"
	g_object_unref (data);
}


/*ImageSurface s;
 constructor */
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
		GSList* _tmp0_;
		GSList* _tmp1_;
		GSList* _tmp2_;
		GravaLayout* _tmp3_;
		GHashTable* _tmp4_;
		_tmp0_ = NULL;
#line 38 "graph.vala"
		self->nodes = (_tmp0_ = NULL, (self->nodes == NULL) ? NULL : (self->nodes = (_g_slist_free_g_object_unref (self->nodes), NULL)), _tmp0_);
#line 973 "graph.c"
		_tmp1_ = NULL;
#line 39 "graph.vala"
		self->edges = (_tmp1_ = NULL, (self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL)), _tmp1_);
#line 977 "graph.c"
		_tmp2_ = NULL;
#line 40 "graph.vala"
		self->selhist = (_tmp2_ = NULL, (self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL)), _tmp2_);
#line 981 "graph.c"
		_tmp3_ = NULL;
#line 41 "graph.vala"
		self->layout = (_tmp3_ = (GravaLayout*) grava_default_layout_new (), (self->layout == NULL) ? NULL : (self->layout = (g_object_unref (self->layout), NULL)), _tmp3_);
#line 985 "graph.c"
		_tmp4_ = NULL;
#line 42 "graph.vala"
		self->data = (_tmp4_ = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)), _tmp4_);
#line 989 "graph.c"
	}
	return obj;
}


static void grava_graph_class_init (GravaGraphClass * klass) {
	grava_graph_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = grava_graph_constructor;
	G_OBJECT_CLASS (klass)->finalize = grava_graph_finalize;
}


static void grava_graph_instance_init (GravaGraph * self) {
	self->zoom = (double) 1;
	self->panx = (double) 0;
	self->pany = (double) 0;
	self->angle = (double) 0;
}


static void grava_graph_finalize (GObject* obj) {
	GravaGraph * self;
	self = GRAVA_GRAPH (obj);
	(self->layout == NULL) ? NULL : (self->layout = (g_object_unref (self->layout), NULL));
	(self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL));
	(self->nodes == NULL) ? NULL : (self->nodes = (_g_slist_free_g_object_unref (self->nodes), NULL));
	(self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL));
	(self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL));
	G_OBJECT_CLASS (grava_graph_parent_class)->finalize (obj);
}


GType grava_graph_get_type (void) {
	static GType grava_graph_type_id = 0;
	if (grava_graph_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaGraphClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_graph_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaGraph), 0, (GInstanceInitFunc) grava_graph_instance_init, NULL };
		grava_graph_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaGraph", &g_define_type_info, 0);
	}
	return grava_graph_type_id;
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return str1 != str2;
	}
	return strcmp (str1, str2);
}




