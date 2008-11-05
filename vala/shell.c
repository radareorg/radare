
#include "shell.h"
#include <vte/vte.h>
#include <stdlib.h>
#include <string.h>




struct _RadareGUIShellPrivate {
	VteTerminal* term;
	GtkEntry* entry;
};

#define RADARE_GUI_SHELL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_SHELL, RadareGUIShellPrivate))
enum  {
	RADARE_GUI_SHELL_DUMMY_PROPERTY
};
static void __lambda0 (GtkEntry* b, RadareGUIShell* self);
static void ___lambda0_gtk_entry_activate (GtkEntry* _sender, gpointer self);
static void __lambda1 (GtkButton* b, RadareGUIShell* self);
static void ___lambda1_gtk_button_clicked (GtkButton* _sender, gpointer self);
static void __lambda2 (GtkButton* b, RadareGUIShell* self);
static void ___lambda2_gtk_button_clicked (GtkButton* _sender, gpointer self);
static GObject * radare_gui_shell_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_shell_parent_class = NULL;
static void radare_gui_shell_finalize (GObject* obj);
static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);



#line 39 "shell.vala"
GtkVBox* radare_gui_shell_get (RadareGUIShell* self) {
	char** _tmp1;
	gint argv_length1;
	char** _tmp0;
	char** argv;
	GtkVBox* _tmp2;
	GtkVBox* _tmp3;
#line 39 "shell.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 41 "shell.vala"
	_tmp1 = NULL;
#line 41 "shell.vala"
	_tmp0 = NULL;
	argv = (_tmp1 = (_tmp0 = g_new0 (char*, 2 + 1), _tmp0[0] = g_strdup ("/bin/bash"), _tmp0[1] = NULL, _tmp0), argv_length1 = 2, _tmp1);
	/*term.fork_command("/bin/bash", argv,  null, "/", false, false, false);*/
#line 43 "shell.vala"
	_tmp2 = NULL;
#line 43 "shell.vala"
	_tmp3 = NULL;
#line 43 "shell.vala"
	return (_tmp3 = (_tmp2 = ((GtkVBox*) (self)), (_tmp2 == NULL ? NULL : g_object_ref (_tmp2))), (argv = (_vala_array_free (argv, argv_length1, ((GDestroyNotify) (g_free))), NULL)), _tmp3);
}


