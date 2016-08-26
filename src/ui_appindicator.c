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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#include <cfg.h>
#include <psensor.h>
#include <ui.h>
#include <ui_appindicator.h>
#include <ui_sensorpref.h>
#include <ui_status.h>
#include <ui_pref.h>

static const char *ICON = "psensor_normal";
static const char *ATTENTION_ICON = "psensor_hot";

static const char *GLADE_FILE
= PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "psensor-appindicator.glade";

static struct psensor **sensors;
static GtkMenuItem **menu_items;
static bool appindicator_supported = true;
static AppIndicator *indicator;
static struct ui_psensor *ui_psensor;

void ui_appindicator_menu_show_cb(GtkMenuItem *mi, gpointer data)
{
	ui_window_show((struct ui_psensor *)data);
}

void ui_appindicator_cb_preferences(GtkMenuItem *mi, gpointer data)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

void ui_appindicator_cb_sensor_preferences(GtkMenuItem *mi, gpointer data)
{
	struct ui_psensor *ui = data;

	if (ui->sensors && *ui->sensors)
		ui_sensorpref_dialog_run(*ui->sensors, ui);
}

static void
update_menu_item(GtkMenuItem *item, struct psensor *s, int use_celsius)
{
	gchar *str;
	char *v;

	v = psensor_current_value_to_str(s, use_celsius);

	str = g_strdup_printf("%s: %s", s->name, v);

	gtk_menu_item_set_label(item, str);

	free(v);
	g_free(str);
}

static void update_menu_items(int use_celsius)
{
	struct psensor **s;
	GtkMenuItem **m;

	if (!sensors)
		return;

	for (s = sensors, m = menu_items; *s; s++, m++)
		update_menu_item(*m, *s, use_celsius);
}

static void
create_sensor_menu_items(const struct ui_psensor *ui, GtkMenu *menu)
{
	int i, j, n, celsius;
	const char *name;
	struct psensor **sorted_sensors;

	if (config_get_temperature_unit() == CELSIUS)
		celsius = 1;
	else
		celsius = 0;

	sorted_sensors = ui_get_sensors_ordered_by_position(ui->sensors);
	n = psensor_list_size(sorted_sensors);
	menu_items = malloc((n + 1) * sizeof(GtkWidget *));

	sensors = malloc((n + 1) * sizeof(struct psensor *));
	for (i = 0, j = 0; i < n; i++) {
		if (config_is_appindicator_enabled(sorted_sensors[i]->id)) {
			sensors[j] = sorted_sensors[i];
			name = sensors[j]->name;

			menu_items[j] = GTK_MENU_ITEM
				(gtk_menu_item_new_with_label(name));

			gtk_menu_shell_insert(GTK_MENU_SHELL(menu),
					      GTK_WIDGET(menu_items[j]),
					      j+2);

			update_menu_item(menu_items[j], sensors[j], celsius);

			j++;
		}
	}

	sensors[j] = NULL;
	menu_items[j] = NULL;

	free(sorted_sensors);
}

static GtkMenu *load_menu(struct ui_psensor *ui)
{
	GError *error;
	GtkMenu *menu;
	guint ok;
	GtkBuilder *builder;

	log_fct_enter();

	builder = gtk_builder_new();

	error = NULL;
	ok = gtk_builder_add_from_file(builder, GLADE_FILE, &error);

	if (!ok) {
		log_err(_("Failed to load glade file %s: %s"),
			GLADE_FILE,
			error->message);
		g_error_free(error);
		return NULL;
	}

	menu = GTK_MENU(gtk_builder_get_object(builder, "appindicator_menu"));
	create_sensor_menu_items(ui, menu);
	gtk_builder_connect_signals(builder, ui);

	g_object_ref(G_OBJECT(menu));
	g_object_unref(G_OBJECT(builder));

	log_fct_exit();

	return menu;
}

