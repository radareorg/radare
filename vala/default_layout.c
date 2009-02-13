/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007, 2008   pancake <youterm.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "edge.h"




struct _GravaDefaultLayoutPrivate {
	GravaGraph* graph;
};

#define GRAVA_DEFAULT_LAYOUT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayoutPrivate))
enum  {
	GRAVA_DEFAULT_LAYOUT_DUMMY_PROPERTY
};
static gboolean grava_default_layout_d = FALSE;
static void _g_slist_free_g_object_unref (GSList* self);
static void grava_default_layout_treenodes (GravaDefaultLayout* self, GravaNode* n);
static void grava_default_layout_real_set_graph (GravaLayout* base, GravaGraph* graph);
static void grava_default_layout_real_run (GravaLayout* base, GravaGraph* graph);
static void _g_object_unref_gdestroy_notify (void* data);
static GObject * grava_default_layout_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_default_layout_parent_class = NULL;
static void grava_default_layout_finalize (GObject* obj);



#line 34 "default_layout.vala"
void grava_default_layout_reset (GravaDefaultLayout* self) {
#line 34 "default_layout.vala"
	g_return_if_fail (self != NULL);
#line 36 "default_layout.vala"
	fprintf (stdout, "RESETING LAYOUT\n");
	{
		GSList* node_collection;
		GSList* node_it;
#line 37 "default_layout.vala"
		node_collection = self->priv->graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 37 "default_layout.vala"
			node = (GravaNode*) node_it->data;
			{
#line 38 "default_layout.vala"
				fprintf (stdout, "RESETING LAYOUT+++++\n");
#line 39 "default_layout.vala"
				grava_default_layout_setxy (self, node);
			}
		}
	}
}


#line 43 "default_layout.vala"
void grava_default_layout_reset_real (GravaDefaultLayout* self) {
#line 43 "default_layout.vala"
	g_return_if_fail (self != NULL);
	{
		GSList* node_collection;
		GSList* node_it;
#line 45 "default_layout.vala"
		node_collection = self->priv->graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 45 "default_layout.vala"
			node = (GravaNode*) node_it->data;
			{
#line 46 "default_layout.vala"
				self->priv->graph->nodes = g_slist_remove (self->priv->graph->nodes, node);
			}
		}
	}
}


static void _g_slist_free_g_object_unref (GSList* self) {
	g_slist_foreach (self, (GFunc) g_object_unref, NULL);
	g_slist_free (self);
}


#line 50 "default_layout.vala"
static void grava_default_layout_treenodes (GravaDefaultLayout* self, GravaNode* n) {
	gint ox;
	GSList* nodes;
#line 50 "default_layout.vala"
	g_return_if_fail (self != NULL);
#line 50 "default_layout.vala"
	g_return_if_fail (n != NULL);
	ox = 0;
	nodes = grava_graph_outer_nodes (self->priv->graph, n);
	{
		GSList* node_collection;
		GSList* node_it;
#line 54 "default_layout.vala"
		node_collection = nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 54 "default_layout.vala"
			node = (GravaNode*) node_it->data;
			{
#line 55 "default_layout.vala"
				node->y = (n->y + n->h) + self->y_offset;
#line 56 "default_layout.vala"
				node->x = node->x + (n->w + ox);
#line 57 "default_layout.vala"
				ox = ox + (50);
#line 58 "default_layout.vala"
				grava_default_layout_treenodes (self, node);
			}
		}
	}
	(nodes == NULL) ? NULL : (nodes = (_g_slist_free_g_object_unref (nodes), NULL));
}


