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

#include "graph.h"
#include "renderer.h"
#include "default_layout.h"




enum  {
	GRAVA_GRAPH_DUMMY_PROPERTY
};
static void _g_slist_free_g_object_unref (GSList* self);
GravaNode* grava_graph_selected = NULL;
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
	guint length;
#line 46 "graph.vala"
	g_return_if_fail (self != NULL);
	length = g_slist_length (self->selhist);
#line 49 "graph.vala"
	grava_graph_selected = (GravaNode*) g_slist_nth_data (self->selhist, length - 1);
#line 50 "graph.vala"
	self->selhist = g_slist_remove (self->selhist, grava_graph_selected);
}


/* TODO: Add boolean argument to reset layout too */
#line 54 "graph.vala"
void grava_graph_reset (GravaGraph* self) {
	GSList* _tmp0;
	GSList* _tmp1;
	GSList* _tmp2;
#line 54 "graph.vala"
	g_return_if_fail (self != NULL);
#line 55 "graph.vala"
	grava_layout_reset (self->layout);
	_tmp0 = NULL;
#line 56 "graph.vala"
	self->nodes = (_tmp0 = NULL, (self->nodes == NULL) ? NULL : (self->nodes = (_g_slist_free_g_object_unref (self->nodes), NULL)), _tmp0);
	_tmp1 = NULL;
#line 57 "graph.vala"
	self->edges = (_tmp1 = NULL, (self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL)), _tmp1);
	_tmp2 = NULL;
#line 58 "graph.vala"
	self->selhist = (_tmp2 = NULL, (self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL)), _tmp2);
}


/*layout = new DefaultLayout();
 add png here */
#line 63 "graph.vala"
void grava_graph_set (GravaGraph* self, const char* key, const char* val) {
	const char* _tmp1;
	const char* _tmp0;
#line 63 "graph.vala"
	g_return_if_fail (self != NULL);
#line 63 "graph.vala"
	g_return_if_fail (key != NULL);
#line 63 "graph.vala"
	g_return_if_fail (val != NULL);
#line 65 "graph.vala"
	_tmp1 = NULL;
#line 65 "graph.vala"
	_tmp0 = NULL;
#line 65 "graph.vala"
	g_hash_table_insert (self->data, (_tmp0 = key, (_tmp0 == NULL) ? NULL : g_strdup (_tmp0)), (_tmp1 = val, (_tmp1 == NULL) ? NULL : g_strdup (_tmp1)));
}


#line 68 "graph.vala"
char* grava_graph_get (GravaGraph* self, const char* key) {
	const char* _tmp0;
#line 68 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 68 "graph.vala"
	g_return_val_if_fail (key != NULL, NULL);
#line 70 "graph.vala"
	_tmp0 = NULL;
#line 70 "graph.vala"
	return (_tmp0 = (const char*) g_hash_table_lookup (self->data, key), (_tmp0 == NULL) ? NULL : g_strdup (_tmp0));
}


