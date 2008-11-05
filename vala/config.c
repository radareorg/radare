
#include "config.h"




enum  {
	RADARE_CONFIG_DUMMY_PROPERTY
};
static GObject * radare_config_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_config_parent_class = NULL;



/* must return value*/
#line 10 "config.vala"
void radare_config_set (RadareConfig* self, const char* key, const char* val) {
#line 10 "config.vala"
	g_return_if_fail (self != NULL);
#line 10 "config.vala"
	g_return_if_fail (key != NULL);
#line 10 "config.vala"
	g_return_if_fail (val != NULL);
#line 12 "config.vala"
	config_set (key, val);
}


#line 15 "config.vala"
void radare_config_set_i (RadareConfig* self, const char* key, glong i) {
#line 15 "config.vala"
	g_return_if_fail (self != NULL);
#line 15 "config.vala"
	g_return_if_fail (key != NULL);
#line 17 "config.vala"
	config_set_i (key, i);
}


#line 20 "config.vala"
char* radare_config_get (RadareConfig* self, const char* key) {
#line 20 "config.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 20 "config.vala"
	g_return_val_if_fail (key != NULL, NULL);
#line 22 "config.vala"
	return config_get (key);
}


#line 25 "config.vala"
glong radare_config_get_i (RadareConfig* self, const char* key) {
#line 25 "config.vala"
	g_return_val_if_fail (self != NULL, 0L);
#line 25 "config.vala"
	g_return_val_if_fail (key != NULL, 0L);
#line 27 "config.vala"
	return config_get_i (key);
}


#line 30 "config.vala"
gboolean radare_config_rm (RadareConfig* self, const char* key) {
#line 30 "config.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 30 "config.vala"
	g_return_val_if_fail (key != NULL, FALSE);
#line 32 "config.vala"
	return config_rm (key);
}


#line 35 "config.vala"
void radare_config_list (RadareConfig* self, const char* mask) {
#line 35 "config.vala"
	g_return_if_fail (self != NULL);
#line 35 "config.vala"
	g_return_if_fail (mask != NULL);
#line 37 "config.vala"
	config_list (mask);
}


#line 3 "config.vala"
RadareConfig* radare_config_construct (GType object_type) {
	RadareConfig * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 3 "config.vala"
RadareConfig* radare_config_new (void) {
#line 3 "config.vala"
	return radare_config_construct (RADARE_TYPE_CONFIG);
}


static GObject * radare_config_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareConfigClass * klass;
	GObjectClass * parent_class;
	RadareConfig * self;
	klass = RADARE_CONFIG_CLASS (g_type_class_peek (RADARE_TYPE_CONFIG));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_CONFIG (obj);
	{
#line 6 "config.vala"
		config_init ();
	}
	return obj;
}


static void radare_config_class_init (RadareConfigClass * klass) {
	radare_config_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = radare_config_constructor;
}


static void radare_config_instance_init (RadareConfig * self) {
}


GType radare_config_get_type (void) {
	static GType radare_config_type_id = 0;
	if (radare_config_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareConfigClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_config_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareConfig), 0, (GInstanceInitFunc) radare_config_instance_init, NULL };
		radare_config_type_id = g_type_register_static (G_TYPE_OBJECT, "RadareConfig", &g_define_type_info, 0);
	}
	return radare_config_type_id;
}




