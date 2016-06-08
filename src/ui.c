/*
 * Copyright (C) 2010-2016 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include <stdlib.h>

#include <cfg.h>
#include <slog.h>
#include <ui.h>
#include <ui_appindicator.h>
#include <ui_graph.h>
#include <ui_pref.h>
#include <ui_sensorlist.h>
#include <ui_sensorpref.h>
#include <ui_status.h>

static GtkWidget *w_sensors_scrolled_tree;
static GtkWidget *w_graph;
static GtkContainer *w_sensor_box;
static GtkContainer *w_main_box;

static void update_layout(void)
{
	enum sensorlist_position sensorlist_pos;

	g_object_ref(w_sensors_scrolled_tree);
	g_object_ref(w_graph);

	gtk_container_remove(w_sensor_box,
			     w_sensors_scrolled_tree);

	gtk_container_remove(w_sensor_box, w_graph);

	gtk_container_remove(w_main_box, GTK_WIDGET(w_sensor_box));

	sensorlist_pos = config_get_sensorlist_position();
	if (sensorlist_pos == SENSORLIST_POSITION_RIGHT
	    || sensorlist_pos == SENSORLIST_POSITION_LEFT)
		w_sensor_box
			= GTK_CONTAINER(gtk_paned_new
					(GTK_ORIENTATION_HORIZONTAL));
	else
		w_sensor_box
			= GTK_CONTAINER(gtk_paned_new
					(GTK_ORIENTATION_VERTICAL));

	gtk_box_pack_end(GTK_BOX(w_main_box),
			 GTK_WIDGET(w_sensor_box), TRUE, TRUE, 2);

	if (sensorlist_pos == SENSORLIST_POSITION_RIGHT
	    || sensorlist_pos == SENSORLIST_POSITION_BOTTOM) {
		gtk_paned_pack1(GTK_PANED(w_sensor_box), w_graph, TRUE, TRUE);
		gtk_paned_pack2(GTK_PANED(w_sensor_box),
				w_sensors_scrolled_tree,
				FALSE,
				TRUE);
	} else {
		gtk_paned_pack1(GTK_PANED(w_sensor_box),
				w_sensors_scrolled_tree,
				FALSE,
				TRUE);
		gtk_paned_pack2(GTK_PANED(w_sensor_box), w_graph, TRUE, TRUE);
	}

	g_object_unref(w_sensors_scrolled_tree);
	g_object_unref(w_graph);

	gtk_widget_show_all(GTK_WIDGET(w_sensor_box));
}

static void set_decoration(GtkWindow *win)
{
	gtk_window_set_decorated(win, config_is_window_decoration_enabled());
}

static void set_keep_below(GtkWindow *win)
{
	gtk_window_set_keep_below(win, config_is_window_keep_below_enabled());
}

static void set_menu_bar_enabled(GtkWidget *bar)
{
	if (config_is_menu_bar_enabled())
		gtk_widget_show(bar);
	else
		gtk_widget_hide(bar);
}

static void
decoration_changed_cbk(GSettings *settings, gchar *key, gpointer data)
{
	set_decoration(GTK_WINDOW(data));
}

static void
keep_below_changed_cbk(GSettings *settings, gchar *key, gpointer data)
{
	set_keep_below(GTK_WINDOW(data));
}

static void
menu_bar_changed_cbk(GSettings *settings, gchar *key, gpointer data)
{
	set_menu_bar_enabled(GTK_WIDGET(data));
}

static void
sensorlist_position_changed_cbk(GSettings *settings, gchar *key, gpointer data)
{
	update_layout();
}

static void connect_cbks(GtkWindow *win, GtkWidget *menu_bar)
{
	log_fct_enter();

	g_signal_connect_after(config_get_GSettings(),
			       "changed::interface-window-decoration-disabled",
			       G_CALLBACK(decoration_changed_cbk),
			       win);

	g_signal_connect_after(config_get_GSettings(),
			       "changed::interface-window-keep-below-enabled",
			       G_CALLBACK(keep_below_changed_cbk),
			       win);

	g_signal_connect_after(config_get_GSettings(),
			       "changed::interface-menu-bar-disabled",
			       G_CALLBACK(menu_bar_changed_cbk),
			       menu_bar);

	g_signal_connect_after(config_get_GSettings(),
			       "changed::interface-sensorlist-position",
			       G_CALLBACK(sensorlist_position_changed_cbk),
			       menu_bar);


	log_fct_exit();
}

static void save_window_pos(struct ui_psensor *ui)
{
	gboolean visible;
	GtkWindow *win;
	struct config *cfg;

	visible = gtk_widget_get_visible(ui->main_window);
	log_debug("Window visible: %d", visible);

	if (visible == TRUE) {
		cfg = ui->config;

		win = GTK_WINDOW(ui->main_window);

		gtk_window_get_position(win, &cfg->window_x, &cfg->window_y);
		log_debug("Window position: %d %d",
			  cfg->window_x,
			  cfg->window_y);

		gtk_window_get_size(win,
				    &cfg->window_w,
				    &cfg->window_h);
		log_debug("Window size: %d %d", cfg->window_w, cfg->window_h);

		cfg->window_divider_pos
			= gtk_paned_get_position(GTK_PANED(w_sensor_box));

		config_save(cfg);
	}
}

static gboolean
on_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	struct ui_psensor *ui = data;

	save_window_pos(ui);

	log_debug("is_status_supported: %d\n", is_status_supported());

	if (is_appindicator_supported() || is_status_supported())
		gtk_widget_hide(ui->main_window);
	else
		ui_psensor_quit(ui);

	return TRUE;
}

void ui_show_about_dialog(GtkWindow *parent)
{
	static const char *const authors[] = { "jeanfi@gmail.com", NULL };

	log_fct("parent=%p", parent);

	gtk_show_about_dialog
		(parent,
		 "authors", authors,
		 "comments",
		 _("Psensor is a GTK+ application for monitoring hardware "
		   "sensors"),
		 "copyright",
		 _("Copyright(c) 2010-2016 jeanfi@gmail.com"),
#if GTK_CHECK_VERSION(3, 12, 0)
		 "license-type", GTK_LICENSE_GPL_2_0,
#endif
		 "logo-icon-name", "psensor",
		 "program-name", "Psensor",
		 "title", _("About Psensor"),
		 "translator-credits", _("translator-credits"),
		 "version", VERSION,
		 "website", PACKAGE_URL,
		 "website-label", _("Psensor Homepage"),
		 NULL);
}

void ui_cb_about(GtkAction *a, gpointer data)
{
	struct ui_psensor *ui;
	GtkWidget *parent;

	ui = (struct ui_psensor *)data;

	log_fct("ui=%p", ui);

	if (ui)
		parent = ui->main_window;
	else
		parent = NULL;

	ui_show_about_dialog(GTK_WINDOW(parent));
}

void ui_cb_menu_quit(GtkMenuItem *mi, gpointer data)
{
	ui_psensor_quit((struct ui_psensor *)data);
}

void ui_cb_preferences(GtkMenuItem *mi, gpointer data)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

void ui_cb_sensor_preferences(GtkMenuItem *mi, gpointer data)
{
	struct ui_psensor *ui = data;

	if (ui->sensors && *ui->sensors)
		ui_sensorpref_dialog_run(*ui->sensors, ui);
}

void ui_psensor_quit(struct ui_psensor *ui)
{
	save_window_pos(ui);

	log_debug("Destroy main window");
	gtk_widget_destroy(ui->main_window);
	gtk_main_quit();
}

void ui_enable_alpha_channel(struct ui_psensor *ui)
{
	GdkScreen *screen;
	GdkVisual *visual;
	struct config *cfg;

	cfg = ui->config;

	screen = gtk_widget_get_screen(ui->main_window);

	log_debug("Config alpha channel enabled: %d",
		  cfg->alpha_channel_enabled);
	if (cfg->alpha_channel_enabled && gdk_screen_is_composited(screen)) {
		log_debug("Screen is composited");
		visual = gdk_screen_get_rgba_visual(screen);
		if (visual) {
			gtk_widget_set_visual(ui->main_window, visual);
		} else {
			cfg->alpha_channel_enabled = 0;
			log_err("Enable alpha channel has failed");
		}
	} else {
		cfg->alpha_channel_enabled = 0;
	}
}

static void slog_enabled_cbk(void *data)
{
	struct ui_psensor *ui;
	struct psensor **sensors;
	pthread_mutex_t *mutex;

	ui = (struct ui_psensor *)data;
	sensors = ui->sensors;
	mutex = &ui->sensors_mutex;

	log_debug("slog_enabled_cbk");

	if (is_slog_enabled())
		slog_activate(NULL, sensors, mutex, config_get_slog_interval());
	else
		slog_close();
}

void ui_window_create(struct ui_psensor *ui)
{
	GtkWidget *window, *menu_bar;
	GdkPixbuf *icon;
	GtkIconTheme *icon_theme;
	struct config *cfg;
	guint ok;
	GtkBuilder *builder;
	GError *error;

	log_fct("ui=%p", ui);

	builder = gtk_builder_new();

	error = NULL;
	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "psensor.glade",
		 &error);

	if (!ok) {
		log_printf(LOG_ERR, error->message);
		g_error_free(error);
		return;
	}

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	gtk_builder_connect_signals(builder, ui);
	cfg = ui->config;
	if (cfg->window_restore_enabled)
		gtk_window_move(GTK_WINDOW(window),
				cfg->window_x,
				cfg->window_y);

	config_set_slog_enabled_changed_cbk(slog_enabled_cbk, ui);

	gtk_window_set_default_size(GTK_WINDOW(window),
				    cfg->window_w,
				    cfg->window_h);

	icon_theme = gtk_icon_theme_get_default();
	icon = gtk_icon_theme_load_icon(icon_theme, "psensor", 48, 0, NULL);
	if (icon)
		gtk_window_set_icon(GTK_WINDOW(window), icon);
	else
		log_err(_("Failed to load Psensor icon."));

	g_signal_connect(window,
			 "delete_event", G_CALLBACK(on_delete_event_cb), ui);

	set_decoration(GTK_WINDOW(window));
	set_keep_below(GTK_WINDOW(window));

	menu_bar = GTK_WIDGET(gtk_builder_get_object(builder, "menu_bar"));
	w_main_box = GTK_CONTAINER(gtk_builder_get_object(builder, "main_box"));
	ui->popup_menu = GTK_WIDGET(gtk_builder_get_object(builder,
							   "popup_menu"));
	g_object_ref(G_OBJECT(ui->popup_menu));
	ui->main_window = window;
	w_graph = GTK_WIDGET(gtk_builder_get_object(builder, "graph"));
	ui_graph_create(ui);

	w_sensor_box = GTK_CONTAINER(gtk_builder_get_object(builder,
							    "sensor_box"));
	ui->sensors_store = GTK_LIST_STORE(gtk_builder_get_object
					   (builder, "sensors_store"));
	ui->sensors_tree = GTK_TREE_VIEW(gtk_builder_get_object
					 (builder, "sensors_tree"));
	w_sensors_scrolled_tree
		= GTK_WIDGET(gtk_builder_get_object
			     (builder, "sensors_scrolled_tree"));

	ui_sensorlist_create(ui);

	connect_cbks(GTK_WINDOW(window), menu_bar);

	log_debug("ui_window_create(): show_all");
	update_layout();
	gtk_widget_show_all(GTK_WIDGET(w_main_box));
	set_menu_bar_enabled(menu_bar);

	g_object_unref(G_OBJECT(builder));

	log_debug("ui_window_create() ends");
}

void ui_window_update(struct ui_psensor *ui)
{
	struct config *cfg;

	log_debug("ui_window_update()");

	cfg = ui->config;

	if (cfg->window_restore_enabled)
		gtk_paned_set_position(GTK_PANED(w_sensor_box),
				       cfg->window_divider_pos);

}

void ui_window_show(struct ui_psensor *ui)
{
	log_debug("ui_window_show()");
	ui_window_update(ui);
	gtk_window_present(GTK_WINDOW(ui->main_window));
}

static int cmp_sensors(const void *p1, const void *p2)
{
	const struct psensor *s1, *s2;
	int pos1, pos2;

	s1 = *(void **)p1;
	s2 = *(void **)p2;

	pos1 = config_get_sensor_position(s1->id);
	pos2 = config_get_sensor_position(s2->id);

	return pos1 - pos2;
}

struct psensor **ui_get_sensors_ordered_by_position(struct psensor **sensors)
{
	struct psensor **result;

	result = psensor_list_copy(sensors);
	qsort(result,
	      psensor_list_size(result),
	      sizeof(struct psensor *),
	      cmp_sensors);

	return result;
}

GtkWidget *ui_get_graph(void)
{
	return w_graph;
}
