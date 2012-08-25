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

static const char *KEY_SENSORS = "/apps/psensor/sensors";

static const char *ATT_SENSOR_ALARM_ENABLED = "alarm/enabled";
static const char *ATT_SENSOR_ALARM_HIGH_THRESHOLD = "alarm/high_threshold";
static const char *ATT_SENSOR_ALARM_LOW_THRESHOLD = "alarm/low_threshold";
static const char *ATT_SENSOR_COLOR = "color";
static const char *ATT_SENSOR_ENABLED = "enabled";
static const char *ATT_SENSOR_NAME = "name";
static const char *ATT_SENSOR_APPINDICATOR_DISABLED = "appindicator/disabled";

static const char *KEY_SENSOR_UPDATE_INTERVAL
= "/apps/psensor/sensor/update_interval";

static const char *KEY_GRAPH_UPDATE_INTERVAL
= "/apps/psensor/graph/update_interval";

static const char *KEY_GRAPH_MONITORING_DURATION
= "/apps/psensor/graph/monitoring_duration";

static const char *KEY_GRAPH_BACKGROUND_COLOR
= "/apps/psensor/graph/background_color";

static const char *DEFAULT_GRAPH_BACKGROUND_COLOR = "#e8f4e8f4a8f5";

static const char *KEY_GRAPH_BACKGROUND_ALPHA
= "/apps/psensor/graph/background_alpha";

static const char *KEY_GRAPH_FOREGROUND_COLOR
= "/apps/psensor/graph/foreground_color";
static const char *DEFAULT_GRAPH_FOREGROUND_COLOR = "#000000000000";

static const char *KEY_ALPHA_CHANNEL_ENABLED
= "/apps/psensor/graph/alpha_channel_enabled";

static const char *KEY_INTERFACE_SENSORLIST_POSITION
= "/apps/psensor/interface/sensorlist_position";

static const char *KEY_INTERFACE_WINDOW_DECORATION_DISABLED
= "/apps/psensor/interface/window_decoration_disabled";

static const char *KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED
= "/apps/psensor/interface/window_keep_below_enabled";

static const char *KEY_INTERFACE_MENU_BAR_DISABLED
= "/apps/psensor/interface/menu_bar_disabled";

static const char *KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED
= "/apps/psensor/interface/unity_launcher_count_disabled";

static const char *KEY_INTERFACE_HIDE_ON_STARTUP
= "/apps/psensor/interface/hide_on_startup";

static const char *KEY_INTERFACE_WINDOW_RESTORE_ENABLED
= "/apps/psensor/interface/window_restore_enabled";

static const char *KEY_INTERFACE_WINDOW_X = "/apps/psensor/interface/window_x";
static const char *KEY_INTERFACE_WINDOW_Y = "/apps/psensor/interface/window_y";
static const char *KEY_INTERFACE_WINDOW_W = "/apps/psensor/interface/window_w";
static const char *KEY_INTERFACE_WINDOW_H = "/apps/psensor/interface/window_h";

static const char *KEY_INTERFACE_WINDOW_DIVIDER_POS
= "/apps/psensor/interface/window_divider_pos";

static const char *KEY_INTERFACE_TEMPERATURE_UNIT
= "/apps/psensor/interface/temperature_unit";

static GConfClient *client;

static char *get_string(const char *key, const char *default_value)
{
	char *value;

	value = gconf_client_get_string(client, key, NULL);

	if (!value) {
		value = strdup(default_value);
		gconf_client_set_string(client, key, default_value, NULL);
	}

	return value;
}

static struct color *get_background_color()
{
	char *scolor;
	struct color *c;

	scolor = get_string(KEY_GRAPH_BACKGROUND_COLOR,
			    DEFAULT_GRAPH_BACKGROUND_COLOR);

	c = str_to_color(scolor);
	free(scolor);

	if (!c)
		return color_new(0xffff, 0xffff, 0xffff);

	return c;
}

static struct color *get_foreground_color()
{
	char *scolor;
	struct color *c;

	scolor = get_string(KEY_GRAPH_FOREGROUND_COLOR,
			    DEFAULT_GRAPH_FOREGROUND_COLOR);

	c = str_to_color(scolor);
	free(scolor);

