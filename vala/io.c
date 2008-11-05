
#include "io.h"
#include <gobject/gvaluecollector.h>




enum  {
	RADARE_IO_DUMMY_PROPERTY
};
static gpointer radare_io_parent_class = NULL;
static void radare_io_finalize (RadareIO* obj);



#line 5 "io.vala"
gint radare_io_open (const char* file, gint flags, gint mode) {
#line 5 "io.vala"
	g_return_val_if_fail (file != NULL, 0);
#line 7 "io.vala"
	return io_open (file, flags, mode);
}


#line 10 "io.vala"
gint radare_io_close (gint fd) {
#line 12 "io.vala"
	return io_close (fd);
}


#line 15 "io.vala"
gint radare_io_read (gint fd, char** buf, glong size) {
	(*buf) = NULL;
#line 17 "io.vala"
	return io_read (fd, &(*buf), size);
}


#line 20 "io.vala"
gint radare_io_write (gint fd, const char* buf, glong size) {
#line 20 "io.vala"
	g_return_val_if_fail (buf != NULL, 0);
#line 22 "io.vala"
	return io_write (fd, buf, size);
}


#line 25 "io.vala"
gint radare_io_system (const char* command) {
#line 25 "io.vala"
	g_return_val_if_fail (command != NULL, 0);
#line 27 "io.vala"
	return io_system (command);
}


#line 3 "io.vala"
RadareIO* radare_io_construct (GType object_type) {
	RadareIO* self;
	self = ((RadareIO*) (g_type_create_instance (object_type)));
	return self;
}


#line 3 "io.vala"
RadareIO* radare_io_new (void) {
#line 3 "io.vala"
	return radare_io_construct (RADARE_TYPE_IO);
}


static void radare_value_io_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void radare_value_io_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		radare_io_unref (value->data[0].v_pointer);
	}
}


static void radare_value_io_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = radare_io_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer radare_value_io_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* radare_value_io_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		RadareIO* object;
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


static gchar* radare_value_io_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	RadareIO** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = radare_io_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* radare_param_spec_io (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	RadareParamSpecIO* spec;
	g_return_val_if_fail (g_type_is_a (object_type, RADARE_TYPE_IO), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer radare_value_get_io (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_IO), NULL);
	return value->data[0].v_pointer;
}


void radare_value_set_io (GValue* value, gpointer v_object) {
	RadareIO* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_IO));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, RADARE_TYPE_IO));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		radare_io_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		radare_io_unref (old);
	}
}


static void radare_io_class_init (RadareIOClass * klass) {
	radare_io_parent_class = g_type_class_peek_parent (klass);
	RADARE_IO_CLASS (klass)->finalize = radare_io_finalize;
}


static void radare_io_instance_init (RadareIO * self) {
	self->ref_count = 1;
}


static void radare_io_finalize (RadareIO* obj) {
	RadareIO * self;
	self = RADARE_IO (obj);
}


GType radare_io_get_type (void) {
	static GType radare_io_type_id = 0;
	if (radare_io_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { radare_value_io_init, radare_value_io_free_value, radare_value_io_copy_value, radare_value_io_peek_pointer, "p", radare_value_io_collect_value, "p", radare_value_io_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (RadareIOClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_io_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareIO), 0, (GInstanceInitFunc) radare_io_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		radare_io_type_id = g_type_register_fundamental (g_type_fundamental_next (), "RadareIO", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return radare_io_type_id;
}


gpointer radare_io_ref (gpointer instance) {
	RadareIO* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void radare_io_unref (gpointer instance) {
	RadareIO* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		RADARE_IO_GET_CLASS (self)->finalize (self);
		g_type_free_instance (((GTypeInstance *) (self)));
	}
}