#line 62 "default_layout.vala"
void grava_default_layout_setxy (GravaDefaultLayout* self, GravaNode* n) {
	GravaNode* _tmp1;
	char* _tmp0;
	GravaNode* _tmp2;
	GravaNode* m;
#line 62 "default_layout.vala"
	g_return_if_fail (self != NULL);
#line 62 "default_layout.vala"
	g_return_if_fail (n != NULL);
#line 64 "default_layout.vala"
	_tmp1 = NULL;
#line 61 "node.vala"
	_tmp0 = NULL;
#line 64 "default_layout.vala"
	_tmp2 = NULL;
	m = (_tmp2 = (_tmp1 = (GravaNode*) g_hash_table_lookup (self->data, _tmp0 = grava_node_get (n, "offset")), (_tmp1 == NULL) ? NULL : g_object_ref (_tmp1)), _tmp0 = (g_free (_tmp0), NULL), _tmp2);
#line 65 "default_layout.vala"
	if (m == NULL) {
		GravaNode* no;
		char* _tmp3;
		GravaNode* _tmp4;
		no = grava_node_new ();
#line 61 "node.vala"
		_tmp3 = NULL;
#line 67 "default_layout.vala"
		grava_node_set (no, "offset", _tmp3 = grava_node_get (n, "offset"));
		_tmp3 = (g_free (_tmp3), NULL);
#line 68 "default_layout.vala"
		no->x = n->x;
#line 69 "default_layout.vala"
		no->y = n->y;
#line 70 "default_layout.vala"
		no->w = n->w;
#line 71 "default_layout.vala"
		no->h = n->h;
#line 72 "default_layout.vala"
		_tmp4 = NULL;
#line 72 "default_layout.vala"
		g_hash_table_insert (self->data, grava_node_get (n, "offset"), (_tmp4 = no, (_tmp4 == NULL) ? NULL : g_object_ref (_tmp4)));
		(no == NULL) ? NULL : (no = (g_object_unref (no), NULL));
	} else {
#line 74 "default_layout.vala"
		m->x = n->x;
#line 75 "default_layout.vala"
		m->y = n->y;
#line 76 "default_layout.vala"
		m->w = n->w;
#line 77 "default_layout.vala"
		m->h = n->h;
	}
	(m == NULL) ? NULL : (m = (g_object_unref (m), NULL));
}


#line 81 "default_layout.vala"
gboolean grava_default_layout_getxy (GravaDefaultLayout* self, GravaNode** n) {
	GravaNode* _tmp1;
	char* _tmp0;
	GravaNode* _tmp2;
	GravaNode* m;
	char* _tmp5;
	gboolean _tmp6;
#line 81 "default_layout.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 81 "default_layout.vala"
	g_return_val_if_fail (n != NULL, FALSE);
#line 83 "default_layout.vala"
	_tmp1 = NULL;
#line 61 "node.vala"
	_tmp0 = NULL;
#line 83 "default_layout.vala"
	_tmp2 = NULL;
	m = (_tmp2 = (_tmp1 = (GravaNode*) g_hash_table_lookup (self->data, _tmp0 = grava_node_get ((*n), "offset")), (_tmp1 == NULL) ? NULL : g_object_ref (_tmp1)), _tmp0 = (g_free (_tmp0), NULL), _tmp2);
#line 84 "default_layout.vala"
	if (m != NULL) {
		char* _tmp3;
		gboolean _tmp4;
#line 85 "default_layout.vala"
		(*n)->x = m->x;
#line 86 "default_layout.vala"
		(*n)->y = m->y;
#line 87 "default_layout.vala"
		(*n)->w = m->w;
#line 88 "default_layout.vala"
		(*n)->h = m->h;
#line 61 "node.vala"
		_tmp3 = NULL;
#line 89 "default_layout.vala"
		fprintf (stdout, "TAKEND FOR %s\n", _tmp3 = grava_node_get ((*n), "offset"));
		_tmp3 = (g_free (_tmp3), NULL);
#line 90 "default_layout.vala"
		return (_tmp4 = TRUE, (m == NULL) ? NULL : (m = (g_object_unref (m), NULL)), _tmp4);
	}
#line 61 "node.vala"
	_tmp5 = NULL;
#line 92 "default_layout.vala"
	fprintf (stdout, "NOT TAKEND FOR %s\n", _tmp5 = grava_node_get ((*n), "offset"));
	_tmp5 = (g_free (_tmp5), NULL);
#line 94 "default_layout.vala"
	return (_tmp6 = FALSE, (m == NULL) ? NULL : (m = (g_object_unref (m), NULL)), _tmp6);
}


#line 97 "default_layout.vala"
void grava_default_layout_walkChild (GravaDefaultLayout* self, GravaNode* node, gint level) {
#line 97 "default_layout.vala"
	g_return_if_fail (self != NULL);
#line 97 "default_layout.vala"
	g_return_if_fail (node != NULL);
#line 99 "default_layout.vala"
	if (level < 1) {
#line 99 "default_layout.vala"
		return;
	}
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 100 "default_layout.vala"
		edge_collection = self->priv->graph->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 100 "default_layout.vala"
			edge = (GravaEdge*) edge_it->data;
			{
#line 101 "default_layout.vala"
				if (edge->orig == node) {
#line 102 "default_layout.vala"
					edge->dest->y = (edge->orig->y + edge->orig->h) + self->y_offset;
#line 103 "default_layout.vala"
					grava_default_layout_walkChild (self, edge->dest, (level = level - 1));
				}
			}
		}
	}
}


