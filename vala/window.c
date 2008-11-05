
#include "window.h"
#include <stdlib.h>
#include <string.h>
#include "panel.h"
#include "core.h"




struct _RadareGUIMainWindowPrivate {
	RadareGUIPanel* panel;
};

#define RADARE_GUI_MAIN_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_MAIN_WINDOW, RadareGUIMainWindowPrivate))
enum  {
	RADARE_GUI_MAIN_WINDOW_DUMMY_PROPERTY
};
GtkStatusbar* radare_gui_main_window_statusbar = NULL;
static void radare_gui_main_window_load (RadareGUIMainWindow* self);
static void __lambda0 (RadareGUIMainWindow* w, RadareGUIMainWindow* self);
static void ___lambda0_gtk_object_destroy (RadareGUIMainWindow* _sender, gpointer self);
static void radare_gui_main_window_create_widgets (RadareGUIMainWindow* self);
static GtkWidget* radare_gui_main_window_comments (RadareGUIMainWindow* self);
static void __lambda1 (GtkMenuItem* m, RadareGUIMainWindow* self);
static void ___lambda1_gtk_menu_item_activate (GtkMenuItem* _sender, gpointer self);
static void __lambda2 (GtkMenuItem* m, RadareGUIMainWindow* self);
static void ___lambda2_gtk_menu_item_activate (GtkMenuItem* _sender, gpointer self);
static GtkMenuBar* radare_gui_main_window_menu (RadareGUIMainWindow* self);
static GtkToolbar* radare_gui_main_window_toolbar (RadareGUIMainWindow* self);
static GObject * radare_gui_main_window_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_main_window_parent_class = NULL;
static void radare_gui_main_window_finalize (GObject* obj);



/* reload information */
#line 24 "window.vala"
static void radare_gui_main_window_load (RadareGUIMainWindow* self) {
#line 24 "window.vala"
	g_return_if_fail (self != NULL);
}


static void __lambda0 (RadareGUIMainWindow* w, RadareGUIMainWindow* self) {
	g_return_if_fail (w != NULL);
#line 31 "window.vala"
	gtk_main_quit ();
}


static void ___lambda0_gtk_object_destroy (RadareGUIMainWindow* _sender, gpointer self) {
	__lambda0 (_sender, self);
}


#line 28 "window.vala"
static void radare_gui_main_window_create_widgets (RadareGUIMainWindow* self) {
	RadareGUITerm* _tmp0;
	RadareGUIVisor* _tmp1;
	RadareGUITerm* _tmp2;
	GtkVBox* root;
	GtkMenuBar* _tmp3;
	GtkHPaned* hp;
	GtkVBox* vb;
	GtkNotebook* nb;
	GtkLabel* _tmp5;
	GtkVBox* _tmp4;
	GtkLabel* _tmp7;
	GtkVBox* _tmp6;
	GtkLabel* _tmp9;
	GtkVBox* _tmp8;
	GtkLabel* _tmp11;
	GtkWidget* _tmp10;
	RadareGUIPanel* _tmp12;
	GtkStatusbar* _tmp13;
#line 28 "window.vala"
	g_return_if_fail (self != NULL);
#line 30 "window.vala"
	g_signal_connect_object (((GtkObject*) (self)), "destroy", ((GCallback) (___lambda0_gtk_object_destroy)), self, 0);
	_tmp0 = NULL;
#line 34 "window.vala"
	self->shell = (_tmp0 = g_object_ref_sink (radare_gui_term_new ()), (self->shell == NULL ? NULL : (self->shell = (g_object_unref (self->shell), NULL))), _tmp0);
	_tmp1 = NULL;
#line 35 "window.vala"
	self->visor = (_tmp1 = g_object_ref_sink (radare_gui_visor_new ()), (self->visor == NULL ? NULL : (self->visor = (g_object_unref (self->visor), NULL))), _tmp1);
	_tmp2 = NULL;
#line 36 "window.vala"
	self->con = (_tmp2 = g_object_ref_sink (radare_gui_term_new ()), (self->con == NULL ? NULL : (self->con = (g_object_unref (self->con), NULL))), _tmp2);
	root = g_object_ref_sink (((GtkVBox*) (gtk_vbox_new (FALSE, 0))));
#line 71 "window.vala"
	_tmp3 = NULL;
#line 39 "window.vala"
	gtk_box_pack_start (((GtkBox*) (root)), ((GtkWidget*) ((_tmp3 = radare_gui_main_window_menu (self)))), FALSE, FALSE, ((guint) (0)));
	(_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref (_tmp3), NULL)));
	/*root.pack_start(toolbar(),false,false, 2);*/
	hp = g_object_ref_sink (((GtkHPaned*) (gtk_hpaned_new ())));
	vb = g_object_ref_sink (((GtkVBox*) (gtk_vbox_new (FALSE, 0))));
	nb = g_object_ref_sink (((GtkNotebook*) (gtk_notebook_new ())));
	_tmp5 = NULL;
