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
#include <string.h>

#include <cfg.h>
#include <ui.h>
#include <ui_color.h>
#include <ui_pref.h>
#include <ui_sensorlist.h>
#include <ui_sensorpref.h>

enum {
	COL_NAME = 0,
	COL_TEMP,
	COL_TEMP_MIN,
	COL_TEMP_MAX,
	COL_COLOR,
	COL_COLOR_STR,
	COL_GRAPH_ENABLED,
	COL_EMPTY,
	COL_SENSOR,
	COL_DISPLAY_ENABLED
};

struct cb_data {
	struct ui_psensor *ui;
	struct psensor *sensor;
};

static int col_index_to_col(int idx)
{
	if (idx == 5)
		return COL_GRAPH_ENABLED;
	else if (idx > 5)
		return -1;

	return idx;
}

static void populate(struct ui_psensor *ui)
{
	GtkTreeIter iter;
	GtkListStore *store;
	GdkRGBA *color;
	char *scolor;
	struct psensor **ordered_sensors, **s_cur, *s;
	unsigned int enabled;

	ordered_sensors = ui_get_sensors_ordered_by_position(ui->sensors);
	store = ui->sensors_store;

	gtk_list_store_clear(store);

	for (s_cur = ordered_sensors; *s_cur; s_cur++) {
		s = *s_cur;

		gtk_list_store_append(store, &iter);

		color = config_get_sensor_color(s->id);

		scolor = gdk_rgba_to_string(color);

		enabled = config_is_sensor_enabled(s->id);
		gtk_list_store_set(store, &iter,
				   COL_NAME, s->name,
				   COL_COLOR_STR, scolor,
				   COL_GRAPH_ENABLED,
				   config_is_sensor_graph_enabled(s->id),
				   COL_SENSOR, s,
				   COL_DISPLAY_ENABLED, enabled,
				   -1);
		free(scolor);
		gdk_rgba_free(color);
	}
	free(ordered_sensors);
}

void ui_sensorlist_update(struct ui_psensor *ui, bool complete)
{
	char *value, *min, *max;
	struct psensor *s;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean valid;
	int use_celsius;
	GtkListStore *store;

	if (complete)
		populate(ui);

	model = gtk_tree_view_get_model(ui->sensors_tree);
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model));

	store = ui->sensors_store;

	if (config_get_temperature_unit() == CELSIUS)
		use_celsius = 1;
	else
		use_celsius = 0;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gtk_tree_model_get(model, &iter, COL_SENSOR, &s, -1);

		value = psensor_value_to_str(s->type,
					     psensor_get_current_value(s),
					     use_celsius);
		min = psensor_value_to_str(s->type,
					   s->sess_lowest,
					   use_celsius);
		max = psensor_value_to_str(s->type,
					   s->sess_highest,
					   use_celsius);

		gtk_list_store_set(store, &iter,
				   COL_TEMP, value,
				   COL_TEMP_MIN, min,
				   COL_TEMP_MAX, max,
				   -1);
		free(value);
		free(min);
		free(max);

		valid = gtk_tree_model_iter_next(model, &iter);
	}
}

/*
 * Returns the sensor corresponding to the x/y position
 * in the table.
 *
 * <null> if none.
 */
static struct psensor *
get_sensor_at_pos(GtkTreeView *view, int x, int y, struct ui_psensor *ui)
{
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct psensor *s;

	gtk_tree_view_get_path_at_pos(view, x, y, &path, NULL, NULL, NULL);
	model = gtk_tree_view_get_model(ui->sensors_tree);

	if (path) {
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, COL_SENSOR, &s, -1);
			return s;
		}
	}
	return NULL;
}

/*
 * Returns the index of the column corresponding
 * to the x position in the table.
 *
 * -1 if none
 */
static int get_col_index_at_pos(GtkTreeView *view, int x)
{
	GList *cols, *node;
	int colx, coli;
	GtkTreeViewColumn *checkcol;

	cols = gtk_tree_view_get_columns(view);
	colx = 0;
	coli = 0;
	for (node = cols; node; node = node->next) {
		checkcol = (GtkTreeViewColumn *)node->data;

		if (x >= colx
		    && x < (colx + gtk_tree_view_column_get_width(checkcol)))
			return coli;

		colx += gtk_tree_view_column_get_width(checkcol);

		coli++;
	}

	return -1;
}

static void preferences_activated_cbk(GtkWidget *menu_item, gpointer data)
{
	struct cb_data *cb_data;

	cb_data = data;
	ui_sensorpref_dialog_run(cb_data->sensor, cb_data->ui);
	free(cb_data);
}

