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
void grava_graph_do_zoom (GravaGraph* self, double z);
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
void grava_graph_do_zoom (GravaGraph* self, double z) {
#line 46 "graph.vala"
	g_return_if_fail (self != NULL);
#line 48 "graph.vala"
	self->zoom = self->zoom + z;
#line 49 "graph.vala"
	if (z > 0) {
#line 50 "graph.vala"
		self->panx = self->panx + (self->panx * z);
#line 51 "graph.vala"
		self->pany = self->pany + (self->pany * z);
#line 211 "graph.c"
	} else {
#line 53 "graph.vala"
		self->panx = self->panx + (self->panx * z);
#line 54 "graph.vala"
		self->pany = self->pany + (self->pany * z);
#line 217 "graph.c"
	}
}


#line 58 "graph.vala"
void grava_graph_undo_select (GravaGraph* self) {
#line 224 "graph.c"
	guint length;
#line 58 "graph.vala"
	g_return_if_fail (self != NULL);
#line 228 "graph.c"
	length = g_slist_length (self->selhist);
#line 61 "graph.vala"
	grava_graph_selected = (GravaNode*) g_slist_nth_data (self->selhist, length - 1);
#line 62 "graph.vala"
	self->selhist = g_slist_remove (self->selhist, grava_graph_selected);
#line 234 "graph.c"
}


/* TODO: Add boolean argument to reset layout too */
#line 66 "graph.vala"
void grava_graph_reset (GravaGraph* self) {
#line 241 "graph.c"
	GSList* _tmp0_;
	GSList* _tmp1_;
	GSList* _tmp2_;
#line 66 "graph.vala"
	g_return_if_fail (self != NULL);
#line 67 "graph.vala"
	grava_layout_reset (self->layout);
#line 249 "graph.c"
	_tmp0_ = NULL;
#line 68 "graph.vala"
	self->nodes = (_tmp0_ = NULL, (self->nodes == NULL) ? NULL : (self->nodes = (_g_slist_free_g_object_unref (self->nodes), NULL)), _tmp0_);
#line 253 "graph.c"
	_tmp1_ = NULL;
#line 69 "graph.vala"
	self->edges = (_tmp1_ = NULL, (self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL)), _tmp1_);
#line 257 "graph.c"
	_tmp2_ = NULL;
#line 70 "graph.vala"
	self->selhist = (_tmp2_ = NULL, (self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL)), _tmp2_);
#line 261 "graph.c"
}


/*layout = new DefaultLayout();
 add png here */
#line 75 "graph.vala"
void grava_graph_set (GravaGraph* self, const char* key, const char* val) {
#line 269 "graph.c"
	const char* _tmp1_;
	const char* _tmp0_;
#line 75 "graph.vala"
	g_return_if_fail (self != NULL);
#line 75 "graph.vala"
	g_return_if_fail (key != NULL);
#line 75 "graph.vala"
	g_return_if_fail (val != NULL);
#line 77 "graph.vala"
	_tmp1_ = NULL;
#line 77 "graph.vala"
	_tmp0_ = NULL;
#line 77 "graph.vala"
	g_hash_table_insert (self->data, (_tmp0_ = key, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)), (_tmp1_ = val, (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_)));
#line 284 "graph.c"
}


#line 80 "graph.vala"
char* grava_graph_get (GravaGraph* self, const char* key) {
#line 290 "graph.c"
	const char* _tmp0_;
#line 80 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 80 "graph.vala"
	g_return_val_if_fail (key != NULL, NULL);
#line 82 "graph.vala"
	_tmp0_ = NULL;
#line 82 "graph.vala"
	return (_tmp0_ = (const char*) g_hash_table_lookup (self->data, key), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 300 "graph.c"
}


#line 85 "graph.vala"
void grava_graph_select_next (GravaGraph* self) {
#line 85 "graph.vala"
	g_return_if_fail (self != NULL);
#line 308 "graph.c"
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
#line 102 "graph.vala"
		node_collection = self->nodes;
#line 329 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 102 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 334 "graph.c"
			{
#line 103 "graph.vala"
				grava_graph_selected = grava_graph_selected = node;
#line 104 "graph.vala"
				break;
#line 340 "graph.c"
			}
		}
	}
}


