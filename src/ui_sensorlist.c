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
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_pref.h"
#include "ui_sensorlist.h"
#include "ui_sensorpref.h"
#include "cfg.h"
#include "ui_color.h"
#include "compat.h"

enum {
	COL_NAME = 0,
	COL_TEMP,
	COL_TEMP_MIN,
	COL_TEMP_MAX,
	COL_COLOR,
	COL_COLOR_STR,
	COL_ENABLED,
	COL_EMPTY,
	COLS_COUNT
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

void ui_sensorlist_update(struct ui_psensor *ui)
{
	char *str;
	struct psensor *s;
	GtkTreeIter iter;
	struct ui_sensorlist *ui_sl = ui->ui_sensorlist;
	GtkTreeModel *model
	    = gtk_tree_view_get_model(ui_sl->treeview);
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	struct psensor **sensor = ui->sensors;
	int use_celcius;

	use_celcius = ui->config->temperature_unit == CELCIUS;

	while (valid && *sensor) {
		s = *sensor;

		str = psensor_value_to_str(s->type,
					   s->measures[s->values_max_length -
						       1].value,
					   use_celcius);

		gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_TEMP, str,
				   -1);
		free(str);

		str = psensor_value_to_str(s->type, s->min, use_celcius);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				   COL_TEMP_MIN, str, -1);
		free(str);

		str = psensor_value_to_str(s->type, s->max, use_celcius);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				   COL_TEMP_MAX, str, -1);
		free(str);

		valid = gtk_tree_model_iter_next(model, &iter);
		sensor++;
	}
}

/*
 * Returns the sensor corresponding to the x/y position
 * in the table.
 *
 * <null> if none.
 */
static struct psensor *
get_sensor_at_pos(GtkTreeView *view, int x, int y, struct psensor **sensors)
{
	GtkTreePath *path;

	gtk_tree_view_get_path_at_pos(view, x, y, &path, NULL, NULL, NULL);

	if (path) {
		gint *i = gtk_tree_path_get_indices(path);
		if (i)
			return *(sensors + *i);
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
	GList *columns = gtk_tree_view_get_columns(view);
	GList *node;
	int colx = 0;
	int coli = 0;

	for (node = columns; node; node = node->next) {
		GtkTreeViewColumn *checkcol = (GtkTreeViewColumn *) node->data;

		if (x >= colx &&
		    x < (colx + gtk_tree_view_column_get_width(checkcol)))
			return coli;
		else
			colx += gtk_tree_view_column_get_width(checkcol);

		coli++;
	}

	return -1;
}

void ui_sensorlist_update_sensors_preferences(struct ui_psensor *ui)
{
	GtkTreeIter iter;
	GtkTreeModel *model
	    = gtk_tree_view_get_model(ui->ui_sensorlist->treeview);
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	struct psensor **sensor = ui->ui_sensorlist->sensors;

	while (valid && *sensor) {
		GdkColor color;
		gchar *scolor;

		color.red = (*sensor)->color->red;
		color.green = (*sensor)->color->green;
		color.blue = (*sensor)->color->blue;

		scolor = gdk_color_to_string(&color);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter, COL_NAME, (*sensor)->name, -1);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter, COL_COLOR_STR, scolor, -1);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter, COL_ENABLED, (*sensor)->enabled, -1);

		free(scolor);

		valid = gtk_tree_model_iter_next(model, &iter);
		sensor++;
	}
}

static void on_preferences_activated(GtkWidget *menu_item, gpointer data)
{
	struct cb_data *cb_data = data;

	ui_sensorpref_dialog_run(cb_data->sensor, cb_data->ui);
}

static GtkWidget *create_sensor_popup(struct ui_psensor *ui,
				      struct psensor *sensor)
{
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *separator;
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
			 G_CALLBACK(on_preferences_activated), data);

	gtk_widget_show_all(menu);

	return menu;
}

