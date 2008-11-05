
#ifndef __IO_H__
#define __IO_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_IO (radare_io_get_type ())
#define RADARE_IO(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_IO, RadareIO))
#define RADARE_IO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_IO, RadareIOClass))
#define RADARE_IS_IO(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_IO))
#define RADARE_IS_IO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_IO))
#define RADARE_IO_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_IO, RadareIOClass))

typedef struct _RadareIO RadareIO;
typedef struct _RadareIOClass RadareIOClass;
typedef struct _RadareIOPrivate RadareIOPrivate;
typedef struct _RadareParamSpecIO RadareParamSpecIO;

struct _RadareIO {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareIOPrivate * priv;
};

struct _RadareIOClass {
	GTypeClass parent_class;
	void (*finalize) (RadareIO *self);
};

struct _RadareParamSpecIO {
	GParamSpec parent_instance;
};


gint radare_io_open (const char* file, gint flags, gint mode);
gint radare_io_close (gint fd);
gint radare_io_read (gint fd, char** buf, glong size);
gint radare_io_write (gint fd, const char* buf, glong size);
gint radare_io_system (const char* command);
RadareIO* radare_io_construct (GType object_type);
RadareIO* radare_io_new (void);
GParamSpec* radare_param_spec_io (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer radare_value_get_io (const GValue* value);
void radare_value_set_io (GValue* value, gpointer v_object);
GType radare_io_get_type (void);
gpointer radare_io_ref (gpointer instance);
void radare_io_unref (gpointer instance);
gint io_open (const char* file, gint flags, gint mode);
gint io_close (gint fd);
gint io_read (gint fd, char** buf, glong size);
gint io_write (gint fd, const char* buf, glong size);
gint io_system (const char* command);


G_END_DECLS

#endif
