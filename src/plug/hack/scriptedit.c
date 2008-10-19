/* license gplv3 */
/* author pancake */
/* script editor for radare/gtk */

#include <gtk/gtk.h>
#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static GtkWidget *my_widget = NULL;
static GtkButton *but = NULL;
static GtkHButtonBox *hbb = NULL;
static const char *filename = NULL;
static GtkWidget *text = NULL;
static GtkWidget *swin = NULL;
static GtkWidget *lang_w = NULL;
static GtkLabel *filename_w = NULL;
static int (*r)(const char *cmd, int log) = NULL;
static const char *lang = "lua";

static int do_save()
{
	const char *buf;
	GtkTextBuffer *tebu;
	GtkTextIter from, to;
	FILE *fd = fopen(filename, "w");
	if (fd != NULL) {
		tebu = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
		gtk_text_buffer_get_start_iter(tebu, &from);
		gtk_text_buffer_get_end_iter(tebu, &to);
		buf = gtk_text_buffer_get_text( tebu, &from, &to, FALSE);
		fwrite(buf, strlen(buf), 1, fd);
		fclose(fd);
	} else {
		printf("ERROR: Cannot save file here\n");
		// TODO: show an error dialog here
		return 0;
	}

	return 1;
}

static int scriptedit_save()
{
	FILE *fd;
	const char *buf;
	const char *file;
	GtkTextBuffer *tebu;
	GtkTextIter from, to;
	GtkWidget *fcd = gtk_file_chooser_dialog_new (
		"Save as...", NULL, // parent
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_set_position( GTK_WINDOW(fcd), GTK_WIN_POS_CENTER);
	if ( gtk_dialog_run(GTK_DIALOG(fcd)) == GTK_RESPONSE_ACCEPT ) {
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
		free(filename);
		filename = strdup(file);
		//printf("FILE NAME IS (%s)\n", file);
		gtk_label_set_text(filename_w, filename);
		if (do_save() == 0) {
			printf("ERROR: Cannot save file here\n");
			// TODO: show an error dialog here
        		gtk_widget_destroy(GTK_WIDGET(fcd));
			return 0;
		}
	}
        gtk_widget_destroy(GTK_WIDGET(fcd));
	return 1;
}

static void scriptedit_execute()
{
	char buf[4096];
	if (filename == NULL) {
		if (! scriptedit_save() )
			return;
	} else do_save();
	switch(gtk_combo_box_get_active(lang_w)) {
	case 0: lang = "lua"; break;
	case 1: lang = "python"; break;
	case 2: lang = "perl"; break;
	case 3: lang = "ruby"; break;
	case 4: lang = "radare"; 
		sprintf(buf, ". %s", filename);
		r(buf, 0);
		return;
	}
	sprintf(buf, "H %s %s", lang, filename);
	r(buf, 0);
}

static void scriptedit_open()
{
	long sz;
	char *buf;
	const char *file;
	FILE *fd;
	GtkWidget *fcd = gtk_file_chooser_dialog_new (
		"Select script...", NULL, // parent
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_window_set_position( GTK_WINDOW(fcd), GTK_WIN_POS_CENTER);
	if ( gtk_dialog_run(GTK_DIALOG(fcd)) == GTK_RESPONSE_ACCEPT ) {
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
		free(filename);
		filename = strdup(file);
		fd = fopen(filename,"r");
		if (fd != NULL) {
			gtk_label_set_text(filename_w, filename);
			fseek(fd, 0, SEEK_END);
			sz = ftell(fd);
			fseek(fd, 0, SEEK_SET);
			buf = (char *)malloc(sz+1);
			fread(buf, sz, 1, fd);
			fclose(fd);
			gtk_text_buffer_set_text(
				gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)),buf, strlen(buf));
			free(buf);
		} else {
			printf("ERROR: Cannot open file\n");
        		gtk_widget_destroy(GTK_WIDGET(fcd));
			// TODO: show an error dialog here
			return 0;
		}
	}
        gtk_widget_destroy(GTK_WIDGET(fcd));
	return 1;
}

static int my_hack(char *input)
{
	static int dry = 0;
	if (dry) return 0; dry=1;

	my_widget = gtk_vbox_new(FALSE, 3);
	/* filename:
          [  text area   ]
          open, save button, run button */

	if (r == NULL)
		r = radare_plugin.resolve("radare_cmd");

	filename_w = gtk_label_new("");
	gtk_box_pack_start(my_widget, filename_w, FALSE, FALSE, 0);

	text = gtk_text_view_new();
gtk_text_view_set_border_window_size(text,GTK_TEXT_WINDOW_TEXT, 2);
	swin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(swin),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_container_add(swin, text);
	gtk_container_add(my_widget, swin);

	hbb = gtk_hbutton_box_new();
	gtk_container_set_border_width(GTK_CONTAINER(hbb), 5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbb), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbb), 5);

	lang_w = gtk_combo_box_new_text();
	gtk_combo_box_insert_text(GTK_COMBO_BOX(lang_w), 0, "lua");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(lang_w), 1, "python");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(lang_w), 2, "perl");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(lang_w), 3, "ruby");
	gtk_combo_box_insert_text(GTK_COMBO_BOX(lang_w), 4, "radare");
	gtk_combo_box_set_active(lang_w, 0);
	gtk_box_pack_end(GTK_CONTAINER(hbb), lang_w, FALSE, FALSE, 0);

	but = gtk_button_new_from_stock("gtk-open");
	g_signal_connect(but, "released", G_CALLBACK(scriptedit_open), NULL);
	g_signal_connect(but, "activate", G_CALLBACK(scriptedit_open), NULL);
	gtk_box_pack_end(GTK_CONTAINER(hbb), but, FALSE, FALSE, 0);

	but = gtk_button_new_from_stock("gtk-save");
	g_signal_connect(but, "released", G_CALLBACK(scriptedit_save), NULL);
	g_signal_connect(but, "activate", G_CALLBACK(scriptedit_save), NULL);
	gtk_box_pack_end(GTK_CONTAINER(hbb), but, FALSE, FALSE, 0);

	but = gtk_button_new_from_stock("gtk-execute");
	g_signal_connect(but, "released", G_CALLBACK(scriptedit_execute), NULL);
	g_signal_connect(but, "activate", G_CALLBACK(scriptedit_execute), NULL);
	gtk_box_pack_end(GTK_CONTAINER(hbb), but, FALSE, FALSE, 0);

	gtk_box_pack_end(GTK_CONTAINER(my_widget), hbb, FALSE, FALSE, 0);

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_GUI;
struct plugin_hack_t radare_plugin = {
	.name = "scriptedit",
	.desc = "Script editor in GTK",
	.callback = &my_hack,
	.widget = &my_widget
};
