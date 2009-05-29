
#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <gobject/gvaluecollector.h>


#define RADARE_TYPE_IN_OUT (radare_in_out_get_type ())
#define RADARE_IN_OUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_IN_OUT, RadareInOut))
#define RADARE_IN_OUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_IN_OUT, RadareInOutClass))
#define RADARE_IS_IN_OUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_IN_OUT))
#define RADARE_IS_IN_OUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_IN_OUT))
#define RADARE_IN_OUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_IN_OUT, RadareInOutClass))

typedef struct _RadareInOut RadareInOut;
typedef struct _RadareInOutClass RadareInOutClass;
typedef struct _RadareInOutPrivate RadareInOutPrivate;
typedef struct _RadareParamSpecInOut RadareParamSpecInOut;

struct _RadareInOut {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareInOutPrivate * priv;
};

struct _RadareInOutClass {
	GTypeClass parent_class;
	void (*finalize) (RadareInOut *self);
};

struct _RadareParamSpecInOut {
	GParamSpec parent_instance;
};



gpointer radare_in_out_ref (gpointer instance);
void radare_in_out_unref (gpointer instance);
GParamSpec* radare_param_spec_in_out (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
void radare_value_set_in_out (GValue* value, gpointer v_object);
gpointer radare_value_get_in_out (const GValue* value);
GType radare_in_out_get_type (void);
enum  {
	RADARE_IN_OUT_DUMMY_PROPERTY
};
gint io_open (const char* file, gint flags, gint mode);
gint radare_in_out_open (const char* file, gint flags, gint mode);
gint io_close (gint fd);
gint radare_in_out_close (gint fd);
gint io_read (gint fd, char** buf, glong size);
gint radare_in_out_read (gint fd, char** buf, glong size);
gint io_write (gint fd, const char* buf, glong size);
gint radare_in_out_write (gint fd, const char* buf, glong size);
gint io_system (const char* command);
gint radare_in_out_system (const char* command);
RadareInOut* radare_in_out_new (void);
RadareInOut* radare_in_out_construct (GType object_type);
RadareInOut* radare_in_out_new (void);
static gpointer radare_in_out_parent_class = NULL;
static void radare_in_out_finalize (RadareInOut* obj);



gint radare_in_out_open (const char* file, gint flags, gint mode) {
	g_return_val_if_fail (file != NULL, 0);
	return io_open (file, flags, mode);
}


gint radare_in_out_close (gint fd) {
	return io_close (fd);
}


gint radare_in_out_read (gint fd, char** buf, glong size) {
	*buf = NULL;
	return io_read (fd, &(*buf), size);
}


gint radare_in_out_write (gint fd, const char* buf, glong size) {
	g_return_val_if_fail (buf != NULL, 0);
	return io_write (fd, buf, size);
}


gint radare_in_out_system (const char* command) {
	g_return_val_if_fail (command != NULL, 0);
	return io_system (command);
}


RadareInOut* radare_in_out_construct (GType object_type) {
	RadareInOut* self;
	self = (RadareInOut*) g_type_create_instance (object_type);
	return self;
}


RadareInOut* radare_in_out_new (void) {
	return radare_in_out_construct (RADARE_TYPE_IN_OUT);
}


static void radare_value_in_out_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void radare_value_in_out_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		radare_in_out_unref (value->data[0].v_pointer);
	}
}


static void radare_value_in_out_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = radare_in_out_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer radare_value_in_out_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* radare_value_in_out_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		RadareInOut* object;
		object = collect_values[0].v_pointer;
		if (object->parent_instance.g_class == NULL) {
			return g_strconcat ("invalid unclassed object pointer for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		} else if (!g_value_type_compatible (G_TYPE_FROM_INSTANCE (object), G_VALUE_TYPE (value))) {
			return g_strconcat ("invalid object type `", g_type_name (G_TYPE_FROM_INSTANCE (object)), "' for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		}
		value->data[0].v_pointer = radare_in_out_ref (object);
	} else {
		value->data[0].v_pointer = NULL;
	}
	return NULL;
}


static gchar* radare_value_in_out_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	RadareInOut** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
		return g_strdup_printf ("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME (value));
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = radare_in_out_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* radare_param_spec_in_out (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	RadareParamSpecInOut* spec;
	g_return_val_if_fail (g_type_is_a (object_type, RADARE_TYPE_IN_OUT), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer radare_value_get_in_out (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_IN_OUT), NULL);
	return value->data[0].v_pointer;
}


void radare_value_set_in_out (GValue* value, gpointer v_object) {
	RadareInOut* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_IN_OUT));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, RADARE_TYPE_IN_OUT));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		radare_in_out_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		radare_in_out_unref (old);
	}
}


static void radare_in_out_class_init (RadareInOutClass * klass) {
	radare_in_out_parent_class = g_type_class_peek_parent (klass);
	RADARE_IN_OUT_CLASS (klass)->finalize = radare_in_out_finalize;
}


static void radare_in_out_instance_init (RadareInOut * self) {
	self->ref_count = 1;
}


static void radare_in_out_finalize (RadareInOut* obj) {
	RadareInOut * self;
	self = RADARE_IN_OUT (obj);
}


GType radare_in_out_get_type (void) {
	static GType radare_in_out_type_id = 0;
	if (radare_in_out_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { radare_value_in_out_init, radare_value_in_out_free_value, radare_value_in_out_copy_value, radare_value_in_out_peek_pointer, "p", radare_value_in_out_collect_value, "p", radare_value_in_out_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (RadareInOutClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_in_out_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareInOut), 0, (GInstanceInitFunc) radare_in_out_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		radare_in_out_type_id = g_type_register_fundamental (g_type_fundamental_next (), "RadareInOut", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return radare_in_out_type_id;
}


gpointer radare_in_out_ref (gpointer instance) {
	RadareInOut* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void radare_in_out_unref (gpointer instance) {
	RadareInOut* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		RADARE_IN_OUT_GET_CLASS (self)->finalize (self);
		g_type_free_instance ((GTypeInstance *) self);
	}
}




