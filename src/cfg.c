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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <cfg.h>
#include <pio.h>
#include <plog.h>

static const char *ATT_SENSOR_ALARM_ENABLED = "alarm_enabled";
static const char *ATT_SENSOR_ALARM_HIGH_THRESHOLD = "alarm_high_threshold";
static const char *ATT_SENSOR_ALARM_LOW_THRESHOLD = "alarm_low_threshold";
static const char *ATT_SENSOR_COLOR = "color";
static const char *ATT_SENSOR_GRAPH_ENABLED = "graph_enabled";
static const char *ATT_SENSOR_NAME = "name";
static const char *ATT_SENSOR_APPINDICATOR_MENU_DISABLED
= "appindicator_menu_disabled";
static const char *ATT_SENSOR_APPINDICATOR_LABEL_ENABLED
= "appindicator_label_enabled";

static const char *ATT_SENSOR_POSITION = "position";

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

static const char *KEY_SLOG_ENABLED = "/apps/psensor/slog/enabled";
static const char *KEY_SLOG_INTERVAL = "/apps/psensor/slog/interval";

static const char *KEY_NOTIFICATION_SCRIPT = "/apps/psensor/notif_script";

static GConfClient *client;

static char *user_dir;

static GKeyFile *key_file;

static char *sensor_config_path;

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

char *config_get_notif_script()
{
	char *str;

	str =  gconf_client_get_string(client, KEY_NOTIFICATION_SCRIPT, NULL);
	if (str && !strlen(str)) {
		free(str);
		str = NULL;
	}

	return str;
}

void config_set_notif_script(const char *str)
{
	if (str && strlen(str) > 0)
		gconf_client_set_string(client,
					KEY_NOTIFICATION_SCRIPT, str, NULL);
	else
		gconf_client_set_string(client,
					KEY_NOTIFICATION_SCRIPT, "", NULL);
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

bool is_slog_enabled()
{
	return gconf_client_get_bool(client, KEY_SLOG_ENABLED, NULL);
}

static void set_slog_enabled(bool enabled)
{
	gconf_client_set_bool(client, KEY_SLOG_ENABLED, enabled, NULL);
}

void config_slog_enabled_notify_add(GConfClientNotifyFunc cbk, void *data)
{
	log_debug("config_slog_enabled_notify_add");
	gconf_client_add_dir(client,
			     KEY_SLOG_ENABLED,
			     GCONF_CLIENT_PRELOAD_NONE,
			     NULL);
	gconf_client_notify_add(client,
				KEY_SLOG_ENABLED,
				cbk,
				data,
				NULL,
				NULL);
}

int config_get_slog_interval()
{
	int res;

	res = gconf_client_get_int(client, KEY_SLOG_INTERVAL, NULL);

	if (res <= 0)
		return 300;
	else
		return res;
}

static void set_slog_interval(int interval)
{
	if (interval <= 0)
		interval = 300;

	gconf_client_set_int(client, KEY_SLOG_INTERVAL, interval, NULL);
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

	config_sync();

	if (user_dir) {
		free(user_dir);
		user_dir = NULL;
	}

	if (key_file) {
		g_key_file_free(key_file);
		key_file = NULL;
	}

	if (sensor_config_path) {
		free(sensor_config_path);
		sensor_config_path = NULL;
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
	c->slog_enabled = is_slog_enabled();
	c->slog_interval = config_get_slog_interval();

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
	set_slog_enabled(c->slog_enabled);
	set_slog_interval(c->slog_interval);

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

const char *get_psensor_user_dir()
{
	const char *home;

	log_fct_enter();

	if (!user_dir) {
		home = getenv("HOME");

		if (!home)
			return NULL;

		user_dir = path_append(home, ".psensor");

		if (mkdir(user_dir, 0700) == -1 && errno != EEXIST) {
			log_err(_("Failed to create the directory %s: %s"),
				user_dir,
				strerror(errno));

			free(user_dir);
			user_dir = NULL;
		}
	}

	log_fct_exit();

	return user_dir;
}

static const char *get_sensor_config_path()
{
	const char *dir;

	if (!sensor_config_path) {
		dir = get_psensor_user_dir();

		if (dir)
			sensor_config_path = path_append(dir, "psensor.cfg");
	}

	return sensor_config_path;
}

static GKeyFile *get_sensor_key_file()
{
	int ret;
	GError *err;
	const char *path;

	if (!key_file) {
		path = get_sensor_config_path();

		key_file = g_key_file_new();

		err = NULL;
		ret = g_key_file_load_from_file(key_file,
						path,
						G_KEY_FILE_KEEP_COMMENTS
						| G_KEY_FILE_KEEP_TRANSLATIONS,
						&err);

		if (!ret) {
			if (err->code == G_KEY_FILE_ERROR_NOT_FOUND) {
				log_fct(_("The configuration file "
					  "does not exist."));
			} else {
				log_err(_("Failed to parse configuration "
					  "file: %s"),
					path);
			}
		}
	}

	return key_file;
}

static void save_sensor_key_file()
{
	GKeyFile *kfile;
	const char *path;
	char *data;

	log_fct_enter();

	kfile = get_sensor_key_file();

	data = g_key_file_to_data(kfile, NULL, NULL);

	path = get_sensor_config_path();

	if (!g_file_set_contents(path, data, -1, NULL))
		log_err(_("Failed to save configuration file %s."), path);

	free(data);

	log_fct_exit();
}

void config_sync()
{
	log_fct_enter();
	save_sensor_key_file();
	log_fct_exit();
}

static void
config_sensor_set_string(const char *sid, const char *att, const char *str)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	g_key_file_set_string(kfile, sid, att, str);
}

static char *config_sensor_get_string(const char *sid, const char *att)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	return g_key_file_get_string(kfile, sid, att, NULL);
}

static bool config_sensor_get_bool(const char *sid, const char *att)
{

	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	return g_key_file_get_boolean(kfile, sid, att, NULL);
}

static void
config_sensor_set_bool(const char *sid, const char *att, bool enabled)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();

	g_key_file_set_boolean(kfile, sid, att, enabled);
}

