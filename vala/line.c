
#include "line.h"




struct _LinePrivate {
	GtkEntry* e;
	GtkLabel* l;
};

#define LINE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_LINE, LinePrivate))
enum  {
	LINE_DUMMY_PROPERTY
};
static GObject * line_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer line_parent_class = NULL;
static void line_finalize (GObject* obj);



#line 15 "line.vala"
Line* line_name (Line* self, const char* name, const char* val) {
	Line* _tmp0;
#line 15 "line.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 15 "line.vala"
	g_return_val_if_fail (name != NULL, NULL);
#line 15 "line.vala"
	g_return_val_if_fail (val != NULL, NULL);
#line 17 "line.vala"
	gtk_label_set_text (self->priv->l, name);
#line 18 "line.vala"
	gtk_entry_set_text (self->priv->e, val);
#line 19 "line.vala"
	g_object_set (self->priv->e, "editable", FALSE, NULL);
#line 20 "line.vala"
	_tmp0 = NULL;
#line 20 "line.vala"
	return (_tmp0 = self, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


#line 23 "line.vala"
Line* line_numeric (Line* self, const char* name, gint number) {
	char* _tmp0;
	Line* _tmp1;
#line 23 "line.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 23 "line.vala"
	g_return_val_if_fail (name != NULL, NULL);
#line 25 "line.vala"
	gtk_label_set_text (self->priv->l, name);
#line 26 "line.vala"
	g_object_set (self->priv->e, "editable", FALSE, NULL);
#line 703 "glib-2.0.vapi"
	_tmp0 = NULL;
#line 27 "line.vala"
	gtk_entry_set_text (self->priv->e, (_tmp0 = g_strdup_printf ("%d", number)));
	_tmp0 = (g_free (_tmp0), NULL);
#line 28 "line.vala"
	_tmp1 = NULL;
#line 28 "line.vala"
	return (_tmp1 = self, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1)));
}


#line 3 "line.vala"
Line* line_construct (GType object_type) {
	Line * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 3 "line.vala"
Line* line_new (void) {
#line 3 "line.vala"
	return line_construct (TYPE_LINE);
}


static GObject * line_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	LineClass * klass;
	GObjectClass * parent_class;
	Line * self;
	klass = LINE_CLASS (g_type_class_peek (TYPE_LINE));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = LINE (obj);
	{
		GtkEntry* _tmp0;
		GtkLabel* _tmp1;
		_tmp0 = NULL;
#line 9 "line.vala"
		self->priv->e = (_tmp0 = g_object_ref_sink (((GtkEntry*) (gtk_entry_new ()))), (self->priv->e == NULL ? NULL : (self->priv->e = (g_object_unref (self->priv->e), NULL))), _tmp0);
		_tmp1 = NULL;
#line 10 "line.vala"
		self->priv->l = (_tmp1 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("")))), (self->priv->l == NULL ? NULL : (self->priv->l = (g_object_unref (self->priv->l), NULL))), _tmp1);
#line 11 "line.vala"
		gtk_box_pack_start (((GtkBox*) (self)), ((GtkWidget*) (self->priv->l)), FALSE, FALSE, ((guint) (5)));
#line 12 "line.vala"
		gtk_container_add (((GtkContainer*) (self)), ((GtkWidget*) (self->priv->e)));
	}
	return obj;
}


static void line_class_init (LineClass * klass) {
	line_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (LinePrivate));
	G_OBJECT_CLASS (klass)->constructor = line_constructor;
	G_OBJECT_CLASS (klass)->finalize = line_finalize;
}


static void line_instance_init (Line * self) {
	self->priv = LINE_GET_PRIVATE (self);
}


static void line_finalize (GObject* obj) {
	Line * self;
	self = LINE (obj);
	(self->priv->e == NULL ? NULL : (self->priv->e = (g_object_unref (self->priv->e), NULL)));
	(self->priv->l == NULL ? NULL : (self->priv->l = (g_object_unref (self->priv->l), NULL)));
	G_OBJECT_CLASS (line_parent_class)->finalize (obj);
}


GType line_get_type (void) {
	static GType line_type_id = 0;
	if (line_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (LineClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) line_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (Line), 0, (GInstanceInitFunc) line_instance_init, NULL };
		line_type_id = g_type_register_static (GTK_TYPE_HBOX, "Line", &g_define_type_info, 0);
	}
	return line_type_id;
}




