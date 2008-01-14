/*
primer incremento les x mentres vaig guardant el totalx
guardo en una llista enlla,cada tots els nodos q he mogut
els recorro again i els delpla,co a lesqerra lo que toquin


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
static GObject * grava_edge_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_edge_parent_class = NULL;
static void grava_edge_dispose (GObject * obj);


#line 34 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
void grava_edge_set (GravaEdge* self, const char* val, const char* key) {
	const char* _tmp1;
	const char* _tmp0;
#line 34 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	g_return_if_fail (GRAVA_IS_EDGE (self));
#line 36 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	_tmp1 = NULL;
#line 36 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	_tmp0 = NULL;
#line 36 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	g_hash_table_insert (self->data, (_tmp0 = val, (_tmp0 == NULL ? NULL : g_strdup (_tmp0))), (_tmp1 = key, (_tmp1 == NULL ? NULL : g_strdup (_tmp1))));
}


#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
GravaEdge* grava_edge_with (GravaEdge* self, GravaNode* a, GravaNode* b) {
	GravaNode* _tmp1;
	GravaNode* _tmp0;
	GravaNode* _tmp3;
	GravaNode* _tmp2;
	GravaEdge* _tmp4;
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	g_return_val_if_fail (GRAVA_IS_EDGE (self), NULL);
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	g_return_val_if_fail (a == NULL || GRAVA_IS_NODE (a), NULL);
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	g_return_val_if_fail (b == NULL || GRAVA_IS_NODE (b), NULL);
	_tmp1 = NULL;
#line 41 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	_tmp0 = NULL;
#line 41 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	self->orig = (_tmp1 = (_tmp0 = a, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))), (self->orig == NULL ? NULL : (self->orig = (g_object_unref (self->orig), NULL))), _tmp1);
	_tmp3 = NULL;
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	_tmp2 = NULL;
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	self->dest = (_tmp3 = (_tmp2 = b, (_tmp2 == NULL ? NULL : g_object_ref (_tmp2))), (self->dest == NULL ? NULL : (self->dest = (g_object_unref (self->dest), NULL))), _tmp3);
#line 43 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	_tmp4 = NULL;
#line 43 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
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
#line 57 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
double grava_edge_distance (GravaEdge* self) {
#line 57 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	g_return_val_if_fail (GRAVA_IS_EDGE (self), 0.0);
#line 59 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
	return sqrt (exp2 (self->orig->x - self->dest->x) + exp2 (self->orig->y - self->dest->y));
}


#line 21 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
GravaEdge* grava_edge_new (void) {
	GravaEdge * self;
	self = g_object_newv (GRAVA_TYPE_EDGE, 0, NULL);
	return self;
}


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
		GravaNode* _tmp2;
		GravaNode* _tmp1;
		_tmp0 = NULL;
#line 29 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
		self->data = (_tmp0 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref), (self->data == NULL ? NULL : (self->data = (g_hash_table_unref (self->data), NULL))), _tmp0);
		_tmp2 = NULL;
		_tmp1 = NULL;
#line 30 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
		self->orig = (_tmp2 = self->dest = (_tmp1 = NULL, (self->dest == NULL ? NULL : (self->dest = (g_object_unref (self->dest), NULL))), _tmp1), (self->orig == NULL ? NULL : (self->orig = (g_object_unref (self->orig), NULL))), _tmp2);
#line 31 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/edge.vala"
		self->visible = TRUE;
	}
	return obj;
}


static void grava_edge_class_init (GravaEdgeClass * klass) {
	grava_edge_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = grava_edge_constructor;
	G_OBJECT_CLASS (klass)->dispose = grava_edge_dispose;
}


static void grava_edge_init (GravaEdge * self) {
}


static void grava_edge_dispose (GObject * obj) {
	GravaEdge * self;
	self = GRAVA_EDGE (obj);
	(self->data == NULL ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)));
	(self->orig == NULL ? NULL : (self->orig = (g_object_unref (self->orig), NULL)));
	(self->dest == NULL ? NULL : (self->dest = (g_object_unref (self->dest), NULL)));
	G_OBJECT_CLASS (grava_edge_parent_class)->dispose (obj);
}


GType grava_edge_get_type (void) {
	static GType grava_edge_type_id = 0;
	if (G_UNLIKELY (grava_edge_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaEdgeClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_edge_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaEdge), 0, (GInstanceInitFunc) grava_edge_init };
		grava_edge_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaEdge", &g_define_type_info, 0);
	}
	return grava_edge_type_id;
}




