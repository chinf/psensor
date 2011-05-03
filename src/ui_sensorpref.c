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

void ui_sensorpref_dialog_run(struct psensor *sensor, struct ui_psensor *ui)
{
	GtkDialog *diag;
	gint result;
	GtkBuilder *builder;
	GError *error = NULL;
	GtkLabel *w_id, *w_type;
	GtkEntry *w_name;
	GtkToggleButton *w_draw, *w_alarm;
	GtkColorButton *w_color;
	GtkSpinButton *w_temp_limit;
	GdkColor *color;
	guint ok;

	builder = gtk_builder_new();

	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "sensor-edit.glade",
		 &error);

	if (!ok) {
		g_warning("%s", error->message);
		g_free(error);
		return ;
	}

	w_id = GTK_LABEL(gtk_builder_get_object(builder, "sensor_id"));
	gtk_label_set_text(w_id, sensor->id);

	w_type = GTK_LABEL(gtk_builder_get_object(builder, "sensor_type"));
	gtk_label_set_text(w_type, psensor_type_to_str(sensor->type));

	w_name = GTK_ENTRY(gtk_builder_get_object(builder, "sensor_name"));
	gtk_entry_set_text(w_name, sensor->name);

	w_draw = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							  "sensor_draw"));
	gtk_toggle_button_set_active(w_draw, sensor->enabled);

	color = color_to_gdkcolor(sensor->color);
	w_color = GTK_COLOR_BUTTON(gtk_builder_get_object(builder,
							  "sensor_color"));
	gtk_color_button_set_color(w_color, color);

	w_alarm = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "sensor_alarm"));
	w_temp_limit
		= GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
							 "sensor_temp_limit"));

	if (is_temp_type(sensor->type)) {
		gtk_toggle_button_set_active(w_alarm, sensor->alarm_enabled);
		gtk_spin_button_set_value(w_temp_limit, sensor->alarm_limit);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(w_alarm), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(w_temp_limit), FALSE);
	}

	diag = GTK_DIALOG(gtk_builder_get_object(builder, "dialog1"));
	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {

		free(sensor->name);
		sensor->name = strdup(gtk_entry_get_text(w_name));
		config_set_sensor_name(sensor->id, sensor->name);

		sensor->enabled = gtk_toggle_button_get_active(w_draw);
		config_set_sensor_enabled(sensor->id, sensor->enabled);

		sensor->alarm_limit = gtk_spin_button_get_value(w_temp_limit);
		config_set_sensor_alarm_limit(sensor->id, sensor->alarm_limit);

		sensor->alarm_enabled = gtk_toggle_button_get_active(w_alarm);
		config_set_sensor_alarm_enabled(sensor->id,
						sensor->alarm_enabled);

		gtk_color_button_get_color(w_color, color);
		color_set(sensor->color, color->red, color->green, color->blue);
		config_set_sensor_color(sensor->id, sensor->color);

		ui_sensorlist_update_sensors_preferences(ui->ui_sensorlist);
	}

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));
}
