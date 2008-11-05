
#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define RADARE_TYPE_DEBUGGER (radare_debugger_get_type ())
#define RADARE_DEBUGGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_DEBUGGER, RadareDebugger))
#define RADARE_DEBUGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_DEBUGGER, RadareDebuggerClass))
#define RADARE_IS_DEBUGGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_DEBUGGER))
#define RADARE_IS_DEBUGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_DEBUGGER))
#define RADARE_DEBUGGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_DEBUGGER, RadareDebuggerClass))

typedef struct _RadareDebugger RadareDebugger;
typedef struct _RadareDebuggerClass RadareDebuggerClass;
typedef struct _RadareDebuggerPrivate RadareDebuggerPrivate;
typedef struct _RadareParamSpecDebugger RadareParamSpecDebugger;

struct _RadareDebugger {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareDebuggerPrivate * priv;
};

struct _RadareDebuggerClass {
	GTypeClass parent_class;
	void (*finalize) (RadareDebugger *self);
};

struct _RadareParamSpecDebugger {
	GParamSpec parent_instance;
};


void radare_debugger_step (void);
void radare_debugger_step_over (void);
void radare_debugger_play (void);
RadareDebugger* radare_debugger_construct (GType object_type);
RadareDebugger* radare_debugger_new (void);
GParamSpec* radare_param_spec_debugger (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer radare_value_get_debugger (const GValue* value);
void radare_value_set_debugger (GValue* value, gpointer v_object);
GType radare_debugger_get_type (void);
gpointer radare_debugger_ref (gpointer instance);
void radare_debugger_unref (gpointer instance);


G_END_DECLS

#endif