#line 23 "visor.vala"
	_tmp4 = NULL;
#line 47 "window.vala"
	gtk_notebook_append_page (nb, ((GtkWidget*) ((_tmp4 = radare_gui_visor_get (self->visor)))), ((GtkWidget*) ((_tmp5 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("Visor"))))))));
	(_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref (_tmp5), NULL)));
	(_tmp4 == NULL ? NULL : (_tmp4 = (g_object_unref (_tmp4), NULL)));
	_tmp7 = NULL;
#line 38 "term.vala"
	_tmp6 = NULL;
#line 48 "window.vala"
	gtk_notebook_append_page (nb, ((GtkWidget*) ((_tmp6 = radare_gui_term_get (self->con)))), ((GtkWidget*) ((_tmp7 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("Terminal"))))))));
	(_tmp7 == NULL ? NULL : (_tmp7 = (g_object_unref (_tmp7), NULL)));
	(_tmp6 == NULL ? NULL : (_tmp6 = (g_object_unref (_tmp6), NULL)));
	_tmp9 = NULL;
#line 38 "term.vala"
	_tmp8 = NULL;
#line 49 "window.vala"
	gtk_notebook_append_page (nb, ((GtkWidget*) ((_tmp8 = radare_gui_term_get (self->shell)))), ((GtkWidget*) ((_tmp9 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("Terminal"))))))));
	(_tmp9 == NULL ? NULL : (_tmp9 = (g_object_unref (_tmp9), NULL)));
	(_tmp8 == NULL ? NULL : (_tmp8 = (g_object_unref (_tmp8), NULL)));
	_tmp11 = NULL;
#line 66 "window.vala"
	_tmp10 = NULL;
#line 50 "window.vala"
	gtk_notebook_append_page (nb, (_tmp10 = radare_gui_main_window_comments (self)), ((GtkWidget*) ((_tmp11 = g_object_ref_sink (((GtkLabel*) (gtk_label_new ("Database"))))))));
	(_tmp11 == NULL ? NULL : (_tmp11 = (g_object_unref (_tmp11), NULL)));
	(_tmp10 == NULL ? NULL : (_tmp10 = (g_object_unref (_tmp10), NULL)));
#line 51 "window.vala"
	gtk_container_add (((GtkContainer*) (vb)), ((GtkWidget*) (nb)));
	_tmp12 = NULL;
#line 53 "window.vala"
	self->priv->panel = (_tmp12 = g_object_ref_sink (radare_gui_panel_new ()), (self->priv->panel == NULL ? NULL : (self->priv->panel = (g_object_unref (self->priv->panel), NULL))), _tmp12);
#line 54 "window.vala"
	gtk_container_add (((GtkContainer*) (hp)), ((GtkWidget*) (self->priv->panel)));
#line 55 "window.vala"
	gtk_container_add (((GtkContainer*) (hp)), ((GtkWidget*) (vb)));
#line 56 "window.vala"
	gtk_container_add (((GtkContainer*) (root)), ((GtkWidget*) (hp)));
#line 57 "window.vala"
	gtk_paned_set_position (((GtkPaned*) (hp)), 170);
	_tmp13 = NULL;