static int on_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct ui_psensor *ui = (struct ui_psensor *)data;
	GtkTreeView *view = ui->ui_sensorlist->treeview;

	struct psensor *sensor = get_sensor_at_pos(view,
						   event->x,
						   event->y,
						   ui->sensors);

	if (sensor) {
		int coli = col_index_to_col(get_col_index_at_pos(view,
								 event->x));

		if (coli == COL_COLOR) {
			if (ui_change_color(_("Select foreground color"),
					    sensor->color)) {
				ui_sensorlist_update_sensors_preferences(ui);
				config_set_sensor_color(sensor->id,
							sensor->color);
			}
		} else if (coli >= 0 && coli != COL_ENABLED) {
			GtkWidget *menu = create_sensor_popup(ui,
							      sensor);

			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);

		}

	}
	return FALSE;
}

static void
on_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
	struct ui_sensorlist *list = (struct ui_sensorlist *)data;
	GtkTreeModel *model
	    = gtk_tree_view_get_model(list->treeview);
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
	gboolean fixed;
	gint *i;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, COL_ENABLED, &fixed, -1);

	fixed ^= 1;

	i = gtk_tree_path_get_indices(path);
	if (i) {
		int n = *i;
		struct psensor **sensor = list->sensors;
		while (n--)
			sensor++;
		(*sensor)->enabled = fixed;
		config_set_sensor_enabled((*sensor)->id, (*sensor)->enabled);
	}

	gtk_list_store_set(GTK_LIST_STORE(model),
			   &iter, COL_ENABLED, fixed, -1);

	gtk_tree_path_free(path);
}

static void create_widget(struct ui_psensor *ui)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	struct psensor **s_cur = ui->sensors;
	struct ui_sensorlist *ui_sl = ui->ui_sensorlist;

	store = gtk_list_store_new(COLS_COUNT,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_BOOLEAN, G_TYPE_STRING);

	ui_sl->treeview = GTK_TREE_VIEW
		(gtk_tree_view_new_with_model(GTK_TREE_MODEL(store)));

	gtk_tree_selection_set_mode
		(gtk_tree_view_get_selection(ui_sl->treeview),
		 GTK_SELECTION_NONE);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    _("Sensor"),
						    renderer,
						    "text", COL_NAME, NULL);

	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    _("Value"),
						    renderer,
						    "text", COL_TEMP, NULL);

	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    _("Min"),
						    renderer,
						    "text", COL_TEMP_MIN, NULL);

	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    _("Max"),
						    renderer,
						    "text", COL_TEMP_MAX, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    _("Color"),
						    renderer,
						    "text", COL_COLOR,
						    "background", COL_COLOR_STR,
						    NULL);

	g_signal_connect(ui_sl->treeview,
			 "button-press-event", (GCallback) on_clicked, ui);

	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    _("Graph"),
						    renderer,
						    "active", COL_ENABLED,
						    NULL);
	g_signal_connect(G_OBJECT(renderer),
			 "toggled", (GCallback) on_toggled, ui_sl);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(ui_sl->treeview,
						    -1,
						    "",
						    renderer,
						    "text", COL_EMPTY, NULL);

	while (*s_cur) {
		GtkTreeIter iter;
		GdkColor color;
		gchar *scolor;
		struct psensor *s = *s_cur;

		color.red = s->color->red;
		color.green = s->color->green;
		color.blue = s->color->blue;

		scolor = gdk_color_to_string(&color);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   COL_NAME, s->name,
				   COL_TEMP, _("N/A"),
				   COL_TEMP_MIN, _("N/A"),
				   COL_TEMP_MAX, _("N/A"),
				   COL_COLOR_STR, scolor,
				   COL_ENABLED, s->enabled, -1);

		free(scolor);

		s_cur++;
	}

	ui_sl->widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ui_sl->widget),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(ui_sl->widget),
			  GTK_WIDGET(ui_sl->treeview));
}

void ui_sensorlist_create(struct ui_psensor *ui)
{
	ui->ui_sensorlist = malloc(sizeof(struct ui_sensorlist));
	ui->ui_sensorlist->sensors = ui->sensors;

	create_widget(ui);
}
