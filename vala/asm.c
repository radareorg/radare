
#include "asm.h"
#include "utils.h"
#include "core.h"
#include <gobject/gvaluecollector.h>




enum  {
	RADARE_ASM_DUMMY_PROPERTY
};
static gpointer radare_asm_parent_class = NULL;
static void radare_asm_finalize (RadareAsm* obj);
static int _vala_strcmp0 (const char * str1, const char * str2);



#line 5 "asm.vala"
gboolean radare_asm_setArchitecture (const char* arch) {
#line 5 "asm.vala"
	g_return_val_if_fail (arch != NULL, FALSE);
#line 7 "asm.vala"
	if (_vala_strcmp0 (arch, "arm") == 0) {
#line 8 "asm.vala"
		radare_utils_set ("ARCH", "arm");
	} else {
#line 10 "asm.vala"
		if (_vala_strcmp0 (arch, "intel") == 0) {
#line 11 "asm.vala"
			radare_utils_set ("ARCH", "intel");
		} else {
#line 13 "asm.vala"
			return FALSE;
		}
	}
#line 14 "asm.vala"
	return TRUE;
}


#line 17 "asm.vala"
char* radare_asm_getArchitecture (void) {
#line 19 "asm.vala"
	return radare_utils_get ("ARCH");
}


#line 22 "asm.vala"
char* radare_asm_asm (const char* code) {
	char* _tmp1;
	char* _tmp0;
	char* _tmp2;
#line 22 "asm.vala"
	g_return_val_if_fail (code != NULL, NULL);
	_tmp1 = NULL;
	_tmp0 = NULL;
#line 24 "asm.vala"
	_tmp2 = NULL;
#line 24 "asm.vala"
	return (_tmp2 = radare_core_command ((_tmp1 = g_strconcat ((_tmp0 = g_strconcat ("!rsc asm '", code, NULL)), "'", NULL))), (_tmp1 = (g_free (_tmp1), NULL)), (_tmp0 = (g_free (_tmp0), NULL)), _tmp2);
}


#line 27 "asm.vala"
char* radare_asm_dasm (const char* hexpairs) {
	char* _tmp1;
	char* _tmp0;
	char* _tmp2;
#line 27 "asm.vala"
	g_return_val_if_fail (hexpairs != NULL, NULL);
	_tmp1 = NULL;
	_tmp0 = NULL;
#line 29 "asm.vala"
	_tmp2 = NULL;
#line 29 "asm.vala"
	return (_tmp2 = radare_core_command ((_tmp1 = g_strconcat ((_tmp0 = g_strconcat ("!rsc dasm '", hexpairs, NULL)), "'", NULL))), (_tmp1 = (g_free (_tmp1), NULL)), (_tmp0 = (g_free (_tmp0), NULL)), _tmp2);
}


#line 3 "asm.vala"
RadareAsm* radare_asm_construct (GType object_type) {
	RadareAsm* self;
	self = ((RadareAsm*) (g_type_create_instance (object_type)));
	return self;
}


#line 3 "asm.vala"
RadareAsm* radare_asm_new (void) {
#line 3 "asm.vala"
	return radare_asm_construct (RADARE_TYPE_ASM);
}


static void radare_value_asm_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void radare_value_asm_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		radare_asm_unref (value->data[0].v_pointer);
	}
}


static void radare_value_asm_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = radare_asm_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer radare_value_asm_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* radare_value_asm_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		RadareAsm* object;
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


static gchar* radare_value_asm_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	RadareAsm** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = radare_asm_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* radare_param_spec_asm (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	RadareParamSpecAsm* spec;
	g_return_val_if_fail (g_type_is_a (object_type, RADARE_TYPE_ASM), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer radare_value_get_asm (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_ASM), NULL);
	return value->data[0].v_pointer;
}


void radare_value_set_asm (GValue* value, gpointer v_object) {
	RadareAsm* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, RADARE_TYPE_ASM));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, RADARE_TYPE_ASM));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		radare_asm_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		radare_asm_unref (old);
	}
}


static void radare_asm_class_init (RadareAsmClass * klass) {
	radare_asm_parent_class = g_type_class_peek_parent (klass);
	RADARE_ASM_CLASS (klass)->finalize = radare_asm_finalize;
}


static void radare_asm_instance_init (RadareAsm * self) {
	self->ref_count = 1;
}


static void radare_asm_finalize (RadareAsm* obj) {
	RadareAsm * self;
	self = RADARE_ASM (obj);
}


GType radare_asm_get_type (void) {
	static GType radare_asm_type_id = 0;
	if (radare_asm_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { radare_value_asm_init, radare_value_asm_free_value, radare_value_asm_copy_value, radare_value_asm_peek_pointer, "p", radare_value_asm_collect_value, "p", radare_value_asm_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (RadareAsmClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_asm_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareAsm), 0, (GInstanceInitFunc) radare_asm_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		radare_asm_type_id = g_type_register_fundamental (g_type_fundamental_next (), "RadareAsm", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return radare_asm_type_id;
}


gpointer radare_asm_ref (gpointer instance) {
	RadareAsm* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void radare_asm_unref (gpointer instance) {
	RadareAsm* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		RADARE_ASM_GET_CLASS (self)->finalize (self);
		g_type_free_instance (((GTypeInstance *) (self)));
	}
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return (str1 != str2);
	}
	return strcmp (str1, str2);
}




