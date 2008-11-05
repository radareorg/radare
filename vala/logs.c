
#include "logs.h"
#include <stdlib.h>
#include <string.h>




enum  {
	RADARE_GUI_LOGS_DUMMY_PROPERTY
};
static GObject * radare_gui_logs_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_logs_parent_class = NULL;



#line 17 "logs.vala"
GtkVBox* radare_gui_logs_get (RadareGUILogs* self) {
	GtkVBox* _tmp0;
#line 17 "logs.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 19 "logs.vala"
	_tmp0 = NULL;
#line 19 "logs.vala"
	return (_tmp0 = ((GtkVBox*) (self)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


#line 5 "logs.vala"
RadareGUILogs* radare_gui_logs_construct (GType object_type) {
	RadareGUILogs * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 5 "logs.vala"
RadareGUILogs* radare_gui_logs_new (void) {
#line 5 "logs.vala"
	return radare_gui_logs_construct (RADARE_GUI_TYPE_LOGS);
}


static GObject * radare_gui_logs_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUILogsClass * klass;
	GObjectClass * parent_class;
	RadareGUILogs * self;
	klass = RADARE_GUI_LOGS_CLASS (g_type_class_peek (RADARE_GUI_TYPE_LOGS));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_LOGS (obj);
	{
		GtkHBox* hb;
		GtkEntry* _tmp0;
		GtkButton* _tmp1;
		hb = g_object_ref_sink (((GtkHBox*) (gtk_hbox_new (FALSE, 3))));
		_tmp0 = NULL;
#line 11 "logs.vala"
		gtk_container_add (((GtkContainer*) (hb)), ((GtkWidget*) ((_tmp0 = g_object_ref_sink (((GtkEntry*) (gtk_entry_new ())))))));
		(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
		_tmp1 = NULL;
#line 12 "logs.vala"
		gtk_box_pack_start (((GtkBox*) (hb)), ((GtkWidget*) ((_tmp1 = g_object_ref_sink (((GtkButton*) (gtk_button_new_from_stock ("gtk-execute"))))))), FALSE, FALSE, ((guint) (0)));
		(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
#line 14 "logs.vala"
		gtk_box_pack_start (((GtkBox*) (self)), ((GtkWidget*) (hb)), FALSE, FALSE, ((guint) (3)));
		(hb == NULL ? NULL : (hb = (g_object_unref (hb), NULL)));
	}
	return obj;
}


static void radare_gui_logs_class_init (RadareGUILogsClass * klass) {
	radare_gui_logs_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = radare_gui_logs_constructor;
}


static void radare_gui_logs_instance_init (RadareGUILogs * self) {
}


GType radare_gui_logs_get_type (void) {
	static GType radare_gui_logs_type_id = 0;
	if (radare_gui_logs_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUILogsClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_logs_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUILogs), 0, (GInstanceInitFunc) radare_gui_logs_instance_init, NULL };
		radare_gui_logs_type_id = g_type_register_static (GTK_TYPE_VBOX, "RadareGUILogs", &g_define_type_info, 0);
	}
	return radare_gui_logs_type_id;
}