#line 108 "default_layout.vala"
GravaNode* grava_default_layout_get_parent (GravaDefaultLayout* self, GravaNode* node) {
#line 108 "default_layout.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 108 "default_layout.vala"
	g_return_val_if_fail (node != NULL, NULL);
	{
		GSList* edge_collection;
		GSList* edge_it;
#line 110 "default_layout.vala"
		edge_collection = self->priv->graph->edges;
		for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
			GravaEdge* edge;
#line 110 "default_layout.vala"
			edge = (GravaEdge*) edge_it->data;
			{
#line 111 "default_layout.vala"
				if (edge->dest == node) {
#line 112 "default_layout.vala"
					return edge->orig;
				}
			}
		}
	}
#line 114 "default_layout.vala"
	return NULL;
}


#line 117 "default_layout.vala"
static void grava_default_layout_real_set_graph (GravaLayout* base, GravaGraph* graph) {
	GravaDefaultLayout * self;
	self = (GravaDefaultLayout*) base;
#line 117 "default_layout.vala"
	g_return_if_fail (graph != NULL);
#line 119 "default_layout.vala"
	self->priv->graph = graph;
}


#line 122 "default_layout.vala"
static void grava_default_layout_real_run (GravaLayout* base, GravaGraph* graph) {
	GravaDefaultLayout * self;
	double last_y;
	gint i;
	gint k;
	GravaNode* n;
	GravaNode* p;
	GravaNode* destn;
	gboolean found;
	self = (GravaDefaultLayout*) base;
#line 122 "default_layout.vala"
	g_return_if_fail (graph != NULL);
#line 124 "default_layout.vala"
	self->priv->graph = graph;
	last_y = 0.0;
	i = 0;
	k = 0;
	n = NULL;
	p = NULL;
	destn = NULL;
	found = FALSE;
	/* reset all node positions*/
#line 132 "default_layout.vala"
	last_y = (double) 50;
	/* Tots vertical, un sota l'altr ordenats per base addr*/
	{
		GSList* node_collection;
		GSList* node_it;
#line 135 "default_layout.vala"
		node_collection = graph->nodes;
		for (node_it = node_collection; node_it != NULL; node_it = node_it->next) {
			GravaNode* node;
#line 135 "default_layout.vala"
			node = (GravaNode*) node_it->data;
			{
#line 136 "default_layout.vala"
				if (!node->visible) {
#line 136 "default_layout.vala"
					continue;
				}
				/* reset node positions*/
#line 138 "default_layout.vala"
				node->x = (double) 50;
#line 139 "default_layout.vala"
				grava_node_fit (node);
#line 140 "default_layout.vala"
				node->y = last_y;
#line 141 "default_layout.vala"
				last_y = (node->y + node->h) + 50;
#line 142 "default_layout.vala"
				if (grava_default_layout_d) {
					char* _tmp0;
#line 61 "node.vala"
					_tmp0 = NULL;
#line 142 "default_layout.vala"
					fprintf (stdout, " at %f %s %x\n", node->y, _tmp0 = grava_node_get (node, "label"), node->baseaddr);
					_tmp0 = (g_free (_tmp0), NULL);
				}
			}
		}
	}
	/* Per cada node. Segueixo la condiciÃ³ certa, tots els nodes que estiguin
	 entre el node que estic mirant i el de la condicio certa els desplaÃ§o a la dreta.
	 Tambe  apropo  el node desti a l'origen.
	
	 Entre el node que miro i el desti vol dir que la x sigui la mateixa.
	*/
#line 151 "default_layout.vala"
	for (i = 0; i < g_slist_length (graph->nodes); i++) {
		GravaNode* _tmp2;
		GravaNode* _tmp1;
		GravaNode* _tmp4;
		char* _tmp3;
		GravaNode* _tmp5;
		GravaNode* m;
		gboolean _tmp9;
		_tmp2 = NULL;
#line 152 "default_layout.vala"
		_tmp1 = NULL;
#line 152 "default_layout.vala"
		n = (_tmp2 = (_tmp1 = (GravaNode*) g_slist_nth_data (graph->nodes, (guint) i), (_tmp1 == NULL) ? NULL : g_object_ref (_tmp1)), (n == NULL) ? NULL : (n = (g_object_unref (n), NULL)), _tmp2);
		/*
		if (getxy(ref n))
		continue;
		*/
#line 157 "default_layout.vala"
		_tmp4 = NULL;
#line 61 "node.vala"
		_tmp3 = NULL;
#line 157 "default_layout.vala"
		_tmp5 = NULL;
		m = (_tmp5 = (_tmp4 = (GravaNode*) g_hash_table_lookup (self->data, _tmp3 = grava_node_get (n, "offset")), (_tmp4 == NULL) ? NULL : g_object_ref (_tmp4)), _tmp3 = (g_free (_tmp3), NULL), _tmp5);
#line 158 "default_layout.vala"
		if (m != NULL) {
#line 159 "default_layout.vala"
			n->x = m->x;
#line 160 "default_layout.vala"
			n->y = m->y;
#line 161 "default_layout.vala"
			n->w = m->w;
#line 162 "default_layout.vala"
			n->h = m->h;
#line 163 "default_layout.vala"
			fprintf (stdout, "FUCKA! %f %f\n", n->x, n->y);
			(m == NULL) ? NULL : (m = (g_object_unref (m), NULL));
#line 164 "default_layout.vala"
			continue;
		}
#line 166 "default_layout.vala"
		fprintf (stdout, "---- not ounfd !\n");
		/*/ busco l'edge verd d'aquest node
		/*/
#line 170 "default_layout.vala"
		found = FALSE;
		{
			GSList* edge_collection;
			GSList* edge_it;
#line 171 "default_layout.vala"
			edge_collection = graph->edges;
			for (edge_it = edge_collection; edge_it != NULL; edge_it = edge_it->next) {
				GravaEdge* edge;
#line 171 "default_layout.vala"
				edge = (GravaEdge*) edge_it->data;
				{
					gboolean _tmp6;
					_tmp6 = FALSE;
#line 172 "default_layout.vala"
					if (edge->orig == n) {
#line 172 "default_layout.vala"
						_tmp6 = edge->jmpcnd == TRUE;
					} else {
#line 172 "default_layout.vala"
						_tmp6 = FALSE;
					}
#line 172 "default_layout.vala"
					if (_tmp6) {
						GravaNode* _tmp8;
						GravaNode* _tmp7;
#line 173 "default_layout.vala"
						if (grava_default_layout_d) {
#line 173 "default_layout.vala"
							fprintf (stdout, "0x%x ----> 0x%x\n", edge->orig->baseaddr, edge->dest->baseaddr);
						}
						_tmp8 = NULL;
#line 174 "default_layout.vala"
						_tmp7 = NULL;
#line 174 "default_layout.vala"
						destn = (_tmp8 = (_tmp7 = edge->dest, (_tmp7 == NULL) ? NULL : g_object_ref (_tmp7)), (destn == NULL) ? NULL : (destn = (g_object_unref (destn), NULL)), _tmp8);
#line 175 "default_layout.vala"
						found = TRUE;
#line 176 "default_layout.vala"
						break;
					}
				}
			}
		}
		/*/ n es el node origen.
		/ destn es el node desti
		/*/
#line 183 "default_layout.vala"
		last_y = (n->y + n->h) + 10;
		_tmp9 = FALSE;
#line 188 "default_layout.vala"
		if ((found == TRUE)) {
#line 188 "default_layout.vala"
			_tmp9 = (n->baseaddr < destn->baseaddr);
		} else {
#line 188 "default_layout.vala"
			_tmp9 = FALSE;
		}
		/* Si la base del node origen es < que le desti .
		 sempre anem avall.
		*/
#line 188 "default_layout.vala"
		if (_tmp9) {
			double maxw;
			/*/ Busco el node mes ample.
			/*/
			maxw = (double) 0;
#line 192 "default_layout.vala"
			for (k = (i + 1); k < g_slist_length (graph->nodes); k++) {
				GravaNode* _tmp11;
				GravaNode* _tmp10;
				gboolean _tmp12;
				_tmp11 = NULL;
#line 193 "default_layout.vala"
				_tmp10 = NULL;
#line 193 "default_layout.vala"
				p = (_tmp11 = (_tmp10 = (GravaNode*) g_slist_nth_data (graph->nodes, (guint) k), (_tmp10 == NULL) ? NULL : g_object_ref (_tmp10)), (p == NULL) ? NULL : (p = (g_object_unref (p), NULL)), _tmp11);
				_tmp12 = FALSE;
#line 194 "default_layout.vala"
				if ((p->x == n->x)) {
#line 194 "default_layout.vala"
					_tmp12 = (p->w > maxw);
				} else {
#line 194 "default_layout.vala"
					_tmp12 = FALSE;
				}
#line 194 "default_layout.vala"
				if (_tmp12) {
#line 195 "default_layout.vala"
					maxw = p->w;
				}
			}
			/*/ DesplaÃ§o
			/*/
#line 199 "default_layout.vala"
			for (k = (i + 1); k < g_slist_length (graph->nodes); k++) {
				GravaNode* _tmp14;
				GravaNode* _tmp13;
				_tmp14 = NULL;
#line 200 "default_layout.vala"
				_tmp13 = NULL;
#line 200 "default_layout.vala"
				p = (_tmp14 = (_tmp13 = (GravaNode*) g_slist_nth_data (graph->nodes, (guint) k), (_tmp13 == NULL) ? NULL : g_object_ref (_tmp13)), (p == NULL) ? NULL : (p = (g_object_unref (p), NULL)), _tmp14);
#line 202 "default_layout.vala"
				if (grava_default_layout_d) {
#line 202 "default_layout.vala"
					fprintf (stdout, "Displacing 0x%x\n", p->baseaddr);
				}
				/* El node estava entre el node origen i el desti*/
#line 205 "default_layout.vala"
				if (p->x == n->x) {
#line 206 "default_layout.vala"
					p->x = p->x + ((maxw + 10));
				}
				/*/ Es ja el node desti.*/
#line 209 "default_layout.vala"
				if (p == destn) {
#line 210 "default_layout.vala"
					if (grava_default_layout_d) {
#line 210 "default_layout.vala"
						fprintf (stdout, "AT 0x%x\n", p->baseaddr);
					}
#line 211 "default_layout.vala"
					destn->x = n->x;
#line 212 "default_layout.vala"
					destn->y = (n->y + n->h) + 50;
#line 213 "default_layout.vala"
					break;
				}
			}
		}
#line 217 "default_layout.vala"
		grava_default_layout_setxy (self, n);
		(m == NULL) ? NULL : (m = (g_object_unref (m), NULL));
	}
	(n == NULL) ? NULL : (n = (g_object_unref (n), NULL));
	(p == NULL) ? NULL : (p = (g_object_unref (p), NULL));
	(destn == NULL) ? NULL : (destn = (g_object_unref (destn), NULL));
#line 220 "default_layout.vala"
	return;
}


