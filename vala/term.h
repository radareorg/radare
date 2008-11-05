
#ifndef __TERM_H__
#define __TERM_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define RADARE_GUI_TYPE_TERM (radare_gui_term_get_type ())
#define RADARE_GUI_TERM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_GUI_TYPE_TERM, RadareGUITerm))
#define RADARE_GUI_TERM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_GUI_TYPE_TERM, RadareGUITermClass))
#define RADARE_GUI_IS_TERM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_GUI_TYPE_TERM))
#define RADARE_GUI_IS_TERM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_GUI_TYPE_TERM))
#define RADARE_GUI_TERM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_GUI_TYPE_TERM, RadareGUITermClass))

typedef struct _RadareGUITerm RadareGUITerm;
typedef struct _RadareGUITermClass RadareGUITermClass;
typedef struct _RadareGUITermPrivate RadareGUITermPrivate;

struct _RadareGUITerm {
	GtkVBox parent_instance;
	RadareGUITermPrivate * priv;
};

struct _RadareGUITermClass {
	GtkVBoxClass parent_class;
};


GtkVBox* radare_gui_term_get (RadareGUITerm* self);
RadareGUITerm* radare_gui_term_construct (GType object_type);
RadareGUITerm* radare_gui_term_new (void);
GType radare_gui_term_get_type (void);


G_END_DECLS

#endif
