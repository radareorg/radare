
#include "plugin.h"




enum  {
	RADARE_PLUGINS_DUMMY_PROPERTY
};
static gpointer radare_plugins_parent_class = NULL;



#line 12 "plugin.vala"
void radare_plugins_list (void) {
#line 14 "plugin.vala"
	plugin_list ();
}


/*
public interface Radare.Plugin
{
public abstract static void run(string arg);
}
*/
#line 10 "plugin.vala"
RadarePlugins* radare_plugins_construct (GType object_type) {
	RadarePlugins * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 10 "plugin.vala"
RadarePlugins* radare_plugins_new (void) {
#line 10 "plugin.vala"
	return radare_plugins_construct (RADARE_TYPE_PLUGINS);
}


static void radare_plugins_class_init (RadarePluginsClass * klass) {
	radare_plugins_parent_class = g_type_class_peek_parent (klass);
}


static void radare_plugins_instance_init (RadarePlugins * self) {
}


GType radare_plugins_get_type (void) {
	static GType radare_plugins_type_id = 0;
	if (radare_plugins_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadarePluginsClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_plugins_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadarePlugins), 0, (GInstanceInitFunc) radare_plugins_instance_init, NULL };
		radare_plugins_type_id = g_type_register_static (G_TYPE_OBJECT, "RadarePlugins", &g_define_type_info, 0);
	}
	return radare_plugins_type_id;
}




