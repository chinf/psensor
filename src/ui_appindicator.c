/*
    Copyright (C) 2010-2011 jeanfi@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#include "psensor.h"
#include "ui.h"
#include "ui_appindicator.h"
#include "ui_sensorpref.h"
#include "ui_pref.h"

static GtkMenuItem **sensor_menu_items;

static void cb_menu_show(GtkMenuItem *mi, gpointer data)
{
	struct ui_psensor *ui = (struct ui_psensor *)data;

	gtk_window_present(GTK_WINDOW(ui->main_window));
}

static void cb_menu_quit(GtkMenuItem *mi, gpointer data)
{
	ui_psensor_quit(data);
}

static void
cb_menu_preferences(GtkMenuItem *mi, gpointer data)
{
#ifdef HAVE_APPINDICATOR_029
	gdk_threads_enter();
#endif

	ui_pref_dialog_run((struct ui_psensor *)data);

#ifdef HAVE_APPINDICATOR_029
	gdk_threads_leave();
#endif
}

static void
cb_sensor_preferences(GtkMenuItem *mi, gpointer data)
{
	struct ui_psensor *ui = data;

#ifdef HAVE_APPINDICATOR_029
	gdk_threads_enter();
#endif

	if (ui->sensors && *ui->sensors)
		ui_sensorpref_dialog_run(*ui->sensors, ui);

#ifdef HAVE_APPINDICATOR_029
	gdk_threads_leave();
#endif
}

static const char *menu_desc =
"<ui>"
"  <popup name='MainMenu'>"
"      <menuitem name='Show' action='ShowAction' />"
"      <separator />"
"      <separator />"
"      <menuitem name='Preferences' action='PreferencesAction' />"
"      <menuitem name='SensorPreferences' action='SensorPreferencesAction' />"
"      <separator />"
"      <menuitem name='Quit' action='QuitAction' />"
"  </popup>"
"</ui>";

static GtkActionEntry entries[] = {
  { "PsensorMenuAction", NULL, "_Psensor" }, /* name, stock id, label */

  { "ShowAction", NULL,        /* name, stock id */
    "_Show", NULL, /* label, accelerator */
    "Show",                    /* tooltip */
    G_CALLBACK(cb_menu_show) },

  { "PreferencesAction", GTK_STOCK_PREFERENCES,     /* name, stock id */
    "_Preferences", NULL,                           /* label, accelerator */
    "Preferences",                                  /* tooltip */
    G_CALLBACK(cb_menu_preferences) },

  { "SensorPreferencesAction", GTK_STOCK_PREFERENCES,
    "S_ensor Preferences",
    NULL,
    "SensorPreferences",
    G_CALLBACK(cb_sensor_preferences) },

  { "QuitAction",
    GTK_STOCK_QUIT, "_Quit", NULL, "Quit", G_CALLBACK(cb_menu_quit) }
};
static guint n_entries = G_N_ELEMENTS(entries);

static void update_sensor_menu_item(GtkMenuItem *item, struct psensor *s)
{
	gchar *str;

	str = g_strdup_printf("%s: %2.f %s",
			      s->name,
			      psensor_get_current_value(s),
			      psensor_type_to_unit_str(s->type));

	gtk_menu_item_set_label(item, str);

	g_free(str);
}

static void update_sensor_menu_items(struct psensor **sensors)
{
	int n = psensor_list_size(sensors);
	int i;

	for (i = 0; i < n; i++)
		update_sensor_menu_item(sensor_menu_items[i],
					sensors[i]);
}

static GtkWidget *get_menu(struct ui_psensor *ui)
{
	GtkActionGroup *action_group;
	GtkUIManager *menu_manager;
	GError *error;
	GtkMenu *menu;
	int i;
	int n = psensor_list_size(ui->sensors);
	struct psensor **sensors = ui->sensors;


	action_group = gtk_action_group_new("PsensorActions");
	gtk_action_group_set_translation_domain(action_group, PACKAGE);
	menu_manager = gtk_ui_manager_new();

	gtk_action_group_add_actions(action_group, entries, n_entries, ui);
	gtk_ui_manager_insert_action_group(menu_manager, action_group, 0);

	error = NULL;
	gtk_ui_manager_add_ui_from_string(menu_manager, menu_desc, -1, &error);

	if (error)
		g_error(_("building menus failed: %s"), error->message);

	menu = GTK_MENU(gtk_ui_manager_get_widget(menu_manager, "/MainMenu"));

	sensor_menu_items = malloc(sizeof(GtkWidget *)*n);
	for (i = 0; i < n; i++) {
		struct psensor *s = sensors[i];

		sensor_menu_items[i]
			= GTK_MENU_ITEM(gtk_menu_item_new_with_label(s->name));

		gtk_menu_shell_insert(GTK_MENU_SHELL(menu),
				      GTK_WIDGET(sensor_menu_items[i]),
				      i+2);

		update_sensor_menu_item(sensor_menu_items[i],
					s);
	}


	return GTK_WIDGET(menu);
}


void ui_appindicator_update(struct ui_psensor *ui)
{
	struct psensor **sensor_cur = ui->sensors;
	AppIndicatorStatus status;
	int attention = 0;

	if (!ui->indicator)
		return;

	while (*sensor_cur) {
		struct psensor *s = *sensor_cur;

		if (s->alarm_enabled && s->alarm_raised) {
			attention = 1;
			break;
		}

		sensor_cur++;
	}

	status = app_indicator_get_status(ui->indicator);

	if (!attention && status == APP_INDICATOR_STATUS_ATTENTION)
		app_indicator_set_status
		    (ui->indicator, APP_INDICATOR_STATUS_ACTIVE);

	if (attention && status == APP_INDICATOR_STATUS_ACTIVE)
		app_indicator_set_status
		    (ui->indicator, APP_INDICATOR_STATUS_ATTENTION);

	update_sensor_menu_items(ui->sensors);
}

void ui_appindicator_init(struct ui_psensor *ui)
{
	GtkWidget *indicatormenu;

	ui->indicator
	    = app_indicator_new("psensor",
				"psensor",
				APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	app_indicator_set_status(ui->indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_attention_icon(ui->indicator, "psensor_hot");

	indicatormenu = get_menu(ui);

	gtk_widget_show_all(indicatormenu);

	app_indicator_set_menu(ui->indicator, GTK_MENU(indicatormenu));
}
