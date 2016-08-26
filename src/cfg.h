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
#ifndef _PSENSOR_CONFIG_H_
#define _PSENSOR_CONFIG_H_

#include <gdk/gdk.h>

#include <bool.h>
#include <color.h>

enum temperature_unit {
	CELSIUS,
	FAHRENHEIT
};

enum sensorlist_position {
	SENSORLIST_POSITION_RIGHT,
	SENSORLIST_POSITION_LEFT,
	SENSORLIST_POSITION_TOP,
	SENSORLIST_POSITION_BOTTOM
};

struct config {
	struct color *graph_bgcolor;
	struct color *graph_fgcolor;

	double graph_bg_alpha;

	bool alpha_channel_enabled;

	bool window_restore_enabled;
	/* Last saved position of the window. */
	int window_x;
	int window_y;
	/* Last saved size of the window. */
	int window_w;
	int window_h;
	/* Last saved position of the window divider. */
	int window_divider_pos;

	int graph_update_interval;
	int graph_monitoring_duration;

	int sensor_values_max_length;
	int sensor_update_interval;

	int hide_on_startup;

	bool slog_enabled;
	int slog_interval;
};

/* Loads psensor configuration */
struct config *config_load(void);

void config_save(const struct config *);

void config_cleanup(void);

GdkRGBA *config_get_sensor_color(const char *);
void config_set_sensor_color(const char *, const GdkRGBA *);

bool config_get_sensor_alarm_high_threshold(const char *, double *);
void config_set_sensor_alarm_high_threshold(const char *, int);

bool config_get_sensor_alarm_low_threshold(const char *, double *);
void config_set_sensor_alarm_low_threshold(const char *, int);

bool config_get_sensor_alarm_enabled(const char *);
void config_set_sensor_alarm_enabled(const char *, bool);

bool config_is_sensor_graph_enabled(const char *);
void config_set_sensor_graph_enabled(const char *, bool);

char *config_get_sensor_name(const char *);
void config_set_sensor_name(const char *, const char *);

bool config_is_appindicator_enabled(const char *);
void config_set_appindicator_enabled(const char *, bool);

bool config_is_appindicator_label_enabled(const char *);
void config_set_appindicator_label_enabled(const char *, bool);

bool is_slog_enabled(void);
void config_set_slog_enabled_changed_cbk(void (*)(void *), void *);

int config_get_slog_interval(void);

bool config_is_smooth_curves_enabled(void);
void config_set_smooth_curves_enabled(bool);

int config_get_sensor_position(const char *);
void config_set_sensor_position(const char *, int);

char *config_get_notif_script(void);
void config_set_notif_script(const char *);

bool config_is_sensor_enabled(const char *sid);
void config_set_sensor_enabled(const char *sid, bool enabled);

bool config_is_lmsensor_enabled(void);
void config_set_lmsensor_enable(bool);

bool config_is_gtop2_enabled(void);
void config_set_gtop2_enable(bool);

bool config_is_udisks2_enabled(void);
void config_set_udisks2_enable(bool);

bool config_is_hddtemp_enabled(void);
void config_set_hddtemp_enable(bool);

bool config_is_libatasmart_enabled(void);
void config_set_libatasmart_enable(bool);

bool config_is_nvctrl_enabled(void);
void config_set_nvctrl_enable(bool);

bool config_is_atiadlsdk_enabled(void);
void config_set_atiadlsdk_enable(bool);

enum temperature_unit config_get_temperature_unit(void);
void config_set_temperature_unit(enum temperature_unit);

double config_get_default_high_threshold_temperature(void);

bool config_is_window_decoration_enabled(void);
void config_set_window_decoration_enabled(bool);

bool config_is_window_keep_below_enabled(void);
void config_set_window_keep_below_enabled(bool);

bool config_is_menu_bar_enabled(void);
void config_set_menu_bar_enabled(bool);

bool config_is_count_visible(void);
void config_set_count_visible(bool);

enum sensorlist_position config_get_sensorlist_position(void);
void config_set_sensorlist_position(enum sensorlist_position pos);

/*
 * Returns the user directory containing psensor data (configuration
 * and log).
 * Corresponds to $HOME/.psensor/
 * Creates the directory if it does not exist;
 * Returns NULL if it cannot be determined.
 */
const char *get_psensor_user_dir(void);

void config_sync(void);

GSettings *config_get_GSettings(void);

#endif
