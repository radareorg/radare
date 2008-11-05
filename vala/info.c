
#include "info.h"
#include "line.h"




struct _RadareGUIRadgetInformationPrivate {
	Line* file;
	Line* size;
	GtkVBox* vb;
};

#define RADARE_GUI_RADGET_INFORMATION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_RADGET_INFORMATION, RadareGUIRadgetInformationPrivate))
enum  {
	RADARE_GUI_RADGET_INFORMATION_DUMMY_PROPERTY,
	RADARE_GUI_RADGET_INFORMATION_BOX,
	RADARE_GUI_RADGET_INFORMATION_NAME
};
static void radare_gui_radget_information_real_refresh (RadareGUIRadget* base);
static GObject * radare_gui_radget_information_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_radget_information_parent_class = NULL;
static RadareGUIRadgetIface* radare_gui_radget_information_radare_gui_radget_parent_iface = NULL;
static void radare_gui_radget_information_finalize (GObject* obj);



#line 22 "info.vala"
static void radare_gui_radget_information_real_refresh (RadareGUIRadget* base) {
	RadareGUIRadgetInformation * self;
	self = ((RadareGUIRadgetInformation*) (base));
}


#line 4 "info.vala"
RadareGUIRadgetInformation* radare_gui_radget_information_construct (GType object_type) {
	RadareGUIRadgetInformation * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 4 "info.vala"
RadareGUIRadgetInformation* radare_gui_radget_information_new (void) {
#line 4 "info.vala"
	return radare_gui_radget_information_construct (RADARE_GUI_TYPE_RADGET_INFORMATION);
}


static GtkBox* radare_gui_radget_information_real_get_box (RadareGUIRadget* base) {
	RadareGUIRadgetInformation* self;
	GtkBox* _tmp0;
	self = ((RadareGUIRadgetInformation*) (base));
#line 10 "info.vala"
	_tmp0 = NULL;
#line 10 "info.vala"
	return (_tmp0 = ((GtkBox*) (self->priv->vb)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


static char* radare_gui_radget_information_real_get_name (RadareGUIRadget* base) {
	RadareGUIRadgetInformation* self;
	self = ((RadareGUIRadgetInformation*) (base));
#line 11 "info.vala"
	return g_strdup ("Information");
}


static GObject * radare_gui_radget_information_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIRadgetInformationClass * klass;
	GObjectClass * parent_class;
	RadareGUIRadgetInformation * self;
	klass = RADARE_GUI_RADGET_INFORMATION_CLASS (g_type_class_peek (RADARE_GUI_TYPE_RADGET_INFORMATION));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_RADGET_INFORMATION (obj);
	{
		GtkVBox* _tmp0;
		Line* _tmp2;
		Line* _tmp1;
		Line* _tmp4;
		Line* _tmp3;
		_tmp0 = NULL;
#line 14 "info.vala"
		self->priv->vb = (_tmp0 = g_object_ref_sink (((GtkVBox*) (gtk_vbox_new (FALSE, 2)))), (self->priv->vb == NULL ? NULL : (self->priv->vb = (g_object_unref (self->priv->vb), NULL))), _tmp0);
		_tmp2 = NULL;
		_tmp1 = NULL;
#line 15 "info.vala"
		self->priv->file = (_tmp2 = line_name ((_tmp1 = g_object_ref_sink (line_new ())), "File", "/bin/ls"), (self->priv->file == NULL ? NULL : (self->priv->file = (g_object_unref (self->priv->file), NULL))), _tmp2);
		(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
#line 16 "info.vala"
		gtk_container_add (((GtkContainer*) (self->priv->vb)), ((GtkWidget*) (self->priv->file)));
		_tmp4 = NULL;
		_tmp3 = NULL;
#line 17 "info.vala"
		self->priv->size = (_tmp4 = line_numeric ((_tmp3 = g_object_ref_sink (line_new ())), "Size", 38432), (self->priv->size == NULL ? NULL : (self->priv->size = (g_object_unref (self->priv->size), NULL))), _tmp4);
		(_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref (_tmp3), NULL)));
#line 18 "info.vala"
		gtk_container_add (((GtkContainer*) (self->priv->vb)), ((GtkWidget*) (self->priv->size)));
#line 19 "info.vala"
		radare_gui_radget_refresh (((RadareGUIRadget*) (self)));
	}
	return obj;
}


static void radare_gui_radget_information_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	RadareGUIRadgetInformation * self;
	self = RADARE_GUI_RADGET_INFORMATION (object);
	switch (property_id) {
		case RADARE_GUI_RADGET_INFORMATION_BOX:
		g_value_set_object (value, radare_gui_radget_get_box (((RadareGUIRadget*) (self))));
		break;
		case RADARE_GUI_RADGET_INFORMATION_NAME:
		g_value_set_string (value, radare_gui_radget_get_name (((RadareGUIRadget*) (self))));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void radare_gui_radget_information_class_init (RadareGUIRadgetInformationClass * klass) {
	radare_gui_radget_information_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIRadgetInformationPrivate));
	G_OBJECT_CLASS (klass)->get_property = radare_gui_radget_information_get_property;
	G_OBJECT_CLASS (klass)->constructor = radare_gui_radget_information_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_radget_information_finalize;
	g_object_class_override_property (G_OBJECT_CLASS (klass), RADARE_GUI_RADGET_INFORMATION_BOX, "box");
	g_object_class_override_property (G_OBJECT_CLASS (klass), RADARE_GUI_RADGET_INFORMATION_NAME, "name");
}


static void radare_gui_radget_information_radare_gui_radget_interface_init (RadareGUIRadgetIface * iface) {
	radare_gui_radget_information_radare_gui_radget_parent_iface = g_type_interface_peek_parent (iface);
	iface->refresh = radare_gui_radget_information_real_refresh;
	iface->get_box = radare_gui_radget_information_real_get_box;
	iface->get_name = radare_gui_radget_information_real_get_name;
}


static void radare_gui_radget_information_instance_init (RadareGUIRadgetInformation * self) {
	self->priv = RADARE_GUI_RADGET_INFORMATION_GET_PRIVATE (self);
}


static void radare_gui_radget_information_finalize (GObject* obj) {
	RadareGUIRadgetInformation * self;
	self = RADARE_GUI_RADGET_INFORMATION (obj);
	(self->priv->file == NULL ? NULL : (self->priv->file = (g_object_unref (self->priv->file), NULL)));
	(self->priv->size == NULL ? NULL : (self->priv->size = (g_object_unref (self->priv->size), NULL)));
	(self->priv->vb == NULL ? NULL : (self->priv->vb = (g_object_unref (self->priv->vb), NULL)));
	G_OBJECT_CLASS (radare_gui_radget_information_parent_class)->finalize (obj);
}


GType radare_gui_radget_information_get_type (void) {
	static GType radare_gui_radget_information_type_id = 0;
	if (radare_gui_radget_information_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIRadgetInformationClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_radget_information_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIRadgetInformation), 0, (GInstanceInitFunc) radare_gui_radget_information_instance_init, NULL };
		static const GInterfaceInfo radare_gui_radget_info = { (GInterfaceInitFunc) radare_gui_radget_information_radare_gui_radget_interface_init, (GInterfaceFinalizeFunc) NULL, NULL};
		radare_gui_radget_information_type_id = g_type_register_static (G_TYPE_OBJECT, "RadareGUIRadgetInformation", &g_define_type_info, 0);
		g_type_add_interface_static (radare_gui_radget_information_type_id, RADARE_GUI_TYPE_RADGET, &radare_gui_radget_info);
	}
	return radare_gui_radget_information_type_id;
}




