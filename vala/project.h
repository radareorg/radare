
#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_PROJECT (radare_project_get_type ())
#define RADARE_PROJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_PROJECT, RadareProject))
#define RADARE_PROJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_PROJECT, RadareProjectClass))
#define RADARE_IS_PROJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_PROJECT))
#define RADARE_IS_PROJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_PROJECT))
#define RADARE_PROJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_PROJECT, RadareProjectClass))

typedef struct _RadareProject RadareProject;
typedef struct _RadareProjectClass RadareProjectClass;
typedef struct _RadareProjectPrivate RadareProjectPrivate;
typedef struct _RadareParamSpecProject RadareParamSpecProject;

struct _RadareProject {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareProjectPrivate * priv;
};

struct _RadareProjectClass {
	GTypeClass parent_class;
	void (*finalize) (RadareProject *self);
};

struct _RadareParamSpecProject {
	GParamSpec parent_instance;
};


gboolean radare_project_open (const char* file);
gboolean radare_project_save (const char* file);
void radare_project_info (const char* file);
RadareProject* radare_project_construct (GType object_type);
RadareProject* radare_project_new (void);
GParamSpec* radare_param_spec_project (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer radare_value_get_project (const GValue* value);
void radare_value_set_project (GValue* value, gpointer v_object);
GType radare_project_get_type (void);
gpointer radare_project_ref (gpointer instance);
void radare_project_unref (gpointer instance);
gboolean project_open (const char* file);
gboolean project_save (const char* file);
void project_info (const char* file);


G_END_DECLS

#endif
