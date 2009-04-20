/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007,2008  pancake <youterm.com>
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


#define GRAVA_TYPE_NODE (grava_node_get_type ())
#define GRAVA_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_NODE, GravaNode))
#define GRAVA_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_NODE, GravaNodeClass))
#define GRAVA_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_NODE))
#define GRAVA_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_NODE))
#define GRAVA_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_NODE, GravaNodeClass))

typedef struct _GravaNode GravaNode;
typedef struct _GravaNodeClass GravaNodeClass;
typedef struct _GravaNodePrivate GravaNodePrivate;

#define GRAVA_TYPE_SHAPE (grava_shape_get_type ())

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

typedef enum  {
	GRAVA_SHAPE_RECTANGLE = 0,
	GRAVA_SHAPE_CIRCLE
} GravaShape;



GType grava_node_get_type (void);
enum  {
	GRAVA_NODE_DUMMY_PROPERTY
};
static void _g_slist_free_g_free (GSList* self);
void grava_node_set (GravaNode* self, const char* key, const char* val);
void grava_node_set_i (GravaNode* self, const char* key, guint64 val);
char* grava_node_get (GravaNode* self, const char* key);
void grava_node_add_call (GravaNode* self, guint64 addr);
void grava_node_add_xref (GravaNode* self, guint64 addr);
gboolean grava_node_overlaps (GravaNode* self, GravaNode* n);
void grava_node_fit (GravaNode* self);
GravaNode* grava_node_new (void);
GravaNode* grava_node_construct (GType object_type);
GravaNode* grava_node_new (void);
static void _g_object_unref_gdestroy_notify (void* data);
GType grava_shape_get_type (void);
static GObject * grava_node_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_node_parent_class = NULL;
static void grava_node_finalize (GObject* obj);
static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func);
static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);
static gint _vala_array_length (gpointer array);



static void _g_slist_free_g_free (GSList* self) {
	g_slist_foreach (self, (GFunc) g_free, NULL);
	g_slist_free (self);
}


#line 50 "node.vala"
void grava_node_set (GravaNode* self, const char* key, const char* val) {
#line 103 "node.c"
	const char* _tmp1_;
	const char* _tmp0_;
#line 50 "node.vala"
	g_return_if_fail (self != NULL);
#line 50 "node.vala"
	g_return_if_fail (key != NULL);
#line 50 "node.vala"
	g_return_if_fail (val != NULL);
#line 52 "node.vala"
	_tmp1_ = NULL;
#line 52 "node.vala"
	_tmp0_ = NULL;
#line 52 "node.vala"
	g_hash_table_insert (self->data, (_tmp0_ = key, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)), (_tmp1_ = val, (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_)));
#line 118 "node.c"
}


#line 55 "node.vala"
void grava_node_set_i (GravaNode* self, const char* key, guint64 val) {
#line 124 "node.c"
	char* str;
	const char* _tmp1_;
	const char* _tmp0_;
#line 55 "node.vala"
	g_return_if_fail (self != NULL);
#line 55 "node.vala"
	g_return_if_fail (key != NULL);
#line 132 "node.c"
	str = g_strdup_printf ("0x%llx", val);
#line 58 "node.vala"
	_tmp1_ = NULL;
#line 58 "node.vala"
	_tmp0_ = NULL;
#line 58 "node.vala"
	g_hash_table_insert (self->data, (_tmp0_ = key, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)), (_tmp1_ = str, (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_)));
#line 140 "node.c"
	str = (g_free (str), NULL);
}


#line 61 "node.vala"
char* grava_node_get (GravaNode* self, const char* key) {
#line 147 "node.c"
	const char* _tmp0_;
#line 61 "node.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 61 "node.vala"
	g_return_val_if_fail (key != NULL, NULL);
#line 63 "node.vala"
	_tmp0_ = NULL;
#line 63 "node.vala"
	return (_tmp0_ = (const char*) g_hash_table_lookup (self->data, key), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 157 "node.c"
}


#line 66 "node.vala"
void grava_node_add_call (GravaNode* self, guint64 addr) {
#line 163 "node.c"
	char* str;
	const char* _tmp0_;
#line 66 "node.vala"
	g_return_if_fail (self != NULL);
#line 168 "node.c"
	str = g_strdup_printf ("0x%08llx", addr);
#line 69 "node.vala"
	_tmp0_ = NULL;
#line 69 "node.vala"
	self->calls = g_slist_append (self->calls, (_tmp0_ = str, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)));
#line 174 "node.c"
	str = (g_free (str), NULL);
}


