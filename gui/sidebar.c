#include "main.h"
#include <string.h>

void arch_mode_changed(GtkComboBox *widget, gpointer user_data)
{
	switch(gtk_combo_box_get_active(widget)) {
	case 0: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = intel16\n\n", 27);
		break;
	case 1: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = intel32\n\n", 27);
		break;
	case 2: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = intel64\n\n", 27);
		break;
	case 3: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = arm\n\n", 23);
		break;
	case 4: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = ppc\n\n", 23);
		break;
	case 5: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = 68k\n\n", 23);
		break;
	case 6: vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.arch = java\n\n", 24);
		break;
	}

	gtk_widget_grab_focus(term);
}

void print_mode_changed(GtkComboBox *widget, gpointer user_data)
{
	vte_terminal_feed_child(VTE_TERMINAL(term), "Q", 1);
	switch(gtk_combo_box_get_active(widget)) {
	case 0: vte_terminal_feed_child(VTE_TERMINAL(term), ":px\n\n", 5);
		break;
	case 1: vte_terminal_feed_child(VTE_TERMINAL(term), ":pv\n\n", 5);
		break;
	case 2: vte_terminal_feed_child(VTE_TERMINAL(term), ":pD\n\n", 5);
		break;
	case 3: vte_terminal_feed_child(VTE_TERMINAL(term), ":po\n\n", 5);
		break;
	case 4: vte_terminal_feed_child(VTE_TERMINAL(term), ":pb\n\n", 5);
		break;
	case 5: vte_terminal_feed_child(VTE_TERMINAL(term), ":pa\n\n", 5);
		break;
	case 6: vte_terminal_feed_child(VTE_TERMINAL(term), ":ps\n\n", 5);
		break;
	case 7: vte_terminal_feed_child(VTE_TERMINAL(term), ":pX\n\n", 5);
		break;
	case 8: vte_terminal_feed_child(VTE_TERMINAL(term), ":pr\n\n", 5);
		break;
	}
	vte_terminal_feed_child(VTE_TERMINAL(term), "V\n", 2);

	gtk_widget_grab_focus(term);
}

void commandline_activated(GtkEntry *entry, gpointer  user_data)
{
	const char *cmd = gtk_entry_get_text(entry);
	gtk_entry_select_region(entry, 0, strlen(cmd));

	vte_terminal_feed_child(VTE_TERMINAL(term), ":", 1);
	vte_terminal_feed_child(VTE_TERMINAL(term), cmd, strlen(cmd));
	vte_terminal_feed_child(VTE_TERMINAL(term), "\n\n", 2);

//	gtk_widget_grab_focus(term);
}

GtkWidget *combo;
GtkWidget *arch;
GtkWidget *entry;

GtkWidget *gradare_sidebar_new()
{
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	GtkWidget *refresh;

	/**
	refresh = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_box_pack_start(GTK_BOX(hbox), refresh, FALSE, FALSE, 0);
	**/

	entry = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox), entry);
	g_signal_connect(entry, "activate", G_CALLBACK(commandline_activated), 0);

	/* add arch combo box */
	combo = gtk_combo_box_new_text();
	gtk_combo_set_use_arrows( GTK_COMBO(combo),1);
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
	gtk_combo_set_use_arrows( GTK_COMBO_BOX(arch),1);
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 0, "intel16");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 1, "intel32");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 2, "intel64");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 3, "arm");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 4, "ppc");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 5, "68k");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(arch), 6, "java");
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 1);
	g_signal_connect(GTK_COMBO_BOX(arch), "changed", GTK_SIGNAL_FUNC(arch_mode_changed), 0);
	gtk_box_pack_end(GTK_BOX(hbox), arch, FALSE, FALSE, 0);

	return hbox;
}
