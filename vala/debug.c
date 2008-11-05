
#include "debug.h"
#include "line.h"




struct _RadareGUIRadgetDebuggerPrivate {
	Line* file;
	Line* size;
};

#define RADARE_GUI_RADGET_DEBUGGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_RADGET_DEBUGGER, RadareGUIRadgetDebuggerPrivate))
enum  {
	RADARE_GUI_RADGET_DEBUGGER_DUMMY_PROPERTY,
	RADARE_GUI_RADGET_DEBUGGER_BOX,
	RADARE_GUI_RADGET_DEBUGGER_NAME
};
static void radare_gui_radget_debugger_real_refresh (RadareGUIRadget* base);
static GObject * radare_gui_radget_debugger_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_radget_debugger_parent_class = NULL;
static RadareGUIRadgetIface* radare_gui_radget_debugger_radare_gui_radget_parent_iface = NULL;
static void radare_gui_radget_debugger_finalize (GObject* obj);



#line 19 "debug.vala"
static void radare_gui_radget_debugger_real_refresh (RadareGUIRadget* base) {
	RadareGUIRadgetDebugger * self;
	self = ((RadareGUIRadgetDebugger*) (base));
}


#line 4 "debug.vala"
RadareGUIRadgetDebugger* radare_gui_radget_debugger_construct (GType object_type) {
	RadareGUIRadgetDebugger * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 4 "debug.vala"
RadareGUIRadgetDebugger* radare_gui_radget_debugger_new (void) {
#line 4 "debug.vala"
	return radare_gui_radget_debugger_construct (RADARE_GUI_TYPE_RADGET_DEBUGGER);
}


static GtkBox* radare_gui_radget_debugger_real_get_box (RadareGUIRadget* base) {
	RadareGUIRadgetDebugger* self;
	GtkBox* _tmp0;
	self = ((RadareGUIRadgetDebugger*) (base));
#line 9 "debug.vala"
	_tmp0 = NULL;
#line 9 "debug.vala"
	return (_tmp0 = ((GtkBox*) (self->vb)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


static char* radare_gui_radget_debugger_real_get_name (RadareGUIRadget* base) {
	RadareGUIRadgetDebugger* self;
	self = ((RadareGUIRadgetDebugger*) (base));
#line 11 "debug.vala"
	return g_strdup ("Debugger");
}


static GObject * radare_gui_radget_debugger_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIRadgetDebuggerClass * klass;
	GObjectClass * parent_class;
	RadareGUIRadgetDebugger * self;
	klass = RADARE_GUI_RADGET_DEBUGGER_CLASS (g_type_class_peek (RADARE_GUI_TYPE_RADGET_DEBUGGER));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_RADGET_DEBUGGER (obj);
	{
		GtkVBox* _tmp0;
		GtkLabel* _tmp1;
		_tmp0 = NULL;
#line 14 "debug.vala"
		self->vb = (_tmp0 = g_object_ref_sink (((GtkVBox*) (gtk_vbox_new (FALSE, 2)))), (self->vb == NULL ? NULL : (self->vb = (g_object_unref (self->vb), NULL))), _tmp0);
		_tmp1 = NULL;
#line 15 "debug.vala"
		gtk_container_add (((GtkContainer*) (self->vb)), ((GtkWidget*) ((_tmp1 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("(TODO)"))))))));
		(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
#line 16 "debug.vala"
		radare_gui_radget_refresh (((RadareGUIRadget*) (self)));
	}
	return obj;
}


static void radare_gui_radget_debugger_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	RadareGUIRadgetDebugger * self;
	self = RADARE_GUI_RADGET_DEBUGGER (object);
	switch (property_id) {
		case RADARE_GUI_RADGET_DEBUGGER_BOX:
		g_value_set_object (value, radare_gui_radget_get_box (((RadareGUIRadget*) (self))));
		break;
		case RADARE_GUI_RADGET_DEBUGGER_NAME:
		g_value_set_string (value, radare_gui_radget_get_name (((RadareGUIRadget*) (self))));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void radare_gui_radget_debugger_class_init (RadareGUIRadgetDebuggerClass * klass) {
	radare_gui_radget_debugger_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIRadgetDebuggerPrivate));
	G_OBJECT_CLASS (klass)->get_property = radare_gui_radget_debugger_get_property;
	G_OBJECT_CLASS (klass)->constructor = radare_gui_radget_debugger_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_radget_debugger_finalize;
	g_object_class_override_property (G_OBJECT_CLASS (klass), RADARE_GUI_RADGET_DEBUGGER_BOX, "box");
	g_object_class_override_property (G_OBJECT_CLASS (klass), RADARE_GUI_RADGET_DEBUGGER_NAME, "name");
}


static void radare_gui_radget_debugger_radare_gui_radget_interface_init (RadareGUIRadgetIface * iface) {
	radare_gui_radget_debugger_radare_gui_radget_parent_iface = g_type_interface_peek_parent (iface);
	iface->refresh = radare_gui_radget_debugger_real_refresh;
	iface->get_box = radare_gui_radget_debugger_real_get_box;
	iface->get_name = radare_gui_radget_debugger_real_get_name;
}


static void radare_gui_radget_debugger_instance_init (RadareGUIRadgetDebugger * self) {
	self->priv = RADARE_GUI_RADGET_DEBUGGER_GET_PRIVATE (self);
}


static void radare_gui_radget_debugger_finalize (GObject* obj) {
	RadareGUIRadgetDebugger * self;
	self = RADARE_GUI_RADGET_DEBUGGER (obj);
	(self->priv->file == NULL ? NULL : (self->priv->file = (g_object_unref (self->priv->file), NULL)));
	(self->priv->size == NULL ? NULL : (self->priv->size = (g_object_unref (self->priv->size), NULL)));
	(self->vb == NULL ? NULL : (self->vb = (g_object_unref (self->vb), NULL)));
	G_OBJECT_CLASS (radare_gui_radget_debugger_parent_class)->finalize (obj);
}


GType radare_gui_radget_debugger_get_type (void) {
	static GType radare_gui_radget_debugger_type_id = 0;
	if (radare_gui_radget_debugger_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIRadgetDebuggerClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_radget_debugger_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIRadgetDebugger), 0, (GInstanceInitFunc) radare_gui_radget_debugger_instance_init, NULL };
		static const GInterfaceInfo radare_gui_radget_info = { (GInterfaceInitFunc) radare_gui_radget_debugger_radare_gui_radget_interface_init, (GInterfaceFinalizeFunc) NULL, NULL};
		radare_gui_radget_debugger_type_id = g_type_register_static (G_TYPE_OBJECT, "RadareGUIRadgetDebugger", &g_define_type_info, 0);
		g_type_add_interface_static (radare_gui_radget_debugger_type_id, RADARE_GUI_TYPE_RADGET, &radare_gui_radget_info);
	}
	return radare_gui_radget_debugger_type_id;
}




