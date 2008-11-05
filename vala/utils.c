
#include "utils.h"
#include <gobject/gvaluecollector.h>




enum  {
	RADARE_UTILS_DUMMY_PROPERTY
};
static gpointer radare_utils_parent_class = NULL;
static void radare_utils_finalize (RadareUtils* obj);



#line 5 "utils.vala"
gulong radare_utils_value (const char* str) {
#line 5 "utils.vala"
	g_return_val_if_fail (str != NULL, 0UL);
#line 7 "utils.vala"
	return get_offset (str);
}


#line 10 "utils.vala"
gulong radare_utils_math (const char* str) {
#line 10 "utils.vala"
	g_return_val_if_fail (str != NULL, 0UL);
#line 12 "utils.vala"
	return get_math (str);
}


#line 15 "utils.vala"
char* radare_utils_get (const char* str) {
#line 15 "utils.vala"
	g_return_val_if_fail (str != NULL, NULL);
#line 17 "utils.vala"
	return getenv (str);
}


#line 20 "utils.vala"
gint radare_utils_set (const char* foo, const char* bar) {
#line 20 "utils.vala"
	g_return_val_if_fail (foo != NULL, 0);
#line 20 "utils.vala"
	g_return_val_if_fail (bar != NULL, 0);
#line 22 "utils.vala"
	return setenv (foo, bar, 1);
}


#line 3 "utils.vala"
RadareUtils* radare_utils_construct (GType object_type) {
	RadareUtils* self;
	self = ((RadareUtils*) (g_type_create_instance (object_type)));
	return self;
}


#line 3 "utils.vala"
RadareUtils* radare_utils_new (void) {
#line 3 "utils.vala"
	return radare_utils_construct (RADARE_TYPE_UTILS);
}


static void radare_value_utils_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void radare_value_utils_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		radare_utils_unref (value->data[0].v_pointer);
	}
}


static void radare_value_utils_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = radare_utils_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer radare_value_utils_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* radare_value_utils_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		RadareUtils* object;
		object = value->data[0].v_pointer;
		if (object->parent_instance.g_class == NULL) {
			return g_strconcat ("invalid unclassed object pointer for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		} else if (!g_value_type_compatible (G_OBJECT_TYPE (object), G_VALUE_TYPE (value))) {
			return g_strconcat ("invalid object type `", G_OBJECT_TYPE (object), "' for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		}
	} else {
		value->data[0].v_pointer = NULL;
	}
	return NULL;
}


static gchar* radare_value_utils_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	RadareUtils** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = radare_utils_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* radare_param_spec_utils (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	RadareParamSpecUtils* spec;
	g_return_val_if_fail (g_type_is_a (object_type, RADARE_TYPE_UTILS), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer radare_value_get_utils (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_UTILS), NULL);
	return value->data[0].v_pointer;
}


void radare_value_set_utils (GValue* value, gpointer v_object) {
	RadareUtils* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_UTILS));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, RADARE_TYPE_UTILS));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		radare_utils_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		radare_utils_unref (old);
	}
}


static void radare_utils_class_init (RadareUtilsClass * klass) {
	radare_utils_parent_class = g_type_class_peek_parent (klass);
	RADARE_UTILS_CLASS (klass)->finalize = radare_utils_finalize;
}


static void radare_utils_instance_init (RadareUtils * self) {
	self->ref_count = 1;
}


static void radare_utils_finalize (RadareUtils* obj) {
	RadareUtils * self;
	self = RADARE_UTILS (obj);
}


GType radare_utils_get_type (void) {
	static GType radare_utils_type_id = 0;
	if (radare_utils_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { radare_value_utils_init, radare_value_utils_free_value, radare_value_utils_copy_value, radare_value_utils_peek_pointer, "p", radare_value_utils_collect_value, "p", radare_value_utils_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (RadareUtilsClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_utils_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareUtils), 0, (GInstanceInitFunc) radare_utils_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		radare_utils_type_id = g_type_register_fundamental (g_type_fundamental_next (), "RadareUtils", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return radare_utils_type_id;
}


gpointer radare_utils_ref (gpointer instance) {
	RadareUtils* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void radare_utils_unref (gpointer instance) {
	RadareUtils* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		RADARE_UTILS_GET_CLASS (self)->finalize (self);
		g_type_free_instance (((GTypeInstance *) (self)));
	}
}