#line 5 "shell.vala"
RadareGUIShell* radare_gui_shell_construct (GType object_type) {
	RadareGUIShell * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 5 "shell.vala"
RadareGUIShell* radare_gui_shell_new (void) {
#line 5 "shell.vala"
	return radare_gui_shell_construct (RADARE_GUI_TYPE_SHELL);
}


static void __lambda0 (GtkEntry* b, RadareGUIShell* self) {
	g_return_if_fail (b != NULL);
#line 17 "shell.vala"
	vte_terminal_feed_child (self->priv->term, gtk_entry_get_text (self->priv->entry), ((glong) (((guint) (strlen (gtk_entry_get_text (self->priv->entry)))))));
#line 18 "shell.vala"
	vte_terminal_feed_child (self->priv->term, "\n", ((glong) (1)));
#line 19 "shell.vala"
	gtk_entry_set_text (self->priv->entry, "");
}


static void ___lambda0_gtk_entry_activate (GtkEntry* _sender, gpointer self) {
	__lambda0 (_sender, self);
}


static void __lambda1 (GtkButton* b, RadareGUIShell* self) {
	g_return_if_fail (b != NULL);
#line 25 "shell.vala"
	vte_terminal_feed_child (self->priv->term, gtk_entry_get_text (self->priv->entry), ((glong) (((guint) (strlen (gtk_entry_get_text (self->priv->entry)))))));
#line 26 "shell.vala"
	vte_terminal_feed_child (self->priv->term, "\n", ((glong) (1)));
#line 27 "shell.vala"
	gtk_entry_set_text (self->priv->entry, "");
}


static void ___lambda1_gtk_button_clicked (GtkButton* _sender, gpointer self) {
	__lambda1 (_sender, self);
}


static void __lambda2 (GtkButton* b, RadareGUIShell* self) {
	g_return_if_fail (b != NULL);
}


static void ___lambda2_gtk_button_clicked (GtkButton* _sender, gpointer self) {
	__lambda2 (_sender, self);
}


static GObject * radare_gui_shell_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIShellClass * klass;
	GObjectClass * parent_class;
	RadareGUIShell * self;
	klass = RADARE_GUI_SHELL_CLASS (g_type_class_peek (RADARE_GUI_TYPE_SHELL));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_SHELL (obj);
	{
		VteTerminal* _tmp0;
		GtkHBox* hb;
		GtkEntry* _tmp1;
		GtkButton* run;
		GtkButton* open;
		_tmp0 = NULL;
#line 12 "shell.vala"
		self->priv->term = (_tmp0 = g_object_ref_sink (((VteTerminal*) (vte_terminal_new ()))), (self->priv->term == NULL ? NULL : (self->priv->term = (g_object_unref (self->priv->term), NULL))), _tmp0);
#line 13 "shell.vala"
		gtk_container_add (((GtkContainer*) (self)), ((GtkWidget*) (self->priv->term)));
		hb = g_object_ref_sink (((GtkHBox*) (gtk_hbox_new (FALSE, 3))));
		_tmp1 = NULL;
#line 15 "shell.vala"
		self->priv->entry = (_tmp1 = g_object_ref_sink (((GtkEntry*) (gtk_entry_new ()))), (self->priv->entry == NULL ? NULL : (self->priv->entry = (g_object_unref (self->priv->entry), NULL))), _tmp1);
#line 16 "shell.vala"
		g_signal_connect_object (self->priv->entry, "activate", ((GCallback) (___lambda0_gtk_entry_activate)), self, 0);
#line 21 "shell.vala"
		gtk_container_add (((GtkContainer*) (hb)), ((GtkWidget*) (self->priv->entry)));
		run = g_object_ref_sink (((GtkButton*) (gtk_button_new_from_stock ("gtk-execute"))));
#line 24 "shell.vala"
		g_signal_connect_object (run, "clicked", ((GCallback) (___lambda1_gtk_button_clicked)), self, 0);
#line 29 "shell.vala"
		gtk_box_pack_start (((GtkBox*) (hb)), ((GtkWidget*) (run)), FALSE, FALSE, ((guint) (0)));
		open = g_object_ref_sink (((GtkButton*) (gtk_button_new_from_stock ("gtk-open"))));
#line 32 "shell.vala"
		g_signal_connect_object (open, "clicked", ((GCallback) (___lambda2_gtk_button_clicked)), self, 0);
#line 34 "shell.vala"
		gtk_box_pack_start (((GtkBox*) (hb)), ((GtkWidget*) (open)), FALSE, FALSE, ((guint) (0)));
#line 36 "shell.vala"
		gtk_box_pack_start (((GtkBox*) (self)), ((GtkWidget*) (hb)), FALSE, FALSE, ((guint) (3)));
		(hb == NULL ? NULL : (hb = (g_object_unref (hb), NULL)));
		(run == NULL ? NULL : (run = (g_object_unref (run), NULL)));
		(open == NULL ? NULL : (open = (g_object_unref (open), NULL)));
	}
	return obj;
}


static void radare_gui_shell_class_init (RadareGUIShellClass * klass) {
	radare_gui_shell_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIShellPrivate));
	G_OBJECT_CLASS (klass)->constructor = radare_gui_shell_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_shell_finalize;
}


static void radare_gui_shell_instance_init (RadareGUIShell * self) {
	self->priv = RADARE_GUI_SHELL_GET_PRIVATE (self);
}


static void radare_gui_shell_finalize (GObject* obj) {
	RadareGUIShell * self;
	self = RADARE_GUI_SHELL (obj);
	(self->priv->term == NULL ? NULL : (self->priv->term = (g_object_unref (self->priv->term), NULL)));
	(self->priv->entry == NULL ? NULL : (self->priv->entry = (g_object_unref (self->priv->entry), NULL)));
	G_OBJECT_CLASS (radare_gui_shell_parent_class)->finalize (obj);
}


GType radare_gui_shell_get_type (void) {
	static GType radare_gui_shell_type_id = 0;
	if (radare_gui_shell_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIShellClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_shell_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIShell), 0, (GInstanceInitFunc) radare_gui_shell_instance_init, NULL };
		radare_gui_shell_type_id = g_type_register_static (GTK_TYPE_VBOX, "RadareGUIShell", &g_define_type_info, 0);
	}
	return radare_gui_shell_type_id;
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if (array != NULL && destroy_func != NULL) {
		int i;
		if (array_length >= 0)
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) (array))[i] != NULL)
			destroy_func (((gpointer*) (array))[i]);
		}
		else
		for (i = 0; ((gpointer*) (array))[i] != NULL; i = i + 1) {
			destroy_func (((gpointer*) (array))[i]);
		}
	}
	g_free (array);
}




