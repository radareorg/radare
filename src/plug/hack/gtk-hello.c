/* example plugin */

#include <gtk/gtk.h>
#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static GtkWidget *my_widget = NULL;

int my_hack(const char *input)
{
	static int dry = 0;
	int (*r)(char *cmd, int log);

	if (dry) return 0; dry=1;

	my_widget = gtk_label_new("Hello World!");

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_GUI;
struct plugin_hack_t radare_plugin = {
	.name = "gtk-hello",
	.desc = "GTK hello hack example",
	.callback = &my_hack,
	.widget = &my_widget
};
