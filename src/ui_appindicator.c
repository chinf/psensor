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

static void cb_menu_show(gpointer data, guint cb_action, GtkWidget *item)
{
	struct ui_psensor *ui = (struct ui_psensor *)data;

	gtk_window_present(GTK_WINDOW(ui->main_window));
}

static void cb_menu_quit(gpointer data, guint cb_action, GtkWidget *item)
{
	ui_psensor_quit(data);
}

static void
cb_menu_preferences(gpointer data, guint cb_action, GtkWidget *item)
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
cb_sensor_preferences(gpointer data, guint cb_action, GtkWidget *item)
{
	struct ui_psensor *ui = data;

	if (ui->sensors && *ui->sensors)
		ui_sensorpref_dialog_run(*ui->sensors, ui);
}

static GtkItemFactoryEntry menu_items[] = {
	{"/Show",
	 NULL, cb_menu_show, 0, "<Item>"},
	{"/Preferences",
	 NULL, cb_menu_preferences, 0, "<Item>"},
	{"/Sensor Preferences",
	 NULL, cb_sensor_preferences, 0, "<Item>"},
	{"/sep1",
	 NULL, NULL, 0, "<Separator>"},
	{"/Quit",
	 "", cb_menu_quit, 0, "<StockItem>", GTK_STOCK_QUIT},
};

static gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
static GtkWidget *get_menu(struct ui_psensor *ui)
{
	GtkItemFactory *item_factory;

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);

	gtk_item_factory_create_items(item_factory,
				      nmenu_items, menu_items, ui);
	return gtk_item_factory_get_widget(item_factory, "<main>");
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
	app_indicator_set_menu(ui->indicator, GTK_MENU(indicatormenu));
}