#line 108 "graph.vala"
void grava_graph_select_true (GravaGraph* self) {
#line 108 "graph.vala"
	g_return_if_fail (self != NULL);
#line 110 "graph.vala"
	if (grava_graph_selected == NULL) {
#line 111 "graph.vala"
		return;
#line 355 "graph.c"
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 113 "graph.vala"
		edge_collection = self->edges;
#line 362 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 113 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 367 "graph.c"
			{
#line 114 "graph.vala"
				if (grava_graph_selected == edge->orig) {
#line 371 "graph.c"
					char* _tmp0_;
					gboolean _tmp1_;
#line 35 "edge.vala"
					_tmp0_ = NULL;
#line 115 "graph.vala"
					if ((_tmp1_ = _vala_strcmp0 (_tmp0_ = grava_edge_get (edge, "color"), "green") == 0, _tmp0_ = (g_free (_tmp0_), NULL), _tmp1_)) {
#line 378 "graph.c"
						GravaNode* _tmp2_;
#line 116 "graph.vala"
						grava_graph_selected = edge->dest;
#line 117 "graph.vala"
						_tmp2_ = NULL;
#line 117 "graph.vala"
						self->selhist = g_slist_append (self->selhist, (_tmp2_ = grava_graph_selected, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)));
#line 118 "graph.vala"
						break;
#line 388 "graph.c"
					}
				}
			}
		}
	}
}


#line 124 "graph.vala"
void grava_graph_select_false (GravaGraph* self) {
#line 124 "graph.vala"
	g_return_if_fail (self != NULL);
#line 126 "graph.vala"
	if (grava_graph_selected == NULL) {
#line 127 "graph.vala"
		return;
#line 405 "graph.c"
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 129 "graph.vala"
		edge_collection = self->edges;
#line 412 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 129 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 417 "graph.c"
			{
#line 130 "graph.vala"
				if (grava_graph_selected == edge->orig) {
#line 421 "graph.c"
					char* _tmp0_;
					gboolean _tmp1_;
#line 35 "edge.vala"
					_tmp0_ = NULL;
#line 131 "graph.vala"
					if ((_tmp1_ = _vala_strcmp0 (_tmp0_ = grava_edge_get (edge, "color"), "red") == 0, _tmp0_ = (g_free (_tmp0_), NULL), _tmp1_)) {
#line 428 "graph.c"
						GravaNode* _tmp2_;
#line 132 "graph.vala"
						grava_graph_selected = edge->dest;
#line 133 "graph.vala"
						_tmp2_ = NULL;
#line 133 "graph.vala"
						self->selhist = g_slist_append (self->selhist, (_tmp2_ = grava_graph_selected, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)));
#line 134 "graph.vala"
						break;
#line 438 "graph.c"
					}
				}
			}
		}
	}
}


#line 140 "graph.vala"
gboolean grava_graph_is_selected (GravaNode* n) {
#line 140 "graph.vala"
	g_return_val_if_fail (n != NULL, FALSE);
#line 142 "graph.vala"
	return n == grava_graph_selected;
#line 453 "graph.c"
}


#line 145 "graph.vala"
void grava_graph_update (GravaGraph* self) {
#line 145 "graph.vala"
	g_return_if_fail (self != NULL);
#line 147 "graph.vala"
	grava_layout_run (self->layout, self);
#line 463 "graph.c"
}


/* esteve modificat perque insereixi a la llista ordenat per baseaddr.
 volia fer servir node.sort, pero no m'ha sortit...*/
#line 152 "graph.vala"
void grava_graph_add_node (GravaGraph* self, GravaNode* n) {
#line 471 "graph.c"
	gint count = {0};
	GravaNode* p;
	gboolean ins = {0};
	guint len;
#line 152 "graph.vala"
	g_return_if_fail (self != NULL);
#line 152 "graph.vala"
	g_return_if_fail (n != NULL);
#line 480 "graph.c"
	p = NULL;
	len = g_slist_length (self->nodes);
#line 159 "graph.vala"
	grava_layout_set_graph (self->layout, self);
#line 160 "graph.vala"
	grava_node_fit (n);
#line 162 "graph.vala"
	ins = FALSE;
#line 163 "graph.vala"
	for (count = 0; count < len; count++) {
#line 491 "graph.c"
		GravaNode* _tmp1_;
		GravaNode* _tmp0_;
		_tmp1_ = NULL;
#line 164 "graph.vala"
		_tmp0_ = NULL;
#line 164 "graph.vala"
		p = (_tmp1_ = (_tmp0_ = (GravaNode*) g_slist_nth_data (self->nodes, (guint) count), (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)), (p == NULL) ? NULL : (p = (g_object_unref (p), NULL)), _tmp1_);
#line 499 "graph.c"
		/*stdout.printf ("adding node base at %x , this is %x\n", p.baseaddr, n.baseaddr );*/
#line 167 "graph.vala"
		if (p->baseaddr >= n->baseaddr) {
#line 503 "graph.c"
			GravaNode* _tmp2_;
#line 168 "graph.vala"
			ins = TRUE;
#line 169 "graph.vala"
			_tmp2_ = NULL;
#line 169 "graph.vala"
			self->nodes = g_slist_insert (self->nodes, (_tmp2_ = n, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)), count);
#line 170 "graph.vala"
			break;
#line 513 "graph.c"
		}
	}
