
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "radget.h"

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_RADGET_DEBUGGER (radare_gui_radget_debugger_get_type ())
#define RADARE_GUI_RADGET_DEBUGGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_RADGET_DEBUGGER, RadareGUIRadgetDebugger))
#define RADARE_GUI_RADGET_DEBUGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_RADGET_DEBUGGER, RadareGUIRadgetDebuggerClass))
#define RADARE_GUI_IS_RADGET_DEBUGGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_RADGET_DEBUGGER))
#define RADARE_GUI_IS_RADGET_DEBUGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_RADGET_DEBUGGER))
#define RADARE_GUI_RADGET_DEBUGGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_RADGET_DEBUGGER, RadareGUIRadgetDebuggerClass))

typedef struct _RadareGUIRadgetDebugger RadareGUIRadgetDebugger;
typedef struct _RadareGUIRadgetDebuggerClass RadareGUIRadgetDebuggerClass;
typedef struct _RadareGUIRadgetDebuggerPrivate RadareGUIRadgetDebuggerPrivate;

struct _RadareGUIRadgetDebugger {
	GObject parent_instance;
	RadareGUIRadgetDebuggerPrivate * priv;
	GtkVBox* vb;
};

struct _RadareGUIRadgetDebuggerClass {
	GObjectClass parent_class;
};


RadareGUIRadgetDebugger* radare_gui_radget_debugger_construct (GType object_type);
RadareGUIRadgetDebugger* radare_gui_radget_debugger_new (void);
GType radare_gui_radget_debugger_get_type (void);


G_END_DECLS

#endif