#line 59 "window.vala"
	radare_gui_main_window_statusbar = (_tmp13 = g_object_ref_sink (((GtkStatusbar*) (gtk_statusbar_new ()))), (radare_gui_main_window_statusbar == NULL ? NULL : (radare_gui_main_window_statusbar = (g_object_unref (radare_gui_main_window_statusbar), NULL))), _tmp13);
#line 60 "window.vala"
	gtk_statusbar_push (radare_gui_main_window_statusbar, ((guint) (0)), "Interface loaded.");
#line 61 "window.vala"
	gtk_box_pack_end (((GtkBox*) (root)), ((GtkWidget*) (radare_gui_main_window_statusbar)), FALSE, FALSE, ((guint) (0)));
#line 63 "window.vala"
	gtk_container_add (((GtkContainer*) (self)), ((GtkWidget*) (root)));
	(root == NULL ? NULL : (root = (g_object_unref (root), NULL)));
	(hp == NULL ? NULL : (hp = (g_object_unref (hp), NULL)));
	(vb == NULL ? NULL : (vb = (g_object_unref (vb), NULL)));
	(nb == NULL ? NULL : (nb = (g_object_unref (nb), NULL)));
}


#line 66 "window.vala"
static GtkWidget* radare_gui_main_window_comments (RadareGUIMainWindow* self) {
#line 66 "window.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 68 "window.vala"
	return ((GtkWidget*) (g_object_ref_sink (((GtkLabel*) (gtk_label_new ("(TODO)"))))));
}


static void __lambda1 (GtkMenuItem* m, RadareGUIMainWindow* self) {
	GtkFileChooserDialog* fcd;
	GtkFileChooserDialog* _tmp0;
	g_return_if_fail (m != NULL);
	fcd = g_object_ref_sink (((GtkFileChooserDialog*) (gtk_file_chooser_dialog_new ("Open file", ((GtkWindow*) (self)), GTK_FILE_CHOOSER_ACTION_OPEN, "gtk-cancel", 0, "gtk-ok", 1, NULL))));
#line 84 "window.vala"
	if (gtk_dialog_run (((GtkDialog*) (fcd))) == 1) {
#line 85 "window.vala"
		radare_core_open (gtk_file_chooser_get_filename (((GtkFileChooser*) (fcd))), FALSE);
#line 86 "window.vala"
		radare_gui_main_window_load (self);
	}
#line 88 "window.vala"
	gtk_object_destroy (((GtkObject*) (fcd)));
	_tmp0 = NULL;
#line 89 "window.vala"
	fcd = (_tmp0 = NULL, (fcd == NULL ? NULL : (fcd = (g_object_unref (fcd), NULL))), _tmp0);
	(fcd == NULL ? NULL : (fcd = (g_object_unref (fcd), NULL)));
}


static void ___lambda1_gtk_menu_item_activate (GtkMenuItem* _sender, gpointer self) {
	__lambda1 (_sender, self);
}


static void __lambda2 (GtkMenuItem* m, RadareGUIMainWindow* self) {
	g_return_if_fail (m != NULL);
#line 94 "window.vala"
	gtk_main_quit ();
}


static void ___lambda2_gtk_menu_item_activate (GtkMenuItem* _sender, gpointer self) {
	__lambda2 (_sender, self);
}


#line 71 "window.vala"
static GtkMenuBar* radare_gui_main_window_menu (RadareGUIMainWindow* self) {
	GtkMenuBar* bar;
	GtkMenu* m;
	GtkMenuItem* mo;
	GtkMenuItem* _tmp0;
	GtkMenuItem* root_menu;
	GtkMenu* _tmp1;
	GtkMenuItem* _tmp2;
	GtkMenuItem* _tmp3;
	GtkMenu* _tmp4;
	GtkMenuItem* _tmp5;
	GtkMenuItem* _tmp6;
	GtkMenuBar* _tmp7;
#line 71 "window.vala"
	g_return_val_if_fail (self != NULL, NULL);
	bar = g_object_ref_sink (((GtkMenuBar*) (gtk_menu_bar_new ())));
	m = g_object_ref_sink (((GtkMenu*) (gtk_menu_new ())));
	mo = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("Open"))));
