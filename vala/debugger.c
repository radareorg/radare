
#include "debugger.h"
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include <gobject/gvaluecollector.h>




enum  {
	RADARE_DEBUGGER_DUMMY_PROPERTY
};
static gpointer radare_debugger_parent_class = NULL;
static void radare_debugger_finalize (RadareDebugger* obj);



#line 3 "debugger.vala"
void radare_debugger_step (void) {
#line 5 "debugger.vala"
	radare_core_cmd ("!step");
}


#line 8 "debugger.vala"
void radare_debugger_step_over (void) {
#line 10 "debugger.vala"
	radare_core_cmd ("!stepo");
}


#line 13 "debugger.vala"
void radare_debugger_play (void) {
#line 15 "debugger.vala"
	radare_core_cmd ("!cont");
}


#line 1 "debugger.vala"
RadareDebugger* radare_debugger_construct (GType object_type) {
	RadareDebugger* self;
	self = ((RadareDebugger*) (g_type_create_instance (object_type)));
	return self;
}


#line 1 "debugger.vala"
RadareDebugger* radare_debugger_new (void) {
#line 1 "debugger.vala"
	return radare_debugger_construct (RADARE_TYPE_DEBUGGER);
}


static void radare_value_debugger_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void radare_value_debugger_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		radare_debugger_unref (value->data[0].v_pointer);
	}
}


static void radare_value_debugger_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = radare_debugger_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer radare_value_debugger_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* radare_value_debugger_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		RadareDebugger* object;
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


static gchar* radare_value_debugger_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	RadareDebugger** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = radare_debugger_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* radare_param_spec_debugger (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	RadareParamSpecDebugger* spec;
	g_return_val_if_fail (g_type_is_a (object_type, RADARE_TYPE_DEBUGGER), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer radare_value_get_debugger (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_DEBUGGER), NULL);
	return value->data[0].v_pointer;
}


void radare_value_set_debugger (GValue* value, gpointer v_object) {
	RadareDebugger* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_DEBUGGER));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, RADARE_TYPE_DEBUGGER));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		radare_debugger_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		radare_debugger_unref (old);
	}
}


static void radare_debugger_class_init (RadareDebuggerClass * klass) {
	radare_debugger_parent_class = g_type_class_peek_parent (klass);
	RADARE_DEBUGGER_CLASS (klass)->finalize = radare_debugger_finalize;
}


static void radare_debugger_instance_init (RadareDebugger * self) {
	self->ref_count = 1;
}


static void radare_debugger_finalize (RadareDebugger* obj) {
	RadareDebugger * self;
	self = RADARE_DEBUGGER (obj);
}


GType radare_debugger_get_type (void) {
	static GType radare_debugger_type_id = 0;
	if (radare_debugger_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { radare_value_debugger_init, radare_value_debugger_free_value, radare_value_debugger_copy_value, radare_value_debugger_peek_pointer, "p", radare_value_debugger_collect_value, "p", radare_value_debugger_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (RadareDebuggerClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_debugger_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareDebugger), 0, (GInstanceInitFunc) radare_debugger_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		radare_debugger_type_id = g_type_register_fundamental (g_type_fundamental_next (), "RadareDebugger", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return radare_debugger_type_id;
}


gpointer radare_debugger_ref (gpointer instance) {
	RadareDebugger* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void radare_debugger_unref (gpointer instance) {
	RadareDebugger* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		RADARE_DEBUGGER_GET_CLASS (self)->finalize (self);
		g_type_free_instance (((GTypeInstance *) (self)));
	}
}




