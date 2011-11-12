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

#ifndef _PSENSOR_CONFIG_H_
#define _PSENSOR_CONFIG_H_

#include "color.h"

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

	int alpha_channel_enabled;

	/*
	   Position of the sensors list table
	 */
	enum sensorlist_position sensorlist_position;

	int window_decoration_enabled;
	int window_keep_below_enabled;
	int window_restore_enabled;
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

	int menu_bar_disabled;

	int unity_launcher_count_disabled;

	int hide_on_startup;
};

/*
  Loads config from GConf
*/
struct config *config_load();

void config_save(struct config *);

void config_init();

void config_cleanup();

struct color *config_get_sensor_color(char *, struct color *);
void config_set_sensor_color(char *, struct color *);

int config_get_sensor_alarm_limit(char *, int);
void config_set_sensor_alarm_limit(char *, int);

int config_get_sensor_alarm_enabled(char *);
void config_set_sensor_alarm_enabled(char *, int);

int config_is_sensor_enabled(char *);
void config_set_sensor_enabled(char *, int);

char *config_get_sensor_name(char *);
void config_set_sensor_name(char *, const char *);

#endif