#line 72 "node.vala"
void grava_node_add_xref (GravaNode* self, guint64 addr) {
#line 181 "node.c"
	char* str;
	const char* _tmp0_;
#line 72 "node.vala"
	g_return_if_fail (self != NULL);
#line 186 "node.c"
	str = g_strdup_printf ("0x%08llx", addr);
#line 75 "node.vala"
	_tmp0_ = NULL;
#line 75 "node.vala"
	self->xrefs = g_slist_append (self->xrefs, (_tmp0_ = str, (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_)));
#line 192 "node.c"
	str = (g_free (str), NULL);
}


#line 78 "node.vala"
gboolean grava_node_overlaps (GravaNode* self, GravaNode* n) {
#line 199 "node.c"
	gboolean _tmp0_ = {0};
	gboolean _tmp1_ = {0};
	gboolean _tmp2_ = {0};
#line 78 "node.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 78 "node.vala"
	g_return_val_if_fail (n != NULL, FALSE);
#line 80 "node.vala"
	if (n->x >= self->x) {
#line 80 "node.vala"
		_tmp2_ = n->x <= (self->x + self->w);
#line 211 "node.c"
	} else {
#line 80 "node.vala"
		_tmp2_ = FALSE;
#line 215 "node.c"
	}
#line 80 "node.vala"
	if (_tmp2_) {
#line 80 "node.vala"
		_tmp1_ = n->y <= self->y;
#line 221 "node.c"
	} else {
#line 80 "node.vala"
		_tmp1_ = FALSE;
#line 225 "node.c"
	}
#line 80 "node.vala"
	if (_tmp1_) {
#line 80 "node.vala"
		_tmp0_ = n->y <= (self->y + self->h);
#line 231 "node.c"
	} else {
#line 80 "node.vala"
		_tmp0_ = FALSE;
#line 235 "node.c"
	}
#line 80 "node.vala"
	return _tmp0_;
#line 239 "node.c"
}