#line 174 "graph.vala"
	if (ins == FALSE) {
#line 518 "graph.c"
		GravaNode* _tmp3_;
#line 175 "graph.vala"
		_tmp3_ = NULL;
#line 175 "graph.vala"
		self->nodes = g_slist_append (self->nodes, (_tmp3_ = n, (_tmp3_ == NULL) ? NULL : g_object_ref (_tmp3_)));
#line 524 "graph.c"
	}
	(p == NULL) ? NULL : (p = (g_object_unref (p), NULL));
}


#line 178 "graph.vala"
void grava_graph_add_edge (GravaGraph* self, GravaEdge* e) {
#line 532 "graph.c"
	GravaEdge* _tmp0_;
#line 178 "graph.vala"
	g_return_if_fail (self != NULL);
#line 178 "graph.vala"
	g_return_if_fail (e != NULL);
#line 180 "graph.vala"
	_tmp0_ = NULL;
#line 180 "graph.vala"
	self->edges = g_slist_append (self->edges, (_tmp0_ = e, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)));
#line 542 "graph.c"
}


#line 183 "graph.vala"
GSList* grava_graph_unlinked_nodes (GravaGraph* self) {
#line 548 "graph.c"
	GSList* ret;
#line 183 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 552 "graph.c"
	ret = NULL;
	{
		GSList* node_collection;
		GSList* node_it;
#line 186 "graph.vala"
		node_collection = self->nodes;
#line 559 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 186 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 564 "graph.c"
			{
				gboolean found;
				found = FALSE;
				{
					GSList* edge_collection;
					GSList* edge_it;
#line 188 "graph.vala"
					edge_collection = self->edges;
#line 573 "graph.c"
					for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
						GravaEdge* edge;
#line 188 "graph.vala"
						edge = (GravaEdge*) edge_it->data;
#line 578 "graph.c"
						{
							gboolean _tmp0_ = {0};
#line 189 "graph.vala"
							if (node == edge->orig) {
#line 189 "graph.vala"
								_tmp0_ = TRUE;
#line 585 "graph.c"
							} else {
#line 189 "graph.vala"
								_tmp0_ = node == edge->dest;
#line 589 "graph.c"
							}
#line 189 "graph.vala"
							if (_tmp0_) {
#line 190 "graph.vala"
								found = TRUE;
#line 191 "graph.vala"
								break;
#line 597 "graph.c"
							}
						}
					}
				}
#line 194 "graph.vala"
				if (!found) {
#line 604 "graph.c"
					GravaNode* _tmp1_;
#line 195 "graph.vala"
					_tmp1_ = NULL;
#line 195 "graph.vala"
					ret = g_slist_append (ret, (_tmp1_ = node, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)));
#line 610 "graph.c"
				}
			}
		}
	}
#line 197 "graph.vala"
	return ret;
#line 617 "graph.c"
}


#line 200 "graph.vala"
GSList* grava_graph_outer_nodes (GravaGraph* self, GravaNode* n) {
#line 623 "graph.c"
	GSList* ret;
#line 200 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 200 "graph.vala"
	g_return_val_if_fail (n != NULL, NULL);
#line 629 "graph.c"
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 203 "graph.vala"
		edge_collection = self->edges;
#line 636 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 203 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 641 "graph.c"
			{
				gboolean _tmp0_ = {0};
#line 204 "graph.vala"
				if (edge->visible) {
#line 204 "graph.vala"
					_tmp0_ = edge->orig == n;
#line 648 "graph.c"
				} else {
#line 204 "graph.vala"
					_tmp0_ = FALSE;
#line 652 "graph.c"
				}
#line 204 "graph.vala"
				if (_tmp0_) {
#line 656 "graph.c"
					GravaNode* _tmp1_;
#line 205 "graph.vala"
					_tmp1_ = NULL;
#line 205 "graph.vala"
					ret = g_slist_append (ret, (_tmp1_ = edge->dest, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)));
#line 662 "graph.c"
				}
			}
		}
	}
