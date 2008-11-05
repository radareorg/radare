
#ifndef __PANEL_H__
#define __PANEL_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_PANEL (radare_gui_panel_get_type ())
#define RADARE_GUI_PANEL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_PANEL, RadareGUIPanel))
#define RADARE_GUI_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_PANEL, RadareGUIPanelClass))
#define RADARE_GUI_IS_PANEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_PANEL))
#define RADARE_GUI_IS_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_PANEL))
#define RADARE_GUI_PANEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_PANEL, RadareGUIPanelClass))

typedef struct _RadareGUIPanel RadareGUIPanel;
typedef struct _RadareGUIPanelClass RadareGUIPanelClass;
typedef struct _RadareGUIPanelPrivate RadareGUIPanelPrivate;

struct _RadareGUIPanel {
	GtkScrolledWindow parent_instance;
	RadareGUIPanelPrivate * priv;
};

struct _RadareGUIPanelClass {
	GtkScrolledWindowClass parent_class;
};


void radare_gui_panel_refresh (RadareGUIPanel* self);
GtkWidget* radare_gui_panel_widget (RadareGUIPanel* self);
RadareGUIPanel* radare_gui_panel_construct (GType object_type);
RadareGUIPanel* radare_gui_panel_new (void);
GType radare_gui_panel_get_type (void);


G_END_DECLS

#endif
