/*
    Copyright (C) 2010-2011 wpitchoune@gmail.com

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
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gconf/gconf-client.h>

#include "cfg.h"

#define KEY_SENSOR_UPDATE_INTERVAL "/apps/psensor/sensor/update_interval"

#define KEY_GRAPH_UPDATE_INTERVAL "/apps/psensor/graph/update_interval"
#define KEY_GRAPH_MONITORING_DURATION "/apps/psensor/graph/monitoring_duration"

#define KEY_GRAPH_BACKGROUND_COLOR "/apps/psensor/graph/background_color"
#define DEFAULT_GRAPH_BACKGROUND_COLOR "#e8f4e8f4a8f5"

#define KEY_GRAPH_BACKGROUND_ALPHA  "/apps/psensor/graph/background_alpha"
#define DEFAULT_GRAPH_BACKGROUND_ALPHA "1.0"

#define KEY_GRAPH_FOREGROUND_COLOR  "/apps/psensor/graph/foreground_color"
#define DEFAULT_GRAPH_FOREGROUND_COLOR "#000000000000"

#define KEY_ALPHA_CHANNEL_ENABLED  "/apps/psensor/graph/alpha_channel_enabled"
#define DEFAULT_ALPHA_CHANNEL_ENABLED 0

#define KEY_INTERFACE_SENSORLIST_POSITION \
"/apps/psensor/interface/sensorlist_position"

#define KEY_INTERFACE_WINDOW_DECORATION_DISABLED \
"/apps/psensor/interface/window_decoration_disabled"

#define KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED \
"/apps/psensor/interface/window_keep_below_enabled"

GConfClient *client;

char *config_get_string(char *key, char *default_value)
{
	char *value = gconf_client_get_string(client,
					      key,
					      NULL);

	if (!value) {
		value = strdup(default_value);

		gconf_client_set_string(client, key, default_value, NULL);
	}

	return value;
}

struct color *config_get_background_color()
{

	char *scolor = config_get_string(KEY_GRAPH_BACKGROUND_COLOR,
					 DEFAULT_GRAPH_BACKGROUND_COLOR);

	struct color *c = string_to_color(scolor);

	free(scolor);

	if (c == NULL)
		return color_new(0xffff, 0xffff, 0xffff);

	return c;
}

struct color *config_get_foreground_color()
{
	char *scolor = config_get_string(KEY_GRAPH_FOREGROUND_COLOR,
					 DEFAULT_GRAPH_FOREGROUND_COLOR);

	struct color *c = string_to_color(scolor);

	free(scolor);

	if (c == NULL)
		return color_new(0x0000, 0x0000, 0x0000);

	return c;
}

int config_is_alpha_channel_enabled()
{
	gboolean b = gconf_client_get_bool(client,
					   KEY_ALPHA_CHANNEL_ENABLED,
					   NULL);

	return b == TRUE;
}

void config_set_alpha_channel_enabled(int enabled)
{
	if (enabled)
		gconf_client_set_bool(client,
				      KEY_ALPHA_CHANNEL_ENABLED, TRUE, NULL);
	else
		gconf_client_set_bool(client,
				      KEY_ALPHA_CHANNEL_ENABLED, FALSE, NULL);
}

int config_get_sensorlist_position()
{
	return gconf_client_get_int(client,
				    KEY_INTERFACE_SENSORLIST_POSITION, NULL);
}

void config_set_sensorlist_position(int pos)
{
	gconf_client_set_int(client,
			     KEY_INTERFACE_SENSORLIST_POSITION, pos, NULL);
}

double config_get_graph_background_alpha()
{
	double a = gconf_client_get_float(client,
					  KEY_GRAPH_BACKGROUND_ALPHA,
					  NULL);

	if (a == 0)
		gconf_client_set_float(client,
				       KEY_GRAPH_BACKGROUND_ALPHA, 1.0, NULL);
	return a;
}

void config_set_graph_background_alpha(double alpha)
{
	gconf_client_set_float(client, KEY_GRAPH_BACKGROUND_ALPHA, alpha, NULL);
}

void config_set_background_color(struct color *color)
{
	char *scolor = color_to_string(color);

	if (!scolor)
		scolor = strdup(DEFAULT_GRAPH_BACKGROUND_COLOR);

	gconf_client_set_string(client,
				KEY_GRAPH_BACKGROUND_COLOR, scolor, NULL);

	free(scolor);
}

void config_set_foreground_color(struct color *color)
{
	char *scolor = color_to_string(color);

	if (!scolor)
		scolor = strdup(DEFAULT_GRAPH_FOREGROUND_COLOR);

	gconf_client_set_string(client,
				KEY_GRAPH_FOREGROUND_COLOR, scolor, NULL);

	free(scolor);
}

char *config_get_sensor_key(char *sensor_name)
{
	char *escaped_name = gconf_escape_key(sensor_name, -1);
	/* /apps/psensor/sensors/[sensor_name]/color */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 6 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/color", escaped_name);

	free(escaped_name);

	return key;
}

