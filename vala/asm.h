
#ifndef __ASM_H__
#define __ASM_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define RADARE_TYPE_ASM (radare_asm_get_type ())
#define RADARE_ASM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), RADARE_TYPE_ASM, RadareAsm))
#define RADARE_ASM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), RADARE_TYPE_ASM, RadareAsmClass))
#define RADARE_IS_ASM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RADARE_TYPE_ASM))
#define RADARE_IS_ASM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RADARE_TYPE_ASM))
#define RADARE_ASM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), RADARE_TYPE_ASM, RadareAsmClass))

typedef struct _RadareAsm RadareAsm;
typedef struct _RadareAsmClass RadareAsmClass;
typedef struct _RadareAsmPrivate RadareAsmPrivate;
typedef struct _RadareParamSpecAsm RadareParamSpecAsm;

struct _RadareAsm {
	GTypeInstance parent_instance;
	volatile int ref_count;
	RadareAsmPrivate * priv;
};

struct _RadareAsmClass {
	GTypeClass parent_class;
	void (*finalize) (RadareAsm *self);
};

struct _RadareParamSpecAsm {
	GParamSpec parent_instance;
};


gboolean radare_asm_setArchitecture (const char* arch);
char* radare_asm_getArchitecture (void);
char* radare_asm_asm (const char* code);
char* radare_asm_dasm (const char* hexpairs);
RadareAsm* radare_asm_construct (GType object_type);
RadareAsm* radare_asm_new (void);
GParamSpec* radare_param_spec_asm (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer radare_value_get_asm (const GValue* value);
void radare_value_set_asm (GValue* value, gpointer v_object);
GType radare_asm_get_type (void);
gpointer radare_asm_ref (gpointer instance);
void radare_asm_unref (gpointer instance);


G_END_DECLS

#endif
