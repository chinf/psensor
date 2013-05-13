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
#include <string.h>

#include "ui.h"
#include "ui_pref.h"
#include "ui_sensorlist.h"
#include "ui_sensorpref.h"
#include "cfg.h"
#include "ui_color.h"

enum {
	COL_NAME = 0,
	COL_TEMP,
	COL_TEMP_MIN,
	COL_TEMP_MAX,
	COL_COLOR,
	COL_COLOR_STR,
	COL_ENABLED,
	COL_EMPTY,
	COL_SENSOR,
};

struct cb_data {
	struct ui_psensor *ui;
	struct psensor *sensor;
};

static int col_index_to_col(int idx)
{
	if (idx == 5)
		return COL_ENABLED;
	else if (idx > 5)
		return -1;

	return idx;
}

static void populate(struct ui_psensor *ui)
{
	GtkTreeIter iter;
	GtkListStore *store;
	GdkColor color;
	char *scolor;
	struct psensor **ordered_sensors, **s_cur, *s;

	ordered_sensors = ui_get_sensors_ordered_by_position(ui);
	store = ui->sensors_store;

	gtk_list_store_clear(store);

	for (s_cur = ordered_sensors; *s_cur; s_cur++) {
		s = *s_cur;

		gtk_list_store_append(store, &iter);

		color.red = s->color->red;
		color.green = s->color->green;
		color.blue = s->color->blue;

		scolor = gdk_color_to_string(&color);

		gtk_list_store_set(store, &iter,
				   COL_NAME, s->name,
				   COL_COLOR_STR, scolor,
				   COL_ENABLED, s->graph_enabled,
				   COL_SENSOR, s,
				   -1);
		free(scolor);
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
	int use_celcius;
	GtkListStore *store;

	if (complete)
		populate(ui);

	model = gtk_tree_view_get_model(ui->sensors_tree);
	store = ui->sensors_store;

	use_celcius = ui->config->temperature_unit == CELCIUS;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gtk_tree_model_get(model, &iter, COL_SENSOR, &s, -1);

		value = psensor_value_to_str(s->type,
					     psensor_get_current_value(s),
					     use_celcius);
		min = psensor_value_to_str(s->type, s->min, use_celcius);
		max = psensor_value_to_str(s->type, s->max, use_celcius);

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
		else
			colx += gtk_tree_view_column_get_width(checkcol);

		coli++;
	}

	return -1;
}

static void preferences_activated_cbk(GtkWidget *menu_item, gpointer data)
{
	struct cb_data *cb_data = data;

	ui_sensorpref_dialog_run(cb_data->sensor, cb_data->ui);
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

	if (event->button != 3)
		return FALSE;

	ui = (struct ui_psensor *)data;
	view = ui->sensors_tree;

	s = get_sensor_at_pos(view, event->x, event->y, ui);

	if (s) {
		coli = col_index_to_col(get_col_index_at_pos(view, event->x));

		if (coli == COL_COLOR) {
			if (ui_change_color(_("Select foreground color"),
					    s->color)) {
				ui_sensorlist_update(ui, 1);
				config_set_sensor_color(s->id, s->color);
			}
		} else if (coli >= 0 && coli != COL_ENABLED) {
			menu = create_sensor_popup(ui, s);

			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
		}

	}
	return TRUE;
}

static void
toggled_cbk(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
	struct ui_psensor *ui;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	struct psensor *s;

	ui = (struct ui_psensor *)data;
	model = gtk_tree_view_get_model(ui->sensors_tree);
	path = gtk_tree_path_new_from_string(path_str);

	gtk_tree_model_get_iter(model, &iter, path);

	gtk_tree_model_get(model, &iter, COL_SENSOR, &s, -1);

	s->graph_enabled ^= 1;
	config_set_sensor_enabled(s->id, s->graph_enabled);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   COL_ENABLED, s->graph_enabled, -1);

	gtk_tree_path_free(path);
}

void ui_sensorlist_create(struct ui_psensor *ui)
{
	GtkCellRenderer *renderer;

	log_debug("ui_sensorlist_create()");

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    _("Sensor"),
						    renderer,
						    "text", COL_NAME, NULL);

	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    _("Value"),
						    renderer,
						    "text", COL_TEMP, NULL);

	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    _("Min"),
						    renderer,
						    "text", COL_TEMP_MIN, NULL);

	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    _("Max"),
						    renderer,
						    "text", COL_TEMP_MAX, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    _("Color"),
						    renderer,
						    "text", COL_COLOR,
						    "background", COL_COLOR_STR,
						    NULL);

	g_signal_connect(ui->sensors_tree,
			 "button-press-event", (GCallback)clicked_cbk, ui);

	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    _("Graph"),
						    renderer,
						    "active", COL_ENABLED,
						    NULL);
	g_signal_connect(G_OBJECT(renderer),
			 "toggled", (GCallback) toggled_cbk, ui);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(ui->sensors_tree,
						    -1,
						    "",
						    renderer,
						    "text", COL_EMPTY, NULL);

	ui_sensorlist_update(ui, 1);
}