struct color *config_get_sensor_color(char *sensor_name,
				      struct color *default_color)
{
	char *key = config_get_sensor_key(sensor_name);

	char *scolor = gconf_client_get_string(client,
					       key,
					       NULL);

	struct color *color = NULL;

	if (scolor)
		color = string_to_color(scolor);

	if (!scolor || !color) {
		color = color_new(default_color->red,
				  default_color->green, default_color->blue);

		scolor = color_to_string(color);

		gconf_client_set_string(client, key, scolor, NULL);
	}

	free(scolor);
	free(key);

	return color;
}

void config_set_sensor_color(char *sensor_name, struct color *color)
{
	char *key = config_get_sensor_key(sensor_name);

	char *scolor = color_to_string(color);

	gconf_client_set_string(client, key, scolor, NULL);

	free(scolor);
}

int config_get_sensor_alarm_limit(char *sensor_name, int def)
{
	int res;
	char *escaped_name = gconf_escape_key(sensor_name, -1);
	/* /apps/psensor/sensors/[sensor_name]/alarmlimit */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 10 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/alarmlimit", escaped_name);

	res = gconf_client_get_int(client, key, NULL);

	free(escaped_name);

	return res ? res : def;
}

void config_set_sensor_alarm_limit(char *sensor_name, int alarm_limit)
{
	char *escaped_name = gconf_escape_key(sensor_name, -1);
	/* /apps/psensor/sensors/[sensor_name]/alarmlimit */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 10 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/alarmlimit", escaped_name);

	gconf_client_set_int(client, key, alarm_limit, NULL);

	free(escaped_name);
}

int config_get_sensor_alarm_enabled(char *sid)
{
	gboolean res;
	char *escaped_name = gconf_escape_key(sid, -1);
	/* /apps/psensor/sensors/[sensor_name]/alarmenabled */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 12 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/alarmenabled", escaped_name);

	res = gconf_client_get_bool(client, key, NULL);

	free(escaped_name);

	return res == TRUE;
}

void config_set_sensor_alarm_enabled(char *sid, int enabled)
{
	char *escaped_name = gconf_escape_key(sid, -1);
	/* /apps/psensor/sensors/[sensor_name]/alarmenabled */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 12 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/alarmenabled", escaped_name);

	gconf_client_set_bool(client, key, enabled, NULL);

	free(escaped_name);
}

int config_is_sensor_enabled(char *sid)
{
	gboolean res;
	char *escaped_name = gconf_escape_key(sid, -1);
	/* /apps/psensor/sensors/[sensor_name]/enabled */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 7 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/enabled", escaped_name);

	res = gconf_client_get_bool(client, key, NULL);

	free(escaped_name);

	return res == TRUE;

}

void config_set_sensor_enabled(char *sid, int enabled)
{
	char *escaped_name = gconf_escape_key(sid, -1);
	/* /apps/psensor/sensors/[sensor_name]/enabled */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 7 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/enabled", escaped_name);

	gconf_client_set_bool(client, key, enabled, NULL);

	free(escaped_name);
}

char *config_get_sensor_name(char *sid)
{
	char *res;
	char *escaped_name = gconf_escape_key(sid, -1);
	/* /apps/psensor/sensors/[sensor_name]/name */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 4 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/name", escaped_name);

	res = gconf_client_get_string(client, key, NULL);

	free(escaped_name);

	return res;
}

