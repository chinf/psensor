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
#include "graph.h"
#include "ui_graph.h"
#include "ui_pref.h"
#include "ui_sensorpref.h"

static void cb_menu_quit(GtkMenuItem *mi, gpointer data)
{
	ui_psensor_quit((struct ui_psensor *)data);
}

static void cb_preferences(GtkMenuItem *mi, gpointer data)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

static void cb_about(GtkMenuItem *mi, gpointer data)
{
	ui_show_about_dialog();
}

static void cb_sensor_preferences(GtkMenuItem *mi, gpointer data)
{
	struct ui_psensor *ui = data;

	if (ui->sensors && *ui->sensors)
		ui_sensorpref_dialog_run(*ui->sensors, ui);
}

static const char *menu_desc =
"<ui>"
"  <popup name='MainMenu'>"
"      <menuitem name='Preferences' action='PreferencesAction' />"
"      <menuitem name='SensorPreferences' action='SensorPreferencesAction' />"
"      <separator />"
"      <menuitem name='About' action='AboutAction' />"
"      <separator />"
"      <menuitem name='Quit' action='QuitAction' />"
"  </popup>"
"</ui>";

static GtkActionEntry entries[] = {
	{ "PsensorMenuAction", NULL, "_Psensor" },

	{ "PreferencesAction", GTK_STOCK_PREFERENCES,
	  "_Preferences", NULL,
	  "Preferences",
	  G_CALLBACK(cb_preferences) },

	{ "SensorPreferencesAction", GTK_STOCK_PREFERENCES,
	  "S_ensor Preferences", NULL,
	  "Sensor Preferences",
	  G_CALLBACK(cb_sensor_preferences) },

	{ "AboutAction", NULL,
	  "_About", NULL,
	  "About",
	  G_CALLBACK(cb_about) },

	{ "QuitAction",
	  GTK_STOCK_QUIT, "_Quit", NULL, "Quit", G_CALLBACK(cb_menu_quit) }
};
static guint n_entries = G_N_ELEMENTS(entries);

static GtkWidget *get_menu(struct ui_psensor *ui)
{
	GtkActionGroup      *action_group;
	GtkUIManager        *menu_manager;
	GError              *error;

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


static int
on_graph_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkWidget *menu;

	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	menu = get_menu((struct ui_psensor *)data);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

static gboolean
on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	struct ui_psensor *ui_psensor = (struct ui_psensor *)data;

	graph_update(ui_psensor->sensors,
		     ui_psensor->w_graph,
		     ui_psensor->config,
		     ui_psensor->main_window);

	return FALSE;
}

GtkWidget *ui_graph_create(struct ui_psensor * ui)
{
	GtkWidget *w_graph;

	w_graph = gtk_drawing_area_new();

	if (GTK_MAJOR_VERSION == 2)
		g_signal_connect(GTK_WIDGET(w_graph),
				 "expose-event",
				 G_CALLBACK(on_expose_event),
				 ui);
	else
		g_signal_connect(GTK_WIDGET(w_graph),
				 "draw",
				 G_CALLBACK(on_expose_event),
				 ui);

	gtk_widget_add_events(w_graph, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(GTK_WIDGET(w_graph),
			   "button_press_event",
			   (GCallback) on_graph_clicked, ui);

	return w_graph;
}