#line 83 "node.vala"
void grava_node_fit (GravaNode* self) {
#line 245 "node.c"
	const char* _tmp0_;
	char* label;
	const char* _tmp1_;
	char* body;
	double _y;
	double _w;
	gboolean _tmp2_ = {0};
#line 83 "node.vala"
	g_return_if_fail (self != NULL);
#line 85 "node.vala"
	_tmp0_ = NULL;
#line 257 "node.c"
	label = (_tmp0_ = (const char*) g_hash_table_lookup (self->data, "label"), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 86 "node.vala"
	_tmp1_ = NULL;
#line 261 "node.c"
	body = (_tmp1_ = (const char*) g_hash_table_lookup (self->data, "body"), (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_));
	_y = (double) 25;
	_w = (double) 0;
#line 90 "node.vala"
	if (label != NULL) {
#line 91 "node.vala"
		_w = (double) (g_utf8_strlen (label, -1) + 2);
#line 269 "node.c"
	}
#line 93 "node.vala"
	if (self->has_body) {
#line 93 "node.vala"
		_tmp2_ = body != NULL;
#line 275 "node.c"
	} else {
#line 93 "node.vala"
		_tmp2_ = FALSE;
#line 279 "node.c"
	}
#line 93 "node.vala"
	if (_tmp2_) {
#line 283 "node.c"
		{
			char** _tmp3_;
			char** str_collection;
			int str_collection_length1;
			int str_it;
			_tmp3_ = NULL;
#line 94 "node.vala"
			str_collection = _tmp3_ = g_strsplit (body, "\n", 0);
#line 292 "node.c"
			str_collection_length1 = _vala_array_length (_tmp3_);
			for (str_it = 0; str_it < _vala_array_length (_tmp3_); str_it = str_it + 1) {
				const char* _tmp4_;
				char* str;
#line 822 "glib-2.0.vapi"
				_tmp4_ = NULL;
#line 299 "node.c"
				str = (_tmp4_ = str_collection[str_it], (_tmp4_ == NULL) ? NULL : g_strdup (_tmp4_));
				{
#line 95 "node.vala"
					_y = _y + ((double) 10);
#line 96 "node.vala"
					if (g_utf8_strlen (str, -1) > ((glong) _w)) {
#line 97 "node.vala"
						_w = (double) g_utf8_strlen (str, -1);
#line 308 "node.c"
					}
					str = (g_free (str), NULL);
				}
			}
#line 94 "node.vala"
			str_collection = (_vala_array_free (str_collection, str_collection_length1, (GDestroyNotify) g_free), NULL);
#line 315 "node.c"
		}
	}
#line 100 "node.vala"
	self->w = _w * 7;
#line 101 "node.vala"
	self->h = _y + 10;
#line 322 "node.c"
	label = (g_free (label), NULL);
	body = (g_free (body), NULL);
}


#line 21 "node.vala"
GravaNode* grava_node_construct (GType object_type) {
#line 330 "node.c"
	GravaNode * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 21 "node.vala"
GravaNode* grava_node_new (void) {
#line 21 "node.vala"
	return grava_node_construct (GRAVA_TYPE_NODE);
#line 341 "node.c"
}


#line 182 "gobject-2.0.vapi"
static void _g_object_unref_gdestroy_notify (void* data) {
#line 347 "node.c"
	g_object_unref (data);
}


static GObject * grava_node_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GravaNodeClass * klass;
	GObjectClass * parent_class;
	GravaNode * self;
	klass = GRAVA_NODE_CLASS (g_type_class_peek (GRAVA_TYPE_NODE));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GRAVA_NODE (obj);
	{
		GHashTable* _tmp0_;
		GSList* _tmp1_;
		GSList* _tmp2_;
		_tmp0_ = NULL;
#line 37 "node.vala"
		self->data = (_tmp0_ = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _g_object_unref_gdestroy_notify), (self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL)), _tmp0_);
#line 368 "node.c"
		_tmp1_ = NULL;
#line 38 "node.vala"
		self->calls = (_tmp1_ = NULL, (self->calls == NULL) ? NULL : (self->calls = (_g_slist_free_g_free (self->calls), NULL)), _tmp1_);
#line 372 "node.c"
		_tmp2_ = NULL;
#line 39 "node.vala"
		self->xrefs = (_tmp2_ = NULL, (self->xrefs == NULL) ? NULL : (self->xrefs = (_g_slist_free_g_free (self->xrefs), NULL)), _tmp2_);
#line 40 "node.vala"
		self->baseaddr = (guint) 0;
#line 41 "node.vala"
		self->x = self->y = (double) 0;
#line 42 "node.vala"
		self->w = (double) 150;
#line 43 "node.vala"
		self->h = (double) 200;
#line 44 "node.vala"
		self->shape = (gint) GRAVA_SHAPE_RECTANGLE;
#line 45 "node.vala"
		self->visible = TRUE;
#line 46 "node.vala"
		self->has_body = TRUE;
#line 47 "node.vala"
		self->selected = FALSE;
#line 392 "node.c"
	}
	return obj;
}


static void grava_node_class_init (GravaNodeClass * klass) {
	grava_node_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = grava_node_constructor;
	G_OBJECT_CLASS (klass)->finalize = grava_node_finalize;
}


static void grava_node_instance_init (GravaNode * self) {
}


static void grava_node_finalize (GObject* obj) {
	GravaNode * self;
	self = GRAVA_NODE (obj);
	(self->data == NULL) ? NULL : (self->data = (g_hash_table_unref (self->data), NULL));
	(self->calls == NULL) ? NULL : (self->calls = (_g_slist_free_g_free (self->calls), NULL));
	(self->xrefs == NULL) ? NULL : (self->xrefs = (_g_slist_free_g_free (self->xrefs), NULL));
	G_OBJECT_CLASS (grava_node_parent_class)->finalize (obj);
}


GType grava_node_get_type (void) {
	static GType grava_node_type_id = 0;
	if (grava_node_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaNodeClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_node_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaNode), 0, (GInstanceInitFunc) grava_node_instance_init, NULL };
		grava_node_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaNode", &g_define_type_info, 0);
	}
	return grava_node_type_id;
}


static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if ((array != NULL) && (destroy_func != NULL)) {
		int i;
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) array)[i] != NULL) {
				destroy_func (((gpointer*) array)[i]);
			}
		}
	}
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	_vala_array_destroy (array, array_length, destroy_func);
	g_free (array);
}


static gint _vala_array_length (gpointer array) {
	int length;
	length = 0;
	if (array) {
		while (((gpointer*) array)[length]) {
			length++;
		}
	}
	return length;
}




