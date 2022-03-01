#include <core.h>
#include <platform.h>
#include <gtk-3.0/gtk/gtk.h>
#include "backend.h"

struct {
	gboolean active;
	GtkTextBuffer *tbuf;
	GtkEntryBuffer *ebuf;
} mainCTX = {
	.active = false
};

void Backend_AddUser(cs_str name) {
	(void)name;
}

void Backend_RemoveUser(cs_str name) {
	(void)name;
}

void Backend_SetConsoleText(cs_str txt) {
	if(mainCTX.active)
		gtk_text_buffer_set_text(mainCTX.tbuf, txt, -1);
}

cs_size Backend_ScrollToEnd(void) {
	return 0;
}

cs_size Backend_GetScrollPosition(void) {
	return 0;
}

cs_size Backend_GetScrollEnd(void) {
	return 0;
}

cs_size Backend_GetInputText(cs_char *buff, cs_size len) {
	(void)buff; (void)len;
	return 0;
}

cs_bool Backend_ClearInputText(void) {
	return false;
}

static void destroy(GtkApplication *app, gpointer ud) {
	mainCTX.tbuf = NULL;
	mainCTX.active = false;
}

static void activate(GtkApplication *app, gpointer ud) {
	(void)ud;

	GtkWidget *window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), CSGUI_WINDOWTITLE);
	gtk_window_set_default_size(GTK_WINDOW(window), 870, 477);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	GtkWidget *grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), true);
	gtk_container_add(GTK_CONTAINER(window), grid);

	GtkWidget *output = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output), false);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(output), false);
	mainCTX.tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));
	gtk_grid_attach(GTK_GRID(grid), output, 0, 0, 1, 1);

	GtkWidget *list = gtk_list_box_new();

	gtk_grid_attach(GTK_GRID(grid), list, 1, 0, 1, 1);

	GtkWidget *entry = gtk_entry_new();
	mainCTX.ebuf = gtk_entry_get_buffer(GTK_ENTRY(entry));
	gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);

	GtkWidget *button = gtk_button_new();
	gtk_button_set_label(GTK_BUTTON(button), "Send");
	gtk_grid_attach(GTK_GRID(grid), button, 1, 1, 1, 1);

	gtk_widget_show_all(window);
	gtk_window_present(GTK_WINDOW(window));
	mainCTX.active = true;
	Backend_UpdateLog();
}

void Backend_WindowLoop(void) {
	GtkApplication *app;
	app = gtk_application_new("ru.igvx.cserver.gui", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);
}

void Backend_CloseWindow(void) {

}
