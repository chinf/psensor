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
#include <locale.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gtk/gtk.h>

#include "config.h"

#include "cfg.h"
#include <hdd.h>
#include "psensor.h"
#include "graph.h"
#include "ui.h"
#include "ui_sensorlist.h"
#include "ui_color.h"
#include "lmsensor.h"
#include "notify_cmd.h"
#include <pmutex.h>
#include <pudisks2.h>
#include "slog.h"
#include "ui_pref.h"
#include "ui_graph.h"
#include "ui_status.h"

#ifdef HAVE_UNITY
#include "ui_unity.h"
#endif

#ifdef HAVE_NVIDIA
#include "nvidia.h"
#endif

#ifdef HAVE_LIBATIADL
#include "amd.h"
#endif

#ifdef HAVE_REMOTE_SUPPORT
#include "rsensor.h"
#endif

#include "ui_appindicator.h"

#ifdef HAVE_LIBNOTIFY
#include "ui_notify.h"
#endif

#ifdef HAVE_GTOP
#include <pgtop2.h>
#endif

static const char *program_name;

static void print_version()
{
	printf("psensor %s\n", VERSION);
	printf(_("Copyright (C) %s jeanfi@gmail.com\n"
		 "License GPLv2: GNU GPL version 2 or later "
		 "<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n"
		 "This is free software: you are free to change and"
		 " redistribute it.\n"
		 "There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2010-2014");
}

static void print_help()
{
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	puts(_("Psensor is a GTK+ application for monitoring hardware sensors, "
	       "including temperatures and fan speeds."));

	puts("");
	puts(_("Options:"));
	puts(_("  -h, --help          display this help and exit\n"
	       "  -v, --version       display version information and exit"));

	puts("");

	puts(_(
"  -u, --url=URL       the URL of the psensor-server,\n"
"                      example: http://hostname:3131"));
	puts(_(
"  -n, --new-instance  force the creation of a new Psensor application"));
	puts("");

	puts(_("  -d, --debug=LEVEL   "
	       "set the debug level, integer between 0 and 3"));

	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

/*
 * Updates the size of the sensor values if different than the
 * configuration.
 */
static void
update_psensor_values_size(struct psensor **sensors, struct config *cfg)
{
	struct psensor **cur, *s;

	for (cur = sensors; *cur; cur++) {
		s = *cur;

		if (s->values_max_length != cfg->sensor_values_max_length)
			psensor_values_resize(s,
					      cfg->sensor_values_max_length);
	}
}

static void *update_measures(void *data)
{
	struct psensor **sensors;
	struct config *cfg;
	int period;
	struct ui_psensor *ui;

	ui = (struct ui_psensor *)data;
	cfg = ui->config;

	while (1) {
		pmutex_lock(&ui->sensors_mutex);

		sensors = ui->sensors;
		if (!sensors)
			pthread_exit(NULL);

		update_psensor_values_size(sensors, cfg);

		lmsensor_psensor_list_update(sensors);
#ifdef HAVE_REMOTE_SUPPORT
		remote_psensor_list_update(sensors);
#endif
#ifdef HAVE_NVIDIA
		nvidia_psensor_list_update(sensors);
#endif
#ifdef HAVE_LIBATIADL
		amd_psensor_list_update(sensors);
#endif
#ifdef HAVE_LIBUDISKS2
		udisks2_psensor_list_update(sensors);
#endif
#ifdef HAVE_GTOP
		gtop2_psensor_list_update(sensors);
#endif
#ifdef HAVE_ATASMART
		atasmart_psensor_list_update(sensors);
#endif

		hddtemp_psensor_list_update(sensors);

		psensor_log_measures(sensors);

		period = cfg->sensor_update_interval;

		pmutex_unlock(&ui->sensors_mutex);

		sleep(period);
	}
}

static void indicators_update(struct ui_psensor *ui)
{
	struct psensor **sensor_cur = ui->sensors;
	unsigned int attention = 0;

	while (*sensor_cur) {
		struct psensor *s = *sensor_cur;

		if (s->alarm_enabled && s->alarm_raised) {
			attention = 1;
			break;
		}

		sensor_cur++;
	}

#if defined(HAVE_APPINDICATOR)
	if (is_appindicator_supported())
		ui_appindicator_update(ui, attention);
#endif

	if (is_status_supported())
		ui_status_update(ui, attention);
}

gboolean ui_refresh_thread(gpointer data)
{
	struct config *cfg;
	gboolean ret;
	struct ui_psensor *ui = (struct ui_psensor *)data;

	ret = TRUE;
	cfg = ui->config;

	pmutex_lock(&ui->sensors_mutex);

	graph_update(ui->sensors, ui->w_graph, ui->config, ui->main_window);

	ui_sensorlist_update(ui, 0);

	if (is_appindicator_supported() || is_status_supported())
		indicators_update(ui);

#ifdef HAVE_UNITY
	ui_unity_launcher_entry_update(ui->sensors,
				       !cfg->unity_launcher_count_disabled,
				       cfg->temperature_unit == CELSIUS);
#endif

	if (ui->graph_update_interval != cfg->graph_update_interval) {
		ui->graph_update_interval = cfg->graph_update_interval;
		ret = FALSE;
	}

	pmutex_unlock(&ui->sensors_mutex);

	if (ret == FALSE)
		g_timeout_add(1000 * ui->graph_update_interval,
			      ui_refresh_thread, ui);

	return ret;
}

static void cb_alarm_raised(struct psensor *sensor, void *data)
{
#ifdef HAVE_LIBNOTIFY
	if (sensor->alarm_enabled)
		ui_notify(sensor, (struct ui_psensor *)data);
#endif

	notify_cmd(sensor);
}

static void associate_colors(struct psensor **sensors)
{
	GdkRGBA rgba;
	/* copied from the default colors of the gtk color color
	 * chooser. */
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
	int i;
	struct psensor **cur;
	struct color c;

	for (cur = sensors, i = 0; *cur; cur++, i++) {
		gdk_rgba_parse(&rgba, default_colors[i % 27]);
		c.red = rgba.red;
		c.green = rgba.green;
		c.blue = rgba.blue;

		(*cur)->color = config_get_sensor_color((*cur)->id, &c);
	}
}

static void
associate_cb_alarm_raised(struct psensor **sensors, struct ui_psensor *ui)
{
	struct psensor **sensor_cur = sensors;

	while (*sensor_cur) {
		struct psensor *s = *sensor_cur;

		s->cb_alarm_raised = cb_alarm_raised;
		s->cb_alarm_raised_data = ui;

		s->alarm_high_threshold
			= config_get_sensor_alarm_high_threshold(s->id);
		s->alarm_low_threshold
			= config_get_sensor_alarm_low_threshold(s->id);

		s->alarm_enabled
			    = config_get_sensor_alarm_enabled(s->id);

		sensor_cur++;
	}
}

static void associate_preferences(struct psensor **sensors)
{
	struct psensor **sensor_cur = sensors;

	while (*sensor_cur) {
		char *n;
		struct psensor *s = *sensor_cur;

		s->graph_enabled = config_is_sensor_graph_enabled(s->id);

		n = config_get_sensor_name(s->id);

		if (n) {
			free(s->name);
			s->name = n;
		}

		s->appindicator_enabled = config_is_appindicator_enabled(s->id);

		sensor_cur++;
	}
}

static void log_init()
{
	const char *dir;
	char *path;

	dir = get_psensor_user_dir();

	if (!dir)
		return;

	path = malloc(strlen(dir)+1+strlen("log")+1);
	sprintf(path, "%s/%s", dir, "log");

	log_open(path);

	free(path);
}

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"url", required_argument, 0, 'u'},
	{"debug", required_argument, 0, 'd'},
	{"new-instance", no_argument, 0, 'n'},
	{0, 0, 0, 0}
};

