
#ifndef __SHELL_H__
#define __SHELL_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_SHELL (radare_gui_shell_get_type ())
#define RADARE_GUI_SHELL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_SHELL, RadareGUIShell))
#define RADARE_GUI_SHELL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_SHELL, RadareGUIShellClass))
#define RADARE_GUI_IS_SHELL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_SHELL))
#define RADARE_GUI_IS_SHELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_SHELL))
#define RADARE_GUI_SHELL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_SHELL, RadareGUIShellClass))

typedef struct _RadareGUIShell RadareGUIShell;
typedef struct _RadareGUIShellClass RadareGUIShellClass;
typedef struct _RadareGUIShellPrivate RadareGUIShellPrivate;

struct _RadareGUIShell {
	GtkVBox parent_instance;
	RadareGUIShellPrivate * priv;
};

struct _RadareGUIShellClass {
	GtkVBoxClass parent_class;
};


GtkVBox* radare_gui_shell_get (RadareGUIShell* self);
RadareGUIShell* radare_gui_shell_construct (GType object_type);
RadareGUIShell* radare_gui_shell_new (void);
GType radare_gui_shell_get_type (void);


G_END_DECLS

#endif
