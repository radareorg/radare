
#include "project.h"
#include <gobject/gvaluecollector.h>




enum  {
	RADARE_PROJECT_DUMMY_PROPERTY
};
static gpointer radare_project_parent_class = NULL;
static void radare_project_finalize (RadareProject* obj);



/* must return value*/
#line 4 "project.vala"
gboolean radare_project_open (const char* file) {
#line 4 "project.vala"
	g_return_val_if_fail (file != NULL, FALSE);
#line 6 "project.vala"
	return project_open (file);
}


#line 9 "project.vala"
gboolean radare_project_save (const char* file) {
#line 9 "project.vala"
	g_return_val_if_fail (file != NULL, FALSE);
#line 11 "project.vala"
	return project_save (file);
}


#line 14 "project.vala"
void radare_project_info (const char* file) {
#line 14 "project.vala"
	g_return_if_fail (file != NULL);
#line 16 "project.vala"
	project_save (file);
}


#line 1 "project.vala"
RadareProject* radare_project_construct (GType object_type) {
	RadareProject* self;
	self = ((RadareProject*) (g_type_create_instance (object_type)));
	return self;
}


#line 1 "project.vala"
RadareProject* radare_project_new (void) {
#line 1 "project.vala"
	return radare_project_construct (RADARE_TYPE_PROJECT);
}


static void radare_value_project_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void radare_value_project_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		radare_project_unref (value->data[0].v_pointer);
	}
}


static void radare_value_project_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = radare_project_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer radare_value_project_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* radare_value_project_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		RadareProject* object;
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


static gchar* radare_value_project_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	RadareProject** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = radare_project_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* radare_param_spec_project (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	RadareParamSpecProject* spec;
	g_return_val_if_fail (g_type_is_a (object_type, RADARE_TYPE_PROJECT), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer radare_value_get_project (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_PROJECT), NULL);
	return value->data[0].v_pointer;
}


void radare_value_set_project (GValue* value, gpointer v_object) {
	RadareProject* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_PROJECT));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, RADARE_TYPE_PROJECT));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		radare_project_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		radare_project_unref (old);
	}
}


static void radare_project_class_init (RadareProjectClass * klass) {
	radare_project_parent_class = g_type_class_peek_parent (klass);
	RADARE_PROJECT_CLASS (klass)->finalize = radare_project_finalize;
}


static void radare_project_instance_init (RadareProject * self) {
	self->ref_count = 1;
}


static void radare_project_finalize (RadareProject* obj) {
	RadareProject * self;
	self = RADARE_PROJECT (obj);
}


GType radare_project_get_type (void) {
	static GType radare_project_type_id = 0;
	if (radare_project_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { radare_value_project_init, radare_value_project_free_value, radare_value_project_copy_value, radare_value_project_peek_pointer, "p", radare_value_project_collect_value, "p", radare_value_project_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (RadareProjectClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_project_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareProject), 0, (GInstanceInitFunc) radare_project_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		radare_project_type_id = g_type_register_fundamental (g_type_fundamental_next (), "RadareProject", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return radare_project_type_id;
}


gpointer radare_project_ref (gpointer instance) {
	RadareProject* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void radare_project_unref (gpointer instance) {
	RadareProject* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		RADARE_PROJECT_GET_CLASS (self)->finalize (self);
		g_type_free_instance (((GTypeInstance *) (self)));
	}
}




