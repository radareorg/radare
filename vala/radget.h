
#ifndef __RADGET_H__
#define __RADGET_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_RADGET (radare_gui_radget_get_type ())
#define RADARE_GUI_RADGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_RADGET, RadareGUIRadget))
#define RADARE_GUI_IS_RADGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_RADGET))
#define RADARE_GUI_RADGET_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), RADARE_GUI_TYPE_RADGET, RadareGUIRadgetIface))

typedef struct _RadareGUIRadget RadareGUIRadget;
typedef struct _RadareGUIRadgetIface RadareGUIRadgetIface;

struct _RadareGUIRadgetIface {
	GTypeInterface parent_iface;
	void (*refresh) (RadareGUIRadget* self);
	char* (*get_name) (RadareGUIRadget* self);
	GtkBox* (*get_box) (RadareGUIRadget* self);
};


void radare_gui_radget_refresh (RadareGUIRadget* self);
char* radare_gui_radget_get_name (RadareGUIRadget* self);
GtkBox* radare_gui_radget_get_box (RadareGUIRadget* self);
GType radare_gui_radget_get_type (void);


G_END_DECLS

#endif
