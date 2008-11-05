
#ifndef __DISASSEMBLER_H__
#define __DISASSEMBLER_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_DISASSEMBLER (radare_disassembler_get_type ())
#define RADARE_DISASSEMBLER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_DISASSEMBLER, RadareDisassembler))
#define RADARE_DISASSEMBLER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_DISASSEMBLER, RadareDisassemblerClass))
#define RADARE_IS_DISASSEMBLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_DISASSEMBLER))
#define RADARE_IS_DISASSEMBLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_DISASSEMBLER))
#define RADARE_DISASSEMBLER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_DISASSEMBLER, RadareDisassemblerClass))

typedef struct _RadareDisassembler RadareDisassembler;
typedef struct _RadareDisassemblerClass RadareDisassemblerClass;
typedef struct _RadareDisassemblerPrivate RadareDisassemblerPrivate;
typedef struct _RadareParamSpecDisassembler RadareParamSpecDisassembler;

struct _RadareDisassembler {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareDisassemblerPrivate * priv;
	char* INTEL;
	char* ARM;
	char* JAVA;
};

struct _RadareDisassemblerClass {
	GTypeClass parent_class;
	void (*finalize) (RadareDisassembler *self);
};

struct _RadareParamSpecDisassembler {
	GParamSpec parent_instance;
};


char* radare_disassembler_arch (void);
RadareDisassembler* radare_disassembler_construct (GType object_type);
RadareDisassembler* radare_disassembler_new (void);
GParamSpec* radare_param_spec_disassembler (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer radare_value_get_disassembler (const GValue* value);
void radare_value_set_disassembler (GValue* value, gpointer v_object);
GType radare_disassembler_get_type (void);
gpointer radare_disassembler_ref (gpointer instance);
void radare_disassembler_unref (gpointer instance);


G_END_DECLS

#endif