static gboolean initial_window_show(gpointer data)
{
	struct ui_psensor *ui;

	log_debug("initial_window_show()");

	ui = (struct ui_psensor *)data;

	log_debug("is_status_supported: %d", is_status_supported());
	log_debug("is_appindicator_supported: %d",
		   is_appindicator_supported());
	log_debug("hide_on_startup: %d", ui->config->hide_on_startup);

	if (!ui->config->hide_on_startup
	    || (!is_appindicator_supported() && !is_status_supported()))
		ui_window_show(ui);

	ui_window_update(ui);

	return FALSE;
}

static void log_glib_info()
{
	log_debug("Compiled with GLib %d.%d.%d",
		  GLIB_MAJOR_VERSION,
		  GLIB_MINOR_VERSION,
		  GLIB_MICRO_VERSION);

	log_debug("Running with GLib %d.%d.%d",
		  glib_major_version,
		  glib_minor_version,
		  glib_micro_version);
}

static void cb_activate(GApplication *application,
			gpointer data)
{
	ui_window_show((struct ui_psensor *)data);
}

/*
 * Release memory for Valgrind.
 */
static void cleanup(struct ui_psensor *ui)
{
	pmutex_lock(&ui->sensors_mutex);

	log_debug("Cleanup...");

#ifdef HAVE_NVIDIA
	nvidia_cleanup();
#endif
#ifdef HAVE_LIBATIADL
	amd_cleanup();
#endif
#ifdef HAVE_REMOTE_SUPPORT
	rsensor_cleanup();
#endif

	psensor_list_free(ui->sensors);
	ui->sensors = NULL;

#if defined(HAVE_APPINDICATOR)
	ui_appindicator_cleanup();
#endif

	ui_status_cleanup();

	pmutex_unlock(&ui->sensors_mutex);

	config_cleanup();

	log_debug("Cleanup done, closing log");
}