	if (!c)
		return color_new(0x0000, 0x0000, 0x0000);

	return c;
}

static bool is_alpha_channel_enabled()
{
	return gconf_client_get_bool(client, KEY_ALPHA_CHANNEL_ENABLED, NULL);
}

static enum sensorlist_position get_sensorlist_position()
{
	return gconf_client_get_int(client,
				    KEY_INTERFACE_SENSORLIST_POSITION, NULL);
}

static void set_sensorlist_position(enum sensorlist_position pos)
{
	gconf_client_set_int(client,
			     KEY_INTERFACE_SENSORLIST_POSITION, pos, NULL);
}

static double get_graph_background_alpha()
{
	double a;

	a = gconf_client_get_float(client, KEY_GRAPH_BACKGROUND_ALPHA, NULL);
	if (a == 0)
		gconf_client_set_float(client,
				       KEY_GRAPH_BACKGROUND_ALPHA, 1.0, NULL);
	return a;
}

static void set_graph_background_alpha(double alpha)
{
	gconf_client_set_float(client, KEY_GRAPH_BACKGROUND_ALPHA, alpha, NULL);
}

static void set_background_color(const struct color *color)
{
	char *scolor;

	scolor = color_to_str(color);
	if (!scolor)
		scolor = strdup(DEFAULT_GRAPH_BACKGROUND_COLOR);

	gconf_client_set_string(client,
				KEY_GRAPH_BACKGROUND_COLOR, scolor, NULL);

	free(scolor);
}

static void set_foreground_color(const struct color *color)
{
	char *str;

	str = color_to_str(color);
	if (!str)
		str = strdup(DEFAULT_GRAPH_FOREGROUND_COLOR);

	gconf_client_set_string(client, KEY_GRAPH_FOREGROUND_COLOR, str, NULL);

	free(str);
}

static char *get_sensor_att_key(const char *sid, const char *att)
{
	char *esc_sid, *key;

	esc_sid = gconf_escape_key(sid, -1);
	/* [KEY_SENSORS]/[esc_sid]/[att] */
	key = malloc(strlen(KEY_SENSORS)
		     + 1 + 2 * strlen(esc_sid)
		     + 1 + strlen(att) + 1);

	sprintf(key, "%s/%s/%s", KEY_SENSORS, esc_sid, att);

	free(esc_sid);

	return key;
}

struct color *
config_get_sensor_color(const char *sid, const struct color *dft)
{
	char *key, *scolor;
	struct color *color;

	key = get_sensor_att_key(sid, ATT_SENSOR_COLOR);

	scolor = gconf_client_get_string(client, key, NULL);

	color = NULL;

	if (scolor)
		color = str_to_color(scolor);

	if (!scolor || !color) {
		color = color_new(dft->red, dft->green, dft->blue);
		scolor = color_to_str(color);
		gconf_client_set_string(client, key, scolor, NULL);
	}

	free(scolor);
	free(key);

	return color;
}

void config_set_sensor_color(const char *sid, const struct color *color)
{
	char *key, *scolor;

	key = get_sensor_att_key(sid, ATT_SENSOR_COLOR);
	scolor = color_to_str(color);

	gconf_client_set_string(client, key, scolor, NULL);

	free(scolor);
	free(key);
}

int config_get_sensor_alarm_high_threshold(const char *sid)
{
	int res;
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ALARM_HIGH_THRESHOLD);
	res = gconf_client_get_int(client, key, NULL);
	free(key);

	return res;
}

void
config_set_sensor_alarm_high_threshold(const char *sid, int threshold)
{
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ALARM_HIGH_THRESHOLD);
	gconf_client_set_int(client, key, threshold, NULL);
	free(key);
}

int config_get_sensor_alarm_low_threshold(const char *sid)
{
	int res;
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ALARM_LOW_THRESHOLD);
	res = gconf_client_get_int(client, key, NULL);
	free(key);

	return res;
}

void
config_set_sensor_alarm_low_threshold(const char *sid, int threshold)
{
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ALARM_LOW_THRESHOLD);
	gconf_client_set_int(client, key, threshold, NULL);
	free(key);
}

