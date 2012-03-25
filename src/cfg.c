/*
 * Copyright (C) 2010-2012 jeanfi@gmail.com
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

#define KEY_INTERFACE_MENU_BAR_DISABLED \
"/apps/psensor/interface/menu_bar_disabled"

#define KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED \
"/apps/psensor/interface/unity_launcher_count_disabled"

#define KEY_INTERFACE_HIDE_ON_STARTUP \
"/apps/psensor/interface/hide_on_startup"

#define KEY_INTERFACE_WINDOW_RESTORE_ENABLED \
"/apps/psensor/interface/window_restore_enabled"

#define KEY_INTERFACE_WINDOW_X "/apps/psensor/interface/window_x"
#define KEY_INTERFACE_WINDOW_Y "/apps/psensor/interface/window_y"
#define KEY_INTERFACE_WINDOW_W "/apps/psensor/interface/window_w"
#define KEY_INTERFACE_WINDOW_H "/apps/psensor/interface/window_h"

#define KEY_INTERFACE_WINDOW_DIVIDER_POS \
"/apps/psensor/interface/window_divider_pos"

GConfClient *client;

static char *get_string(char *key, char *default_value)
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

static struct color *config_get_background_color()
{

	char *scolor = get_string(KEY_GRAPH_BACKGROUND_COLOR,
					 DEFAULT_GRAPH_BACKGROUND_COLOR);

	struct color *c = string_to_color(scolor);

	free(scolor);

	if (c == NULL)
		return color_new(0xffff, 0xffff, 0xffff);

	return c;
}

static struct color *config_get_foreground_color()
{
	char *scolor = get_string(KEY_GRAPH_FOREGROUND_COLOR,
				  DEFAULT_GRAPH_FOREGROUND_COLOR);

	struct color *c = string_to_color(scolor);

	free(scolor);

	if (c == NULL)
		return color_new(0x0000, 0x0000, 0x0000);

	return c;
}

static int config_is_alpha_channel_enabled()
{
	gboolean b = gconf_client_get_bool(client,
					   KEY_ALPHA_CHANNEL_ENABLED,
					   NULL);

	return b == TRUE;
}

static int config_get_sensorlist_position()
{
	return gconf_client_get_int(client,
				    KEY_INTERFACE_SENSORLIST_POSITION, NULL);
}

static void config_set_sensorlist_position(int pos)
{
	gconf_client_set_int(client,
			     KEY_INTERFACE_SENSORLIST_POSITION, pos, NULL);
}

static double config_get_graph_background_alpha()
{
	double a = gconf_client_get_float(client,
					  KEY_GRAPH_BACKGROUND_ALPHA,
					  NULL);

	if (a == 0)
		gconf_client_set_float(client,
				       KEY_GRAPH_BACKGROUND_ALPHA, 1.0, NULL);
	return a;
}

static void config_set_graph_background_alpha(double alpha)
{
	gconf_client_set_float(client, KEY_GRAPH_BACKGROUND_ALPHA, alpha, NULL);
}

static void config_set_background_color(struct color *color)
{
	char *scolor = color_to_string(color);

	if (!scolor)
		scolor = strdup(DEFAULT_GRAPH_BACKGROUND_COLOR);

	gconf_client_set_string(client,
				KEY_GRAPH_BACKGROUND_COLOR, scolor, NULL);

	free(scolor);
}

static void config_set_foreground_color(struct color *color)
{
	char *scolor = color_to_string(color);

	if (!scolor)
		scolor = strdup(DEFAULT_GRAPH_FOREGROUND_COLOR);

	gconf_client_set_string(client,
				KEY_GRAPH_FOREGROUND_COLOR, scolor, NULL);

	free(scolor);
}

static char *config_get_sensor_key(char *sensor_name)
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

static int config_is_window_decoration_enabled()
{
	gboolean b;

	b = gconf_client_get_bool(client,
				  KEY_INTERFACE_WINDOW_DECORATION_DISABLED,
				  NULL);

	return b == FALSE;
}

static int config_is_window_keep_below_enabled()
{
	gboolean b;

	b = gconf_client_get_bool(client,
				  KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED,
				  NULL);

	return b == TRUE;
}

static void config_set_window_decoration_enabled(int enabled)
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

static void config_set_window_keep_below_enabled(int enabled)
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

void config_cleanup()
{
	if (client) {
		g_object_unref(client);
		client = NULL;
	}
}

struct config *config_load()
{
	struct config *c;

	c = malloc(sizeof(struct config));

	c->graph_bgcolor = config_get_background_color();
	c->graph_fgcolor = config_get_foreground_color();
	c->graph_bg_alpha = config_get_graph_background_alpha();
	c->alpha_channel_enabled = config_is_alpha_channel_enabled();
	c->sensorlist_position = config_get_sensorlist_position();
	c->window_decoration_enabled = config_is_window_decoration_enabled();
	c->window_keep_below_enabled = config_is_window_keep_below_enabled();

	c->sensor_update_interval
	    = gconf_client_get_int(client, KEY_SENSOR_UPDATE_INTERVAL, NULL);
	if (c->sensor_update_interval < 1)
		c->sensor_update_interval = 1;

	c->graph_update_interval
	    = gconf_client_get_int(client, KEY_GRAPH_UPDATE_INTERVAL, NULL);
	if (c->graph_update_interval < 1)
		c->graph_update_interval = 1;

	c->graph_monitoring_duration
	    = gconf_client_get_int(client, KEY_GRAPH_MONITORING_DURATION, NULL);

	if (c->graph_monitoring_duration < 1)
		c->graph_monitoring_duration = 10;

	c->sensor_values_max_length
	    =
	    (c->graph_monitoring_duration * 60) / c->sensor_update_interval;
	if (c->sensor_values_max_length < 3)
		c->sensor_values_max_length = 3;

	c->menu_bar_disabled
		= gconf_client_get_bool(client,
					KEY_INTERFACE_MENU_BAR_DISABLED,
					NULL);

	c->unity_launcher_count_disabled
		= gconf_client_get_bool
		(client,
		 KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED,
		 NULL);

	c->hide_on_startup
		= gconf_client_get_bool(client,
					KEY_INTERFACE_HIDE_ON_STARTUP,
					NULL);

	c->window_restore_enabled
		= gconf_client_get_bool(client,
					KEY_INTERFACE_WINDOW_RESTORE_ENABLED,
					NULL);

	c->window_x = gconf_client_get_int(client,
					   KEY_INTERFACE_WINDOW_X,
					   NULL);
	c->window_y = gconf_client_get_int(client,
					   KEY_INTERFACE_WINDOW_Y,
					   NULL);
	c->window_w = gconf_client_get_int(client,
					   KEY_INTERFACE_WINDOW_W,
					   NULL);
	c->window_h = gconf_client_get_int(client,
					   KEY_INTERFACE_WINDOW_H,
					   NULL);
	c->window_divider_pos
		= gconf_client_get_int(client,
				       KEY_INTERFACE_WINDOW_DIVIDER_POS,
				       NULL);

	if (!c->window_restore_enabled || !c->window_w || !c->window_h) {
		c->window_w = 800;
		c->window_h = 200;
	}

	return c;
}

void config_save(struct config *c)
{
	config_set_background_color(c->graph_bgcolor);
	config_set_foreground_color(c->graph_fgcolor);
	config_set_graph_background_alpha(c->graph_bg_alpha);
	config_set_sensorlist_position(c->sensorlist_position);
	config_set_window_decoration_enabled(c->window_decoration_enabled);
	config_set_window_keep_below_enabled(c->window_keep_below_enabled);

	gconf_client_set_int(client,
			     KEY_GRAPH_UPDATE_INTERVAL,
			     c->graph_update_interval, NULL);

	gconf_client_set_int(client,
			     KEY_GRAPH_MONITORING_DURATION,
			     c->graph_monitoring_duration, NULL);

	gconf_client_set_int(client,
			     KEY_SENSOR_UPDATE_INTERVAL,
			     c->sensor_update_interval, NULL);

	gconf_client_set_bool(client,
			      KEY_INTERFACE_MENU_BAR_DISABLED,
			      c->menu_bar_disabled, NULL);

	gconf_client_set_bool(client,
			      KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED,
			      c->unity_launcher_count_disabled, NULL);

	gconf_client_set_bool(client,
			      KEY_INTERFACE_HIDE_ON_STARTUP,
			      c->hide_on_startup, NULL);

	gconf_client_set_bool(client,
			      KEY_INTERFACE_WINDOW_RESTORE_ENABLED,
			      c->window_restore_enabled,
			      NULL);

	gconf_client_set_int(client,
			     KEY_INTERFACE_WINDOW_X,
			     c->window_x,
			     NULL);
	gconf_client_set_int(client,
			     KEY_INTERFACE_WINDOW_Y,
			     c->window_y,
			     NULL);
	gconf_client_set_int(client,
			     KEY_INTERFACE_WINDOW_W,
			     c->window_w,
			     NULL);
	gconf_client_set_int(client,
			     KEY_INTERFACE_WINDOW_H,
			     c->window_h,
			     NULL);

	gconf_client_set_int(client,
			     KEY_INTERFACE_WINDOW_DIVIDER_POS,
			     c->window_divider_pos,
			     NULL);
}