/*
 * Creates the list of sensors.
 *
 * 'url': remote psensor server url, null for local monitoring.
 */
static struct psensor **create_sensors_list(const char *url)
{
	struct psensor **sensors;

	if (url) {
#ifdef HAVE_REMOTE_SUPPORT
		rsensor_init();
		sensors = get_remote_sensors(url, 600);
#else
		log_err(_("Psensor has not been compiled with remote "
			  "sensor support."));
		exit(EXIT_FAILURE);
#endif
	} else {
		sensors = malloc(sizeof(struct psensor *));
		*sensors = NULL;

		if (config_is_lmsensor_enabled())
			lmsensor_psensor_list_append(&sensors, 600);

		if (config_is_hddtemp_enabled())
			hddtemp_psensor_list_append(&sensors, 600);

#ifdef HAVE_ATASMART
		if (config_is_libatasmart_enabled())
			atasmart_psensor_list_append(&sensors, 600);
#endif

#ifdef HAVE_NVIDIA
		if (config_is_nvctrl_enabled())
			nvidia_psensor_list_append(&sensors, 600);
#endif
#ifdef HAVE_LIBATIADL
		if (config_is_atiadlsdk_enabled())
			amd_psensor_list_append(&sensors, 600);
#endif
#ifdef HAVE_GTOP
		if (config_is_gtop2_enabled())
			gtop2_psensor_list_append(&sensors, 600);
#endif
#ifdef HAVE_LIBUDISKS2
		if (config_is_udisks2_enabled())
			udisks2_psensor_list_append(&sensors, 600);
#endif
	}

	associate_preferences(sensors);
	associate_colors(sensors);

	return sensors;
}

int main(int argc, char **argv)
{
	struct ui_psensor ui;
	pthread_t thread;
	int optc, cmdok, opti, new_instance, ret;
	char *url = NULL;
	GApplication *app;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	new_instance = 0;

	cmdok = 1;
	while ((optc = getopt_long(argc, argv, "vhd:u:n", long_options,
				   &opti)) != -1) {
		switch (optc) {
		case 'u':
			if (optarg)
				url = strdup(optarg);
			break;
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'd':
			log_level = atoi(optarg);
			log_info(_("Enables debug mode."));
			break;
		case 'n':
			new_instance = 1;
			break;
		default:
			cmdok = 0;
			break;
		}
	}

	if (!cmdok || optind != argc) {
		fprintf(stderr, _("Try `%s --help' for more information.\n"),
			program_name);
		exit(EXIT_FAILURE);
	}

	log_init();

	app = g_application_new("wpitchoune.psensor", 0);

	g_application_register(app, NULL, NULL);

	if (!new_instance && g_application_get_is_remote(app)) {
		g_application_activate(app);
		log_warn(_("A Psensor instance already exists."));
		exit(EXIT_SUCCESS);
	}

	g_signal_connect(app, "activate", G_CALLBACK(cb_activate), &ui);

	log_glib_info();
#if !(GLIB_CHECK_VERSION(2, 31, 0))
	/*
	 * Since GLib 2.31 g_thread_init call is deprecated and not
	 * needed.
	 */
	log_debug("Calling g_thread_init(NULL)");
	g_thread_init(NULL);
#endif

	gtk_init(NULL, NULL);

	pmutex_init(&ui.sensors_mutex);

	ui.config = config_load();

	ui.sensors = create_sensors_list(url);
	associate_cb_alarm_raised(ui.sensors, &ui);

	if (ui.config->slog_enabled)
		slog_activate(NULL,
			      ui.sensors,
			      &ui.sensors_mutex,
			      config_get_slog_interval());

#if !defined(HAVE_APPINDICATOR)
	ui_status_init(&ui);
	ui_status_set_visible(1);
#endif

	/* main window */
	ui_window_create(&ui);

	ui_enable_alpha_channel(&ui);

	ret = pthread_create(&thread, NULL, update_measures, &ui);

	if (ret)
		log_err(_("Failed to create thread for monitoring sensors"));

	ui.graph_update_interval = ui.config->graph_update_interval;

	g_timeout_add(1000 * ui.graph_update_interval, ui_refresh_thread, &ui);

#if defined(HAVE_APPINDICATOR)
	ui_appindicator_init(&ui);
#endif

	gdk_notify_startup_complete();

	/*
	 * hack, did not find a cleaner solution.
	 * wait 30s to ensure that the status icon is attempted to be
	 * drawn before determining whether the main window must be
	 * show.
	 */
	if  (ui.config->hide_on_startup)
		g_timeout_add(30000, (GSourceFunc)initial_window_show, &ui);
	else
		initial_window_show(&ui);

	log_debug("translators: %s\n", _("translator-credits"));

	/* main loop */
	gtk_main();

	g_object_unref(app);
	cleanup(&ui);

	log_debug("Quitting...");
	log_close();

	if (url)
		free(url);

	return 0;
}
