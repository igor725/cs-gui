#include <core.h>
#include <platform.h>
#include <gtk-3.0/gtk/gtk.h>
#include "backend.h"

struct {
	gboolean active;
	GtkTextBuffer *tbuf;
	GtkEntryBuffer *ebuf;
	GtkWidget *list;
	Mutex *mutex;
} mainCTX = {
	.active = false
};

#define LockContext(ret) \
if(!mainCTX.active) return ret; \
Mutex_Lock(mainCTX.mutex); \
if(!mainCTX.active) { \
	Mutex_Unlock(mainCTX.mutex); \
	return ret; \
}

#define UnlockContext() \
Mutex_Unlock(mainCTX.mutex)

void Backend_AddUser(cs_str name) {
	LockContext((void)0);
	GtkWidget *user = gtk_menu_item_new_with_label(name);
	gtk_container_add(GTK_CONTAINER(mainCTX.list), user);
	gtk_widget_show(user);
	UnlockContext();
}

void Backend_RemoveUser(cs_str name) {
	(void)name;
	// TODO: Разобраться, как удалять строки из листбокса
	// LockContext((void)0);
	// GList *start = gtk_container_get_children(GTK_CONTAINER(mainCTX.list));
	// for(GList *c = start; c != NULL; c = g_list_next(c)) {
		// GtkWidget *w = gtk_bin_get_child(GTK_BIN(c->data));
		// cs_str tgname = gtk_menu_item_get_label(GTK_MENU_ITEM(w));
		// if(String_CaselessCompare(tgname, name)) {
			// gtk_widget_destroy(w);
			// gtk_container_remove(GTK_CONTAINER(mainCTX.list), w);
			// break;
		// }
	// }
	// UnlockContext();
}

void Backend_SetConsoleText(cs_str txt) {
	LockContext((void)0);
	gtk_text_buffer_set_text(mainCTX.tbuf, txt, -1);
	UnlockContext();
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
	(void)app; (void)ud;
	LockContext((void)0);
	mainCTX.active = false;
	mainCTX.tbuf = NULL;
	mainCTX.ebuf = NULL;
	UnlockContext();
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

	mainCTX.list = gtk_list_box_new();
	gtk_grid_attach(GTK_GRID(grid), mainCTX.list, 1, 0, 1, 1);

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
	Mutex_Unlock(mainCTX.mutex);
}

void Backend_PreLaunch(void) {
	mainCTX.mutex = Mutex_Create();
}

void Backend_WindowLoop(void) {
	Mutex_Lock(mainCTX.mutex);
	GtkApplication *app;
	app = gtk_application_new("ru.igvx.cserver.gui", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	g_application_run(G_APPLICATION(app), 0, NULL);
	Mutex_Lock(mainCTX.mutex);
	g_object_unref(app);
	Mutex_Unlock(mainCTX.mutex);
	Mutex_Free(mainCTX.mutex);
}

void Backend_CloseWindow(void) {

}
