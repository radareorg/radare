
#include "term.h"
#include <stdlib.h>
#include <string.h>
#include "core.h"




struct _RadareGUITermPrivate {
	GtkEntry* entry;
	GtkLabel* label;
};

#define RADARE_GUI_TERM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_TERM, RadareGUITermPrivate))
enum  {
	RADARE_GUI_TERM_DUMMY_PROPERTY
};
static void __lambda0 (GtkEntry* b, RadareGUITerm* self);
static void ___lambda0_gtk_entry_activate (GtkEntry* _sender, gpointer self);
static void __lambda1 (GtkButton* b, RadareGUITerm* self);
static void ___lambda1_gtk_button_clicked (GtkButton* _sender, gpointer self);
static void __lambda2 (GtkButton* b, RadareGUITerm* self);
static void ___lambda2_gtk_button_clicked (GtkButton* _sender, gpointer self);
static GObject * radare_gui_term_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_term_parent_class = NULL;
static void radare_gui_term_finalize (GObject* obj);



#line 38 "term.vala"
GtkVBox* radare_gui_term_get (RadareGUITerm* self) {
	GtkVBox* _tmp0;
#line 38 "term.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 40 "term.vala"
	_tmp0 = NULL;
#line 40 "term.vala"
	return (_tmp0 = ((GtkVBox*) (self)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


#line 5 "term.vala"
RadareGUITerm* radare_gui_term_construct (GType object_type) {
	RadareGUITerm * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 5 "term.vala"
RadareGUITerm* radare_gui_term_new (void) {
#line 5 "term.vala"
	return radare_gui_term_construct (RADARE_GUI_TYPE_TERM);
}


static void __lambda0 (GtkEntry* b, RadareGUITerm* self) {
	char* _tmp0;
	g_return_if_fail (b != NULL);
#line 28 "core.vala"
	_tmp0 = NULL;
#line 18 "term.vala"
	gtk_label_set_text (self->priv->label, (_tmp0 = radare_core_command (gtk_entry_get_text (self->priv->entry))));
	_tmp0 = (g_free (_tmp0), NULL);
#line 19 "term.vala"
	gtk_entry_set_text (self->priv->entry, "");
}


static void ___lambda0_gtk_entry_activate (GtkEntry* _sender, gpointer self) {
	__lambda0 (_sender, self);
}


static void __lambda1 (GtkButton* b, RadareGUITerm* self) {
	char* _tmp0;
	g_return_if_fail (b != NULL);
#line 28 "core.vala"
	_tmp0 = NULL;
#line 25 "term.vala"
	gtk_label_set_text (self->priv->label, (_tmp0 = radare_core_command (gtk_entry_get_text (self->priv->entry))));
	_tmp0 = (g_free (_tmp0), NULL);
#line 26 "term.vala"
	gtk_entry_set_text (self->priv->entry, "");
}


static void ___lambda1_gtk_button_clicked (GtkButton* _sender, gpointer self) {
	__lambda1 (_sender, self);
}


static void __lambda2 (GtkButton* b, RadareGUITerm* self) {
	g_return_if_fail (b != NULL);
}


static void ___lambda2_gtk_button_clicked (GtkButton* _sender, gpointer self) {
	__lambda2 (_sender, self);
}


static GObject * radare_gui_term_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUITermClass * klass;
	GObjectClass * parent_class;
	RadareGUITerm * self;
	klass = RADARE_GUI_TERM_CLASS (g_type_class_peek (RADARE_GUI_TYPE_TERM));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_TERM (obj);
	{
		GtkHBox* hb;
		GtkLabel* _tmp0;
		GtkEntry* _tmp1;
		GtkButton* run;
		GtkButton* open;
		hb = g_object_ref_sink (((GtkHBox*) (gtk_hbox_new (FALSE, 3))));
		_tmp0 = NULL;
#line 14 "term.vala"
		self->priv->label = (_tmp0 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("")))), (self->priv->label == NULL ? NULL : (self->priv->label = (g_object_unref (self->priv->label), NULL))), _tmp0);
#line 15 "term.vala"
		gtk_container_add (((GtkContainer*) (self)), ((GtkWidget*) (self->priv->label)));
		_tmp1 = NULL;
#line 16 "term.vala"
		self->priv->entry = (_tmp1 = g_object_ref_sink (((GtkEntry*) (gtk_entry_new ()))), (self->priv->entry == NULL ? NULL : (self->priv->entry = (g_object_unref (self->priv->entry), NULL))), _tmp1);
#line 17 "term.vala"
		g_signal_connect_object (self->priv->entry, "activate", ((GCallback) (___lambda0_gtk_entry_activate)), self, 0);
#line 21 "term.vala"
		gtk_container_add (((GtkContainer*) (hb)), ((GtkWidget*) (self->priv->entry)));
		run = g_object_ref_sink (((GtkButton*) (gtk_button_new_from_stock ("gtk-execute"))));
#line 24 "term.vala"
		g_signal_connect_object (run, "clicked", ((GCallback) (___lambda1_gtk_button_clicked)), self, 0);
#line 28 "term.vala"
		gtk_box_pack_start (((GtkBox*) (hb)), ((GtkWidget*) (run)), FALSE, FALSE, ((guint) (0)));
		open = g_object_ref_sink (((GtkButton*) (gtk_button_new_from_stock ("gtk-open"))));
#line 31 "term.vala"
		g_signal_connect_object (open, "clicked", ((GCallback) (___lambda2_gtk_button_clicked)), self, 0);
#line 33 "term.vala"
		gtk_box_pack_start (((GtkBox*) (hb)), ((GtkWidget*) (open)), FALSE, FALSE, ((guint) (0)));
#line 35 "term.vala"
		gtk_box_pack_start (((GtkBox*) (self)), ((GtkWidget*) (hb)), FALSE, FALSE, ((guint) (3)));
		(hb == NULL ? NULL : (hb = (g_object_unref (hb), NULL)));
		(run == NULL ? NULL : (run = (g_object_unref (run), NULL)));
		(open == NULL ? NULL : (open = (g_object_unref (open), NULL)));
	}
	return obj;
}


static void radare_gui_term_class_init (RadareGUITermClass * klass) {
	radare_gui_term_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUITermPrivate));
	G_OBJECT_CLASS (klass)->constructor = radare_gui_term_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_term_finalize;
}


static void radare_gui_term_instance_init (RadareGUITerm * self) {
	self->priv = RADARE_GUI_TERM_GET_PRIVATE (self);
}


static void radare_gui_term_finalize (GObject* obj) {
	RadareGUITerm * self;
	self = RADARE_GUI_TERM (obj);
	(self->priv->entry == NULL ? NULL : (self->priv->entry = (g_object_unref (self->priv->entry), NULL)));
	(self->priv->label == NULL ? NULL : (self->priv->label = (g_object_unref (self->priv->label), NULL)));
	G_OBJECT_CLASS (radare_gui_term_parent_class)->finalize (obj);
}


GType radare_gui_term_get_type (void) {
	static GType radare_gui_term_type_id = 0;
	if (radare_gui_term_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUITermClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_term_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUITerm), 0, (GInstanceInitFunc) radare_gui_term_instance_init, NULL };
		radare_gui_term_type_id = g_type_register_static (GTK_TYPE_VBOX, "RadareGUITerm", &g_define_type_info, 0);
	}
	return radare_gui_term_type_id;
}




