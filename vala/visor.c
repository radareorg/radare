
#include "visor.h"
#include <stdlib.h>
#include <string.h>




struct _RadareGUIVisorPrivate {
	GtkLabel* label;
};

#define RADARE_GUI_VISOR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_VISOR, RadareGUIVisorPrivate))
enum  {
	RADARE_GUI_VISOR_DUMMY_PROPERTY
};
static GObject * radare_gui_visor_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_visor_parent_class = NULL;
static void radare_gui_visor_finalize (GObject* obj);



#line 23 "visor.vala"
GtkVBox* radare_gui_visor_get (RadareGUIVisor* self) {
	GtkVBox* _tmp0;
#line 23 "visor.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 25 "visor.vala"
	_tmp0 = NULL;
#line 25 "visor.vala"
	return (_tmp0 = ((GtkVBox*) (self)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


#line 4 "visor.vala"
RadareGUIVisor* radare_gui_visor_construct (GType object_type) {
	RadareGUIVisor * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 4 "visor.vala"
RadareGUIVisor* radare_gui_visor_new (void) {
#line 4 "visor.vala"
	return radare_gui_visor_construct (RADARE_GUI_TYPE_VISOR);
}


/*List3 list;*/
static GObject * radare_gui_visor_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIVisorClass * klass;
	GObjectClass * parent_class;
	RadareGUIVisor * self;
	klass = RADARE_GUI_VISOR_CLASS (g_type_class_peek (RADARE_GUI_TYPE_VISOR));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_VISOR (obj);
	{
		GtkHBox* hb;
		GtkLabel* _tmp0;
		/*list = new List3().with_title("Address","Hexdump","Ascii");
		list.add("0x00000000", "00 00 7f 31 42 03 99 42 53 23", ".....B.0B97");
		list.add("0x00000010", "82 1a 00 81 42 93 72 42 1c 10", "82| rsa93..");
		add(list.widget);
		*/
		hb = g_object_ref_sink (((GtkHBox*) (gtk_hbox_new (FALSE, 3))));
		_tmp0 = NULL;
#line 18 "visor.vala"
		self->priv->label = (_tmp0 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("miau")))), (self->priv->label == NULL ? NULL : (self->priv->label = (g_object_unref (self->priv->label), NULL))), _tmp0);
#line 19 "visor.vala"
		gtk_box_pack_start (((GtkBox*) (hb)), ((GtkWidget*) (self->priv->label)), FALSE, FALSE, ((guint) (3)));
#line 20 "visor.vala"
		gtk_box_pack_start (((GtkBox*) (self)), ((GtkWidget*) (hb)), FALSE, FALSE, ((guint) (3)));
		(hb == NULL ? NULL : (hb = (g_object_unref (hb), NULL)));
	}
	return obj;
}


static void radare_gui_visor_class_init (RadareGUIVisorClass * klass) {
	radare_gui_visor_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIVisorPrivate));
	G_OBJECT_CLASS (klass)->constructor = radare_gui_visor_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_visor_finalize;
}


static void radare_gui_visor_instance_init (RadareGUIVisor * self) {
	self->priv = RADARE_GUI_VISOR_GET_PRIVATE (self);
}


static void radare_gui_visor_finalize (GObject* obj) {
	RadareGUIVisor * self;
	self = RADARE_GUI_VISOR (obj);
	(self->priv->label == NULL ? NULL : (self->priv->label = (g_object_unref (self->priv->label), NULL)));
	G_OBJECT_CLASS (radare_gui_visor_parent_class)->finalize (obj);
}


GType radare_gui_visor_get_type (void) {
	static GType radare_gui_visor_type_id = 0;
	if (radare_gui_visor_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIVisorClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_visor_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIVisor), 0, (GInstanceInitFunc) radare_gui_visor_instance_init, NULL };
		radare_gui_visor_type_id = g_type_register_static (GTK_TYPE_VBOX, "RadareGUIVisor", &g_define_type_info, 0);
	}
	return radare_gui_visor_type_id;
}




