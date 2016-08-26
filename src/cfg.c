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
#include <graph.h>
#include <pio.h>
#include <plog.h>

/* Properties of each sensor */
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
static const char *ATT_SENSOR_HIDE = "hide";

/* Update interval of the measures of the sensors */
static const char *KEY_SENSOR_UPDATE_INTERVAL
= "sensor-update-interval";

/* Graph settings */
static const char *KEY_GRAPH_UPDATE_INTERVAL = "graph-update-interval";
static const char *KEY_GRAPH_MONITORING_DURATION = "graph-monitoring-duration";
static const char *KEY_GRAPH_BACKGROUND_COLOR = "graph-background-color";
static const char *DEFAULT_GRAPH_BACKGROUND_COLOR = "#e8f4e8f4a8f5";
static const char *KEY_GRAPH_BACKGROUND_ALPHA = "graph-background-alpha";
static const char *KEY_GRAPH_FOREGROUND_COLOR
= "graph-foreground-color";
static const char *KEY_GRAPH_SMOOTH_CURVES_ENABLED
= "graph-smooth-curves-enabled";

static const char *DEFAULT_GRAPH_FOREGROUND_COLOR = "#000000000000";

static const char *KEY_ALPHA_CHANNEL_ENABLED = "graph-alpha-channel-enabled";

/* Inteface settings */
static const char *KEY_INTERFACE_SENSORLIST_POSITION
= "interface-sensorlist-position";

static const char *KEY_INTERFACE_WINDOW_DECORATION_DISABLED
= "interface-window-decoration-disabled";

static const char *KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED
= "interface-window-keep-below-enabled";

static const char *KEY_INTERFACE_MENU_BAR_DISABLED
= "interface-menu-bar-disabled";

static const char *KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED
= "interface-unity-launcher-count-disabled";

static const char *KEY_INTERFACE_HIDE_ON_STARTUP
= "interface-hide-on-startup";

static const char *KEY_INTERFACE_WINDOW_RESTORE_ENABLED
= "interface-window-restore-enabled";

static const char *KEY_INTERFACE_WINDOW_X = "interface-window-x";
static const char *KEY_INTERFACE_WINDOW_Y = "interface-window-y";
static const char *KEY_INTERFACE_WINDOW_W = "interface-window-w";
static const char *KEY_INTERFACE_WINDOW_H = "interface-window-h";

static const char *KEY_INTERFACE_WINDOW_DIVIDER_POS
= "interface-window-divider-pos";

static const char *KEY_INTERFACE_TEMPERATURE_UNIT
= "interface-temperature-unit";

/* Sensor logging settings */
static const char *KEY_SLOG_ENABLED = "slog-enabled";
static const char *KEY_SLOG_INTERVAL = "slog-interval";

/* Path to the script called when a notification is raised */
static const char *KEY_NOTIFICATION_SCRIPT = "notif-script";

/* Provider settings */
static const char *KEY_PROVIDER_LMSENSORS_ENABLED
= "provider-lmsensors-enabled";
static const char *KEY_PROVIDER_ATIADLSDK_ENABLED
= "provider-atiadlsdk-enabled";
static const char *KEY_PROVIDER_GTOP2_ENABLED = "provider-gtop2-enabled";
static const char *KEY_PROVIDER_HDDTEMP_ENABLED = "provider-hddtemp-enabled";
static const char *KEY_PROVIDER_LIBATASMART_ENABLED
= "provider-libatasmart-enabled";
static const char *KEY_PROVIDER_NVCTRL_ENABLED = "provider-nvctrl-enabled";
static const char *KEY_PROVIDER_UDISKS2_ENABLED = "provider-udisks2-enabled";

static const char *KEY_DEFAULT_HIGH_THRESHOLD_TEMPERATURE
= "default-high-threshold-temperature";
static const char *KEY_DEFAULT_SENSOR_ALARM_ENABLED
= "default-sensor-alarm-enabled";

