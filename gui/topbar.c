/*
 * Copyright (C) 2008
 *       pancake <@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "main.h"
#include <string.h>
#include <stdlib.h>

GtkWidget *combo;
GtkWidget *arch;
GtkWidget *entry;

void core_cmd(const char *cmd)
{
	vte_terminal_feed_child(VTE_TERMINAL(term), cmd, strlen(cmd));
}

void core_cmd_end()
{
	gtk_widget_grab_focus(term);
}

void arch_mode_changed(GtkComboBox *widget, gpointer user_data)
{
	switch(gtk_combo_box_get_active(widget)) {
	case 0: core_cmd(":eval asm.arch = intel16\n\n"); break;
	case 1: core_cmd(":eval asm.arch = intel32\n\n"); break;
	case 2: core_cmd(":eval asm.arch = intel64\n\n"); break;
	case 3: core_cmd(":eval asm.arch = arm\n\n"); break;
	case 4: core_cmd(":eval asm.arch = ppc\n\n"); break;
	case 5: core_cmd(":eval asm.arch = 68k\n\n"); break;
	case 6: core_cmd(":eval asm.arch = java\n\n"); break;
	case 7: core_cmd(":eval asm.arch = mips\n\n"); break;
	case 8: core_cmd(":eval asm.arch = csr\n\n"); break;
	case 9: core_cmd(":eval asm.arch = sparc\n\n"); break;
	}

	core_cmd_end();
}

void print_mode_changed(GtkComboBox *widget, gpointer user_data)
{
	core_cmd("Q");

	switch(gtk_combo_box_get_active(widget)) {
	case 0: core_cmd(":px\n\n"); break;
	case 1: core_cmd(":pv\n\n"); break;
	case 2: core_cmd(":pD\n\n"); break;
	case 3: core_cmd(":po\n\n"); break;
	case 4: core_cmd(":pb\n\n"); break;
	case 5: core_cmd(":pa\n\n"); break;
	case 6: core_cmd(":ps\n\n"); break;
	case 7: core_cmd(":pX\n\n"); break;
	case 8: core_cmd(":pr\n\n"); break;
	}
	core_cmd("V\n");

	core_cmd_end();
}

void commandline_activated(GtkEntry *entry, gpointer  user_data)
{
	char *buf;
	const char *cmd = gtk_entry_get_text(entry);
	int len = strlen(cmd);

	gtk_entry_select_region(entry, 0, len);
	buf = (char *)malloc(len+5);
	snprintf(buf, len+4, ":%s\n\n", cmd);
	core_cmd(buf);
}

GtkWidget *gradare_topbar_new()
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

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

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
 #if __i386__
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 1);
 #elif __x86_x64__
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 2);
 #elif __arm__
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 3);
 #elif __powerpc__ || __POWERPC__
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 4);
 #elif __mips__
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 7);
 #else
	/* default is x86-32 */
	gtk_combo_box_set_active(GTK_COMBO_BOX(arch), 1);
 #endif
#endif
	g_signal_connect(GTK_COMBO_BOX(arch), "changed", GTK_SIGNAL_FUNC(arch_mode_changed), 0);
	gtk_box_pack_end(GTK_BOX(hbox), arch, FALSE, FALSE, 0);

	return hbox;
}
