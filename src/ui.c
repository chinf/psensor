/*
 * Copyright (C) 2010-2013 jeanfi@gmail.com
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

#include "cfg.h"
#include "slog.h"
#include "ui.h"
#include "ui_graph.h"
#include "ui_pref.h"
#include "ui_sensorpref.h"
#include "ui_sensorlist.h"
#include "ui_status.h"
#include "ui_appindicator.h"

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
			= gtk_paned_get_position(GTK_PANED(ui->sensor_box));

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

void ui_show_about_dialog()
{
	gtk_show_about_dialog
		(NULL,
		 "comments",
		 _("Psensor is a GTK+ application for monitoring hardware "
		   "sensors"),
		 "copyright",
		 _("Copyright(c) 2010-2012\njeanfi@gmail.com"),
		 "logo-icon-name", "psensor",
		 "program-name", "Psensor",
		 "title", _("About Psensor"),
		 "version", VERSION,
		 "website", PACKAGE_URL,
		 "website-label", _("Psensor Homepage"),
		 NULL);
}

void ui_cb_about(GtkMenuItem *mi, gpointer data)
{
	ui_show_about_dialog();
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

static void
slog_enabled_cbk(GConfClient *client, guint id, GConfEntry *e, gpointer data)
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
	GtkWidget *window;
	GdkPixbuf *icon;
	GtkIconTheme *icon_theme;
	struct config *cfg;
	guint ok;
	GtkBuilder *builder;
	GError *error;

	builder = gtk_builder_new();

	error = NULL;
	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "psensor.glade",
		 &error);

	if (!ok) {
		log_printf(LOG_ERR, error->message);
		g_error_free(error);
		return ;
	}

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	gtk_builder_connect_signals(builder, ui);
	cfg = ui->config;
	if (cfg->window_restore_enabled)
		gtk_window_move(GTK_WINDOW(window),
				cfg->window_x,
				cfg->window_y);

	config_slog_enabled_notify_add(slog_enabled_cbk, ui);

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

	gtk_window_set_decorated(GTK_WINDOW(window),
				 cfg->window_decoration_enabled);

	gtk_window_set_keep_below(GTK_WINDOW(window),
				  cfg->window_keep_below_enabled);

	ui->menu_bar = GTK_WIDGET(gtk_builder_get_object(builder, "menu_bar"));
	ui->main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main_box"));
	ui->popup_menu = GTK_WIDGET(gtk_builder_get_object(builder,
							   "popup_menu"));
	g_object_ref(G_OBJECT(ui->popup_menu));
	ui->main_window = window;
	ui->w_graph = GTK_WIDGET(gtk_builder_get_object(builder,
							"graph"));
	ui_graph_create(ui);

	ui->sensor_box = GTK_PANED(gtk_builder_get_object(builder,
							  "sensor_box"));
	ui->sensors_store = GTK_LIST_STORE(gtk_builder_get_object
					   (builder, "sensors_store"));
	ui->sensors_tree = GTK_TREE_VIEW(gtk_builder_get_object
					 (builder, "sensors_tree"));
	ui->sensors_scrolled_tree
		= GTK_SCROLLED_WINDOW(gtk_builder_get_object
				      (builder, "sensors_scrolled_tree"));

	ui_sensorlist_create(ui);

	log_debug("ui_window_create(): show_all");
	gtk_widget_show_all(ui->main_box);

	g_object_unref(G_OBJECT(builder));

	log_debug("ui_window_create() ends");
}

static void menu_bar_show(unsigned int show, struct ui_psensor *ui)
{
	if (show)
		gtk_widget_show(ui->menu_bar);
	else
		gtk_widget_hide(ui->menu_bar);
}

void ui_window_update(struct ui_psensor *ui)
{
	struct config *cfg;

	log_debug("ui_window_update()");

	cfg = ui->config;

	g_object_ref(GTK_WIDGET(ui->sensors_scrolled_tree));
	g_object_ref(GTK_WIDGET(ui->w_graph));

	gtk_container_remove(GTK_CONTAINER(ui->sensor_box),
			     GTK_WIDGET(ui->sensors_scrolled_tree));

	gtk_container_remove(GTK_CONTAINER(ui->sensor_box), ui->w_graph);

	gtk_container_remove(GTK_CONTAINER(ui->main_box),
			     GTK_WIDGET(ui->sensor_box));

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_LEFT)
		ui->sensor_box
			= GTK_PANED(gtk_paned_new(GTK_ORIENTATION_HORIZONTAL));
	else
		ui->sensor_box
			= GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));

	gtk_box_pack_end(GTK_BOX(ui->main_box),
			 GTK_WIDGET(ui->sensor_box), TRUE, TRUE, 2);

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_BOTTOM) {
		gtk_paned_pack1(ui->sensor_box,
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
		gtk_paned_pack2(ui->sensor_box,
				GTK_WIDGET(ui->sensors_scrolled_tree),
				FALSE, TRUE);
	} else {
		gtk_paned_pack1(ui->sensor_box,
				GTK_WIDGET(ui->sensors_scrolled_tree),
				FALSE, TRUE);
		gtk_paned_pack2(ui->sensor_box,
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
	}

	if (cfg->window_restore_enabled)
		gtk_paned_set_position(ui->sensor_box, cfg->window_divider_pos);

	g_object_unref(GTK_WIDGET(ui->sensors_scrolled_tree));
	g_object_unref(GTK_WIDGET(ui->w_graph));

	gtk_widget_show_all(GTK_WIDGET(ui->sensor_box));

	if (cfg->menu_bar_disabled)
		menu_bar_show(0, ui);
	else
		menu_bar_show(1, ui);
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

struct psensor **ui_get_sensors_ordered_by_position(struct ui_psensor *ui)
{
	struct psensor **result;

	result = psensor_list_copy(ui->sensors);
	qsort(result,
	      psensor_list_size(result),
	      sizeof(struct psensor *),
	      cmp_sensors);

	return result;
}