#line 76 "window.vala"
	g_signal_connect_object (mo, "activate", ((GCallback) (___lambda1_gtk_menu_item_activate)), self, 0);
#line 91 "window.vala"
	gtk_menu_shell_append (((GtkMenuShell*) (m)), ((GtkWidget*) (mo)));
	_tmp0 = NULL;
#line 92 "window.vala"
	mo = (_tmp0 = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("Exit")))), (mo == NULL ? NULL : (mo = (g_object_unref (mo), NULL))), _tmp0);
#line 93 "window.vala"
	g_signal_connect_object (mo, "activate", ((GCallback) (___lambda2_gtk_menu_item_activate)), self, 0);
#line 96 "window.vala"
	gtk_menu_shell_append (((GtkMenuShell*) (m)), ((GtkWidget*) (mo)));
	root_menu = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("File"))));
#line 99 "window.vala"
	gtk_menu_item_set_submenu (root_menu, ((GtkWidget*) (m)));
#line 100 "window.vala"
	gtk_menu_shell_append (((GtkMenuShell*) (bar)), ((GtkWidget*) (root_menu)));
	_tmp1 = NULL;
#line 102 "window.vala"
	m = (_tmp1 = g_object_ref_sink (((GtkMenu*) (gtk_menu_new ()))), (m == NULL ? NULL : (m = (g_object_unref (m), NULL))), _tmp1);
	_tmp2 = NULL;
#line 103 "window.vala"
	mo = (_tmp2 = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("Copy")))), (mo == NULL ? NULL : (mo = (g_object_unref (mo), NULL))), _tmp2);
#line 104 "window.vala"
	gtk_menu_shell_append (((GtkMenuShell*) (m)), ((GtkWidget*) (mo)));
	_tmp3 = NULL;
#line 105 "window.vala"
	root_menu = (_tmp3 = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("Edit")))), (root_menu == NULL ? NULL : (root_menu = (g_object_unref (root_menu), NULL))), _tmp3);
#line 106 "window.vala"
	gtk_menu_item_set_submenu (root_menu, ((GtkWidget*) (m)));
	/* Help menu */
	_tmp4 = NULL;
#line 109 "window.vala"
	m = (_tmp4 = g_object_ref_sink (((GtkMenu*) (gtk_menu_new ()))), (m == NULL ? NULL : (m = (g_object_unref (m), NULL))), _tmp4);
	_tmp5 = NULL;
#line 110 "window.vala"
	mo = (_tmp5 = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("About")))), (mo == NULL ? NULL : (mo = (g_object_unref (mo), NULL))), _tmp5);
#line 111 "window.vala"
	gtk_menu_shell_append (((GtkMenuShell*) (m)), ((GtkWidget*) (mo)));
	_tmp6 = NULL;
#line 112 "window.vala"
	root_menu = (_tmp6 = g_object_ref_sink (((GtkMenuItem*) (gtk_menu_item_new_with_label ("Help")))), (root_menu == NULL ? NULL : (root_menu = (g_object_unref (root_menu), NULL))), _tmp6);
#line 113 "window.vala"
	gtk_menu_item_set_submenu (root_menu, ((GtkWidget*) (m)));
#line 115 "window.vala"
	gtk_menu_shell_append (((GtkMenuShell*) (bar)), ((GtkWidget*) (root_menu)));
#line 117 "window.vala"
	_tmp7 = NULL;
#line 117 "window.vala"
	return (_tmp7 = bar, (m == NULL ? NULL : (m = (g_object_unref (m), NULL))), (mo == NULL ? NULL : (mo = (g_object_unref (mo), NULL))), (root_menu == NULL ? NULL : (root_menu = (g_object_unref (root_menu), NULL))), _tmp7);
}


