
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define RADARE_TYPE_PLUGINS (radare_plugins_get_type ())
#define RADARE_PLUGINS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_PLUGINS, RadarePlugins))
#define RADARE_PLUGINS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_PLUGINS, RadarePluginsClass))
#define RADARE_IS_PLUGINS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_PLUGINS))
#define RADARE_IS_PLUGINS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_PLUGINS))
#define RADARE_PLUGINS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_PLUGINS, RadarePluginsClass))

typedef struct _RadarePlugins RadarePlugins;
typedef struct _RadarePluginsClass RadarePluginsClass;
typedef struct _RadarePluginsPrivate RadarePluginsPrivate;

/*
public interface Radare.Plugin
{
public abstract static void run(string arg);
}
*/
struct _RadarePlugins {
	GObject parent_instance;
	RadarePluginsPrivate * priv;
};

struct _RadarePluginsClass {
	GObjectClass parent_class;
};


void radare_plugins_list (void);
RadarePlugins* radare_plugins_construct (GType object_type);
RadarePlugins* radare_plugins_new (void);
GType radare_plugins_get_type (void);
void plugin_list (void);


G_END_DECLS

#endif