#line 207 "graph.vala"
	return ret;
#line 669 "graph.c"
}


#line 210 "graph.vala"
GSList* grava_graph_inner_nodes (GravaGraph* self, GravaNode* n) {
#line 675 "graph.c"
	GSList* ret;
#line 210 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 210 "graph.vala"
	g_return_val_if_fail (n != NULL, NULL);
#line 681 "graph.c"
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 213 "graph.vala"
		edge_collection = self->edges;
#line 688 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 213 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 693 "graph.c"
			{
				gboolean _tmp0_ = {0};
#line 214 "graph.vala"
				if (edge->visible) {
#line 214 "graph.vala"
					_tmp0_ = edge->dest == n;
#line 700 "graph.c"
				} else {
#line 214 "graph.vala"
					_tmp0_ = FALSE;
#line 704 "graph.c"
				}
#line 214 "graph.vala"
				if (_tmp0_) {
#line 708 "graph.c"
					GravaNode* _tmp1_;
#line 215 "graph.vala"
					_tmp1_ = NULL;
#line 215 "graph.vala"
					ret = g_slist_append (ret, (_tmp1_ = edge->orig, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)));
#line 714 "graph.c"
				}
			}
		}
	}
#line 217 "graph.vala"
	return ret;
#line 721 "graph.c"
}


#line 220 "graph.vala"
GravaNode* grava_graph_click (GravaGraph* self, double x, double y) {
#line 727 "graph.c"
	double z;
#line 220 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 731 "graph.c"
	z = self->zoom;
	{
		GSList* node_collection;
		GSList* node_it;
#line 223 "graph.vala"
		node_collection = self->nodes;
#line 738 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 223 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 743 "graph.c"
			{
				gboolean _tmp0_ = {0};
				gboolean _tmp1_ = {0};
				gboolean _tmp2_ = {0};
#line 224 "graph.vala"
				if (x >= (node->x * z)) {
#line 224 "graph.vala"
					_tmp2_ = x <= ((node->x * z) + (node->w * z));
#line 752 "graph.c"
				} else {
#line 224 "graph.vala"
					_tmp2_ = FALSE;
#line 756 "graph.c"
				}
#line 224 "graph.vala"
				if (_tmp2_) {
#line 225 "graph.vala"
					_tmp1_ = y >= (node->y * z);
#line 762 "graph.c"
				} else {
#line 224 "graph.vala"
					_tmp1_ = FALSE;
#line 766 "graph.c"
				}
#line 224 "graph.vala"
				if (_tmp1_) {
#line 225 "graph.vala"
					_tmp0_ = y <= ((node->y * z) + (node->h * z));
#line 772 "graph.c"
				} else {
#line 224 "graph.vala"
					_tmp0_ = FALSE;
#line 776 "graph.c"
				}
#line 224 "graph.vala"
				if (_tmp0_) {
#line 226 "graph.vala"
					return node;
#line 782 "graph.c"
				}
			}
		}
	}
#line 228 "graph.vala"
	return NULL;
#line 789 "graph.c"
}


#line 231 "graph.vala"
gboolean grava_graph_overlaps (GravaGraph* self, GravaNode* n) {
#line 231 "graph.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 231 "graph.vala"
	g_return_val_if_fail (n != NULL, FALSE);
#line 799 "graph.c"
	{
		GSList* node_collection;
		GSList* node_it;
#line 233 "graph.vala"
		node_collection = self->nodes;
#line 805 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 233 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 810 "graph.c"
			{
				gboolean _tmp0_ = {0};
#line 234 "graph.vala"
				if (node != n) {
#line 234 "graph.vala"
					_tmp0_ = grava_node_overlaps (node, n);
#line 817 "graph.c"
				} else {
#line 234 "graph.vala"
					_tmp0_ = FALSE;
#line 821 "graph.c"
				}
#line 234 "graph.vala"
				if (_tmp0_) {
#line 235 "graph.vala"
					return TRUE;
#line 827 "graph.c"
				}
			}
		}
	}
#line 237 "graph.vala"
	return FALSE;
#line 834 "graph.c"
}


