#include <gtk/gtk.h>
#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static int (*r)(const char *cmd, int log);
static GtkWidget *combo;
static GtkWidget *arch;
static GtkWidget *entry;
static GtkWidget *my_widget = NULL;

static int core_cmd(const char *cmd)
{
	return r(cmd, 0);
#if 0
	vte_terminal_feed_child(VTE_TERMINAL(term), cmd, strlen(cmd));
#endif
}

static void core_cmd_end()
{
#if 0
	gtk_widget_grab_focus(term);
#endif
}

static void arch_mode_changed(GtkComboBox *widget, gpointer user_data)
{
	switch(gtk_combo_box_get_active(widget)) {
	case 0: core_cmd(":eval asm.arch = intel16"); break;
	case 1: core_cmd(":eval asm.arch = intel32"); break;
	case 2: core_cmd(":eval asm.arch = intel64"); break;
	case 3: core_cmd(":eval asm.arch = arm"); break;
	case 4: core_cmd(":eval asm.arch = ppc"); break;
	case 5: core_cmd(":eval asm.arch = 68k"); break;
	case 6: core_cmd(":eval asm.arch = java"); break;
	case 7: core_cmd(":eval asm.arch = mips"); break;
	case 8: core_cmd(":eval asm.arch = csr"); break;
	case 9: core_cmd(":eval asm.arch = sparc"); break;
	}

	core_cmd_end();
}

static void print_mode_changed(GtkComboBox *widget, gpointer user_data)
{
	core_cmd("Q");

	switch(gtk_combo_box_get_active(widget)) {
	case 0: core_cmd(":px\n"); break;
	case 1: core_cmd(":pv\n"); break;
	case 2: core_cmd(":pD\n"); break;
	case 3: core_cmd(":po\n"); break;
	case 4: core_cmd(":pb\n"); break;
	case 5: core_cmd(":pa\n"); break;
	case 6: core_cmd(":ps\n"); break;
	case 7: core_cmd(":pX\n"); break;
	case 8: core_cmd(":pr\n"); break;
	}
	core_cmd("V\n");

	core_cmd_end();
}

static void commandline_activated(GtkEntry *entry, gpointer  user_data)
{
	char *buf;
	const char *cmd = gtk_entry_get_text(entry);
	int len = strlen(cmd);

	gtk_entry_select_region(entry, 0, len);
	buf = (char *)malloc(len+5);
	snprintf(buf, len+4, ":%s\n", cmd);
	core_cmd(buf);
}

static GtkWidget *gradare_topbar_new()
{
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	//GtkWidget *refresh;

	/**
	refresh = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_box_pack_start(GTK_BOX(hbox), refresh, FALSE, FALSE, 0);
	**/

	entry = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox), entry);
	g_signal_connect(entry, "activate", G_CALLBACK(commandline_activated), 0);

	/* add arch combo box */
	combo = gtk_combo_box_new_text();
	//gtk_combo_set_use_arrows( GTK_COMBO(combo),1);
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 0, "hexadecimal");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 1, "debugger");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 2, "disassembly");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 3, "octal");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 4, "binary");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 5, "shellcode");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 6, "string");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 7, "hex pairs");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), 8, "raw");

	gtk_combo_box_set_active(GTK_COMBO(combo), 0);

	g_signal_connect(GTK_COMBO_BOX(combo), "changed", GTK_SIGNAL_FUNC(print_mode_changed), 0);
	gtk_box_pack_end(GTK_BOX(hbox), combo, FALSE, FALSE, 0);

	/* add arch combo box */
	arch = gtk_combo_box_new_text();
	//gtk_combo_set_use_arrows( GTK_COMBO_BOX(arch),1);
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 0, "intel16");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 1, "intel32");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 2, "intel64");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 3, "arm");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 4, "ppc");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 5, "68k");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 6, "java");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 7, "mips");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 8, "csr");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 9, "sparc");
#if _MAEMO_
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 3);
#else
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 1);
#endif
	g_signal_connect(GTK_COMBO_BOX(arch), "changed", GTK_SIGNAL_FUNC(arch_mode_changed), 0);
	gtk_box_pack_end(GTK_BOX(hbox), arch, FALSE, FALSE, 0);

//gtk_widget_show_all(hbox);
	return hbox;
}

/* STUB */
static int my_hack(char *input)
{
	static int dry = 0;
	if (dry) return 0; dry=1;
	r = radare_plugin.resolve("radare_cmd");
	my_widget = gradare_topbar_new();
	if (r != NULL)
		return 1;
	return 0;
}
int radare_plugin_type = PLUGIN_TYPE_GUI;
struct plugin_hack_t radare_plugin = {
	.name     = "gtk-topbar",
	.desc     = "GTK top bar (entry cmd and arch/view)",
	.callback = &my_hack,
	.widget   = &my_widget
};