void config_set_sensor_name(char *sid, const char *name)
{
	char *escaped_name = gconf_escape_key(sid, -1);
	/* /apps/psensor/sensors/[sensor_name]/name */
	char *key = malloc(22 + 2 * strlen(escaped_name) + 1 + 4 + 1);

	sprintf(key, "/apps/psensor/sensors/%s/name", escaped_name);

	gconf_client_set_string(client, key, name, NULL);

	free(escaped_name);
}

int config_is_window_decoration_enabled()
{
	gboolean b;

	b = gconf_client_get_bool(client,
				  KEY_INTERFACE_WINDOW_DECORATION_DISABLED,
				  NULL);

	return b == FALSE;
}

int config_is_window_keep_below_enabled()
{
	gboolean b;

	b = gconf_client_get_bool(client,
				  KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED,
				  NULL);

	return b == TRUE;
}

void config_set_window_decoration_enabled(int enabled)
{
	if (enabled)
		gconf_client_set_bool
		    (client,
		     KEY_INTERFACE_WINDOW_DECORATION_DISABLED, FALSE, NULL);
	else
		gconf_client_set_bool
		    (client,
		     KEY_INTERFACE_WINDOW_DECORATION_DISABLED, TRUE, NULL);
}

void config_set_window_keep_below_enabled(int enabled)
{
	if (enabled)
		gconf_client_set_bool(client,
				      KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED,
				      TRUE, NULL);
	else
		gconf_client_set_bool(client,
				      KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED,
				      FALSE, NULL);
}

void config_init()
{
	client = gconf_client_get_default();
}

struct config *config_load()
{
	struct config *cfg = malloc(sizeof(struct config));

	cfg->graph_bgcolor = config_get_background_color();
	cfg->graph_fgcolor = config_get_foreground_color();
	cfg->graph_bg_alpha = config_get_graph_background_alpha();
	cfg->alpha_channel_enabled = config_is_alpha_channel_enabled();
	cfg->sensorlist_position = config_get_sensorlist_position();
	cfg->window_decoration_enabled = config_is_window_decoration_enabled();
	cfg->window_keep_below_enabled = config_is_window_keep_below_enabled();

	cfg->sensor_update_interval
	    = gconf_client_get_int(client, KEY_SENSOR_UPDATE_INTERVAL, NULL);
	if (cfg->sensor_update_interval < 1)
		cfg->sensor_update_interval = 1;

	cfg->graph_update_interval
	    = gconf_client_get_int(client, KEY_GRAPH_UPDATE_INTERVAL, NULL);
	if (cfg->graph_update_interval < 1)
		cfg->graph_update_interval = 1;

	cfg->graph_monitoring_duration
	    = gconf_client_get_int(client, KEY_GRAPH_MONITORING_DURATION, NULL);

	if (cfg->graph_monitoring_duration < 1)
		cfg->graph_monitoring_duration = 10;

	cfg->sensor_values_max_length
	    =
	    (cfg->graph_monitoring_duration * 60) / cfg->sensor_update_interval;
	if (cfg->sensor_values_max_length < 3)
		cfg->sensor_values_max_length = 3;

	return cfg;
}

void config_save(struct config *cfg)
{
	config_set_background_color(cfg->graph_bgcolor);
	config_set_foreground_color(cfg->graph_fgcolor);
	config_set_graph_background_alpha(cfg->graph_bg_alpha);
	config_set_sensorlist_position(cfg->sensorlist_position);
	config_set_window_decoration_enabled(cfg->window_decoration_enabled);
	config_set_window_keep_below_enabled(cfg->window_keep_below_enabled);

	gconf_client_set_int(client,
			     KEY_GRAPH_UPDATE_INTERVAL,
			     cfg->graph_update_interval, NULL);

	gconf_client_set_int(client,
			     KEY_GRAPH_MONITORING_DURATION,
			     cfg->graph_monitoring_duration, NULL);

	gconf_client_set_int(client,
			     KEY_SENSOR_UPDATE_INTERVAL,
			     cfg->sensor_update_interval, NULL);

}
