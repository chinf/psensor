/*
 * Copyright (C) 2010-2014 jeanfi@gmail.com
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

#include <gtk/gtk.h>

#include <cfg.h>
#include <temperature.h>
#include <ui_appindicator.h>
#include <ui_pref.h>
#include <ui_sensorlist.h>
#include <ui_sensorpref.h>
#include <ui_color.h>

enum {
	COL_NAME = 0,
	COL_SENSOR_PREF
};

struct cb_data {
	struct ui_psensor *ui;
	GtkBuilder *builder;
};

static struct psensor *get_selected_sensor(GtkBuilder *builder)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct psensor *s;
	GtkTreeSelection *selection;
	GtkTreeView *tree;

	tree = GTK_TREE_VIEW(gtk_builder_get_object(builder, "sensors_list"));

	selection = gtk_tree_view_get_selection(tree);

	s = NULL;
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_tree_model_get(model, &iter, COL_SENSOR_PREF, &s, -1);

	return s;
}

static void apply_config(struct ui_psensor *ui)
{
	config_sync();

	ui_sensorlist_update(ui, 1);
	ui_appindicator_update_menu(ui);
}

void ui_sensorpref_name_changed_cb(GtkEntry *entry, gpointer data)
{
	struct psensor *s;
	struct cb_data *cbdata;

	const gchar *str;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	str = gtk_entry_get_text(entry);

	if (s) {
		if (strcmp(str, s->name)) {
			free(s->name);
			s->name = strdup(str);
			config_set_sensor_name(s->id, str);

			apply_config(cbdata->ui);
		}
	}
}

void ui_sensorpref_draw_toggled_cb(GtkToggleButton *btn, gpointer data)
{
	gboolean active;
	struct cb_data *cbdata;
	struct psensor *s;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);
	if (s) {
		active = gtk_toggle_button_get_active(btn);
		config_set_sensor_graph_enabled(s->id, active);

		apply_config(cbdata->ui);
	}
}

void ui_sensorpref_display_toggled_cb(GtkToggleButton *btn, gpointer data)
{
	gboolean active;
	struct cb_data *cbdata;
	struct psensor *s;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		active = gtk_toggle_button_get_active(btn);
		config_set_sensor_enabled(s->id, active);

		apply_config(cbdata->ui);
	}

}

void ui_sensorpref_alarm_toggled_cb(GtkToggleButton *btn, gpointer data)
{
	gboolean active;
	struct cb_data *cbdata;
	struct psensor *s;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		active = gtk_toggle_button_get_active(btn);
		config_set_sensor_alarm_enabled(s->id, active);

		apply_config(cbdata->ui);
	}
}

void
ui_sensorpref_appindicator_menu_toggled_cb(GtkToggleButton *btn, gpointer data)
{
	gboolean active;
	struct cb_data *cbdata;
	struct psensor *s;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		active = gtk_toggle_button_get_active(btn);
		config_set_appindicator_enabled(s->id, active);

		apply_config(cbdata->ui);
	}
}

void
ui_sensorpref_appindicator_label_toggled_cb(GtkToggleButton *btn, gpointer data)
{
	gboolean active;
	struct cb_data *cbdata;
	struct psensor *s;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		active = gtk_toggle_button_get_active(btn);
		config_set_appindicator_label_enabled(s->id, active);

		apply_config(cbdata->ui);
	}
}

void ui_sensorpref_color_set_cb(GtkColorButton *widget, gpointer data)
{
	struct cb_data *cbdata;
	struct psensor *s;
	GdkRGBA color;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &color);
		config_set_sensor_color(s->id, &color);

		apply_config(cbdata->ui);
	}
}

void
ui_sensorpref_alarm_high_threshold_changed_cb(GtkSpinButton *btn, gpointer data)
{
	struct cb_data *cbdata;
	struct psensor *s;
	gdouble v;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		v = gtk_spin_button_get_value(btn);
		config_set_sensor_alarm_high_threshold(s->id, v);

		apply_config(cbdata->ui);

		s->alarm_high_threshold = v;
	}
}

void
ui_sensorpref_alarm_low_threshold_changed_cb(GtkSpinButton *btn, gpointer data)
{
	struct cb_data *cbdata;
	struct psensor *s;
	gdouble v;

	cbdata = (struct cb_data *)data;

	s = get_selected_sensor(cbdata->builder);

	if (s) {
		v = gtk_spin_button_get_value(btn);
		config_set_sensor_alarm_low_threshold(s->id, v);

		apply_config(cbdata->ui);

		s->alarm_low_threshold = v;
	}
}

static void
update_pref(struct psensor *s, struct config *cfg, GtkBuilder *builder)
{
	GtkLabel *w_id, *w_type, *w_high_threshold_unit, *w_low_threshold_unit,
		*w_chipname;
	GtkEntry *w_name;
	GtkToggleButton *w_draw, *w_alarm, *w_appindicator_enabled,
		*w_appindicator_label_enabled, *w_display;
	GtkColorButton *w_color;
	GtkSpinButton *w_high_threshold, *w_low_threshold;
	int use_celsius, threshold;
	GdkRGBA *color;

	w_id = GTK_LABEL(gtk_builder_get_object(builder, "sensor_id"));
	gtk_label_set_text(w_id, s->id);

	w_type = GTK_LABEL(gtk_builder_get_object(builder, "sensor_type"));
	gtk_label_set_text(w_type, psensor_type_to_str(s->type));

	w_name = GTK_ENTRY(gtk_builder_get_object(builder, "sensor_name"));
	gtk_entry_set_text(w_name, s->name);

	w_chipname = GTK_LABEL(gtk_builder_get_object(builder, "chip_name"));
	if (s->chip)
		gtk_label_set_text(w_chipname, s->chip);
	else
		gtk_label_set_text(w_chipname, _("Unknown"));

	w_draw = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							  "sensor_draw"));
	gtk_toggle_button_set_active(w_draw,
				     config_is_sensor_graph_enabled(s->id));

	w_display = GTK_TOGGLE_BUTTON(gtk_builder_get_object
				      (builder,
				       "sensor_enable_checkbox"));
	gtk_toggle_button_set_active(w_display,
				     config_is_sensor_enabled(s->id));

	w_color = GTK_COLOR_BUTTON(gtk_builder_get_object(builder,
							  "sensor_color"));
	color = config_get_sensor_color(s->id);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w_color), color);
	gdk_rgba_free(color);

	w_alarm = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "sensor_alarm"));
	w_high_threshold = GTK_SPIN_BUTTON(gtk_builder_get_object
					  (builder,
					   "sensor_alarm_high_threshold"));
	w_low_threshold = GTK_SPIN_BUTTON(gtk_builder_get_object
					 (builder,
					  "sensor_alarm_low_threshold"));

	w_high_threshold_unit = GTK_LABEL(gtk_builder_get_object
					 (builder,
					  "sensor_alarm_high_threshold_unit"));
	w_low_threshold_unit = GTK_LABEL(gtk_builder_get_object
					(builder,
					 "sensor_alarm_low_threshold_unit"));

	use_celsius = cfg->temperature_unit == CELSIUS ? 1 : 0;
	gtk_label_set_text(w_high_threshold_unit,
			   psensor_type_to_unit_str(s->type, use_celsius));
	gtk_label_set_text(w_low_threshold_unit,
			   psensor_type_to_unit_str(s->type, use_celsius));

	w_appindicator_enabled = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "indicator_checkbox"));
	w_appindicator_label_enabled = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "indicator_label_checkbox"));

	if (is_appindicator_supported()) {
		gtk_widget_set_has_tooltip
			(GTK_WIDGET(w_appindicator_label_enabled), FALSE);
		gtk_widget_set_has_tooltip
			(GTK_WIDGET(w_appindicator_enabled), FALSE);
	} else {
		gtk_widget_set_sensitive
			(GTK_WIDGET(w_appindicator_label_enabled), FALSE);
		gtk_widget_set_has_tooltip
			(GTK_WIDGET(w_appindicator_label_enabled), TRUE);
		gtk_widget_set_sensitive
			(GTK_WIDGET(w_appindicator_enabled), FALSE);
		gtk_widget_set_has_tooltip
			(GTK_WIDGET(w_appindicator_enabled), TRUE);
	}

	gtk_toggle_button_set_active(w_alarm,
				     config_get_sensor_alarm_enabled(s->id));

	threshold = config_get_sensor_alarm_high_threshold(s->id);
	if (!use_celsius)
		threshold = celsius_to_fahrenheit(threshold);
	gtk_spin_button_set_value(w_high_threshold, threshold);

	threshold = config_get_sensor_alarm_low_threshold(s->id);
	if (!use_celsius)
		threshold = celsius_to_fahrenheit(threshold);
	gtk_spin_button_set_value(w_low_threshold, threshold);

	gtk_widget_set_sensitive(GTK_WIDGET(w_alarm), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(w_high_threshold), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(w_low_threshold), TRUE);

	gtk_toggle_button_set_active(w_appindicator_enabled,
				     config_is_appindicator_enabled(s->id));

	gtk_toggle_button_set_active
		(w_appindicator_label_enabled,
		 config_is_appindicator_label_enabled(s->id));
}

static void on_changed(GtkTreeSelection *selection, gpointer data)
{
	struct cb_data *cbdata = data;
	struct ui_psensor *ui = cbdata->ui;
	struct psensor *s;

	s = get_selected_sensor(cbdata->builder);

	update_pref(s, ui->config, cbdata->builder);
}

static void
select_sensor(struct psensor *s, struct psensor **sensors, GtkTreeView *tree)
{
	struct psensor **s_cur;
	int i;
	GtkTreePath *p;
	GtkTreeSelection *sel;

	p = NULL;
	for (s_cur = sensors, i = 0; *s_cur; s_cur++, i++)
		if (s == *s_cur) {
			p = gtk_tree_path_new_from_indices(i, -1);
			break;
		}

	if (p) {
		sel = gtk_tree_view_get_selection(tree);

		gtk_tree_selection_select_path(sel, p);
		gtk_tree_path_free(p);
	}
}

void ui_sensorpref_dialog_run(struct psensor *sensor, struct ui_psensor *ui)
{
	GtkDialog *diag;
	guint ok;
	GtkBuilder *builder;
	GError *error;
	GtkTreeView *w_sensors_list;
	GtkListStore *store;
	struct psensor **s_cur, *s, **ordered_sensors;
	GtkTreeSelection *selection;
	struct cb_data cbdata;
	GtkTreeIter iter;

	cbdata.ui = ui;

	builder = gtk_builder_new();
	cbdata.builder = builder;

	error = NULL;
	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "sensor-edit.glade",
		 &error);

	if (!ok) {
		log_printf(LOG_ERR, error->message);
		g_error_free(error);
		return;
	}

	w_sensors_list
		= GTK_TREE_VIEW(gtk_builder_get_object(builder,
						       "sensors_list"));

	store = GTK_LIST_STORE(gtk_builder_get_object(builder,
						      "sensors_liststore"));

	ordered_sensors = ui_get_sensors_ordered_by_position(ui);
	for (s_cur = ordered_sensors; *s_cur; s_cur++) {
		s = *s_cur;
		gtk_list_store_append(store, &iter);

		gtk_list_store_set(store, &iter,
				   COL_NAME, s->name,
				   COL_SENSOR_PREF, s,
				   -1);

		if (s == sensor)
			update_pref(s, ui->config, builder);
	}

	selection = gtk_tree_view_get_selection(w_sensors_list);
	g_signal_connect(selection, "changed", G_CALLBACK(on_changed), &cbdata);
	select_sensor(sensor, ordered_sensors, w_sensors_list);

	free(ordered_sensors);

	diag = GTK_DIALOG(gtk_builder_get_object(builder, "dialog1"));

	gtk_builder_connect_signals(builder, &cbdata);

	gtk_dialog_run(diag);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));
}
