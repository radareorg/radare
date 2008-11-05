
#ifndef __UTILS_H__
#define __UTILS_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_UTILS (radare_utils_get_type ())
#define RADARE_UTILS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_UTILS, RadareUtils))
#define RADARE_UTILS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_UTILS, RadareUtilsClass))
#define RADARE_IS_UTILS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_UTILS))
#define RADARE_IS_UTILS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_UTILS))
#define RADARE_UTILS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_UTILS, RadareUtilsClass))

typedef struct _RadareUtils RadareUtils;
typedef struct _RadareUtilsClass RadareUtilsClass;
typedef struct _RadareUtilsPrivate RadareUtilsPrivate;
typedef struct _RadareParamSpecUtils RadareParamSpecUtils;

struct _RadareUtils {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareUtilsPrivate * priv;
};

struct _RadareUtilsClass {
	GTypeClass parent_class;
	void (*finalize) (RadareUtils *self);
};

struct _RadareParamSpecUtils {
	GParamSpec parent_instance;
};


gulong radare_utils_value (const char* str);
gulong radare_utils_math (const char* str);
char* radare_utils_get (const char* str);
gint radare_utils_set (const char* foo, const char* bar);
RadareUtils* radare_utils_construct (GType object_type);
RadareUtils* radare_utils_new (void);
GParamSpec* radare_param_spec_utils (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer radare_value_get_utils (const GValue* value);
void radare_value_set_utils (GValue* value, gpointer v_object);
GType radare_utils_get_type (void);
gpointer radare_utils_ref (gpointer instance);
void radare_utils_unref (gpointer instance);
char* getenv (const char* str);
gint setenv (const char* foo, const char* bar, gint force);
gulong get_offset (const char* str);
gulong get_math (const char* str);


G_END_DECLS

#endif
