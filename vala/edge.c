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

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


#define GRAVA_TYPE_EDGE (grava_edge_get_type ())
#define GRAVA_EDGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_EDGE, GravaEdge))
#define GRAVA_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_EDGE, GravaEdgeClass))
#define GRAVA_IS_EDGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_EDGE))
#define GRAVA_IS_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_EDGE))
#define GRAVA_EDGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_EDGE, GravaEdgeClass))

typedef struct _GravaEdge GravaEdge;
typedef struct _GravaEdgeClass GravaEdgeClass;
typedef struct _GravaEdgePrivate GravaEdgePrivate;

#define GRAVA_TYPE_NODE (grava_node_get_type ())
#define GRAVA_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_NODE, GravaNode))
#define GRAVA_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_NODE, GravaNodeClass))
#define GRAVA_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_NODE))
#define GRAVA_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_NODE))
#define GRAVA_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_NODE, GravaNodeClass))

typedef struct _GravaNode GravaNode;
typedef struct _GravaNodeClass GravaNodeClass;
typedef struct _GravaNodePrivate GravaNodePrivate;

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



GType grava_edge_get_type (void);
GType grava_node_get_type (void);
enum  {
	GRAVA_EDGE_DUMMY_PROPERTY
};
char* grava_edge_get (GravaEdge* self, const char* val);
void grava_edge_set (GravaEdge* self, const char* val, const char* key);
GravaEdge* grava_edge_with (GravaEdge* self, GravaNode* a, GravaNode* b);
double grava_edge_distance (GravaEdge* self);
GravaEdge* grava_edge_new (void);
GravaEdge* grava_edge_construct (GType object_type);
GravaEdge* grava_edge_new (void);
static void _g_object_unref_gdestroy_notify (void* data);
static GObject * grava_edge_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_edge_parent_class = NULL;
static void grava_edge_finalize (GObject* obj);



#line 35 "edge.vala"
char* grava_edge_get (GravaEdge* self, const char* val) {
#line 108 "edge.c"
	const char* _tmp0_;
#line 35 "edge.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 35 "edge.vala"
	g_return_val_if_fail (val != NULL, NULL);
#line 37 "edge.vala"
	_tmp0_ = NULL;
#line 37 "edge.vala"
	return (_tmp0_ = (const char*) g_hash_table_lookup (self->data, val), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 118 "edge.c"
}


#line 40 "edge.vala"
void grava_edge_set (GravaEdge* self, const char* val, const char* key) {
#line 124 "edge.c"
	const char* _tmp1_;
	const char* _tmp0_;
#line 40 "edge.vala"
	g_return_if_fail (self != NULL);
#line 40 "edge.vala"
	g_return_if_fail (val != NULL);
#line 40 "edge.vala"
	g_return_if_fail (key != NULL);
#line 42 "edge.vala"
	_tmp1_ = NULL;
#line 42 "edge.vala"
	_tmp0_ = NULL;
#line 42 "edge.vala"
	g_hash_table_insert (self->data, (_tmp0_ = val, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)), (_tmp1_ = key, (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_)));
#line 139 "edge.c"
}


#line 45 "edge.vala"
GravaEdge* grava_edge_with (GravaEdge* self, GravaNode* a, GravaNode* b) {
#line 145 "edge.c"
	GravaNode* _tmp1_;
	GravaNode* _tmp0_;
	GravaNode* _tmp3_;
	GravaNode* _tmp2_;
	GravaEdge* _tmp4_;
#line 45 "edge.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 45 "edge.vala"
	g_return_val_if_fail (a != NULL, NULL);
#line 45 "edge.vala"
	g_return_val_if_fail (b != NULL, NULL);
#line 157 "edge.c"
	_tmp1_ = NULL;
#line 47 "edge.vala"
	_tmp0_ = NULL;
#line 47 "edge.vala"
	self->orig = (_tmp1_ = (_tmp0_ = a, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_)), (self->orig == NULL) ? NULL : (self->orig = (g_object_unref (self->orig), NULL)), _tmp1_);