#line 120 "window.vala"
static GtkToolbar* radare_gui_main_window_toolbar (RadareGUIMainWindow* self) {
	GtkToolbar* t;
	GtkToolItem* ti;
	GtkToolbar* _tmp0;
#line 120 "window.vala"
	g_return_val_if_fail (self != NULL, NULL);
	t = g_object_ref_sink (((GtkToolbar*) (gtk_toolbar_new ())));
#line 123 "window.vala"
	t->style = GTK_TOOLBAR_ICONS;
	ti = g_object_ref_sink (gtk_tool_item_new ());
#line 126 "window.vala"
	gtk_container_add (((GtkContainer*) (t)), ((GtkWidget*) (ti)));
	/*
	t.append_item("new", "new", "", new Gtk.Image.from_stock(Gtk.STOCK_SAVE),
	Gtk.IconSize.MENU, null, 0);
	*/
#line 131 "window.vala"
	_tmp0 = NULL;
#line 131 "window.vala"
	return (_tmp0 = t, (ti == NULL ? NULL : (ti = (g_object_unref (ti), NULL))), _tmp0);
}


#line 4 "window.vala"
RadareGUIMainWindow* radare_gui_main_window_construct (GType object_type) {
	RadareGUIMainWindow * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 4 "window.vala"
RadareGUIMainWindow* radare_gui_main_window_new (void) {
#line 4 "window.vala"
	return radare_gui_main_window_construct (RADARE_GUI_TYPE_MAIN_WINDOW);
}


static GObject * radare_gui_main_window_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIMainWindowClass * klass;
	GObjectClass * parent_class;
	RadareGUIMainWindow * self;
	klass = RADARE_GUI_MAIN_WINDOW_CLASS (g_type_class_peek (RADARE_GUI_TYPE_MAIN_WINDOW));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_MAIN_WINDOW (obj);
	{
#line 16 "window.vala"
		radare_gui_main_window_create_widgets (self);
#line 17 "window.vala"
		gtk_window_set_title (((GtkWindow*) (self)), "gradare vala frontend");
#line 18 "window.vala"
		gtk_container_set_border_width (((GtkContainer*) (self)), ((guint) (3)));
#line 19 "window.vala"
		gtk_window_set_position (((GtkWindow*) (self)), GTK_WIN_POS_CENTER);
#line 20 "window.vala"
		gtk_window_resize (((GtkWindow*) (self)), 700, 500);
	}
	return obj;
}


static void radare_gui_main_window_class_init (RadareGUIMainWindowClass * klass) {
	radare_gui_main_window_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIMainWindowPrivate));
	G_OBJECT_CLASS (klass)->constructor = radare_gui_main_window_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_main_window_finalize;
}


static void radare_gui_main_window_instance_init (RadareGUIMainWindow * self) {
	self->priv = RADARE_GUI_MAIN_WINDOW_GET_PRIVATE (self);
}


static void radare_gui_main_window_finalize (GObject* obj) {
	RadareGUIMainWindow * self;
	self = RADARE_GUI_MAIN_WINDOW (obj);
	(self->left_list == NULL ? NULL : (self->left_list = (g_object_unref (self->left_list), NULL)));
	(self->right_list == NULL ? NULL : (self->right_list = (g_object_unref (self->right_list), NULL)));
	(self->shell == NULL ? NULL : (self->shell = (g_object_unref (self->shell), NULL)));
	(self->visor == NULL ? NULL : (self->visor = (g_object_unref (self->visor), NULL)));
	(self->con == NULL ? NULL : (self->con = (g_object_unref (self->con), NULL)));
	(self->priv->panel == NULL ? NULL : (self->priv->panel = (g_object_unref (self->priv->panel), NULL)));
	G_OBJECT_CLASS (radare_gui_main_window_parent_class)->finalize (obj);
}


GType radare_gui_main_window_get_type (void) {
	static GType radare_gui_main_window_type_id = 0;
	if (radare_gui_main_window_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIMainWindowClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_main_window_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIMainWindow), 0, (GInstanceInitFunc) radare_gui_main_window_instance_init, NULL };
		radare_gui_main_window_type_id = g_type_register_static (GTK_TYPE_WINDOW, "RadareGUIMainWindow", &g_define_type_info, 0);
	}
	return radare_gui_main_window_type_id;
}




