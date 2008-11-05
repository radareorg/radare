
#include "panel.h"
#include <stdlib.h>
#include <string.h>
#include "radget.h"
#include "info.h"
#include "search.h"
#include "debug.h"




struct _RadareGUIPanelPrivate {
	GSList* radgets;
};

#define RADARE_GUI_PANEL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_PANEL, RadareGUIPanelPrivate))
enum  {
	RADARE_GUI_PANEL_DUMMY_PROPERTY
};
static void _g_slist_free_g_object_unref (GSList* self);
static GtkWidget* radare_gui_panel_load (RadareGUIPanel* self);
static GObject * radare_gui_panel_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_panel_parent_class = NULL;
static void radare_gui_panel_finalize (GObject* obj);



static void _g_slist_free_g_object_unref (GSList* self) {
	g_slist_foreach (self, ((GFunc) (g_object_unref)), NULL);
	g_slist_free (self);
}


#line 14 "panel.vala"
void radare_gui_panel_refresh (RadareGUIPanel* self) {
#line 14 "panel.vala"
	g_return_if_fail (self != NULL);
	{
		GSList* r_collection;
		GSList* r_it;
#line 16 "panel.vala"
		r_collection = self->priv->radgets;
		for (r_it = r_collection; r_it != NULL; r_it = r_it->next) {
			RadareGUIRadget* _tmp0;
			RadareGUIRadget* r;
#line 6 "panel.vala"
			_tmp0 = NULL;
#line 16 "panel.vala"
			r = (_tmp0 = ((RadareGUIRadget*) (r_it->data)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
			{
#line 17 "panel.vala"
				radare_gui_radget_refresh (r);
				(r == NULL ? NULL : (r = (g_object_unref (r), NULL)));
			}
		}
	}
}


#line 21 "panel.vala"
static GtkWidget* radare_gui_panel_load (RadareGUIPanel* self) {
	GtkVBox* vb;
#line 21 "panel.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 23 "panel.vala"
	self->priv->radgets = g_slist_append (self->priv->radgets, ((RadareGUIRadget*) (radare_gui_radget_information_new ())));
#line 24 "panel.vala"
	self->priv->radgets = g_slist_append (self->priv->radgets, ((RadareGUIRadget*) (radare_gui_radget_search_new ())));
#line 25 "panel.vala"
	self->priv->radgets = g_slist_append (self->priv->radgets, ((RadareGUIRadget*) (radare_gui_radget_debugger_new ())));
	vb = g_object_ref_sink (((GtkVBox*) (gtk_vbox_new (FALSE, 5))));
	{
		GSList* r_collection;
		GSList* r_it;
#line 29 "panel.vala"
		r_collection = self->priv->radgets;
		for (r_it = r_collection; r_it != NULL; r_it = r_it->next) {
			RadareGUIRadget* _tmp0;
			RadareGUIRadget* r;
#line 6 "panel.vala"
			_tmp0 = NULL;
#line 29 "panel.vala"
			r = (_tmp0 = ((RadareGUIRadget*) (r_it->data)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
			{
				GtkExpander* ex;
				ex = g_object_ref_sink (((GtkExpander*) (gtk_expander_new (radare_gui_radget_get_name (r)))));
#line 31 "panel.vala"
				gtk_container_add (((GtkContainer*) (ex)), ((GtkWidget*) (radare_gui_radget_get_box (r))));
#line 32 "panel.vala"
				gtk_box_pack_start (((GtkBox*) (vb)), ((GtkWidget*) (ex)), FALSE, FALSE, ((guint) (0)));
				(r == NULL ? NULL : (r = (g_object_unref (r), NULL)));
				(ex == NULL ? NULL : (ex = (g_object_unref (ex), NULL)));
			}
		}
	}
#line 35 "panel.vala"
	return ((GtkWidget*) (vb));
}


#line 38 "panel.vala"
GtkWidget* radare_gui_panel_widget (RadareGUIPanel* self) {
	GtkWidget* _tmp0;
#line 38 "panel.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 40 "panel.vala"
	_tmp0 = NULL;
#line 40 "panel.vala"
	return (_tmp0 = ((GtkWidget*) (self)), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
}


#line 4 "panel.vala"
RadareGUIPanel* radare_gui_panel_construct (GType object_type) {
	RadareGUIPanel * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 4 "panel.vala"
RadareGUIPanel* radare_gui_panel_new (void) {
#line 4 "panel.vala"
	return radare_gui_panel_construct (RADARE_GUI_TYPE_PANEL);
}


static GObject * radare_gui_panel_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIPanelClass * klass;
	GObjectClass * parent_class;
	RadareGUIPanel * self;
	klass = RADARE_GUI_PANEL_CLASS (g_type_class_peek (RADARE_GUI_TYPE_PANEL));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_PANEL (obj);
	{
		GSList* _tmp0;
		GtkWidget* _tmp1;
		_tmp0 = NULL;
#line 9 "panel.vala"
		self->priv->radgets = (_tmp0 = NULL, (self->priv->radgets == NULL ? NULL : (self->priv->radgets = (_g_slist_free_g_object_unref (self->priv->radgets), NULL))), _tmp0);
#line 10 "panel.vala"
		gtk_scrolled_window_set_policy (((GtkScrolledWindow*) (self)), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#line 21 "panel.vala"
		_tmp1 = NULL;
#line 11 "panel.vala"
		gtk_container_add (((GtkContainer*) (self)), (_tmp1 = radare_gui_panel_load (self)));
		(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
	}
	return obj;
}


static void radare_gui_panel_class_init (RadareGUIPanelClass * klass) {
	radare_gui_panel_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIPanelPrivate));
	G_OBJECT_CLASS (klass)->constructor = radare_gui_panel_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_panel_finalize;
}


static void radare_gui_panel_instance_init (RadareGUIPanel * self) {
	self->priv = RADARE_GUI_PANEL_GET_PRIVATE (self);
}


static void radare_gui_panel_finalize (GObject* obj) {
	RadareGUIPanel * self;
	self = RADARE_GUI_PANEL (obj);
	(self->priv->radgets == NULL ? NULL : (self->priv->radgets = (_g_slist_free_g_object_unref (self->priv->radgets), NULL)));
	G_OBJECT_CLASS (radare_gui_panel_parent_class)->finalize (obj);
}


GType radare_gui_panel_get_type (void) {
	static GType radare_gui_panel_type_id = 0;
	if (radare_gui_panel_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIPanelClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_panel_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIPanel), 0, (GInstanceInitFunc) radare_gui_panel_instance_init, NULL };
		radare_gui_panel_type_id = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW, "RadareGUIPanel", &g_define_type_info, 0);
	}
	return radare_gui_panel_type_id;
}




