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
#ifndef _PSENSOR_CONFIG_H_
#define _PSENSOR_CONFIG_H_

#include <gconf/gconf-client.h>

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

	/* Position of the sensors list table */
	enum sensorlist_position sensorlist_position;

	bool window_decoration_enabled;
	bool window_keep_below_enabled;
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

	bool menu_bar_disabled;

	bool unity_launcher_count_disabled;

	int hide_on_startup;

	enum temperature_unit temperature_unit;

	bool slog_enabled;
	int slog_interval;
};

/* Loads psensor configuration */
struct config *config_load();

void config_save(const struct config *);

void config_cleanup();

struct color *config_get_sensor_color(const char *sid, const struct color *);
void config_set_sensor_color(const char *sid, const struct color *);

int config_get_sensor_alarm_high_threshold(const char *);
void config_set_sensor_alarm_high_threshold(const char *, int);

int config_get_sensor_alarm_low_threshold(const char *);
void config_set_sensor_alarm_low_threshold(const char *, int);

bool config_get_sensor_alarm_enabled(const char *);
void config_set_sensor_alarm_enabled(const char *, bool);

bool config_is_sensor_enabled(const char *);
void config_set_sensor_enabled(const char *, bool);

char *config_get_sensor_name(const char *);
void config_set_sensor_name(const char *, const char *);

bool config_is_appindicator_enabled(const char *);
void config_set_appindicator_enabled(const char *, bool);

bool config_is_appindicator_label_enabled(const char *);
void config_set_appindicator_label_enabled(const char *, bool);

void config_slog_enabled_notify_add(GConfClientNotifyFunc cbk, void *data);
bool is_slog_enabled();

int config_get_slog_interval();

int config_get_sensor_position(const char *);
void config_set_sensor_position(const char *, int);

char *config_get_notif_script();
void config_set_notif_script(const char *);

/*
 * Returns the user directory containing psensor data (configuration
 * and log).
 * Corresponds to $HOME/.psensor/
 * Creates the directory if it does not exist;
 * Returns NULL if it cannot be determined.
 */
const char *get_psensor_user_dir();

void config_sync();

#endif