#line 163 "edge.c"
	_tmp3_ = NULL;
#line 48 "edge.vala"
	_tmp2_ = NULL;
#line 48 "edge.vala"
	self->dest = (_tmp3_ = (_tmp2_ = b, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)), (self->dest == NULL) ? NULL : (self->dest = (g_object_unref (self->dest), NULL)), _tmp3_);
#line 49 "edge.vala"
	_tmp4_ = NULL;
#line 49 "edge.vala"
	return (_tmp4_ = self, (_tmp4_ == NULL) ? NULL : g_object_ref (_tmp4_));
#line 173 "edge.c"
}


/* workaround for netbsd's libm 
private double exp2(double x)
{
return Math.exp(x * Math.log(2));
}

public double distance()
{
return Math.sqrt( this.exp2(orig.x-dest.x) + this.exp2(orig.y-dest.y));
}
*/
#line 63 "edge.vala"
double grava_edge_distance (GravaEdge* self) {
#line 63 "edge.vala"
	g_return_val_if_fail (self != NULL, 0.0);
#line 65 "edge.vala"
	return sqrt (pow (self->orig->x - self->dest->x, (double) 2) + pow (self->orig->y - self->dest->y, (double) 2));
#line 194 "edge.c"
}


#line 21 "edge.vala"
GravaEdge* grava_edge_construct (GType object_type) {
#line 200 "edge.c"
	GravaEdge * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 21 "edge.vala"
GravaEdge* grava_edge_new (void) {
#line 21 "edge.vala"
	return grava_edge_construct (GRAVA_TYPE_EDGE);
#line 211 "edge.c"
}


#line 182 "gobject-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
#line 217 "edge.c"
	g_object_unref (data);
}


/* verd == true , vermell == false*/
static GObject * grava_edge_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GravaEdgeClass * klass;
	GObjectClass * parent_class;
	GravaEdge * self;
	klass = GRAVA_EDGE_CLASS (g_type_class_peek (GRAVA_TYPE_EDGE));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GRAVA_EDGE (obj);
	{
		GHashTable* _tmp0_;
		GravaNode* _tmp3_;
		GravaNode* _tmp2_;
		GravaNode* _tmp1_;
		_tmp0_ = NULL;
#line 30 "edge.vala"
		self->data = (_tmp0_ = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)), _tmp0_);
#line 240 "edge.c"
		_tmp3_ = NULL;
#line 31 "edge.vala"
		_tmp2_ = NULL;
#line 244 "edge.c"
		_tmp1_ = NULL;
#line 31 "edge.vala"
		self->orig = (_tmp3_ = (_tmp2_ = self->dest = (_tmp1_ = NULL, (self->dest == NULL) ? NULL : (self->dest = (g_object_unref (self->dest), NULL)), _tmp1_), (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)), (self->orig == NULL) ? NULL : (self->orig = (g_object_unref (self->orig), NULL)), _tmp3_);
#line 32 "edge.vala"
		self->visible = TRUE;
#line 250 "edge.c"
	}
	return obj;
}


static void grava_edge_class_init (GravaEdgeClass * klass) {
	grava_edge_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = grava_edge_constructor;
	G_OBJECT_CLASS (klass)->finalize = grava_edge_finalize;
}


static void grava_edge_instance_init (GravaEdge * self) {
}


static void grava_edge_finalize (GObject* obj) {
	GravaEdge * self;
	self = GRAVA_EDGE (obj);
	(self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL));
	(self->orig == NULL) ? NULL : (self->orig = (g_object_unref (self->orig), NULL));
	(self->dest == NULL) ? NULL : (self->dest = (g_object_unref (self->dest), NULL));
	G_OBJECT_CLASS (grava_edge_parent_class)->finalize (obj);
}


GType grava_edge_get_type (void) {
	static GType grava_edge_type_id = 0;
	if (grava_edge_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaEdgeClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_edge_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaEdge), 0, (GInstanceInitFunc) grava_edge_instance_init, NULL };
		grava_edge_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaEdge", &g_define_type_info, 0);
	}
	return grava_edge_type_id;
}




