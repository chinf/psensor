/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
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
#include "cfg.h"
#include "ui_pref.h"
#include "ui_color.h"
#include "compat.h"

GdkColor *color_to_gdkcolor(struct color *color)
{
	GdkColor *c = malloc(sizeof(GdkColor));

	c->red = color->red;
	c->green = color->green;
	c->blue = color->blue;

	return c;
}

void ui_pref_dialog_run(struct ui_psensor *ui)
{
	GtkDialog *diag;
	gint result;
	struct config *cfg;
	GtkBuilder *builder;
	guint ok;
	GError *error = NULL;
	GdkColor *color_fg, *color_bg;
	GtkColorButton *w_color_fg, *w_color_bg;
	GtkHScale *w_bg_opacity;
	GtkSpinButton *w_update_interval, *w_monitoring_duration,
		*w_s_update_interval;
	GtkComboBox *w_sensorlist_pos;
	GtkToggleButton *w_hide_window_decoration, *w_keep_window_below,
		*w_enable_menu, *w_enable_launcher_counter, *w_hide_on_startup,
		*w_win_restore;

	cfg = ui->config;

	builder = gtk_builder_new();

	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "psensor-pref.glade",
		 &error);

	if (!ok) {
		log_printf(LOG_ERR, error->message);
		g_error_free(error);
		return ;
	}

	diag = GTK_DIALOG(gtk_builder_get_object(builder, "dialog1"));

	color_fg = color_to_gdkcolor(cfg->graph_fgcolor);
	w_color_fg = GTK_COLOR_BUTTON(gtk_builder_get_object(builder,
							     "color_fg"));
	gtk_color_button_set_color(w_color_fg, color_fg);

	color_bg = color_to_gdkcolor(cfg->graph_bgcolor);
	w_color_bg = GTK_COLOR_BUTTON(gtk_builder_get_object(builder,
							     "color_bg"));
	gtk_color_button_set_color(w_color_bg, color_bg);

	w_bg_opacity = GTK_HSCALE(gtk_builder_get_object(builder,
							 "bg_opacity"));
	gtk_range_set_value(GTK_RANGE(w_bg_opacity), cfg->graph_bg_alpha);

	w_update_interval = GTK_SPIN_BUTTON(gtk_builder_get_object
					    (builder, "update_interval"));
	gtk_spin_button_set_value(w_update_interval,
				  cfg->graph_update_interval);

	w_s_update_interval
		= GTK_SPIN_BUTTON(gtk_builder_get_object
				  (builder, "sensor_update_interval"));
	gtk_spin_button_set_value(w_s_update_interval,
				  cfg->sensor_update_interval);

	w_monitoring_duration
		= GTK_SPIN_BUTTON(gtk_builder_get_object
				  (builder, "monitoring_duration"));
	gtk_spin_button_set_value(w_monitoring_duration,
				  cfg->graph_monitoring_duration);

	w_sensorlist_pos = GTK_COMBO_BOX
		(gtk_builder_get_object(builder, "sensors_list_position"));
	gtk_combo_box_set_active(w_sensorlist_pos, cfg->sensorlist_position);

	w_hide_window_decoration = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "hide_window_decoration"));
	gtk_toggle_button_set_active(w_hide_window_decoration,
				     !cfg->window_decoration_enabled);

	w_keep_window_below = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "keep_window_below"));
	gtk_toggle_button_set_active(w_keep_window_below,
				     cfg->window_keep_below_enabled);

	w_enable_menu = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "enable_menu"));
	gtk_toggle_button_set_active(w_enable_menu, !cfg->menu_bar_disabled);

	w_enable_launcher_counter = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "enable_launcher_counter"));
	gtk_toggle_button_set_active(w_enable_launcher_counter,
				     !cfg->unity_launcher_count_disabled);

	w_hide_on_startup
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "hide_on_startup"));
	gtk_toggle_button_set_active(w_hide_on_startup, cfg->hide_on_startup);

	w_win_restore
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "restore_window"));
	gtk_toggle_button_set_active(w_win_restore,
				     cfg->window_restore_enabled);

	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {
		double value;
		GdkColor color;

		g_mutex_lock(ui->sensors_mutex);

		gtk_color_button_get_color(w_color_fg, &color);
		color_set(cfg->graph_fgcolor,
			  color.red, color.green, color.blue);

		gtk_color_button_get_color(w_color_bg, &color);
		color_set(cfg->graph_bgcolor,
			  color.red, color.green, color.blue);

		value = gtk_range_get_value(GTK_RANGE(w_bg_opacity));
		cfg->graph_bg_alpha = value;

		if (value == 1.0)
			cfg->alpha_channel_enabled = 0;
		else
			cfg->alpha_channel_enabled = 1;

		cfg->sensorlist_position
			= gtk_combo_box_get_active(w_sensorlist_pos);

		cfg->window_decoration_enabled =
			!gtk_toggle_button_get_active(w_hide_window_decoration);

		cfg->window_keep_below_enabled
			= gtk_toggle_button_get_active(w_keep_window_below);

		cfg->menu_bar_disabled
			= !gtk_toggle_button_get_active(w_enable_menu);

		cfg->unity_launcher_count_disabled
			= !gtk_toggle_button_get_active
			(w_enable_launcher_counter);

		gtk_window_set_decorated(GTK_WINDOW(ui->main_window),
					 cfg->window_decoration_enabled);

		gtk_window_set_keep_below(GTK_WINDOW(ui->main_window),
					  cfg->window_keep_below_enabled);

		cfg->sensor_update_interval
			= gtk_spin_button_get_value_as_int(w_s_update_interval);

		cfg->graph_update_interval = gtk_spin_button_get_value_as_int
			(w_update_interval);

		cfg->graph_monitoring_duration
		    = gtk_spin_button_get_value_as_int
			(w_monitoring_duration);

		cfg->sensor_values_max_length
		    = (cfg->graph_monitoring_duration * 60) /
		    cfg->sensor_update_interval;

		cfg->hide_on_startup
			= gtk_toggle_button_get_active(w_hide_on_startup);

		cfg->window_restore_enabled
			= gtk_toggle_button_get_active(w_win_restore);

		config_save(cfg);

		g_mutex_unlock(ui->sensors_mutex);

		ui_window_update(ui);
	}
	g_object_unref(G_OBJECT(builder));
	gtk_widget_destroy(GTK_WIDGET(diag));
}