#line 73 "graph.vala"
void grava_graph_select_next (GravaGraph* self) {
#line 73 "graph.vala"
	g_return_if_fail (self != NULL);
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
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 90 "graph.vala"
			node = (GravaNode*) node_it->data;
			{
#line 91 "graph.vala"
				grava_graph_selected = grava_graph_selected = node;
#line 92 "graph.vala"
				break;
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
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 101 "graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 101 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
			{
#line 102 "graph.vala"
				if (grava_graph_selected == edge->orig) {
					char* _tmp0;
					gboolean _tmp1;
#line 35 "edge.vala"
					_tmp0 = NULL;
#line 103 "graph.vala"
					if ((_tmp1 = _vala_strcmp0 (_tmp0 = grava_edge_get (edge, "color"), "green") == 0, _tmp0 = (g_free (_tmp0), NULL), _tmp1)) {
						GravaNode* _tmp2;
#line 104 "graph.vala"
						grava_graph_selected = edge->dest;
#line 105 "graph.vala"
						_tmp2 = NULL;
#line 105 "graph.vala"
						self->selhist = g_slist_append (self->selhist, (_tmp2 = grava_graph_selected, (_tmp2 == NULL) ? NULL : g_object_ref (_tmp2)));
#line 106 "graph.vala"
						break;
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
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 117 "graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 117 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
			{
#line 118 "graph.vala"
				if (grava_graph_selected == edge->orig) {
					char* _tmp0;
					gboolean _tmp1;
#line 35 "edge.vala"
					_tmp0 = NULL;
#line 119 "graph.vala"
					if ((_tmp1 = _vala_strcmp0 (_tmp0 = grava_edge_get (edge, "color"), "red") == 0, _tmp0 = (g_free (_tmp0), NULL), _tmp1)) {
						GravaNode* _tmp2;
#line 120 "graph.vala"
						grava_graph_selected = edge->dest;
#line 121 "graph.vala"
						_tmp2 = NULL;
#line 121 "graph.vala"
						self->selhist = g_slist_append (self->selhist, (_tmp2 = grava_graph_selected, (_tmp2 == NULL) ? NULL : g_object_ref (_tmp2)));
#line 122 "graph.vala"
						break;
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
}


#line 133 "graph.vala"
void grava_graph_update (GravaGraph* self) {
#line 133 "graph.vala"
	g_return_if_fail (self != NULL);
#line 135 "graph.vala"
	grava_layout_run (self->layout, self);
}


/* esteve modificat perque insereixi a la llista ordenat per baseaddr.
 volia fer servir node.sort, pero no m'ha sortit...*/
#line 140 "graph.vala"
void grava_graph_add_node (GravaGraph* self, GravaNode* n) {
	gint count;
	GravaNode* p;
	gboolean ins;
	guint len;
#line 140 "graph.vala"
	g_return_if_fail (self != NULL);
#line 140 "graph.vala"
	g_return_if_fail (n != NULL);
	count = 0;
	p = NULL;
	ins = FALSE;
	len = g_slist_length (self->nodes);
#line 147 "graph.vala"
	grava_layout_set_graph (self->layout, self);
#line 148 "graph.vala"
	grava_node_fit (n);
#line 150 "graph.vala"
	ins = FALSE;
#line 151 "graph.vala"
	for (count = 0; count < len; count++) {
		GravaNode* _tmp1;
		GravaNode* _tmp0;
		_tmp1 = NULL;
#line 152 "graph.vala"
		_tmp0 = NULL;
#line 152 "graph.vala"
		p = (_tmp1 = (_tmp0 = (GravaNode*) g_slist_nth_data (self->nodes, (guint) count), (_tmp0 == NULL) ? NULL : g_object_ref (_tmp0)), (p == NULL) ? NULL : (p = (g_object_unref (p), NULL)), _tmp1);
		/*stdout.printf ("adding node base at %x , this is %x\n", p.baseaddr, n.baseaddr );*/
#line 155 "graph.vala"
		if (p->baseaddr >= n->baseaddr) {
			GravaNode* _tmp2;
#line 156 "graph.vala"
			ins = TRUE;
#line 157 "graph.vala"
			_tmp2 = NULL;
#line 157 "graph.vala"
			self->nodes = g_slist_insert (self->nodes, (_tmp2 = n, (_tmp2 == NULL) ? NULL : g_object_ref (_tmp2)), count);
#line 158 "graph.vala"
			break;
		}
	}
#line 162 "graph.vala"
	if (ins == FALSE) {
		GravaNode* _tmp3;
#line 163 "graph.vala"
		_tmp3 = NULL;
#line 163 "graph.vala"
		self->nodes = g_slist_append (self->nodes, (_tmp3 = n, (_tmp3 == NULL) ? NULL : g_object_ref (_tmp3)));
	}
	(p == NULL) ? NULL : (p = (g_object_unref (p), NULL));
}


#line 166 "graph.vala"
void grava_graph_add_edge (GravaGraph* self, GravaEdge* e) {
	GravaEdge* _tmp0;
#line 166 "graph.vala"
	g_return_if_fail (self != NULL);
#line 166 "graph.vala"
	g_return_if_fail (e != NULL);
#line 168 "graph.vala"
	_tmp0 = NULL;
#line 168 "graph.vala"
	self->edges = g_slist_append (self->edges, (_tmp0 = e, (_tmp0 == NULL) ? NULL : g_object_ref (_tmp0)));
}


#line 171 "graph.vala"
GSList* grava_graph_unlinked_nodes (GravaGraph* self) {
	GSList* ret;
#line 171 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
	ret = NULL;
	{
		GSList* node_collection;
		GSList* node_it;
#line 174 "graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 174 "graph.vala"
			node = (GravaNode*) node_it->data;
			{
				gboolean found;
				found = FALSE;
				{
					GSList* edge_collection;
					GSList* edge_it;
#line 176 "graph.vala"
					edge_collection = self->edges;
					for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
						GravaEdge* edge;
#line 176 "graph.vala"
						edge = (GravaEdge*) edge_it->data;
						{
							gboolean _tmp0;
							_tmp0 = FALSE;
#line 177 "graph.vala"
							if (node == edge->orig) {
#line 177 "graph.vala"
								_tmp0 = TRUE;
							} else {
#line 177 "graph.vala"
								_tmp0 = node == edge->dest;
							}
#line 177 "graph.vala"
							if (_tmp0) {
#line 178 "graph.vala"
								found = TRUE;
#line 179 "graph.vala"
								break;
							}
						}
					}
				}
#line 182 "graph.vala"
				if (!found) {
					GravaNode* _tmp1;
#line 183 "graph.vala"
					_tmp1 = NULL;
#line 183 "graph.vala"
					ret = g_slist_append (ret, (_tmp1 = node, (_tmp1 == NULL) ? NULL : g_object_ref (_tmp1)));
				}
			}
		}
	}
#line 185 "graph.vala"
	return ret;
}


#line 188 "graph.vala"
GSList* grava_graph_outer_nodes (GravaGraph* self, GravaNode* n) {
	GSList* ret;
#line 188 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 188 "graph.vala"
	g_return_val_if_fail (n != NULL, NULL);
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 191 "graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 191 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
			{
				gboolean _tmp0;
				_tmp0 = FALSE;
#line 192 "graph.vala"
				if (edge->visible) {
#line 192 "graph.vala"
					_tmp0 = edge->orig == n;
				} else {
#line 192 "graph.vala"
					_tmp0 = FALSE;
				}
#line 192 "graph.vala"
				if (_tmp0) {
					GravaNode* _tmp1;
#line 193 "graph.vala"
					_tmp1 = NULL;
#line 193 "graph.vala"
					ret = g_slist_append (ret, (_tmp1 = edge->dest, (_tmp1 == NULL) ? NULL : g_object_ref (_tmp1)));
				}
			}
		}
	}
#line 195 "graph.vala"
	return ret;
}


#line 198 "graph.vala"
GSList* grava_graph_inner_nodes (GravaGraph* self, GravaNode* n) {
	GSList* ret;
#line 198 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 198 "graph.vala"
	g_return_val_if_fail (n != NULL, NULL);
	ret = NULL;
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 201 "graph.vala"
		edge_collection = self->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 201 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
			{
				gboolean _tmp0;
				_tmp0 = FALSE;
#line 202 "graph.vala"
				if (edge->visible) {
#line 202 "graph.vala"
					_tmp0 = edge->dest == n;
				} else {
#line 202 "graph.vala"
					_tmp0 = FALSE;
				}
#line 202 "graph.vala"
				if (_tmp0) {
					GravaNode* _tmp1;
#line 203 "graph.vala"
					_tmp1 = NULL;
#line 203 "graph.vala"
					ret = g_slist_append (ret, (_tmp1 = edge->orig, (_tmp1 == NULL) ? NULL : g_object_ref (_tmp1)));
				}
			}
		}
	}
#line 205 "graph.vala"
	return ret;
}


#line 208 "graph.vala"
GravaNode* grava_graph_click (GravaGraph* self, double x, double y) {
	double z;
#line 208 "graph.vala"
	g_return_val_if_fail (self != NULL, NULL);
	z = self->zoom;
	{
		GSList* node_collection;
		GSList* node_it;
#line 211 "graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 211 "graph.vala"
			node = (GravaNode*) node_it->data;
			{
				gboolean _tmp0;
				gboolean _tmp1;
				gboolean _tmp2;
				_tmp0 = FALSE;
				_tmp1 = FALSE;
				_tmp2 = FALSE;
#line 212 "graph.vala"
				if (x >= (node->x * z)) {
#line 212 "graph.vala"
					_tmp2 = x <= ((node->x * z) + (node->w * z));
				} else {
#line 212 "graph.vala"
					_tmp2 = FALSE;
				}
#line 212 "graph.vala"
				if (_tmp2) {
#line 213 "graph.vala"
					_tmp1 = y >= (node->y * z);
				} else {
#line 212 "graph.vala"
					_tmp1 = FALSE;
				}
#line 212 "graph.vala"
				if (_tmp1) {
#line 213 "graph.vala"
					_tmp0 = y <= ((node->y * z) + (node->h * z));
				} else {
#line 212 "graph.vala"
					_tmp0 = FALSE;
				}
#line 212 "graph.vala"
				if (_tmp0) {
#line 214 "graph.vala"
					return node;
				}
			}
		}
	}
#line 216 "graph.vala"
	return NULL;
}


