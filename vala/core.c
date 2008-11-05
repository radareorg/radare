
#include "core.h"




enum  {
	RADARE_CORE_DUMMY_PROPERTY
};
char* radare_core_VERSION = NULL;
static gboolean radare_core_isinit = FALSE;
static void _radare_core_init_ (void);
static GObject * radare_core_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer radare_core_parent_class = NULL;
static void radare_core_finalize (GObject* obj);



/*
public static block_size {
get {
}
set {
}
}
*/
#line 17 "core.vala"
static void _radare_core_init_ (void) {
#line 18 "core.vala"
	radare_init ();
#line 19 "core.vala"
	plugin_init ();
#line 20 "core.vala"
	prepare_environment ("");
#line 21 "core.vala"
	radare_core_isinit = TRUE;
}


#line 28 "core.vala"
char* radare_core_command (const char* command) {
#line 28 "core.vala"
	g_return_val_if_fail (command != NULL, NULL);
#line 30 "core.vala"
	return pipe_command_to_string (command);
}


#line 33 "core.vala"
gint radare_core_cmd (const char* command) {
#line 33 "core.vala"
	g_return_val_if_fail (command != NULL, 0);
#line 35 "core.vala"
	return radare_cmd (command, 0);
}


#line 38 "core.vala"
gboolean radare_core_open (const char* file, gboolean write_mode) {
	gint ret;
#line 38 "core.vala"
	g_return_val_if_fail (file != NULL, FALSE);
#line 40 "core.vala"
	if (!radare_core_isinit) {
#line 40 "core.vala"
		_radare_core_init_ ();
	}
	ret = radare_open (file, (write_mode ? 1 : 0));
#line 42 "core.vala"
	return (ret == 0);
}


#line 45 "core.vala"
gboolean radare_core_seek (gulong offset) {
	gboolean ret;
	ret = radare_seek (offset, 0);
#line 48 "core.vala"
	radare_read (0);
#line 49 "core.vala"
	return ret;
}


/**
 * Returns the delta byte of the current block
 */
#line 55 "core.vala"
guchar radare_core_get (gint delta) {
#line 57 "core.vala"
	return radare_get (delta);
}


#line 60 "core.vala"
void radare_core_close (void) {
#line 62 "core.vala"
	radare_close ();
}


#line 3 "core.vala"
RadareCore* radare_core_construct (GType object_type) {
	RadareCore * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 3 "core.vala"
RadareCore* radare_core_new (void) {
#line 3 "core.vala"
	return radare_core_construct (RADARE_TYPE_CORE);
}


static GObject * radare_core_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	RadareCoreClass * klass;
	GObjectClass * parent_class;
	RadareCore * self;
	klass = RADARE_CORE_CLASS (g_type_class_peek (RADARE_TYPE_CORE));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = RADARE_CORE (obj);
	{
#line 25 "core.vala"
		_radare_core_init_ ();
	}
	return obj;
}


static void radare_core_class_init (RadareCoreClass * klass) {
	radare_core_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = radare_core_constructor;
	G_OBJECT_CLASS (klass)->finalize = radare_core_finalize;
	radare_core_VERSION = g_strdup ("0.8.8");
}


static void radare_core_instance_init (RadareCore * self) {
}


static void radare_core_finalize (GObject* obj) {
	RadareCore * self;
	self = RADARE_CORE (obj);
	G_OBJECT_CLASS (radare_core_parent_class)->finalize (obj);
}


GType radare_core_get_type (void) {
	static GType radare_core_type_id = 0;
	if (radare_core_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (RadareCoreClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) radare_core_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (RadareCore), 0, (GInstanceInitFunc) radare_core_instance_init, NULL };
		radare_core_type_id = g_type_register_static (G_TYPE_OBJECT, "RadareCore", &g_define_type_info, 0);
	}
	return radare_core_type_id;
}