static void update_label(struct ui_psensor *ui)
{
	char *label, *str, *tmp, *guide;
	struct psensor **p;
	int use_celsius;

	p =  ui_get_sensors_ordered_by_position(ui->sensors);
	label = NULL;
	guide = NULL;

	if (config_get_temperature_unit() == CELSIUS)
		use_celsius = 1;
	else
		use_celsius = 0;

	while (*p) {
		if (config_is_appindicator_label_enabled((*p)->id)) {
			str = psensor_current_value_to_str(*p, use_celsius);

			if (label == NULL) {
				label = str;
			} else {
				tmp = malloc(strlen(label)
					     + 1
					     + strlen(str)
					     + 1);
				sprintf(tmp, "%s %s", label, str);
				free(label);
				free(str);
				label = tmp;
			}

			if (is_temp_type((*p)->type))
				str = "999UUU";
			else if ((*p)->type & SENSOR_TYPE_RPM)
				str = "999UUU";
			else /* percent */
				str = "999%";

			if (guide == NULL) {
				guide = strdup(str);
			} else {
				tmp = malloc(strlen(guide)
					     + 1
					     + strlen(str)
					     + 1);
				sprintf(tmp, "%sW%s", guide, str);
				free(guide);
				guide = tmp;
			}

		}
		p++;
	}

	app_indicator_set_label(indicator, label, guide);
}

void ui_appindicator_update(struct ui_psensor *ui, bool attention)
{
	AppIndicatorStatus status;

	if (!indicator)
		return;

	update_label(ui);

	status = app_indicator_get_status(indicator);

	if (!attention && status == APP_INDICATOR_STATUS_ATTENTION)
		app_indicator_set_status(indicator,
					 APP_INDICATOR_STATUS_ACTIVE);

	if (attention && status == APP_INDICATOR_STATUS_ACTIVE)
		app_indicator_set_status(indicator,
		APP_INDICATOR_STATUS_ATTENTION);

	if (config_get_temperature_unit() == CELSIUS)
		update_menu_items(1);
	else
		update_menu_items(0);
}

static GtkStatusIcon *unity_fallback(AppIndicator *indicator)
{
	GtkStatusIcon *ico;

	log_debug("ui_appindicator.unity_fallback()");

	appindicator_supported = false;

	ico = ui_status_get_icon(ui_psensor);

	ui_status_set_visible(1);

	return ico;
}

static void
unity_unfallback(AppIndicator *indicator, GtkStatusIcon *status_icon)
{
	log_debug("ui_appindicator.unity_unfallback()");

	ui_status_set_visible(0);

	appindicator_supported = true;
}

static void remove_sensor_menu_items(GtkMenu *menu)
{
	GtkMenuItem **items;

	if (!menu_items)
		return;

	items = menu_items;
	while (*items) {
		gtk_container_remove(GTK_CONTAINER(menu), GTK_WIDGET(*items));

		items++;
	}

	free(menu_items);
	free(sensors);
}

void ui_appindicator_update_menu(struct ui_psensor *ui)
{
	GtkMenu *menu;

	menu = GTK_MENU(app_indicator_get_menu(indicator));

	if (menu) {
		remove_sensor_menu_items(menu);
		create_sensor_menu_items(ui, menu);
	} else {
		menu = load_menu(ui);

		if (menu) {
			app_indicator_set_menu(indicator, menu);
			g_object_unref(G_OBJECT(menu));
		}
	}

	if (menu)
		gtk_widget_show_all(GTK_WIDGET(menu));
}

void ui_appindicator_init(struct ui_psensor *ui)
{
	ui_psensor = ui;

	indicator = app_indicator_new
		("psensor",
		 ICON,
		 APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	APP_INDICATOR_GET_CLASS(indicator)->fallback = unity_fallback;
	APP_INDICATOR_GET_CLASS(indicator)->unfallback = unity_unfallback;

	app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_attention_icon(indicator, ATTENTION_ICON);

	ui_appindicator_update_menu(ui);
}

bool is_appindicator_supported(void)
{
	return appindicator_supported;
}

void ui_appindicator_cleanup(void)
{
	free(sensors);
}