#line 21 "default_layout.vala"
GravaDefaultLayout* grava_default_layout_construct (GType object_type) {
	GravaDefaultLayout * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 21 "default_layout.vala"
GravaDefaultLayout* grava_default_layout_new (void) {
#line 21 "default_layout.vala"
	return grava_default_layout_construct (GRAVA_TYPE_DEFAULT_LAYOUT);
}


#line 182 "gobject-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
	g_object_unref (data);
}


/* debug output*/
static GObject * grava_default_layout_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GravaDefaultLayoutClass * klass;
	GObjectClass * parent_class;
	GravaDefaultLayout * self;
	klass = GRAVA_DEFAULT_LAYOUT_CLASS (g_type_class_peek (GRAVA_TYPE_DEFAULT_LAYOUT));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GRAVA_DEFAULT_LAYOUT (obj);
	{
		GHashTable* _tmp0;
		_tmp0 = NULL;
#line 31 "default_layout.vala"
		self->data = (_tmp0 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)), _tmp0);
	}
	return obj;
}


static void grava_default_layout_class_init (GravaDefaultLayoutClass * klass) {
	grava_default_layout_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (GravaDefaultLayoutPrivate));
	G_OBJECT_CLASS (klass)->constructor = grava_default_layout_constructor;
	G_OBJECT_CLASS (klass)->finalize = grava_default_layout_finalize;
	GRAVA_LAYOUT_CLASS (klass)->set_graph = grava_default_layout_real_set_graph;
	GRAVA_LAYOUT_CLASS (klass)->run = grava_default_layout_real_run;
}


static void grava_default_layout_instance_init (GravaDefaultLayout * self) {
	self->priv = GRAVA_DEFAULT_LAYOUT_GET_PRIVATE (self);
	self->y_offset = (double) 100;
	self->x_offset = (double) 50;
}


static void grava_default_layout_finalize (GObject* obj) {
	GravaDefaultLayout * self;
	self = GRAVA_DEFAULT_LAYOUT (obj);
	(self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL));
	G_OBJECT_CLASS (grava_default_layout_parent_class)->finalize (obj);
}


GType grava_default_layout_get_type (void) {
	static GType grava_default_layout_type_id = 0;
	if (grava_default_layout_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaDefaultLayoutClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_default_layout_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaDefaultLayout), 0, (GInstanceInitFunc) grava_default_layout_instance_init, NULL };
		grava_default_layout_type_id = g_type_register_static (GRAVA_TYPE_LAYOUT, "GravaDefaultLayout", &g_define_type_info, 0);
	}
	return grava_default_layout_type_id;
}




