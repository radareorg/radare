/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
 *
 * bluspam is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bluspam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bluspam; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "list.h"




struct _RadareGUIListPrivate {
	GtkTreeView* tv;
	GtkTreeViewColumn* col;
	GtkCellRendererText* cr;
	GtkListStore* ls;
	GtkScrolledWindow* _widget;
};

#define RADARE_GUI_LIST_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RADARE_GUI_TYPE_LIST, RadareGUIListPrivate))
enum  {
	RADARE_GUI_LIST_DUMMY_PROPERTY,
	RADARE_GUI_LIST_WIDGET
};
static GObject * radare_gui_list_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_gui_list_parent_class = NULL;
static void radare_gui_list_finalize (GObject* obj);



#line 55 "list.vala"
void radare_gui_list_update (RadareGUIList* self) {
#line 55 "list.vala"
	g_return_if_fail (self != NULL);
#line 57 "list.vala"
	g_signal_emit_by_name (G_OBJECT (self), "action");
}


#line 60 "list.vala"
RadareGUIList* radare_gui_list_with_title (RadareGUIList* self, const char* title) {
	GtkCellRendererText* _tmp0;
	RadareGUIList* _tmp1;
#line 60 "list.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 60 "list.vala"
	g_return_val_if_fail (title != NULL, NULL);
	_tmp0 = NULL;
#line 62 "list.vala"
	gtk_tree_view_insert_column_with_attributes (self->priv->tv, 0, title, ((GtkCellRenderer*) ((_tmp0 = g_object_ref_sink (((GtkCellRendererText*) (gtk_cell_renderer_text_new ())))))), "text", 0, NULL, NULL);
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
#line 64 "list.vala"
	_tmp1 = NULL;
#line 64 "list.vala"
	return (_tmp1 = self, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1)));
}


#line 67 "list.vala"
void radare_gui_list_add (RadareGUIList* self, const char* item) {
	GtkTreeIter iter = {0};
#line 67 "list.vala"
	g_return_if_fail (self != NULL);
#line 67 "list.vala"
	g_return_if_fail (item != NULL);
#line 70 "list.vala"
	gtk_list_store_append (self->priv->ls, &iter);
#line 71 "list.vala"
	gtk_list_store_set (self->priv->ls, &iter, 0, item, -1);
}


#line 74 "list.vala"
char* radare_gui_list_get (RadareGUIList* self) {
	GtkTreeIter iter = {0};
	GtkTreeModel* model;
	char* str;
	GtkTreeSelection* _tmp0;
	GtkTreeSelection* sel;
	char* _tmp7;
#line 74 "list.vala"
	g_return_val_if_fail (self != NULL, NULL);
	model = NULL;
	str = g_strdup ("");
#line 80 "list.vala"
	_tmp0 = NULL;
	sel = (_tmp0 = gtk_tree_view_get_selection (self->priv->tv), (_tmp0 == NULL ? NULL : g_object_ref (_tmp0)));
#line 82 "list.vala"
	if (gtk_tree_selection_count_selected_rows (sel) == 1) {
		GString* foo;
		GtkTreeModel* _tmp4;
		GtkTreeModel* _tmp3;
		gboolean _tmp2;
		GtkTreeModel* _tmp1;
		char* _tmp6;
		const char* _tmp5;
		foo = g_string_sized_new (((gulong) (1024)));
#line 84 "list.vala"
		_tmp4 = NULL;
		_tmp3 = NULL;
		_tmp1 = NULL;
#line 84 "list.vala"
		_tmp2 = gtk_tree_selection_get_selected (sel, &_tmp1, &iter);
#line 84 "list.vala"
		model = (_tmp3 = (_tmp4 = _tmp1, (_tmp4 == NULL ? NULL : g_object_ref (_tmp4))), (model == NULL ? NULL : (model = (g_object_unref (model), NULL))), _tmp3);
#line 84 "list.vala"
		_tmp2;
#line 85 "list.vala"
		gtk_tree_model_get (gtk_tree_view_get_model (self->priv->tv), &iter, 0, foo, -1);
		_tmp6 = NULL;
#line 86 "list.vala"
		_tmp5 = NULL;
#line 86 "list.vala"
		str = (_tmp6 = (_tmp5 = foo->str, (_tmp5 == NULL ? NULL : g_strdup (_tmp5))), (str = (g_free (str), NULL)), _tmp6);
		(foo == NULL ? NULL : (foo = (g_string_free (foo, TRUE), NULL)));
	}
#line 89 "list.vala"
	_tmp7 = NULL;
#line 89 "list.vala"
	return (_tmp7 = str, (model == NULL ? NULL : (model = (g_object_unref (model), NULL))), (sel == NULL ? NULL : (sel = (g_object_unref (sel), NULL))), _tmp7);
}


#line 92 "list.vala"
void radare_gui_list_clear (RadareGUIList* self) {
#line 92 "list.vala"
	g_return_if_fail (self != NULL);
#line 94 "list.vala"
	gtk_list_store_clear (self->priv->ls);
}


/*
 *
 * Implements a Widget for GTK containing a list
 * of selectable items with scrollbar and treeview.
 *         - Yay
 *
 */
