
#include "search.h"
#include "line.h"




struct _RadareGUIRadgetSearchPrivate {
	Line* file;
};

#define RADARE_GUI_RADGET_SEARCH_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_RADGET_SEARCH, RadareGUIRadgetSearchPrivate))
enum  {
	RADARE_GUI_RADGET_SEARCH_DUMMY_PROPERTY,
	RADARE_GUI_RADGET_SEARCH_BOX,
	RADARE_GUI_RADGET_SEARCH_NAME
};
static void radare_gui_radget_search_real_refresh (RadareGUIRadget* base);
static GObject * radare_gui_radget_search_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_radget_search_parent_class = NULL;
static RadareGUIRadgetIface* radare_gui_radget_search_radare_gui_radget_parent_iface = NULL;
static void radare_gui_radget_search_finalize (GObject* obj);



#line 27 "search.vala"
static void radare_gui_radget_search_real_refresh (RadareGUIRadget* base) {
	RadareGUIRadgetSearch * self;
	self = ((RadareGUIRadgetSearch*) (base));
}


#line 4 "search.vala"
RadareGUIRadgetSearch* radare_gui_radget_search_construct (GType object_type) {
	RadareGUIRadgetSearch * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 4 "search.vala"
RadareGUIRadgetSearch* radare_gui_radget_search_new (void) {
#line 4 "search.vala"
	return radare_gui_radget_search_construct (RADARE_GUI_TYPE_RADGET_SEARCH);
}


static GtkBox* radare_gui_radget_search_real_get_box (RadareGUIRadget* base) {
	RadareGUIRadgetSearch* self;
	GtkBox* _tmp0;
	self = ((RadareGUIRadgetSearch*) (base));
#line 9 "search.vala"
	_tmp0 = NULL;
#line 9 "search.vala"
	return (_tmp0 = ((GtkBox*) (self->vb)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


static char* radare_gui_radget_search_real_get_name (RadareGUIRadget* base) {
	RadareGUIRadgetSearch* self;
	self = ((RadareGUIRadgetSearch*) (base));
#line 10 "search.vala"
	return g_strdup ("Search");
}


static GObject * radare_gui_radget_search_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIRadgetSearchClass * klass;
	GObjectClass * parent_class;
	RadareGUIRadgetSearch * self;
	klass = RADARE_GUI_RADGET_SEARCH_CLASS (g_type_class_peek (RADARE_GUI_TYPE_RADGET_SEARCH));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_RADGET_SEARCH (obj);
	{
		GtkVBox* _tmp0;
		Line* _tmp2;
		Line* _tmp1;
		Line* _tmp4;
		Line* _tmp3;
		GtkHButtonBox* hbb;
		GtkButton* _tmp5;
		_tmp0 = NULL;
#line 13 "search.vala"
		self->vb = (_tmp0 = g_object_ref_sink (((GtkVBox*) (gtk_vbox_new (FALSE, 2)))), (self->vb == NULL ? NULL : (self->vb = (g_object_unref (self->vb), NULL))), _tmp0);
		_tmp2 = NULL;
		_tmp1 = NULL;
#line 14 "search.vala"
		self->priv->file = (_tmp2 = line_name ((_tmp1 = g_object_ref_sink (line_new ())), "String", ""), (self->priv->file == NULL ? NULL : (self->priv->file = (g_object_unref (self->priv->file), NULL))), _tmp2);
		(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
#line 15 "search.vala"
		gtk_container_add (((GtkContainer*) (self->vb)), ((GtkWidget*) (self->priv->file)));
		_tmp4 = NULL;
		_tmp3 = NULL;
#line 16 "search.vala"
		self->priv->file = (_tmp4 = line_name ((_tmp3 = g_object_ref_sink (line_new ())), "HexPairs", ""), (self->priv->file == NULL ? NULL : (self->priv->file = (g_object_unref (self->priv->file), NULL))), _tmp4);
		(_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref (_tmp3), NULL)));
#line 17 "search.vala"
		gtk_container_add (((GtkContainer*) (self->vb)), ((GtkWidget*) (self->priv->file)));
		hbb = g_object_ref_sink (((GtkHButtonBox*) (gtk_hbutton_box_new ())));
#line 20 "search.vala"
		gtk_button_box_set_layout (((GtkButtonBox*) (hbb)), GTK_BUTTONBOX_END);
		_tmp5 = NULL;
#line 21 "search.vala"
		gtk_container_add (((GtkContainer*) (hbb)), ((GtkWidget*) ((_tmp5 = g_object_ref_sink (((GtkButton*) (gtk_button_new_from_stock ("gtk-find"))))))));
		(_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref (_tmp5), NULL)));
#line 22 "search.vala"
		gtk_container_add (((GtkContainer*) (self->vb)), ((GtkWidget*) (hbb)));
#line 24 "search.vala"
		radare_gui_radget_refresh (((RadareGUIRadget*) (self)));
		(hbb == NULL ? NULL : (hbb = (g_object_unref (hbb), NULL)));
	}
	return obj;
}


