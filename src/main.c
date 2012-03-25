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
#include "psensor.h"
#include "graph.h"
#include "ui.h"
#include "ui_sensorlist.h"
#include "ui_color.h"
#include "lmsensor.h"
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
#include "cpu.h"
#endif

#include "compat.h"

static const char *program_name;

static void print_version()
{
	printf("psensor %s\n", VERSION);
	printf(_("Copyright (C) %s jeanfi@gmail.com\n"
		 "License GPLv2: GNU GPL version 2 or later "
		 "<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n"
		 "This is free software: you are free to change and "
		 " redistribute it.\n"
		 "There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2010-2012");
}

static void print_help()
{
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	puts(_("psensor is a GTK application for monitoring hardware sensors, "
	       "including temperatures and fan speeds."));

	puts("");
	puts(_("Options:"));
	puts(_("  -h, --help          display this help and exit\n"
	       "  -v, --version       display version information and exit"));

	puts("");

	puts(_("  -u, --url=URL       "
	       "the URL of the psensor-server, example: http://hostname:3131"));

	puts("");

	puts(_("  -d, --debug=LEVEL   "
	       "set the debug level, integer between 0 and 3"));

	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

/*
  Updates the size of the sensor values if different than the
  configuration.
 */
void
update_psensor_values_size(struct psensor **sensors, struct config *cfg)
{
	struct psensor **cur;

	cur = sensors;
	while (*cur) {
		struct psensor *s = *cur;

		if (s->values_max_length != cfg->sensor_values_max_length)
			psensor_values_resize(s,
					      cfg->sensor_values_max_length);

		cur++;
	}
}

static void log_measures(struct psensor **sensors)
{
	if (log_level == LOG_DEBUG)
		while (*sensors) {
			log_debug("Measure: %s %.2f",
				   (*sensors)->name,
				   psensor_get_current_value(*sensors));

			sensors++;
		}
}

void update_psensor_measures(struct ui_psensor *ui)
{
	struct psensor **sensors = ui->sensors;
	struct config *cfg = ui->config;

	while (1) {
		g_mutex_lock(ui->sensors_mutex);

		if (!sensors)
			return;

		update_psensor_values_size(sensors, ui->config);

		psensor_list_update_measures(sensors);
#ifdef HAVE_REMOTE_SUPPORT
		remote_psensor_list_update(sensors);
#endif
#ifdef HAVE_NVIDIA
		nvidia_psensor_list_update(sensors);
#endif
#ifdef HAVE_LIBATIADL
		amd_psensor_list_update(sensors);
#endif

		log_measures(sensors);

		g_mutex_unlock(ui->sensors_mutex);

		sleep(cfg->sensor_update_interval);
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

#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
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

	g_mutex_lock(ui->sensors_mutex);

	graph_update(ui->sensors, ui->w_graph, ui->config);

	ui_sensorlist_update(ui);

	if (is_appindicator_supported() || is_status_supported())
		indicators_update(ui);

#ifdef HAVE_UNITY
	ui_unity_launcher_entry_update(ui->sensors,
				       !cfg->unity_launcher_count_disabled);
#endif

	if (ui->graph_update_interval != cfg->graph_update_interval) {
		ui->graph_update_interval = cfg->graph_update_interval;
		ret = FALSE;
	}

	g_mutex_unlock(ui->sensors_mutex);

	if (ret == FALSE)
		g_timeout_add(1000 * ui->graph_update_interval,
			      ui_refresh_thread, ui);

	return ret;
}

void cb_alarm_raised(struct psensor *sensor, void *data)
{
#ifdef HAVE_LIBNOTIFY
	if (sensor->enabled)
		ui_notify(sensor, (struct ui_psensor *)data);
#endif
}

static void associate_colors(struct psensor **sensors)
{
	/* number of uniq colors */
#define COLORS_COUNT 8

	unsigned int colors[COLORS_COUNT][3] = {
		{0x0000, 0x0000, 0x0000},	/* black */
		{0xffff, 0x0000, 0x0000},	/* red */
		{0x0000, 0.0000, 0xffff},	/* blue */
		{0x0000, 0xffff, 0x0000},	/* green */

		{0x7fff, 0x7fff, 0x7fff},	/* grey */
		{0x7fff, 0x0000, 0x0000},	/* dark red */
		{0x0000, 0x0000, 0x7fff},	/* dark blue */
		{0x0000, 0x7fff, 0x0000}	/* dark green */
	};

	struct psensor **sensor_cur = sensors;
	int i = 0;
	while (*sensor_cur) {
		struct color default_color;
		color_set(&default_color,
			  colors[i % COLORS_COUNT][0],
			  colors[i % COLORS_COUNT][1],
			  colors[i % COLORS_COUNT][2]);

		(*sensor_cur)->color
		    = config_get_sensor_color((*sensor_cur)->id,
					      &default_color);

		sensor_cur++;
		i++;
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

		if (is_temp_type(s->type)) {
			s->alarm_limit
			    = config_get_sensor_alarm_limit(s->id, 60);
			s->alarm_enabled
			    = config_get_sensor_alarm_enabled(s->id);
		} else {
			s->alarm_limit = 0;
			s->alarm_enabled = 0;
		}

		sensor_cur++;
	}
}

static void associate_preferences(struct psensor **sensors)
{
	struct psensor **sensor_cur = sensors;
	while (*sensor_cur) {
		char *n;
		struct psensor *s = *sensor_cur;

		s->enabled = config_is_sensor_enabled(s->id);

		n = config_get_sensor_name(s->id);

		if (n)
			s->name = n;

		sensor_cur++;
	}
}

static void log_init()
{
	char *home, *path, *dir;

	home = getenv("HOME");

	if (!home)
		return ;

	dir = malloc(strlen(home)+1+strlen(".psensor")+1);
	sprintf(dir, "%s/%s", home, ".psensor");
	mkdir(dir, 0777);

	path = malloc(strlen(dir)+1+strlen("log")+1);
	sprintf(path, "%s/%s", dir, "log");

	log_open(path);

	free(dir);
	free(path);
}

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"url", required_argument, 0, 'u'},
	{"debug", required_argument, 0, 'd'},
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

int main(int argc, char **argv)
{
	struct ui_psensor ui;
	GError *error;
	GThread *thread;
	int optc, cmdok;
	char *url = NULL;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	cmdok = 1;
	while ((optc = getopt_long(argc, argv, "vhd:u:", long_options,
				   NULL)) != -1) {
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
			log_printf(LOG_INFO, _("Enables debug mode."));
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

	log_glib_info();
#if !(GLIB_CHECK_VERSION(2, 31, 0))
	/*
	 * Since GLib 2.31 g_thread_init call is deprecated and not
	 * needed.
	 */
	log_debug("Calling g_thread_init(NULL)");
	g_thread_init(NULL);
#endif

	gdk_threads_init();

	gtk_init(NULL, NULL);

	ui.sensors_mutex = g_mutex_new();

	config_init();

	ui.config = config_load();

	psensor_init();

	if (url) {
#ifdef HAVE_REMOTE_SUPPORT
		rsensor_init();
		ui.sensors = get_remote_sensors(url, 600);
#else
		fprintf(stderr,
			_("ERROR: Not compiled with remote sensor support.\n"));
		exit(EXIT_FAILURE);
#endif
	} else {
		ui.sensors = get_all_sensors(600);
#ifdef HAVE_NVIDIA
		ui.sensors = nvidia_psensor_list_add(ui.sensors, 600);
#endif
#ifdef HAVE_LIBATIADL
		ui.sensors = amd_psensor_list_add(ui.sensors, 600);
#endif
#ifdef HAVE_GTOP
		ui.sensors = cpu_psensor_list_add(ui.sensors, 600);
#endif
	}

	associate_preferences(ui.sensors);
	associate_colors(ui.sensors);
	associate_cb_alarm_raised(ui.sensors, &ui);

#if !defined(HAVE_APPINDICATOR) && !defined(HAVE_APPINDICATOR_029)
	ui_status_init(&ui);
#endif

	/* main window */
	ui_window_create(&ui);
	ui.sensor_box = NULL;

	/* drawing box */
	ui.w_graph = ui_graph_create(&ui);

	/* sensor list */
	ui_sensorlist_create(&ui);

	thread = g_thread_create((GThreadFunc) update_psensor_measures,
				 &ui, TRUE, &error);

	if (!thread)
		g_error_free(error);

	ui.graph_update_interval = ui.config->graph_update_interval;

	g_timeout_add(1000 * ui.graph_update_interval, ui_refresh_thread, &ui);

#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
	ui_appindicator_init(&ui);
#endif

	/*
	 * show the window as soon as all gtk events have been processed
	 * in order to ensure that the status icon is attempted to be
	 * drawn before. If not, there is no way to detect that it is
	 * visible.
	*/
	g_idle_add((GSourceFunc)initial_window_show, &ui);

	gdk_notify_startup_complete();

	/* main loop */
	gtk_main();

	g_mutex_lock(ui.sensors_mutex);

	psensor_cleanup();

#ifdef HAVE_NVIDIA
	nvidia_cleanup();
#endif
#ifdef HAVE_LIBATIADL
	amd_cleanup();
#endif
#ifdef HAVE_REMOTE_SUPPORT
	rsensor_cleanup();
#endif

	psensor_list_free(ui.sensors);
	ui.sensors = NULL;

#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
	ui_appindicator_cleanup();
#endif

	ui_status_cleanup();

	g_mutex_unlock(ui.sensors_mutex);

	config_cleanup();

	log_close();

	return 0;
}
