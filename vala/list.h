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

#ifndef __LIST_H__
#define __LIST_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_LIST (radare_gui_list_get_type ())
#define RADARE_GUI_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_LIST, RadareGUIList))
#define RADARE_GUI_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_LIST, RadareGUIListClass))
#define RADARE_GUI_IS_LIST(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_LIST))
#define RADARE_GUI_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_LIST))
#define RADARE_GUI_LIST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_LIST, RadareGUIListClass))

typedef struct _RadareGUIList RadareGUIList;
typedef struct _RadareGUIListClass RadareGUIListClass;
typedef struct _RadareGUIListPrivate RadareGUIListPrivate;

/*
 *
 * Implements a Widget for GTK containing a list
 * of selectable items with scrollbar and treeview.
 *         - Yay
 *
 */
struct _RadareGUIList {
	GObject parent_instance;
	RadareGUIListPrivate * priv;
};

struct _RadareGUIListClass {
	GObjectClass parent_class;
};


void radare_gui_list_update (RadareGUIList* self);
RadareGUIList* radare_gui_list_with_title (RadareGUIList* self, const char* title);
void radare_gui_list_add (RadareGUIList* self, const char* item);
char* radare_gui_list_get (RadareGUIList* self);
void radare_gui_list_clear (RadareGUIList* self);
RadareGUIList* radare_gui_list_construct (GType object_type);
RadareGUIList* radare_gui_list_new (void);
GtkScrolledWindow* radare_gui_list_get_widget (RadareGUIList* self);
void radare_gui_list_set_widget (RadareGUIList* self, GtkScrolledWindow* value);
GType radare_gui_list_get_type (void);


G_END_DECLS

#endif
