
#ifndef __INOUT_H__
#define __INOUT_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_IN_OUT (radare_in_out_get_type ())
#define RADARE_IN_OUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_IN_OUT, RadareInOut))
#define RADARE_IN_OUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_IN_OUT, RadareInOutClass))
#define RADARE_IS_IN_OUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_IN_OUT))
#define RADARE_IS_IN_OUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_IN_OUT))
#define RADARE_IN_OUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_IN_OUT, RadareInOutClass))

typedef struct _RadareInOut RadareInOut;
typedef struct _RadareInOutClass RadareInOutClass;
typedef struct _RadareInOutPrivate RadareInOutPrivate;

struct _RadareInOut {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareInOutPrivate * priv;
};

struct _RadareInOutClass {
	GTypeClass parent_class;
	void (*finalize) (RadareInOut *self);
};


gpointer radare_in_out_ref (gpointer instance);
void radare_in_out_unref (gpointer instance);
GParamSpec* radare_param_spec_in_out (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
void radare_value_set_in_out (GValue* value, gpointer v_object);
gpointer radare_value_get_in_out (const GValue* value);
GType radare_in_out_get_type (void);
gint radare_in_out_open (const char* file, gint flags, gint mode);
gint radare_in_out_close (gint fd);
gint radare_in_out_read (gint fd, char** buf, glong size);
gint radare_in_out_write (gint fd, const char* buf, glong size);
gint radare_in_out_system (const char* command);
RadareInOut* radare_in_out_new (void);
RadareInOut* radare_in_out_construct (GType object_type);
gint io_open (const char* file, gint flags, gint mode);
gint io_close (gint fd);
gint io_read (gint fd, char** buf, glong size);
gint io_write (gint fd, const char* buf, glong size);
gint io_system (const char* command);


G_END_DECLS

#endif