static void hide_activated_cbk(GtkWidget *menu_item, gpointer data)
{
	struct psensor *s, *s2;
	GtkTreeModel *model, *fmodel;
	GtkTreeIter iter;
	struct cb_data *cb_data;
	gboolean valid;

	log_fct_enter();

	cb_data = data;
	s = cb_data->sensor;
	config_set_sensor_enabled(s->id, false);
	config_sync();

	fmodel = gtk_tree_view_get_model(cb_data->ui->sensors_tree);
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(fmodel));
	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gtk_tree_model_get(model, &iter, COL_SENSOR, &s2, -1);

		if (s == s2)
			gtk_list_store_set(cb_data->ui->sensors_store,
					   &iter,
					   COL_DISPLAY_ENABLED,
					   false,
					   -1);
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	free(cb_data);

	log_fct_exit();
}

static GtkWidget *
create_sensor_popup(struct ui_psensor *ui, struct psensor *sensor)
{
	GtkWidget *menu, *item, *separator;
	struct cb_data *data;

	menu = gtk_menu_new();

	item = gtk_menu_item_new_with_label(sensor->name);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

	item = gtk_menu_item_new_with_label(_("Hide"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	data = malloc(sizeof(struct cb_data));
	data->ui = ui;
	data->sensor = sensor;
	g_signal_connect(item,
			 "activate",
			 G_CALLBACK(hide_activated_cbk), data);

	item = gtk_menu_item_new_with_label(_("Preferences"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	data = malloc(sizeof(struct cb_data));
	data->ui = ui;
	data->sensor = sensor;
	g_signal_connect(item,
			 "activate",
			 G_CALLBACK(preferences_activated_cbk), data);

	gtk_widget_show_all(menu);

	return menu;
}

static int clicked_cbk(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkWidget *menu;
	struct ui_psensor *ui;
	GtkTreeView *view;
	struct psensor *s;
	int coli;
	GdkRGBA *color;

	ui = (struct ui_psensor *)data;
	view = ui->sensors_tree;

	s = get_sensor_at_pos(view, event->x, event->y, ui);

	if (s) {
		coli = col_index_to_col(get_col_index_at_pos(view, event->x));

		if (coli == COL_COLOR) {
			color = config_get_sensor_color(s->id);
			if (ui_change_color(_("Select sensor color"),
					    color,
					    GTK_WINDOW(ui->main_window))) {
				config_set_sensor_color(s->id, color);
				ui_sensorlist_update(ui, 1);
				config_sync();
			}
			gdk_rgba_free(color);
			return TRUE;
		} else if (coli >= 0 && coli != COL_GRAPH_ENABLED) {
			menu = create_sensor_popup(ui, s);

			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
			return TRUE;
		}

	}
	return FALSE;
}

void ui_sensorlist_cb_graph_toggled(GtkCellRendererToggle *cell,
				    gchar *path_str,
				    gpointer data)
{
	struct ui_psensor *ui;
	GtkTreeModel *model, *fmodel;
	GtkTreeIter iter;
	GtkTreePath *path;
	struct psensor *s, *s2;
	gboolean valid;
	bool b;

	ui = (struct ui_psensor *)data;
	fmodel = gtk_tree_view_get_model(ui->sensors_tree);

	path = gtk_tree_path_new_from_string(path_str);

	gtk_tree_model_get_iter(fmodel, &iter, path);

	gtk_tree_model_get(fmodel, &iter, COL_SENSOR, &s, -1);

	b = config_is_sensor_graph_enabled(s->id) ^ 1;
	config_set_sensor_graph_enabled(s->id, b);

	config_sync();

	gtk_tree_path_free(path);

	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(fmodel));
	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gtk_tree_model_get(model, &iter, COL_SENSOR, &s2, -1);

		if (s == s2)
			gtk_list_store_set(ui->sensors_store,
					   &iter,
					   COL_GRAPH_ENABLED,
					   b,
					   -1);
		valid = gtk_tree_model_iter_next(model, &iter);
	}
}

void ui_sensorlist_create(struct ui_psensor *ui)
{
	GtkTreeModel *fmodel, *model;

	log_fct_enter();

	model = gtk_tree_view_get_model(ui->sensors_tree);
	fmodel = gtk_tree_model_filter_new(model, NULL);
	gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(fmodel),
						 COL_DISPLAY_ENABLED);

	gtk_tree_view_set_model(ui->sensors_tree, fmodel);

	g_signal_connect(ui->sensors_tree,
			 "button-press-event", (GCallback)clicked_cbk, ui);

	ui_sensorlist_update(ui, 1);

	log_fct_exit();
}