static GSettings *settings;

static char *user_dir;

static GKeyFile *key_file;

static char *sensor_config_path;

static void (*slog_enabled_cbk)(void *);

static char *get_string(const char *key)
{
	return g_settings_get_string(settings, key);
}

static void set_string(const char *key, const char *str)
{
	g_settings_set_string(settings, key, str);
}

static void set_bool(const char *k, bool b)
{
	g_settings_set_boolean(settings, k, b);
}

static bool get_bool(const char *k)
{
	return g_settings_get_boolean(settings, k);
}

static void set_int(const char *k, int i)
{
	g_settings_set_int(settings, k, i);
}

static double get_double(const char *k)
{
	return g_settings_get_double(settings, k);
}

static void set_double(const char *k, double d)
{
	g_settings_set_double(settings, k, d);
}

static int get_int(const char *k)
{
	return g_settings_get_int(settings, k);
}

char *config_get_notif_script(void)
{
	char *str;

	str =  get_string(KEY_NOTIFICATION_SCRIPT);
	if (str && !strlen(str)) {
		free(str);
		str = NULL;
	}

	return str;
}

void config_set_notif_script(const char *str)
{
	if (str && strlen(str) > 0)
		set_string(KEY_NOTIFICATION_SCRIPT, str);
	else
		set_string(KEY_NOTIFICATION_SCRIPT, "");
}

static struct color *get_background_color(void)
{
	char *scolor;
	struct color *c;

	scolor = get_string(KEY_GRAPH_BACKGROUND_COLOR);

	c = str_to_color(scolor);
	free(scolor);

	if (!c)
		return color_new(1, 1, 1);

	return c;
}

static struct color *get_foreground_color(void)
{
	char *scolor;
	struct color *c;

	scolor = get_string(KEY_GRAPH_FOREGROUND_COLOR);

	c = str_to_color(scolor);
	free(scolor);

	if (!c)
		return color_new(0, 0, 0);

	return c;
}

static bool is_alpha_channel_enabled(void)
{
	return get_bool(KEY_ALPHA_CHANNEL_ENABLED);
}

static void set_alpha_channeld_enabled(bool b)
{
	set_bool(KEY_ALPHA_CHANNEL_ENABLED, b);
}

enum sensorlist_position config_get_sensorlist_position(void)
{
	return get_int(KEY_INTERFACE_SENSORLIST_POSITION);
}

void config_set_sensorlist_position(enum sensorlist_position pos)
{
	set_int(KEY_INTERFACE_SENSORLIST_POSITION, pos);
}

static double get_graph_background_alpha(void)
{
	return get_double(KEY_GRAPH_BACKGROUND_ALPHA);
}

static void set_graph_background_alpha(double alpha)
{
	set_double(KEY_GRAPH_BACKGROUND_ALPHA, alpha);
}

static void set_background_color(const struct color *color)
{
	char *scolor;

	scolor = color_to_str(color);
	if (!scolor)
		scolor = strdup(DEFAULT_GRAPH_BACKGROUND_COLOR);

	set_string(KEY_GRAPH_BACKGROUND_COLOR, scolor);

	free(scolor);
}

static void set_foreground_color(const struct color *color)
{
	char *str;

	str = color_to_str(color);
	if (!str)
		str = strdup(DEFAULT_GRAPH_FOREGROUND_COLOR);

	set_string(KEY_GRAPH_FOREGROUND_COLOR, str);

	free(str);
}

bool is_slog_enabled(void)
{
	return get_bool(KEY_SLOG_ENABLED);
}

static void set_slog_enabled(bool enabled)
{
	set_bool(KEY_SLOG_ENABLED, enabled);
}

static void slog_enabled_changed_cbk(GSettings *settings,
				     gchar *key,
				     gpointer data)
{
	if (slog_enabled_cbk)
		slog_enabled_cbk(data);
}

