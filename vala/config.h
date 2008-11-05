
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_CONFIG (radare_config_get_type ())
#define RADARE_CONFIG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_CONFIG, RadareConfig))
#define RADARE_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_CONFIG, RadareConfigClass))
#define RADARE_IS_CONFIG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_CONFIG))
#define RADARE_IS_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_CONFIG))
#define RADARE_CONFIG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_CONFIG, RadareConfigClass))

typedef struct _RadareConfig RadareConfig;
typedef struct _RadareConfigClass RadareConfigClass;
typedef struct _RadareConfigPrivate RadareConfigPrivate;

struct _RadareConfig {
	GObject parent_instance;
	RadareConfigPrivate * priv;
};

struct _RadareConfigClass {
	GObjectClass parent_class;
};


void radare_config_set (RadareConfig* self, const char* key, const char* val);
void radare_config_set_i (RadareConfig* self, const char* key, glong i);
char* radare_config_get (RadareConfig* self, const char* key);
glong radare_config_get_i (RadareConfig* self, const char* key);
gboolean radare_config_rm (RadareConfig* self, const char* key);
void radare_config_list (RadareConfig* self, const char* mask);
RadareConfig* radare_config_construct (GType object_type);
RadareConfig* radare_config_new (void);
GType radare_config_get_type (void);
void config_init (void);
void* config_set (const char* key, const char* val);
void* config_set_i (const char* key, glong i);
char* config_get (const char* key);
glong config_get_i (const char* key);
gboolean config_rm (const char* key);
void config_list (const char* mask);


G_END_DECLS

#endif
