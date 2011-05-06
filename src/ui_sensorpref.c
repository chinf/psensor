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

#include <stdlib.h>

#include <gtk/gtk.h>

#include "cfg.h"
#include "ui_pref.h"
#include "ui_sensorlist.h"
#include "ui_sensorpref.h"
#include "ui_color.h"

struct sensor_pref {
	struct psensor *sensor;
	char *name;
	int enabled;
	struct color *color;
	int alarm_enabled;
	double alarm_limit;
};

struct cb_data {
	struct ui_psensor *ui;
	GtkBuilder *builder;
	struct sensor_pref **prefs;
};

static struct sensor_pref *sensor_pref_new(struct psensor *s)
{
	struct sensor_pref *p = malloc(sizeof(struct sensor_pref));

	p->sensor = s;

	p->name = strdup(s->name);
	p->enabled = s->enabled;
	p->alarm_enabled = s->alarm_enabled;
	p->alarm_limit = s->alarm_limit;
	p->color = color_dup(s->color);

	return p;
}

static void sensor_pref_free(struct sensor_pref *p)
{
	if (!p)
		return ;

	free(p->name);
	free(p->color);

	free(p);
}

static struct sensor_pref **sensor_pref_list_new(struct psensor **sensors)
{
	int n, i;
	struct sensor_pref **pref_list;

	n = psensor_list_size(sensors);
	pref_list = malloc(sizeof(struct sensor_pref *) * (n+1));

	for (i = 0; i < n; i++)
		pref_list[i] = sensor_pref_new(sensors[i]);

	pref_list[n] = NULL;

	return pref_list;
}

static void sensor_pref_list_free(struct sensor_pref **list)
{
	struct sensor_pref **cur = list;

	while (*cur) {
		sensor_pref_free(*cur);

		cur++;
	}

	free(list);
}

static struct sensor_pref *
sensor_pref_get(struct sensor_pref **ps, struct psensor *s)
{
	struct sensor_pref **p_cur = ps;

	while (*p_cur) {
		struct sensor_pref *p = *p_cur;

		if (p->sensor == s)
			return p;

		p_cur++;
	}

	return NULL;
}

static struct sensor_pref *
get_seleted_sensor_pref(GtkBuilder *builder, struct sensor_pref **ps)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct sensor_pref *pref = NULL;
	GtkTreeSelection *selection;
	GtkTreeView *tree;

	tree = GTK_TREE_VIEW(gtk_builder_get_object(builder,
				     "sensors_list"));

	selection = gtk_tree_view_get_selection(tree);

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		GtkTreePath *p = gtk_tree_model_get_path(model, &iter);
		gint *indices = gtk_tree_path_get_indices(p);

		pref = ps[*indices];

		gtk_tree_path_free(p);
	}

	return pref;
}

static void on_name_changed(GtkEntry *entry, gpointer data)
{
	struct cb_data *cbdata = data;
	struct sensor_pref *p;
	const char *str;

	str = gtk_entry_get_text(entry);

	p = get_seleted_sensor_pref(cbdata->builder, cbdata->prefs);

	if (p && strcmp(p->name, str)) {
		free(p->name);
		p->name = strdup(str);
	}
}

static void
on_drawed_toggled(GtkToggleButton *btn, gpointer data)
{
	struct cb_data *cbdata = data;
	struct sensor_pref *p;

	p = get_seleted_sensor_pref(cbdata->builder, cbdata->prefs);

	if (p)
		p->enabled = gtk_toggle_button_get_active(btn);
}

static void
on_alarm_toggled(GtkToggleButton *btn, gpointer data)
{
	struct cb_data *cbdata = data;
	struct sensor_pref *p;

	p = get_seleted_sensor_pref(cbdata->builder, cbdata->prefs);

	if (p)
		p->alarm_enabled = gtk_toggle_button_get_active(btn);
}

static void on_color_set(GtkColorButton *widget, gpointer data)
{
	struct cb_data *cbdata = data;
	struct sensor_pref *p;
	GdkColor color;

	p = get_seleted_sensor_pref(cbdata->builder, cbdata->prefs);

	if (p) {
		gtk_color_button_get_color(widget, &color);
		color_set(p->color, color.red, color.green, color.blue);
	}
}

static void on_temp_limit_changed(GtkSpinButton *btn, gpointer data)
{
	struct cb_data *cbdata = data;
	struct sensor_pref *p;

	p = get_seleted_sensor_pref(cbdata->builder, cbdata->prefs);

	if (p)
		p->alarm_limit = gtk_spin_button_get_value(btn);
}

static void connect_signals(GtkBuilder *builder, struct cb_data *cbdata)
{
	g_signal_connect(gtk_builder_get_object(builder, "sensor_name"),
			 "changed", G_CALLBACK(on_name_changed), cbdata);

	g_signal_connect(gtk_builder_get_object(builder, "sensor_draw"),
			 "toggled", G_CALLBACK(on_drawed_toggled), cbdata);

	g_signal_connect(gtk_builder_get_object(builder, "sensor_color"),
			 "color-set", G_CALLBACK(on_color_set), cbdata);

	g_signal_connect(gtk_builder_get_object(builder, "sensor_alarm"),
			 "toggled", G_CALLBACK(on_alarm_toggled), cbdata);

	g_signal_connect(gtk_builder_get_object(builder, "sensor_temp_limit"),
			 "value-changed", G_CALLBACK(on_temp_limit_changed),
			 cbdata);
}