#line 240 "graph.vala"
void grava_graph_draw (GravaGraph* self, cairo_t* ctx) {
#line 240 "graph.vala"
	g_return_if_fail (self != NULL);
#line 240 "graph.vala"
	g_return_if_fail (ctx != NULL);
#line 242 "graph.vala"
	if (ctx == NULL) {
#line 243 "graph.vala"
		return;
#line 848 "graph.c"
	}
#line 245 "graph.vala"
	cairo_set_source_rgba (ctx, (double) 1, (double) 1, (double) 1, (double) 1);
#line 246 "graph.vala"
	cairo_translate (ctx, self->panx, self->pany);
#line 247 "graph.vala"
	cairo_scale (ctx, self->zoom, self->zoom);
#line 856 "graph.c"
	/*ctx.translate( panx*zoom, pany*zoom);*/
#line 249 "graph.vala"
	cairo_rotate (ctx, self->angle);
#line 860 "graph.c"
	/* blank screen */
#line 252 "graph.vala"
	cairo_paint (ctx);
#line 864 "graph.c"
	/* draw bg picture
	ctx.save();
	ctx.set_source_surface(s, panx, pany);
	ctx.paint();
	ctx.restore ();
	*/
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 261 "graph.vala"
		edge_collection = self->edges;
#line 876 "graph.c"
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 261 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
#line 881 "graph.c"
			{
#line 262 "graph.vala"
				if (edge->visible) {
#line 263 "graph.vala"
					grava_renderer_draw_edge (ctx, edge);
#line 887 "graph.c"
				}
			}
		}
	}
	{
		GSList* node_collection;
		GSList* node_it;
#line 265 "graph.vala"
		node_collection = self->nodes;
#line 897 "graph.c"
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 265 "graph.vala"
			node = (GravaNode*) node_it->data;
#line 902 "graph.c"
			{
#line 266 "graph.vala"
				if (node->visible) {
#line 267 "graph.vala"
					grava_renderer_draw_node (ctx, node);
#line 908 "graph.c"
				}
			}
		}
	}
}


#line 271 "graph.vala"
void grava_graph_add (GravaGraph* self, GravaNode* n) {
#line 918 "graph.c"
	GravaNode* _tmp0_;
#line 271 "graph.vala"
	g_return_if_fail (self != NULL);
#line 271 "graph.vala"
	g_return_if_fail (n != NULL);
#line 273 "graph.vala"
	_tmp0_ = NULL;
#line 273 "graph.vala"
	self->nodes = g_slist_append (self->nodes, (_tmp0_ = n, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)));
#line 928 "graph.c"
}


#line 276 "graph.vala"
void grava_graph_link (GravaGraph* self, GravaNode* n, GravaNode* n2) {
#line 934 "graph.c"
	GravaEdge* _tmp0_;
#line 276 "graph.vala"
	g_return_if_fail (self != NULL);
#line 276 "graph.vala"
	g_return_if_fail (n != NULL);
#line 276 "graph.vala"
	g_return_if_fail (n2 != NULL);
#line 942 "graph.c"
	_tmp0_ = NULL;
#line 278 "graph.vala"
	self->edges = g_slist_append (self->edges, grava_edge_with (_tmp0_ = grava_edge_new (), n, n2));
#line 946 "graph.c"
	(_tmp0_ == NULL) ? NULL : (_tmp0_ = (g_object_unref (_tmp0_), NULL));
}


#line 22 "graph.vala"
GravaGraph* grava_graph_construct (GType object_type) {
#line 953 "graph.c"
	GravaGraph * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 22 "graph.vala"
GravaGraph* grava_graph_new (void) {
#line 22 "graph.vala"
	return grava_graph_construct (GRAVA_TYPE_GRAPH);
#line 964 "graph.c"
}


#line 182 "gobject-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
#line 970 "graph.c"
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
#line 995 "graph.c"
		_tmp1_ = NULL;
#line 39 "graph.vala"
		self->edges = (_tmp1_ = NULL, (self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL)), _tmp1_);
#line 999 "graph.c"
		_tmp2_ = NULL;
#line 40 "graph.vala"
		self->selhist = (_tmp2_ = NULL, (self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL)), _tmp2_);
#line 1003 "graph.c"
		_tmp3_ = NULL;
#line 41 "graph.vala"
		self->layout = (_tmp3_ = (GravaLayout*) grava_default_layout_new (), (self->layout == NULL) ? NULL : (self->layout = (g_object_unref (self->layout), NULL)), _tmp3_);
#line 1007 "graph.c"
		_tmp4_ = NULL;
#line 42 "graph.vala"
		self->data = (_tmp4_ = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)), _tmp4_);
#line 1011 "graph.c"
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