#line 32 "list.vala"
RadareGUIList* radare_gui_list_construct (GType object_type) {
	RadareGUIList * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 32 "list.vala"
RadareGUIList* radare_gui_list_new (void) {
#line 32 "list.vala"
	return radare_gui_list_construct (RADARE_GUI_TYPE_LIST);
}


GtkScrolledWindow* radare_gui_list_get_widget (RadareGUIList* self) {
	g_return_val_if_fail (self != NULL, NULL);
#line 40 "list.vala"
	return self->priv->_widget;
}


void radare_gui_list_set_widget (RadareGUIList* self, GtkScrolledWindow* value) {
	GtkScrolledWindow* _tmp2;
	GtkScrolledWindow* _tmp1;
	g_return_if_fail (self != NULL);
	_tmp2 = NULL;
#line 40 "list.vala"
	_tmp1 = NULL;
	self->priv->_widget = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_widget == NULL ? NULL : (self->priv->_widget = (g_object_unref (self->priv->_widget), NULL))), _tmp2);
	g_object_notify (((GObject *) (self)), "widget");
}


static GObject * radare_gui_list_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareGUIListClass * klass;
	GObjectClass * parent_class;
	RadareGUIList * self;
	klass = RADARE_GUI_LIST_CLASS (g_type_class_peek (RADARE_GUI_TYPE_LIST));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_GUI_LIST (obj);
	{
		GtkScrolledWindow* _tmp0;
		GtkTreeView* _tmp1;
		GtkListStore* _tmp2;
		_tmp0 = NULL;
#line 44 "list.vala"
		radare_gui_list_set_widget (self, (_tmp0 = g_object_ref_sink (((GtkScrolledWindow*) (gtk_scrolled_window_new (NULL, NULL))))));
		(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
#line 45 "list.vala"
		gtk_scrolled_window_set_policy (self->priv->_widget, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		_tmp1 = NULL;
#line 47 "list.vala"
		self->priv->tv = (_tmp1 = g_object_ref_sink (((GtkTreeView*) (gtk_tree_view_new ()))), (self->priv->tv == NULL ? NULL : (self->priv->tv = (g_object_unref (self->priv->tv), NULL))), _tmp1);
#line 48 "list.vala"
		gtk_tree_view_set_reorderable (self->priv->tv, TRUE);
#line 49 "list.vala"
		gtk_tree_view_set_rules_hint (self->priv->tv, TRUE);
		_tmp2 = NULL;
#line 50 "list.vala"
		gtk_tree_view_set_model (self->priv->tv, ((GtkTreeModel*) (self->priv->ls = (_tmp2 = gtk_list_store_new (1, G_TYPE_STRING, NULL), (self->priv->ls == NULL ? NULL : (self->priv->ls = (g_object_unref (self->priv->ls), NULL))), _tmp2))));
#line 52 "list.vala"
		gtk_container_add (((GtkContainer*) (self->priv->_widget)), ((GtkWidget*) (self->priv->tv)));
	}
	return obj;
}


static void radare_gui_list_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	RadareGUIList * self;
	self = RADARE_GUI_LIST (object);
	switch (property_id) {
		case RADARE_GUI_LIST_WIDGET:
		g_value_set_object (value, radare_gui_list_get_widget (self));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void radare_gui_list_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	RadareGUIList * self;
	self = RADARE_GUI_LIST (object);
	switch (property_id) {
		case RADARE_GUI_LIST_WIDGET:
		radare_gui_list_set_widget (self, g_value_get_object (value));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void radare_gui_list_class_init (RadareGUIListClass * klass) {
	radare_gui_list_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (RadareGUIListPrivate));
	G_OBJECT_CLASS (klass)->get_property = radare_gui_list_get_property;
	G_OBJECT_CLASS (klass)->set_property = radare_gui_list_set_property;
	G_OBJECT_CLASS (klass)->constructor = radare_gui_list_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_gui_list_finalize;
	g_object_class_install_property (G_OBJECT_CLASS (klass), RADARE_GUI_LIST_WIDGET, g_param_spec_object ("widget", "widget", "widget", GTK_TYPE_SCROLLED_WINDOW, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_signal_new ("action", RADARE_GUI_TYPE_LIST, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void radare_gui_list_instance_init (RadareGUIList * self) {
	self->priv = RADARE_GUI_LIST_GET_PRIVATE (self);
}


static void radare_gui_list_finalize (GObject* obj) {
	RadareGUIList * self;
	self = RADARE_GUI_LIST (obj);
	(self->priv->tv == NULL ? NULL : (self->priv->tv = (g_object_unref (self->priv->tv), NULL)));
	(self->priv->col == NULL ? NULL : (self->priv->col = (g_object_unref (self->priv->col), NULL)));
	(self->priv->cr == NULL ? NULL : (self->priv->cr = (g_object_unref (self->priv->cr), NULL)));
	(self->priv->ls == NULL ? NULL : (self->priv->ls = (g_object_unref (self->priv->ls), NULL)));
	(self->priv->_widget == NULL ? NULL : (self->priv->_widget = (g_object_unref (self->priv->_widget), NULL)));
	G_OBJECT_CLASS (radare_gui_list_parent_class)->finalize (obj);
}


GType radare_gui_list_get_type (void) {
	static GType radare_gui_list_type_id = 0;
	if (radare_gui_list_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareGUIListClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_gui_list_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareGUIList), 0, (GInstanceInitFunc) radare_gui_list_instance_init, NULL };
		radare_gui_list_type_id = g_type_register_static (G_TYPE_OBJECT, "RadareGUIList", &g_define_type_info, 0);
	}
	return radare_gui_list_type_id;
}




