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
#include <string.h>

#include <ui.h>
#include <cfg.h>
#include <graph.h>
#include <ui_pref.h>
#include <ui_color.h>
#include <pxdg.h>

GdkRGBA color_to_GdkRGBA(struct color *color)
{
	GdkRGBA c;

	c.red = color->red;
	c.green = color->green;
	c.blue = color->blue;
	c.alpha = 1.0;

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
	GdkRGBA color_fg, color_bg;
	GtkColorChooser *w_color_fg, *w_color_bg;
	GtkScale *w_bg_opacity;
	GtkSpinButton *w_update_interval, *w_monitoring_duration,
		*w_s_update_interval, *w_slog_interval;
	GtkComboBox *w_sensorlist_pos;
	GtkToggleButton *w_hide_window_decoration, *w_keep_window_below,
		*w_enable_menu, *w_enable_launcher_counter, *w_hide_on_startup,
		*w_win_restore, *w_slog_enabled, *w_autostart, *w_smooth_curves,
		*w_atiadlsdk, *w_lmsensors, *w_nvctrl, *w_gtop2, *w_hddtemp,
		*w_libatasmart, *w_udisks2;
	GtkComboBoxText *w_temp_unit;
	GtkEntry *w_notif_script;
	char *notif_script;

	cfg = ui->config;

	builder = gtk_builder_new();

	ok = gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "psensor-pref.glade",
		 &error);

	if (!ok) {
		log_printf(LOG_ERR, error->message);
		g_error_free(error);
		return;
	}

	diag = GTK_DIALOG(gtk_builder_get_object(builder, "dialog1"));

	w_notif_script = GTK_ENTRY(gtk_builder_get_object(builder,
							  "notif_script"));
	notif_script = config_get_notif_script();
	if (notif_script) {
		gtk_entry_set_text(GTK_ENTRY(w_notif_script), notif_script);
		free(notif_script);
	}

	color_fg = color_to_GdkRGBA(cfg->graph_fgcolor);
	w_color_fg = GTK_COLOR_CHOOSER(gtk_builder_get_object(builder,
							      "color_fg"));
	gtk_color_chooser_set_rgba(w_color_fg, &color_fg);

	color_bg = color_to_GdkRGBA(cfg->graph_bgcolor);
	w_color_bg = GTK_COLOR_CHOOSER(gtk_builder_get_object(builder,
							     "color_bg"));
	gtk_color_chooser_set_rgba(w_color_bg, &color_bg);

	w_bg_opacity = GTK_SCALE(gtk_builder_get_object(builder,
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

	w_autostart = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "autostart"));
	gtk_toggle_button_set_active(w_autostart, pxdg_is_autostarted());

	w_enable_menu = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "enable_menu"));
	gtk_toggle_button_set_active(w_enable_menu, !cfg->menu_bar_disabled);

	w_enable_launcher_counter = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "enable_launcher_counter"));
	gtk_toggle_button_set_active(w_enable_launcher_counter,
				     !cfg->unity_launcher_count_disabled);

	w_smooth_curves = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "graph_smooth_curves"));
	gtk_toggle_button_set_active(w_smooth_curves,
				     config_is_smooth_curves_enabled());

	w_slog_enabled = GTK_TOGGLE_BUTTON
		(gtk_builder_get_object(builder, "enable_slog"));
	gtk_toggle_button_set_active(w_slog_enabled, cfg->slog_enabled);

	w_slog_interval = GTK_SPIN_BUTTON
		(gtk_builder_get_object(builder, "slog_interval"));
	gtk_spin_button_set_value(w_slog_interval, cfg->slog_interval);

	w_hide_on_startup
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "hide_on_startup"));
	gtk_toggle_button_set_active(w_hide_on_startup, cfg->hide_on_startup);

	w_win_restore
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "restore_window"));
	gtk_toggle_button_set_active(w_win_restore,
				     cfg->window_restore_enabled);

	w_temp_unit
		= GTK_COMBO_BOX_TEXT(gtk_builder_get_object
				     (builder, "temperature_unit"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(w_temp_unit),
				 cfg->temperature_unit);

	/* providers */
	w_lmsensors
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "lmsensors"));
	gtk_toggle_button_set_active(w_lmsensors, config_is_lmsensor_enabled());

	w_nvctrl
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "nvctrl"));
#if !HAVE_NVIDIA
	gtk_widget_set_sensitive(GTK_WIDGET(w_nvctrl), 0);
