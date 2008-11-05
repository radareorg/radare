
#ifndef __LINE_H__
#define __LINE_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define TYPE_LINE (line_get_type ())
#define LINE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_LINE, Line))
#define LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_LINE, LineClass))
#define IS_LINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_LINE))
#define IS_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_LINE))
#define LINE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_LINE, LineClass))

typedef struct _Line Line;
typedef struct _LineClass LineClass;
typedef struct _LinePrivate LinePrivate;

struct _Line {
	GtkHBox parent_instance;
	LinePrivate * priv;
};

struct _LineClass {
	GtkHBoxClass parent_class;
};


Line* line_name (Line* self, const char* name, const char* val);
Line* line_numeric (Line* self, const char* name, gint number);
Line* line_construct (GType object_type);
Line* line_new (void);
GType line_get_type (void);


G_END_DECLS

#endif