void config_set_slog_enabled_changed_cbk(void (*cbk)(void *), void *data)
{
	log_fct_enter();

	slog_enabled_cbk = cbk;

	g_signal_connect_after(settings,
			       "changed::slog-enabled",
			       G_CALLBACK(slog_enabled_changed_cbk),
			       data);

	log_fct_exit();
}

int config_get_slog_interval(void)
{
	return get_int(KEY_SLOG_INTERVAL);
}

static void set_slog_interval(int interval)
{
	if (interval <= 0)
		interval = 300;

	set_int(KEY_SLOG_INTERVAL, interval);
}

bool config_is_window_decoration_enabled(void)
{
	return !get_bool(KEY_INTERFACE_WINDOW_DECORATION_DISABLED);
}

bool config_is_window_keep_below_enabled(void)
{
	return get_bool(KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED);
}

void config_set_window_decoration_enabled(bool enabled)
{
	set_bool(KEY_INTERFACE_WINDOW_DECORATION_DISABLED, !enabled);
}

void config_set_window_keep_below_enabled(bool enabled)
{
	set_bool(KEY_INTERFACE_WINDOW_KEEP_BELOW_ENABLED, enabled);
}

bool config_is_smooth_curves_enabled(void)
{
	return get_bool(KEY_GRAPH_SMOOTH_CURVES_ENABLED);
}

void config_set_smooth_curves_enabled(bool b)
{
	set_bool(KEY_GRAPH_SMOOTH_CURVES_ENABLED, b);
}

double config_get_default_high_threshold_temperature(void)
{
	return get_double(KEY_DEFAULT_HIGH_THRESHOLD_TEMPERATURE);
}

static bool config_get_default_sensor_alarm_enabled(void)
{
	return get_bool(KEY_DEFAULT_SENSOR_ALARM_ENABLED);
}

static void init(void)
{
	log_fct_enter();

	if (!settings)
		settings = g_settings_new("psensor");

	log_fct_exit();
}

void config_cleanup(void)
{
	config_sync();

	if (settings) {
		g_settings_sync();
		g_object_unref(settings);
		settings = NULL;
	}

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

	slog_enabled_cbk = NULL;
}

struct config *config_load(void)
{
	struct config *c;

	init();

	c = malloc(sizeof(struct config));

	c->graph_bgcolor = get_background_color();
	c->graph_fgcolor = get_foreground_color();
	c->graph_bg_alpha = get_graph_background_alpha();
	c->alpha_channel_enabled = is_alpha_channel_enabled();
	c->slog_enabled = is_slog_enabled();
	c->slog_interval = config_get_slog_interval();

	c->sensor_update_interval
	    = get_int(KEY_SENSOR_UPDATE_INTERVAL);
	if (c->sensor_update_interval < 1)
		c->sensor_update_interval = 1;

	c->graph_update_interval = get_int(KEY_GRAPH_UPDATE_INTERVAL);
	if (c->graph_update_interval < 1)
		c->graph_update_interval = 1;

	c->graph_monitoring_duration = get_int(KEY_GRAPH_MONITORING_DURATION);

	if (c->graph_monitoring_duration < 1)
		c->graph_monitoring_duration = 10;

	c->window_restore_enabled
		= get_bool(KEY_INTERFACE_WINDOW_RESTORE_ENABLED);

	c->window_x = get_int(KEY_INTERFACE_WINDOW_X);
	c->window_y = get_int(KEY_INTERFACE_WINDOW_Y);
	c->window_w = get_int(KEY_INTERFACE_WINDOW_W);
	c->window_h = get_int(KEY_INTERFACE_WINDOW_H);

	c->window_divider_pos = get_int(KEY_INTERFACE_WINDOW_DIVIDER_POS);

	if (!c->window_restore_enabled || !c->window_w || !c->window_h) {
		c->window_w = 800;
		c->window_h = 200;
	}

