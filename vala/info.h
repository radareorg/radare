
#ifndef __INFO_H__
#define __INFO_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "radget.h"

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_RADGET_INFORMATION (radare_gui_radget_information_get_type ())
#define RADARE_GUI_RADGET_INFORMATION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_RADGET_INFORMATION, RadareGUIRadgetInformation))
#define RADARE_GUI_RADGET_INFORMATION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_RADGET_INFORMATION, RadareGUIRadgetInformationClass))
#define RADARE_GUI_IS_RADGET_INFORMATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_RADGET_INFORMATION))
#define RADARE_GUI_IS_RADGET_INFORMATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_RADGET_INFORMATION))
#define RADARE_GUI_RADGET_INFORMATION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_RADGET_INFORMATION, RadareGUIRadgetInformationClass))

typedef struct _RadareGUIRadgetInformation RadareGUIRadgetInformation;
typedef struct _RadareGUIRadgetInformationClass RadareGUIRadgetInformationClass;
typedef struct _RadareGUIRadgetInformationPrivate RadareGUIRadgetInformationPrivate;

struct _RadareGUIRadgetInformation {
	GObject parent_instance;
	RadareGUIRadgetInformationPrivate * priv;
};

struct _RadareGUIRadgetInformationClass {
	GObjectClass parent_class;
};


RadareGUIRadgetInformation* radare_gui_radget_information_construct (GType object_type);
RadareGUIRadgetInformation* radare_gui_radget_information_new (void);
GType radare_gui_radget_information_get_type (void);


G_END_DECLS

#endif