bool config_get_sensor_alarm_enabled(const char *sid)
{
	gboolean b;
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ALARM_ENABLED);
	b = gconf_client_get_bool(client, key, NULL);
	free(key);

	return b;
}

void config_set_sensor_alarm_enabled(const char *sid, bool enabled)
{
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ALARM_ENABLED);
	gconf_client_set_bool(client, key, enabled, NULL);
	free(key);
}

bool config_is_sensor_enabled(const char *sid)
{
	gboolean b;
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ENABLED);
	b = gconf_client_get_bool(client, key, NULL);
	free(key);

	return b;
}

void config_set_sensor_enabled(const char *sid, bool enabled)
{
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_ENABLED);
	gconf_client_set_bool(client, key, enabled, NULL);
	free(key);
}

char *config_get_sensor_name(const char *sid)
{
	char *name, *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_NAME);
	name = gconf_client_get_string(client, key, NULL);
	free(key);

	return name;
}

void config_set_sensor_name(const char *sid, const char *name)
{
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_NAME);
	gconf_client_set_string(client, key, name, NULL);
	free(key);
}

bool config_is_appindicator_enabled(const char *sid)
{
	char *key;
	gboolean b;

	key = get_sensor_att_key(sid, ATT_SENSOR_APPINDICATOR_DISABLED);
	b = gconf_client_get_bool(client, key, NULL);
	free(key);

	return !b;
}

void config_set_appindicator_enabled(const char *sid, bool enabled)
{
	char *key;

	key = get_sensor_att_key(sid, ATT_SENSOR_APPINDICATOR_DISABLED);
	gconf_client_set_bool(client, key, !enabled, NULL);
	free(key);
}


static bool is_window_decoration_enabled()
{
	return !gconf_client_get_bool(client,
				      KEY_INTERFACE_WINDOW_DECORATION_DISABLED,
				      NULL);
}

static bool is_window_keep_below_enabled()
{
	return gconf_client_get_bool(client,
				     KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED,
				     NULL);
}

static void set_window_decoration_enabled(bool enabled)
{
	gconf_client_set_bool
		(client,
		 KEY_INTERFACE_WINDOW_DECORATION_DISABLED, !enabled, NULL);
}

static void set_window_keep_below_enabled(bool enabled)
{
	gconf_client_set_bool(client,
			      KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED,
			      enabled, NULL);
}

/*
 * Initializes the GConf client.
 */
static void init()
{
	if (!client)
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

	init();

	c = malloc(sizeof(struct config));

	c->graph_bgcolor = get_background_color();
	c->graph_fgcolor = get_foreground_color();
	c->graph_bg_alpha = get_graph_background_alpha();
	c->alpha_channel_enabled = is_alpha_channel_enabled();
	c->sensorlist_position = get_sensorlist_position();
	c->window_decoration_enabled = is_window_decoration_enabled();
	c->window_keep_below_enabled = is_window_keep_below_enabled();

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

	c->temperature_unit = gconf_client_get_int
		(client, KEY_INTERFACE_TEMPERATURE_UNIT, NULL);

	return c;
}

void config_save(const struct config *c)
{
	set_background_color(c->graph_bgcolor);
	set_foreground_color(c->graph_fgcolor);
	set_graph_background_alpha(c->graph_bg_alpha);
	set_sensorlist_position(c->sensorlist_position);
	set_window_decoration_enabled(c->window_decoration_enabled);
	set_window_keep_below_enabled(c->window_keep_below_enabled);

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

	gconf_client_set_int(client, KEY_INTERFACE_WINDOW_X, c->window_x, NULL);
	gconf_client_set_int(client, KEY_INTERFACE_WINDOW_Y, c->window_y, NULL);
	gconf_client_set_int(client, KEY_INTERFACE_WINDOW_W, c->window_w, NULL);
	gconf_client_set_int(client, KEY_INTERFACE_WINDOW_H, c->window_h, NULL);

	gconf_client_set_int(client,
			     KEY_INTERFACE_WINDOW_DIVIDER_POS,
			     c->window_divider_pos,
			     NULL);

	gconf_client_set_int(client,
			     KEY_INTERFACE_TEMPERATURE_UNIT,
			     c->temperature_unit,
			     NULL);
}