	c->sensor_values_max_length = compute_values_max_length(c);

	return c;
}

void config_save(const struct config *c)
{
	set_alpha_channeld_enabled(c->alpha_channel_enabled);
	set_background_color(c->graph_bgcolor);
	set_foreground_color(c->graph_fgcolor);
	set_graph_background_alpha(c->graph_bg_alpha);
	set_slog_enabled(c->slog_enabled);
	set_slog_interval(c->slog_interval);

	set_int(KEY_GRAPH_UPDATE_INTERVAL, c->graph_update_interval);

	set_int(KEY_GRAPH_MONITORING_DURATION, c->graph_monitoring_duration);

	set_int(KEY_SENSOR_UPDATE_INTERVAL, c->sensor_update_interval);

	set_bool(KEY_INTERFACE_HIDE_ON_STARTUP, c->hide_on_startup);

	set_bool(KEY_INTERFACE_WINDOW_RESTORE_ENABLED,
		 c->window_restore_enabled);

	set_int(KEY_INTERFACE_WINDOW_X, c->window_x);
	set_int(KEY_INTERFACE_WINDOW_Y, c->window_y);
	set_int(KEY_INTERFACE_WINDOW_W, c->window_w);
	set_int(KEY_INTERFACE_WINDOW_H, c->window_h);

	set_int(KEY_INTERFACE_WINDOW_DIVIDER_POS, c->window_divider_pos);
}

const char *get_psensor_user_dir(void)
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

static const char *get_sensor_config_path(void)
{
	const char *dir;

	if (!sensor_config_path) {
		dir = get_psensor_user_dir();

		if (dir)
			sensor_config_path = path_append(dir, "psensor.cfg");
	}

	return sensor_config_path;
}

static GKeyFile *get_sensor_key_file(void)
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
			log_warn(_("Failed to load configuration file %s: %s"),
				 path,
				 err->message);
			g_error_free(err);
		}
	}

	return key_file;
}

static void save_sensor_key_file(void)
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

void config_sync(void)
{
	log_fct_enter();
	if (settings)
		g_settings_sync();
	save_sensor_key_file();
	log_fct_exit();
}

static void sensor_set_str(const char *sid, const char *att, const char *str)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	g_key_file_set_string(kfile, sid, att, str);
}

static char *sensor_get_str(const char *sid, const char *att)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	return g_key_file_get_string(kfile, sid, att, NULL);
}

static bool sensor_get_double(const char *sid, const char *att, double *d)
{
	GKeyFile *kfile;
	GError *err;
	double v;

	kfile = get_sensor_key_file();

	err = NULL;
	v = g_key_file_get_double(kfile, sid, att, &err);

	if (err) {
		log_err(err->message);

		g_error_free(err);

		return false;
	}

	*d = v;
	return true;
}

static bool sensor_get_bool(const char *sid, const char *att, bool dft)
{
	GKeyFile *kfile;
	GError *err;
	bool ret;

	kfile = get_sensor_key_file();

	err = NULL;
	ret = g_key_file_get_boolean(kfile, sid, att, &err);

	if (err) {
		if (err->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)
			ret = dft;
		else
			log_err(err->message);

		g_error_free(err);
	}

	return ret;
}

static void sensor_set_bool(const char *sid, const char *att, bool enabled)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();

	g_key_file_set_boolean(kfile, sid, att, enabled);
}

static int sensor_get_int(const char *sid, const char *att)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();
	return g_key_file_get_integer(kfile, sid, att, NULL);
}

static void sensor_set_int(const char *sid, const char *att, int i)
{
	GKeyFile *kfile;

	kfile = get_sensor_key_file();

	g_key_file_set_integer(kfile, sid, att, i);
}

char *config_get_sensor_name(const char *sid)
{
	return sensor_get_str(sid, ATT_SENSOR_NAME);
}

