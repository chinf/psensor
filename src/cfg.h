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

#define SENSORLIST_POSITION_RIGHT 0
#define SENSORLIST_POSITION_LEFT 1
#define SENSORLIST_POSITION_TOP 2
#define SENSORLIST_POSITION_BOTTOM 3

struct config {
	struct color *graph_bgcolor;
	struct color *graph_fgcolor;

	double graph_bg_alpha;

	int alpha_channel_enabled;

	/*
	   Position of the sensors list table
	 */
	int sensorlist_position;

	int window_decoration_enabled;

	int window_keep_below_enabled;

	int graph_update_interval;
	int graph_monitoring_duration;

	int sensor_values_max_length;
	int sensor_update_interval;

	int menu_bar_disabled;
};

/*
  Loads config from GConf
*/
struct config *config_load();

void config_save(struct config *);

void config_init();

struct color *config_get_sensor_color(char *sensor_name,
				      struct color *default_color);

void config_set_sensor_color(char *sensor_name, struct color *color);

int config_get_sensor_alarm_limit(char *, int);
void config_set_sensor_alarm_limit(char *sensor_name, int alarm_limit);

int config_get_sensor_alarm_enabled(char *);
void config_set_sensor_alarm_enabled(char *, int);

int config_is_sensor_enabled(char *);
void config_set_sensor_enabled(char *, int);

char *config_get_sensor_name(char *);
void config_set_sensor_name(char *, const char *);

#endif