static void radare_gui_radget_search_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	RadareGUIRadgetSearch * self;
	self = RADARE_GUI_RADGET_SEARCH (object);
	switch (property_id) {
		case RADARE_GUI_RADGET_SEARCH_BOX:
		g_value_set_object (value, radare_gui_radget_get_box (((RadareGUIRadget*) (self))));
		break;
		case RADARE_GUI_RADGET_SEARCH_NAME:
		g_value_set_string (value, radare_gui_radget_get_name (((RadareGUIRadget*) (self))));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void radare_gui_radget_search_class_init (RadareGUIRadgetSearchClass * klass) {
	radare_gui_radget_search_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIRadgetSearchPrivate));
	G_OBJECT_CLASS (klass)->get_property = radare_gui_radget_search_get_property;
	G_OBJECT_CLASS (klass)->constructor = radare_gui_radget_search_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_radget_search_finalize;
	g_object_class_override_property (G_OBJECT_CLASS (klass), RADARE_GUI_RADGET_SEARCH_BOX, "box");
	g_object_class_override_property (G_OBJECT_CLASS (klass), RADARE_GUI_RADGET_SEARCH_NAME, "name");
}


static void radare_gui_radget_search_radare_gui_radget_interface_init (RadareGUIRadgetIface * iface) {
	radare_gui_radget_search_radare_gui_radget_parent_iface = g_type_interface_peek_parent (iface);
	iface->refresh = radare_gui_radget_search_real_refresh;
	iface->get_box = radare_gui_radget_search_real_get_box;
	iface->get_name = radare_gui_radget_search_real_get_name;
}


static void radare_gui_radget_search_instance_init (RadareGUIRadgetSearch * self) {
	self->priv = RADARE_GUI_RADGET_SEARCH_GET_PRIVATE (self);
}


static void radare_gui_radget_search_finalize (GObject* obj) {
	RadareGUIRadgetSearch * self;
	self = RADARE_GUI_RADGET_SEARCH (obj);
	(self->priv->file == NULL ? NULL : (self->priv->file = (g_object_unref (self->priv->file), NULL)));
	(self->vb == NULL ? NULL : (self->vb = (g_object_unref (self->vb), NULL)));
	G_OBJECT_CLASS (radare_gui_radget_search_parent_class)->finalize (obj);
}


GType radare_gui_radget_search_get_type (void) {
	static GType radare_gui_radget_search_type_id = 0;
	if (radare_gui_radget_search_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIRadgetSearchClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_radget_search_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIRadgetSearch), 0, (GInstanceInitFunc) radare_gui_radget_search_instance_init, NULL };
		static const GInterfaceInfo radare_gui_radget_info = { (GInterfaceInitFunc) radare_gui_radget_search_radare_gui_radget_interface_init, (GInterfaceFinalizeFunc) NULL, NULL};
		radare_gui_radget_search_type_id = g_type_register_static (G_TYPE_OBJECT, "RadareGUIRadgetSearch", &g_define_type_info, 0);
		g_type_add_interface_static (radare_gui_radget_search_type_id, RADARE_GUI_TYPE_RADGET, &radare_gui_radget_info);
	}
	return radare_gui_radget_search_type_id;
}