static int config_sensor_get_int(const char *sid, const char *att)
{

	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	return g_key_file_get_integer(kfile, sid, att, NULL);
}

static void
config_sensor_set_int(const char *sid, const char *att, int i)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();

	g_key_file_set_integer(kfile, sid, att, i);
}

char *config_get_sensor_name(const char *sid)
{
	return config_sensor_get_string(sid, ATT_SENSOR_NAME);
}

void config_set_sensor_name(const char *sid, const char *name)
{
	config_sensor_set_string(sid, ATT_SENSOR_NAME, name);
}

void config_set_sensor_color(const char *sid, const struct color *color)
{
	char *scolor;

	scolor = color_to_str(color);

	config_sensor_set_string(sid, ATT_SENSOR_COLOR, scolor);

	free(scolor);
}

struct color *
config_get_sensor_color(const char *sid, const struct color *dft)
{
	char *scolor;
	struct color *color;

	scolor = config_sensor_get_string(sid, ATT_SENSOR_COLOR);

	if (scolor)
		color = str_to_color(scolor);
	else
		color = NULL;

	if (!color) {
		color = color_new(dft->red, dft->green, dft->blue);
		config_set_sensor_color(sid, color);
	}

	free(scolor);

	return color;
}

bool config_is_sensor_enabled(const char *sid)
{
	return config_sensor_get_bool(sid, ATT_SENSOR_GRAPH_ENABLED);
}

void config_set_sensor_enabled(const char *sid, bool enabled)
{
	config_sensor_set_bool(sid, ATT_SENSOR_GRAPH_ENABLED, enabled);
}

int config_get_sensor_alarm_high_threshold(const char *sid)
{
	return config_sensor_get_int(sid, ATT_SENSOR_ALARM_HIGH_THRESHOLD);
}

void config_set_sensor_alarm_high_threshold(const char *sid, int threshold)
{
	config_sensor_set_int(sid, ATT_SENSOR_ALARM_HIGH_THRESHOLD, threshold);
}

int config_get_sensor_alarm_low_threshold(const char *sid)
{
	return config_sensor_get_int(sid, ATT_SENSOR_ALARM_LOW_THRESHOLD);
}

void config_set_sensor_alarm_low_threshold(const char *sid, int threshold)
{
	config_sensor_set_int(sid, ATT_SENSOR_ALARM_LOW_THRESHOLD, threshold);
}

bool config_is_appindicator_enabled(const char *sid)
{
	return !config_sensor_get_bool(sid,
				       ATT_SENSOR_APPINDICATOR_MENU_DISABLED);
}

void config_set_appindicator_enabled(const char *sid, bool enabled)
{
	return config_sensor_set_bool(sid,
				      ATT_SENSOR_APPINDICATOR_MENU_DISABLED,
				      !enabled);
}

int config_get_sensor_position(const char *sid)
{
	return config_sensor_get_int(sid, ATT_SENSOR_POSITION);
}

void config_set_sensor_position(const char *sid, int pos)
{
	return config_sensor_set_int(sid, ATT_SENSOR_POSITION, pos);
}

bool config_get_sensor_alarm_enabled(const char *sid)
{
	return config_sensor_get_bool(sid, ATT_SENSOR_ALARM_ENABLED);
}

void config_set_sensor_alarm_enabled(const char *sid, bool enabled)
{
	config_sensor_set_bool(sid, ATT_SENSOR_ALARM_ENABLED, enabled);
}

bool config_is_appindicator_label_enabled(const char *sid)
{
	return config_sensor_get_bool(sid,
				      ATT_SENSOR_APPINDICATOR_LABEL_ENABLED);
}

void config_set_appindicator_label_enabled(const char *sid, bool enabled)
{
	config_sensor_set_bool(sid,
			       ATT_SENSOR_APPINDICATOR_LABEL_ENABLED,
			       enabled);
}