#line 219 "graph.vala"
gboolean grava_graph_overlaps (GravaGraph* self, GravaNode* n) {
#line 219 "graph.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 219 "graph.vala"
	g_return_val_if_fail (n != NULL, FALSE);
	{
		GSList* node_collection;
		GSList* node_it;
#line 221 "graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 221 "graph.vala"
			node = (GravaNode*) node_it->data;
			{
				gboolean _tmp0;
				_tmp0 = FALSE;
#line 222 "graph.vala"
				if (node != n) {
#line 222 "graph.vala"
					_tmp0 = grava_node_overlaps (node, n);
				} else {
#line 222 "graph.vala"
					_tmp0 = FALSE;
				}
#line 222 "graph.vala"
				if (_tmp0) {
#line 223 "graph.vala"
					return TRUE;
				}
			}
		}
	}
#line 225 "graph.vala"
	return FALSE;
}


#line 228 "graph.vala"
void grava_graph_draw (GravaGraph* self, cairo_t* ctx) {
#line 228 "graph.vala"
	g_return_if_fail (self != NULL);
#line 228 "graph.vala"
	g_return_if_fail (ctx != NULL);
	/*ctx.set_operator (Cairo.Operator.SOURCE);
	 XXX THIS FLICKERS! MUST USE DOUBLE BUFFER*/
#line 232 "graph.vala"
	if (ctx == NULL) {
#line 233 "graph.vala"
		return;
	}
#line 235 "graph.vala"
	cairo_set_source_rgba (ctx, (double) 1, (double) 1, (double) 1, (double) 1);
#line 236 "graph.vala"
	cairo_translate (ctx, self->panx, self->pany);
#line 237 "graph.vala"
	cairo_scale (ctx, self->zoom, self->zoom);
#line 238 "graph.vala"
	cairo_rotate (ctx, self->angle);
	/* blank screen */
#line 241 "graph.vala"
	cairo_paint (ctx);
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
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 250 "graph.vala"
			edge = (GravaEdge*) edge_it->data;
			{
#line 251 "graph.vala"
				if (edge->visible) {
#line 252 "graph.vala"
					grava_renderer_draw_edge (ctx, edge);
				}
			}
		}
	}
	{
		GSList* node_collection;
		GSList* node_it;
#line 254 "graph.vala"
		node_collection = self->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 254 "graph.vala"
			node = (GravaNode*) node_it->data;
			{
#line 255 "graph.vala"
				if (node->visible) {
#line 256 "graph.vala"
					grava_renderer_draw_node (ctx, node);
				}
			}
		}
	}
}


