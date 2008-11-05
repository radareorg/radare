
#ifndef __LOGS_H__
#define __LOGS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_LOGS (radare_gui_logs_get_type ())
#define RADARE_GUI_LOGS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_LOGS, RadareGUILogs))
#define RADARE_GUI_LOGS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_LOGS, RadareGUILogsClass))
#define RADARE_GUI_IS_LOGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_LOGS))
#define RADARE_GUI_IS_LOGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_LOGS))
#define RADARE_GUI_LOGS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_LOGS, RadareGUILogsClass))

typedef struct _RadareGUILogs RadareGUILogs;
typedef struct _RadareGUILogsClass RadareGUILogsClass;
typedef struct _RadareGUILogsPrivate RadareGUILogsPrivate;

struct _RadareGUILogs {
	GtkVBox parent_instance;
	RadareGUILogsPrivate * priv;
};

struct _RadareGUILogsClass {
	GtkVBoxClass parent_class;
};


GtkVBox* radare_gui_logs_get (RadareGUILogs* self);
RadareGUILogs* radare_gui_logs_construct (GType object_type);
RadareGUILogs* radare_gui_logs_new (void);
GType radare_gui_logs_get_type (void);


G_END_DECLS

#endif
