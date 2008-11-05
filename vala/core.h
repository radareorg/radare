
#ifndef __CORE_H__
#define __CORE_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_CORE (radare_core_get_type ())
#define RADARE_CORE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_CORE, RadareCore))
#define RADARE_CORE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_CORE, RadareCoreClass))
#define RADARE_IS_CORE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_CORE))
#define RADARE_IS_CORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_CORE))
#define RADARE_CORE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_CORE, RadareCoreClass))

typedef struct _RadareCore RadareCore;
typedef struct _RadareCoreClass RadareCoreClass;
typedef struct _RadareCorePrivate RadareCorePrivate;

struct _RadareCore {
	GObject parent_instance;
	RadareCorePrivate * priv;
};

struct _RadareCoreClass {
	GObjectClass parent_class;
};


extern char* radare_core_VERSION;
char* radare_core_command (const char* command);
gint radare_core_cmd (const char* command);
gboolean radare_core_open (const char* file, gboolean write_mode);
gboolean radare_core_seek (gulong offset);
guchar radare_core_get (gint delta);
void radare_core_close (void);
RadareCore* radare_core_construct (GType object_type);
RadareCore* radare_core_new (void);
GType radare_core_get_type (void);
char* pipe_command_to_string (const char* command);
gint radare_cmd (const char* command, gint arg);
gint system (const char* str);
gint radare_init (void);
gint plugin_init (void);
void radare_read (gint next);
gint radare_close (void);
gint radare_open (const char* file, gint allow_write);
void prepare_environment (const char* str);
gboolean radare_seek (gulong offset, gint which);
guchar radare_get (gint delta);


G_END_DECLS

#endif