void config_set_sensor_name(const char *sid, const char *name)
{
	sensor_set_str(sid, ATT_SENSOR_NAME, name);
}

void config_set_sensor_color(const char *sid, const GdkRGBA *color)
{
	gchar *str;

	str = gdk_rgba_to_string(color);

	sensor_set_str(sid, ATT_SENSOR_COLOR, str);

	g_free(str);
}

static const char *next_default_color(void)
{
	/* copied from the default colors of the gtk color color
	 * chooser.
	 */
	const char *default_colors[27] = {
		"#ef2929",  /* Scarlet Red */
		"#fcaf3e",  /* Orange */
		"#fce94f",  /* Butter */
		"#8ae234",  /* Chameleon */
		"#729fcf",  /* Sky Blue */
		"#ad7fa8",  /* Plum */
		"#e9b96e",  /* Chocolate */
		"#888a85",  /* Aluminum 1 */
		"#eeeeec",  /* Aluminum 2 */
		"#cc0000",
		"#f57900",
		"#edd400",
		"#73d216",
		"#3465a4",
		"#75507b",
		"#c17d11",
		"#555753",
		"#d3d7cf",
		"#a40000",
		"#ce5c00",
		"#c4a000",
		"#4e9a06",
		"#204a87",
		"#5c3566",
		"#8f5902",
		"#2e3436",
		"#babdb6"
	};
	static int next_idx;
	const char *c;

	c = default_colors[next_idx % 27];

	next_idx++;

	return c;
}

GdkRGBA *config_get_sensor_color(const char *sid)
{
	GdkRGBA rgba;
	char *str;
	gboolean ret;

	str = sensor_get_str(sid, ATT_SENSOR_COLOR);

	if (str) {
		ret = gdk_rgba_parse(&rgba, str);
		free(str);
	}

	if (!str || !ret) {
		gdk_rgba_parse(&rgba, next_default_color());
		config_set_sensor_color(sid, &rgba);
	}

	return gdk_rgba_copy(&rgba);
}

bool config_is_sensor_graph_enabled(const char *sid)
{
	return sensor_get_bool(sid, ATT_SENSOR_GRAPH_ENABLED, false);
}

void config_set_sensor_graph_enabled(const char *sid, bool enabled)
{
	sensor_set_bool(sid, ATT_SENSOR_GRAPH_ENABLED, enabled);
}

bool config_get_sensor_alarm_high_threshold(const char *sid, double *v)
{
	return sensor_get_double(sid, ATT_SENSOR_ALARM_HIGH_THRESHOLD, v);
}

void config_set_sensor_alarm_high_threshold(const char *sid, int threshold)
{
	sensor_set_int(sid, ATT_SENSOR_ALARM_HIGH_THRESHOLD, threshold);
}

bool config_get_sensor_alarm_low_threshold(const char *sid, double *v)
{
	return sensor_get_double(sid, ATT_SENSOR_ALARM_LOW_THRESHOLD, v);
}

void config_set_sensor_alarm_low_threshold(const char *sid, int threshold)
{
	sensor_set_int(sid, ATT_SENSOR_ALARM_LOW_THRESHOLD, threshold);
}

bool config_is_appindicator_enabled(const char *sid)
{
	return !sensor_get_bool(sid,
				ATT_SENSOR_APPINDICATOR_MENU_DISABLED,
				false);
}

void config_set_appindicator_enabled(const char *sid, bool enabled)
{
	sensor_set_bool(sid,
			ATT_SENSOR_APPINDICATOR_MENU_DISABLED,
			!enabled);
}

int config_get_sensor_position(const char *sid)
{
	return sensor_get_int(sid, ATT_SENSOR_POSITION);
}

void config_set_sensor_position(const char *sid, int pos)
{
	sensor_set_int(sid, ATT_SENSOR_POSITION, pos);
}

bool config_get_sensor_alarm_enabled(const char *sid)
{
	return sensor_get_bool(sid, ATT_SENSOR_ALARM_ENABLED, false);
}

