
#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "radget.h"

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_RADGET_SEARCH (radare_gui_radget_search_get_type ())
#define RADARE_GUI_RADGET_SEARCH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_RADGET_SEARCH, RadareGUIRadgetSearch))
#define RADARE_GUI_RADGET_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_RADGET_SEARCH, RadareGUIRadgetSearchClass))
#define RADARE_GUI_IS_RADGET_SEARCH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_RADGET_SEARCH))
#define RADARE_GUI_IS_RADGET_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_RADGET_SEARCH))
#define RADARE_GUI_RADGET_SEARCH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_RADGET_SEARCH, RadareGUIRadgetSearchClass))

typedef struct _RadareGUIRadgetSearch RadareGUIRadgetSearch;
typedef struct _RadareGUIRadgetSearchClass RadareGUIRadgetSearchClass;
typedef struct _RadareGUIRadgetSearchPrivate RadareGUIRadgetSearchPrivate;

struct _RadareGUIRadgetSearch {
	GObject parent_instance;
	RadareGUIRadgetSearchPrivate * priv;
	GtkVBox* vb;
};

struct _RadareGUIRadgetSearchClass {
	GObjectClass parent_class;
};


RadareGUIRadgetSearch* radare_gui_radget_search_construct (GType object_type);
RadareGUIRadgetSearch* radare_gui_radget_search_new (void);
GType radare_gui_radget_search_get_type (void);


G_END_DECLS

#endif
