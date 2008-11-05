
#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "list.h"
#include "term.h"
#include "visor.h"

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_MAIN_WINDOW (radare_gui_main_window_get_type ())
#define RADARE_GUI_MAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_MAIN_WINDOW, RadareGUIMainWindow))
#define RADARE_GUI_MAIN_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_MAIN_WINDOW, RadareGUIMainWindowClass))
#define RADARE_GUI_IS_MAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_MAIN_WINDOW))
#define RADARE_GUI_IS_MAIN_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_MAIN_WINDOW))
#define RADARE_GUI_MAIN_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_MAIN_WINDOW, RadareGUIMainWindowClass))

typedef struct _RadareGUIMainWindow RadareGUIMainWindow;
typedef struct _RadareGUIMainWindowClass RadareGUIMainWindowClass;
typedef struct _RadareGUIMainWindowPrivate RadareGUIMainWindowPrivate;

struct _RadareGUIMainWindow {
	GtkWindow parent_instance;
	RadareGUIMainWindowPrivate * priv;
	RadareGUIList* left_list;
	RadareGUIList* right_list;
	RadareGUITerm* shell;
	RadareGUIVisor* visor;
	RadareGUITerm* con;
};

struct _RadareGUIMainWindowClass {
	GtkWindowClass parent_class;
};


extern GtkStatusbar* radare_gui_main_window_statusbar;
RadareGUIMainWindow* radare_gui_main_window_construct (GType object_type);
RadareGUIMainWindow* radare_gui_main_window_new (void);
GType radare_gui_main_window_get_type (void);


G_END_DECLS

#endif