void config_set_sensor_alarm_enabled(const char *sid, bool enabled)
{
	sensor_set_bool(sid, ATT_SENSOR_ALARM_ENABLED, enabled);
}

bool config_is_sensor_enabled(const char *sid)
{
	return !sensor_get_bool(sid,
				ATT_SENSOR_HIDE,
				config_get_default_sensor_alarm_enabled());
}

void config_set_sensor_enabled(const char *sid, bool enabled)
{
	sensor_set_bool(sid, ATT_SENSOR_HIDE, !enabled);
}

bool config_is_appindicator_label_enabled(const char *sid)
{
	return sensor_get_bool(sid,
			       ATT_SENSOR_APPINDICATOR_LABEL_ENABLED,
			       false);
}

void config_set_appindicator_label_enabled(const char *sid, bool enabled)
{
	sensor_set_bool(sid, ATT_SENSOR_APPINDICATOR_LABEL_ENABLED, enabled);
}

GSettings *config_get_GSettings(void)
{
	return settings;
}

bool config_is_lmsensor_enabled(void)
{
	return get_bool(KEY_PROVIDER_LMSENSORS_ENABLED);
}

bool config_is_gtop2_enabled(void)
{
	return get_bool(KEY_PROVIDER_GTOP2_ENABLED);
}

bool config_is_udisks2_enabled(void)
{
	return get_bool(KEY_PROVIDER_UDISKS2_ENABLED);
}

bool config_is_hddtemp_enabled(void)
{
	return get_bool(KEY_PROVIDER_HDDTEMP_ENABLED);
}

bool config_is_libatasmart_enabled(void)
{
	return get_bool(KEY_PROVIDER_LIBATASMART_ENABLED);
}

bool config_is_nvctrl_enabled(void)
{
	return get_bool(KEY_PROVIDER_NVCTRL_ENABLED);
}

bool config_is_atiadlsdk_enabled(void)
{
	return get_bool(KEY_PROVIDER_ATIADLSDK_ENABLED);
}

void config_set_lmsensor_enable(bool b)
{
	set_bool(KEY_PROVIDER_LMSENSORS_ENABLED, b);
}

void config_set_nvctrl_enable(bool b)
{
	set_bool(KEY_PROVIDER_NVCTRL_ENABLED, b);
}

void config_set_atiadlsdk_enable(bool b)
{
	set_bool(KEY_PROVIDER_ATIADLSDK_ENABLED, b);
}

void config_set_gtop2_enable(bool b)
{
	set_bool(KEY_PROVIDER_GTOP2_ENABLED, b);
}

void config_set_hddtemp_enable(bool b)
{
	set_bool(KEY_PROVIDER_HDDTEMP_ENABLED, b);
}

void config_set_libatasmart_enable(bool b)
{
	set_bool(KEY_PROVIDER_LIBATASMART_ENABLED, b);
}

void config_set_udisks2_enable(bool b)
{
	set_bool(KEY_PROVIDER_UDISKS2_ENABLED, b);
}

enum temperature_unit config_get_temperature_unit(void)
{
	return get_int(KEY_INTERFACE_TEMPERATURE_UNIT);
}

void config_set_temperature_unit(enum temperature_unit u)
{
	set_int(KEY_INTERFACE_TEMPERATURE_UNIT, u);
}

bool config_is_menu_bar_enabled(void)
{
	return !get_bool(KEY_INTERFACE_MENU_BAR_DISABLED);
}

void config_set_menu_bar_enabled(bool enabled)
{
	set_bool(KEY_INTERFACE_MENU_BAR_DISABLED, !enabled);
}

bool config_is_count_visible(void)
{
	return !get_bool(KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED);
}

void config_set_count_visible(bool visible)
{
	set_bool(KEY_INTERFACE_UNITY_LAUNCHER_COUNT_DISABLED, !visible);
}
