
#include "radget.h"







#line 8 "radget.vala"
void radare_gui_radget_refresh (RadareGUIRadget* self) {
#line 8 "radget.vala"
	RADARE_GUI_RADGET_GET_INTERFACE (self)->refresh (self);
}


char* radare_gui_radget_get_name (RadareGUIRadget* self) {
	return RADARE_GUI_RADGET_GET_INTERFACE (self)->get_name (self);
}


GtkBox* radare_gui_radget_get_box (RadareGUIRadget* self) {
	return RADARE_GUI_RADGET_GET_INTERFACE (self)->get_box (self);
}


static void radare_gui_radget_base_init (RadareGUIRadgetIface * iface) {
	static gboolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		g_object_interface_install_property (iface, g_param_spec_string ("name", "name", "name", NULL, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
		g_object_interface_install_property (iface, g_param_spec_object ("box", "box", "box", GTK_TYPE_BOX, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
	}
}


GType radare_gui_radget_get_type (void) {
	static GType radare_gui_radget_type_id = 0;
	if (radare_gui_radget_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIRadgetIface), (GBaseInitFunc) radare_gui_radget_base_init, (GBaseFinalizeFunc) NULL, (GClassInitFunc) NULL, (GClassFinalizeFunc) NULL, NULL, 0, 0, (GInstanceInitFunc) NULL, NULL };
		radare_gui_radget_type_id = g_type_register_static (G_TYPE_INTERFACE, "RadareGUIRadget", &g_define_type_info, 0);
		g_type_interface_add_prerequisite (radare_gui_radget_type_id, G_TYPE_OBJECT);
	}
	return radare_gui_radget_type_id;
}




