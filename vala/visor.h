
#ifndef __VISOR_H__
#define __VISOR_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_VISOR (radare_gui_visor_get_type ())
#define RADARE_GUI_VISOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_VISOR, RadareGUIVisor))
#define RADARE_GUI_VISOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_VISOR, RadareGUIVisorClass))
#define RADARE_GUI_IS_VISOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_VISOR))
#define RADARE_GUI_IS_VISOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_VISOR))
#define RADARE_GUI_VISOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_VISOR, RadareGUIVisorClass))

typedef struct _RadareGUIVisor RadareGUIVisor;
typedef struct _RadareGUIVisorClass RadareGUIVisorClass;
typedef struct _RadareGUIVisorPrivate RadareGUIVisorPrivate;

struct _RadareGUIVisor {
	GtkVBox parent_instance;
	RadareGUIVisorPrivate * priv;
};

struct _RadareGUIVisorClass {
	GtkVBoxClass parent_class;
};


GtkVBox* radare_gui_visor_get (RadareGUIVisor* self);
RadareGUIVisor* radare_gui_visor_construct (GType object_type);
RadareGUIVisor* radare_gui_visor_new (void);
GType radare_gui_visor_get_type (void);


G_END_DECLS

#endif