#endif
	gtk_toggle_button_set_active(w_nvctrl, config_is_nvctrl_enabled());

	w_atiadlsdk
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "atiadlsdk"));
#if !HAVE_LIBATIADL
	gtk_widget_set_sensitive(GTK_WIDGET(w_atiadlsdk), 0);
#endif
	gtk_toggle_button_set_active(w_atiadlsdk,
				     config_is_atiadlsdk_enabled());

	w_gtop2
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "gtop2"));
#if !HAVE_GTOP
	gtk_widget_set_sensitive(GTK_WIDGET(w_gtop2), 0);
#endif
	gtk_toggle_button_set_active(w_gtop2, config_is_gtop2_enabled());

	w_hddtemp
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "hddtemp"));
	gtk_toggle_button_set_active(w_hddtemp, config_is_hddtemp_enabled());


	w_libatasmart
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "libatasmart"));
#if !HAVE_ATASMART
	gtk_widget_set_sensitive(GTK_WIDGET(w_libatasmart), 0);
#endif
	gtk_toggle_button_set_active(w_libatasmart,
				     config_is_libatasmart_enabled());

	w_udisks2
		= GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
							   "udisks2"));
#if !HAVE_LIBUDISKS2
	gtk_widget_set_sensitive(GTK_WIDGET(w_udisks2), 0);
#endif
	gtk_toggle_button_set_active(w_udisks2, config_is_udisks2_enabled());

	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {
		double value;
		GdkRGBA color;

		pthread_mutex_lock(&ui->sensors_mutex);

		config_set_notif_script
			(gtk_entry_get_text(GTK_ENTRY(w_notif_script)));

		gtk_color_chooser_get_rgba(w_color_fg, &color);
		color_set(cfg->graph_fgcolor,
			  color.red,
			  color.green,
			  color.blue);

		gtk_color_chooser_get_rgba(w_color_bg, &color);
		color_set(cfg->graph_bgcolor,
			  color.red,
			  color.green,
			  color.blue);

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

		cfg->hide_on_startup
			= gtk_toggle_button_get_active(w_hide_on_startup);

		cfg->window_restore_enabled
			= gtk_toggle_button_get_active(w_win_restore);

		cfg->slog_enabled
			= gtk_toggle_button_get_active(w_slog_enabled);

		cfg->slog_interval
			= gtk_spin_button_get_value_as_int(w_slog_interval);

		cfg->temperature_unit
			= gtk_combo_box_get_active(GTK_COMBO_BOX(w_temp_unit));

		cfg->sensor_values_max_length = compute_values_max_length(cfg);

		config_save(cfg);

		config_set_smooth_curves_enabled
			(gtk_toggle_button_get_active(w_smooth_curves));

		config_set_lmsensor_enable
			(gtk_toggle_button_get_active(w_lmsensors));

		config_set_nvctrl_enable
			(gtk_toggle_button_get_active(w_nvctrl));

		config_set_atiadlsdk_enable
			(gtk_toggle_button_get_active(w_atiadlsdk));

		config_set_gtop2_enable
			(gtk_toggle_button_get_active(w_gtop2));

		config_set_hddtemp_enable
			(gtk_toggle_button_get_active(w_hddtemp));

		config_set_libatasmart_enable
			(gtk_toggle_button_get_active(w_libatasmart));

		config_set_udisks2_enable
			(gtk_toggle_button_get_active(w_udisks2));

		pxdg_set_autostart(gtk_toggle_button_get_active(w_autostart));

		pthread_mutex_unlock(&ui->sensors_mutex);

		ui_window_update(ui);
	}
	g_object_unref(G_OBJECT(builder));
	gtk_widget_destroy(GTK_WIDGET(diag));
}
