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

#include "edge.h"




enum  {
	GRAVA_EDGE_DUMMY_PROPERTY
};
static void _g_object_unref_gdestroy_notify (void* data);
static GObject * grava_edge_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_edge_parent_class = NULL;
static void grava_edge_finalize (GObject* obj);



#line 35 "edge.vala"
char* grava_edge_get (GravaEdge* self, const char* val) {
	const char* _tmp0;
#line 35 "edge.vala"
	g_return_val_if_fail (GRAVA_IS_EDGE (self), NULL);
#line 35 "edge.vala"
	g_return_val_if_fail (val != NULL, NULL);
#line 37 "edge.vala"
	_tmp0 = NULL;
#line 37 "edge.vala"
	return (_tmp0 = ((const char*) (g_hash_table_lookup (self->data, val))), (_tmp0 == NULL ? NULL : g_strdup (_tmp0)));
}


#line 40 "edge.vala"
void grava_edge_set (GravaEdge* self, const char* val, const char* key) {
	const char* _tmp1;
	const char* _tmp0;
#line 40 "edge.vala"
	g_return_if_fail (GRAVA_IS_EDGE (self));
#line 40 "edge.vala"
	g_return_if_fail (val != NULL);
#line 40 "edge.vala"
	g_return_if_fail (key != NULL);
#line 42 "edge.vala"
	_tmp1 = NULL;
#line 42 "edge.vala"
	_tmp0 = NULL;
#line 42 "edge.vala"
	g_hash_table_insert (self->data, (_tmp0 = val, (_tmp0 == NULL ? NULL : g_strdup (_tmp0))), (_tmp1 = key, (_tmp1 == NULL ? NULL : g_strdup (_tmp1))));
}


#line 45 "edge.vala"
GravaEdge* grava_edge_with (GravaEdge* self, GravaNode* a, GravaNode* b) {
	GravaNode* _tmp1;
	GravaNode* _tmp0;
	GravaNode* _tmp3;
	GravaNode* _tmp2;
	GravaEdge* _tmp4;
#line 45 "edge.vala"
	g_return_val_if_fail (GRAVA_IS_EDGE (self), NULL);
#line 45 "edge.vala"
	g_return_val_if_fail (GRAVA_IS_NODE (a), NULL);
#line 45 "edge.vala"
	g_return_val_if_fail (GRAVA_IS_NODE (b), NULL);
	_tmp1 = NULL;
#line 47 "edge.vala"
	_tmp0 = NULL;
#line 47 "edge.vala"
	self->orig = (_tmp1 = (_tmp0 = a, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))), (self->orig == NULL ? NULL : (self->orig = (g_object_unref (self->orig), NULL))), _tmp1);
	_tmp3 = NULL;
#line 48 "edge.vala"
	_tmp2 = NULL;
#line 48 "edge.vala"
	self->dest = (_tmp3 = (_tmp2 = b, (_tmp2 == NULL ? NULL : g_object_ref (_tmp2))), (self->dest == NULL ? NULL : (self->dest = (g_object_unref (self->dest), NULL))), _tmp3);
#line 49 "edge.vala"
	_tmp4 = NULL;
#line 49 "edge.vala"
	return (_tmp4 = self, (_tmp4 == NULL ? NULL : g_object_ref (_tmp4)));
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
	g_return_val_if_fail (GRAVA_IS_EDGE (self), 0.0);
#line 65 "edge.vala"
	return sqrt (pow (self->orig->x - self->dest->x, ((double) (2))) + pow (self->orig->y - self->dest->y, ((double) (2))));
}


#line 21 "edge.vala"
GravaEdge* grava_edge_construct (GType object_type) {
	GravaEdge * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 21 "edge.vala"
GravaEdge* grava_edge_new (void) {
#line 21 "edge.vala"
	return grava_edge_construct (GRAVA_TYPE_EDGE);
}


#line 955 "glib-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
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
		GHashTable* _tmp0;
		GravaNode* _tmp3;
		GravaNode* _tmp2;
		GravaNode* _tmp1;
		_tmp0 = NULL;
#line 30 "edge.vala"
		self->data = (_tmp0 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL ? NULL : (self->data = (g_hash_table_unref (self->data), NULL))), _tmp0);
		_tmp3 = NULL;
#line 31 "edge.vala"
		_tmp2 = NULL;
		_tmp1 = NULL;
#line 31 "edge.vala"
		self->orig = (_tmp3 = (_tmp2 = self->dest = (_tmp1 = NULL, (self->dest == NULL ? NULL : (self->dest = (g_object_unref (self->dest), NULL))), _tmp1), (_tmp2 == NULL ? NULL : g_object_ref (_tmp2))), (self->orig == NULL ? NULL : (self->orig = (g_object_unref (self->orig), NULL))), _tmp3);
#line 32 "edge.vala"
		self->visible = TRUE;
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
	(self->data == NULL ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)));
	(self->orig == NULL ? NULL : (self->orig = (g_object_unref (self->orig), NULL)));
	(self->dest == NULL ? NULL : (self->dest = (g_object_unref (self->dest), NULL)));
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