/* TODO: double buffering*/
#line 262 "graph.vala"
void grava_graph_add (GravaGraph* self, GravaNode* n) {
	GravaNode* _tmp0;
#line 262 "graph.vala"
	g_return_if_fail (self != NULL);
#line 262 "graph.vala"
	g_return_if_fail (n != NULL);
#line 264 "graph.vala"
	_tmp0 = NULL;
#line 264 "graph.vala"
	self->nodes = g_slist_append (self->nodes, (_tmp0 = n, (_tmp0 == NULL) ? NULL : g_object_ref (_tmp0)));
}


#line 267 "graph.vala"
void grava_graph_link (GravaGraph* self, GravaNode* n, GravaNode* n2) {
	GravaEdge* _tmp0;
#line 267 "graph.vala"
	g_return_if_fail (self != NULL);
#line 267 "graph.vala"
	g_return_if_fail (n != NULL);
#line 267 "graph.vala"
	g_return_if_fail (n2 != NULL);
	_tmp0 = NULL;
#line 269 "graph.vala"
	self->edges = g_slist_append (self->edges, grava_edge_with (_tmp0 = grava_edge_new (), n, n2));
	(_tmp0 == NULL) ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL));
}


#line 22 "graph.vala"
GravaGraph* grava_graph_construct (GType object_type) {
	GravaGraph * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 22 "graph.vala"
GravaGraph* grava_graph_new (void) {
#line 22 "graph.vala"
	return grava_graph_construct (GRAVA_TYPE_GRAPH);
}


#line 182 "gobject-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
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
		GSList* _tmp0;
		GSList* _tmp1;
		GSList* _tmp2;
		GravaLayout* _tmp3;
		GHashTable* _tmp4;
		_tmp0 = NULL;
#line 38 "graph.vala"
		self->nodes = (_tmp0 = NULL, (self->nodes == NULL) ? NULL : (self->nodes = (_g_slist_free_g_object_unref (self->nodes), NULL)), _tmp0);
		_tmp1 = NULL;
#line 39 "graph.vala"
		self->edges = (_tmp1 = NULL, (self->edges == NULL) ? NULL : (self->edges = (_g_slist_free_g_object_unref (self->edges), NULL)), _tmp1);
		_tmp2 = NULL;
#line 40 "graph.vala"
		self->selhist = (_tmp2 = NULL, (self->selhist == NULL) ? NULL : (self->selhist = (_g_slist_free_g_object_unref (self->selhist), NULL)), _tmp2);
		_tmp3 = NULL;
#line 41 "graph.vala"
		self->layout = (_tmp3 = (GravaLayout*) grava_default_layout_new (), (self->layout == NULL) ? NULL : (self->layout = (g_object_unref (self->layout), NULL)), _tmp3);
		_tmp4 = NULL;
#line 42 "graph.vala"
		self->data = (_tmp4 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)), _tmp4);
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