static void
update_pref(struct psensor *s, struct sensor_pref **prefs, GtkBuilder *builder)
{
	GtkLabel *w_id, *w_type;
	GtkEntry *w_name;
	GtkToggleButton *w_draw, *w_alarm;
	GtkColorButton *w_color;
	GtkSpinButton *w_temp_limit;
	GdkColor *color;
	struct sensor_pref *p = sensor_pref_get(prefs, s);

	w_id = GTK_LABEL(gtk_builder_get_object(builder, "sensor_id"));
	gtk_label_set_text(w_id, s->id);

	w_type = GTK_LABEL(gtk_builder_get_object(builder, "sensor_type"));
	gtk_label_set_text(w_type, psensor_type_to_str(s->type));

	w_name = GTK_ENTRY(gtk_builder_get_object(builder, "sensor_name"));
	gtk_entry_set_text(w_name, p->name);

	w_draw = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							  "sensor_draw"));
	gtk_toggle_button_set_active(w_draw, p->enabled);

	color = color_to_gdkcolor(p->color);
	w_color = GTK_COLOR_BUTTON(gtk_builder_get_object(builder,
							  "sensor_color"));
	gtk_color_button_set_color(w_color, color);

	w_alarm = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "sensor_alarm"));
	w_temp_limit
		= GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
							 "sensor_temp_limit"));

	if (is_temp_type(s->type)) {
		gtk_toggle_button_set_active(w_alarm, p->alarm_enabled);
		gtk_spin_button_set_value(w_temp_limit, p->alarm_limit);
		gtk_widget_set_sensitive(GTK_WIDGET(w_alarm), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(w_temp_limit), TRUE);
	} else {
		gtk_toggle_button_set_active(w_alarm, 0);
		gtk_spin_button_set_value(w_temp_limit, 0);
		gtk_widget_set_sensitive(GTK_WIDGET(w_alarm), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(w_temp_limit), FALSE);
	}
}

static void on_changed(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct cb_data *cbdata = data;
	struct ui_psensor *ui = cbdata->ui;

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		GtkTreePath *p = gtk_tree_model_get_path(model, &iter);
		gint *indices = gtk_tree_path_get_indices(p);
		struct psensor *s = *(ui->sensors + *indices);

		update_pref(s, cbdata->prefs, cbdata->builder);

		gtk_tree_path_free(p);
	}
}

static void
select_sensor(struct psensor *s, struct psensor **sensors, GtkTreeView *tree)
{
	struct psensor **s_cur = sensors;
	int i = 0;
	GtkTreePath *p = NULL;

	while (*s_cur) {
		if (s == *s_cur) {
			p = gtk_tree_path_new_from_indices(i, -1);
			break;
		}

		i++;
		s_cur++;
	}

	if (p) {
		GtkTreeSelection *s = gtk_tree_view_get_selection(tree);

		gtk_tree_selection_select_path(s, p);
		gtk_tree_path_free(p);
	}
}

static void
apply_prefs(struct sensor_pref **prefs, struct psensor **sensors)
{
	int n = psensor_list_size(sensors);
	int i;

	for (i = 0; i < n; i++) {
		struct psensor *s = sensors[i];
		struct sensor_pref *p = prefs[i];

		if (strcmp(p->name, s->name)) {
			free(s->name);
			s->name = strdup(p->name);
			config_set_sensor_name(s->id, s->name);
		}

		if (s->enabled != p->enabled) {
			s->enabled = p->enabled;
			config_set_sensor_enabled(s->id, s->enabled);
		}

		if (s->alarm_limit != p->alarm_limit) {
			s->alarm_limit = p->alarm_limit;
			config_set_sensor_alarm_limit(s->id,
						      s->alarm_limit);
		}

		if (s->alarm_enabled != p->alarm_enabled) {
			s->alarm_enabled = p->alarm_enabled;
			config_set_sensor_alarm_enabled(s->id,
							s->alarm_enabled);
		}

		color_set(s->color,
			  p->color->red, p->color->green, p->color->blue);
		config_set_sensor_color(s->id, s->color);
	}
}

void ui_sensorpref_dialog_run(struct psensor *sensor, struct ui_psensor *ui)
{
	GtkDialog *diag;
	gint result;
	GtkBuilder *builder;
	GError *error = NULL;
	GtkTreeView *w_sensors_list;
	guint ok;
	GtkCellRenderer *renderer;
	GtkListStore *store;
	struct psensor **s_cur;
	GtkTreeSelection *selection;
	struct cb_data cbdata;

	cbdata.ui = ui;
	cbdata.prefs = sensor_pref_list_new(ui->sensors);

	builder = gtk_builder_new();
	cbdata.builder = builder;

	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "sensor-edit.glade",
		 &error);

	if (!ok) {
		g_warning("%s", error->message);
		g_free(error);
		return ;
	}

	update_pref(sensor, cbdata.prefs, builder);
	connect_signals(builder, &cbdata);

	w_sensors_list
		= GTK_TREE_VIEW(gtk_builder_get_object(builder,
						       "sensors_list"));

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(w_sensors_list,
						    -1,
						    _("Sensor Name"),
						    renderer,
						    "text", 0, NULL);

	store = GTK_LIST_STORE(gtk_tree_view_get_model(w_sensors_list));

	s_cur = ui->sensors;
	while (*s_cur) {
		GtkTreeIter iter;
		struct psensor *s = *s_cur;

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, s->name, -1);

		s_cur++;
	}

	selection = gtk_tree_view_get_selection(w_sensors_list);
	g_signal_connect(selection, "changed", G_CALLBACK(on_changed), &cbdata);
	select_sensor(sensor, ui->sensors, w_sensors_list);

	diag = GTK_DIALOG(gtk_builder_get_object(builder, "dialog1"));
	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {
		apply_prefs(cbdata.prefs, ui->sensors);
		ui_sensorlist_update_sensors_preferences(ui);
	}

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));

	sensor_pref_list_free(cbdata.prefs);
}
