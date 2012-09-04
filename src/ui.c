/*
 * Copyright (C) 2010-2012 jeanfi@gmail.com
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

static void cb_about(GtkMenuItem *mi, gpointer data)
{
	ui_show_about_dialog();
}

static void cb_menu_quit(GtkMenuItem *mi, gpointer data)
{
	ui_psensor_quit((struct ui_psensor *)data);
}

static void cb_preferences(GtkMenuItem *mi, gpointer data)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

static void cb_sensor_preferences(GtkMenuItem *mi, gpointer data)
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

static const char *menu_desc =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu name='Psensor' action='PsensorMenuAction'>"
"      <menuitem name='Preferences' action='PreferencesAction' />"
"      <menuitem name='SensorPreferences' action='SensorPreferencesAction' />"
"      <separator />"
"      <menuitem name='Quit' action='QuitAction' />"
"    </menu>"
"    <menu name='Help' action='HelpMenuAction'>"
"      <menuitem name='About' action='AboutAction' />"
"    </menu>"
"  </menubar>"
"</ui>";

static GtkActionEntry entries[] = {
	{ "PsensorMenuAction", NULL, "_Psensor" },

	{ "PreferencesAction", GTK_STOCK_PREFERENCES,
	  N_("_Preferences"), NULL,
	  N_("Preferences"),
	  G_CALLBACK(cb_preferences) },

	{ "SensorPreferencesAction", GTK_STOCK_PREFERENCES,
	  N_("S_ensor Preferences"), NULL,
	  N_("Sensor Preferences"),
	  G_CALLBACK(cb_sensor_preferences) },

	{ "QuitAction",
	  GTK_STOCK_QUIT, N_("_Quit"), NULL, N_("Quit"),
	  G_CALLBACK(cb_menu_quit) },

	{ "HelpMenuAction", NULL, N_("_Help") },

	{ "AboutAction", GTK_STOCK_PREFERENCES,
	  N_("_About"), NULL,
	  N_("About"),
	  G_CALLBACK(cb_about) }
};
static guint n_entries = G_N_ELEMENTS(entries);

static GtkWidget *get_menu(struct ui_psensor *ui)
{
	GtkActionGroup *action_group;
	GtkUIManager *menu_manager;
	GError *error;

	action_group = gtk_action_group_new("PsensorActions");
	gtk_action_group_set_translation_domain(action_group, PACKAGE);
	menu_manager = gtk_ui_manager_new();

	gtk_action_group_add_actions(action_group, entries, n_entries, ui);
	gtk_ui_manager_insert_action_group(menu_manager, action_group, 0);

	error = NULL;
	gtk_ui_manager_add_ui_from_string(menu_manager, menu_desc, -1, &error);

	if (error)
		g_error(_("building menus failed: %s"), error->message);

	return gtk_ui_manager_get_widget(menu_manager, "/MainMenu");
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

static void on_slog_enabled_cb(GConfClient *client,
			     guint cnxn_id,
			     GConfEntry *entry,
			     gpointer user_data)
{
	struct psensor **sensors;

	sensors = (struct psensor **)user_data;

	log_debug("cbk_slog_enabled");

	if (is_slog_enabled())
		slog_init(NULL, sensors);
	else
		slog_close(NULL, sensors);
}

void ui_window_create(struct ui_psensor *ui)
{
	GtkWidget *window, *menubar;
	GdkPixbuf *icon;
	GtkIconTheme *icon_theme;
	struct config *cfg;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	cfg = ui->config;
	if (cfg->window_restore_enabled)
		gtk_window_move(GTK_WINDOW(window),
				cfg->window_x,
				cfg->window_y);

	config_slog_enabled_notify_add(on_slog_enabled_cb, ui->sensors);

	gtk_window_set_default_size(GTK_WINDOW(window),
				    cfg->window_w,
				    cfg->window_h);

	gtk_window_set_title(GTK_WINDOW(window),
			     _("Psensor - Temperature Monitor"));
	gtk_window_set_role(GTK_WINDOW(window), "psensor");

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

	/* main box */
	menubar = get_menu(ui);

	ui->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_set_homogeneous(GTK_BOX(ui->main_box), FALSE);
	gtk_box_pack_start(GTK_BOX(ui->main_box), menubar,
			   FALSE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(window), ui->main_box);

	ui->main_window = window;
	ui->menu_bar = menubar;

	gtk_widget_show_all(ui->main_box);
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
	int init = 1;

	cfg = ui->config;

	if (ui->sensor_box) {
		g_object_ref(GTK_WIDGET(ui->ui_sensorlist->widget));

		gtk_container_remove(GTK_CONTAINER(ui->sensor_box),
				     ui->ui_sensorlist->widget);

		gtk_container_remove(GTK_CONTAINER(ui->main_box),
				     ui->sensor_box);

		ui->w_graph = ui_graph_create(ui);

		init = 0;
	}

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_LEFT)
		ui->sensor_box = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	else
		ui->sensor_box = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

	gtk_box_pack_end(GTK_BOX(ui->main_box), ui->sensor_box, TRUE, TRUE, 2);

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_BOTTOM) {
		gtk_paned_pack1(GTK_PANED(ui->sensor_box),
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
		gtk_paned_pack2(GTK_PANED(ui->sensor_box),
				ui->ui_sensorlist->widget, FALSE, TRUE);
	} else {
		gtk_paned_pack1(GTK_PANED(ui->sensor_box),
				ui->ui_sensorlist->widget, FALSE, TRUE);
		gtk_paned_pack2(GTK_PANED(ui->sensor_box),
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
	}

	if (cfg->window_restore_enabled)
		gtk_paned_set_position(GTK_PANED(ui->sensor_box),
				       ui->config->window_divider_pos);

	if (!init)
		g_object_unref(GTK_WIDGET(ui->ui_sensorlist->widget));

	gtk_widget_show_all(ui->sensor_box);

	if (cfg->menu_bar_disabled)
		menu_bar_show(0, ui);
	else
		menu_bar_show(1, ui);
}

void ui_window_show(struct ui_psensor *ui)
{
	log_debug("ui_window_show()");
	gtk_window_present(GTK_WINDOW(ui->main_window));
}
