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
#include <libintl.h>
#define _(str) gettext(str)

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <getopt.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <microhttpd.h>

#ifdef HAVE_GTOP
#include "sysinfo.h"
#include "cpu.h"
#endif

#include "log.h"
#include "psensor_json.h"
#include "url.h"
#include "server.h"
#include "slog.h"

static const char *DEFAULT_LOG_FILE = "/var/log/psensor-server.log";

#define HTML_STOP_REQUESTED \
(_("<html><body><p>Server stop requested</p></body></html>"))

static const char *program_name;

#define DEFAULT_PORT 3131

#define PAGE_NOT_FOUND (_("<html><body><p>"\
"Page not found - Go to <a href='/'>Main page</a></p></body>"))

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"port", required_argument, 0, 'p'},
	{"wdir", required_argument, 0, 'w'},
	{"debug", required_argument, 0, 'd'},
	{"log-file", required_argument, 0, 'l'},
	{"sensor-log-file", required_argument, 0, 's'},
	{0, 0, 0, 0}
};

static struct server_data server_data;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int server_stop_requested;

void print_version()
{
	printf("psensor-server %s\n", VERSION);
	printf(_("Copyright (C) %s jeanfi@gmail.com\n"
		 "License GPLv2: GNU GPL version 2 or later "
		 "<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n"
		 "This is free software: you are free to change and redistribute it.\n"
		 "There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2010-2012");
}

void print_help()
{
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	puts(_("psensor-server is an HTTP server for monitoring hardware "
	       "sensors remotely."));

	puts("");
	puts("Options:");
	puts(_("  -h, --help		display this help and exit\n"
	       "  -v, --version		display version information and exit"));

	puts("");
	puts(_("  -p,--port=PORT	webserver port\n"
	       "  -w,--wdir=DIR		directory containing webserver pages"));

	puts("");
	puts(_("  -d, --debug=LEVEL     "
	       "set the debug level, integer between 0 and 3"));
	puts(_("  -l, --log-file=PATH   set the log file to PATH"));
	puts(_("  -s, --sensor-log-file=PATH set the sensor log file to PATH"));

	puts("");
	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

/*
  Returns the file path corresponding to a given URL
*/
char *get_path(const char *url, const char *www_dir)
{
	const char *p;
	char *res;

	if (!strlen(url) || !strcmp(url, ".") || !strcmp(url, "/"))
		p = "/index.html";
	else
		p = url;

	res = malloc(strlen(www_dir)+strlen(p)+1);

	strcpy(res, www_dir);
	strcat(res, p);

	return res;
}

#if MHD_VERSION >= 0x00090200
static ssize_t
file_reader(void *cls, uint64_t pos, char *buf, size_t max)
#else
static int
file_reader(void *cls, uint64_t pos, char *buf, int max)
#endif
{
	FILE *file = cls;

	fseek(file, pos, SEEK_SET);
	return fread(buf, 1, max, file);
}

struct MHD_Response *
create_response_api(const char *nurl,
		    const char *method,
		    unsigned int *rp_code)
{
	struct MHD_Response *resp;
	struct psensor *s;
	char *page = NULL;

	if (!strcmp(nurl, URL_BASE_API_1_0_SENSORS))  {
		page = sensors_to_json_string(server_data.sensors);
#ifdef HAVE_GTOP
	} else if (!strcmp(nurl, URL_API_1_0_SYSINFO)) {
		page = sysinfo_to_json_string(&server_data.psysinfo);
	} else if (!strcmp(nurl, URL_API_1_0_CPU_USAGE)) {
		page = sensor_to_json_string(server_data.cpu_usage);
#endif
	} else if (!strncmp(nurl, URL_BASE_API_1_0_SENSORS,
			    strlen(URL_BASE_API_1_0_SENSORS))
		   && nurl[strlen(URL_BASE_API_1_0_SENSORS)] == '/') {

		const char *sid = nurl + strlen(URL_BASE_API_1_0_SENSORS) + 1;

		s = psensor_list_get_by_id(server_data.sensors, sid);

		if (s)
			page = sensor_to_json_string(s);

	} else if (!strcmp(nurl, URL_API_1_0_SERVER_STOP)) {

		server_stop_requested = 1;
		page = strdup(HTML_STOP_REQUESTED);
	}

	if (page) {
		*rp_code = MHD_HTTP_OK;

		resp = MHD_create_response_from_data(strlen(page), page,
						     MHD_YES, MHD_NO);

		MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE,
					"application/json");

		return resp;
	}

	return NULL;
}

struct MHD_Response *
create_response_file(const char *nurl,
		     const char *method,
		     unsigned int *rp_code,
		     const char *fpath)
{
	struct stat st;
	int ret;
	FILE *file;

	ret = stat(fpath, &st);

	if (!ret && (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode))) {
		file = fopen(fpath, "rb");

		if (file) {
			*rp_code = MHD_HTTP_OK;

			if (!st.st_size) {
				fclose(file);
				return MHD_create_response_from_data
					(0, NULL, MHD_NO, MHD_NO);
			}

			return MHD_create_response_from_callback
				(st.st_size,
				 32 * 1024,
				 &file_reader,
				 file,
				 (MHD_ContentReaderFreeCallback)&fclose);

		} else {
			log_err("Failed to open: %s.", fpath);
		}
	}

	return NULL;
}

struct MHD_Response *
create_response(const char *nurl, const char *method, unsigned int *rp_code)
{
	struct MHD_Response *resp = NULL;

	if (!strncmp(nurl, URL_BASE_API_1_0, strlen(URL_BASE_API_1_0))) {
		resp = create_response_api(nurl, method, rp_code);
	} else {
		char *fpath = get_path(nurl, server_data.www_dir);

		resp = create_response_file(nurl, method, rp_code, fpath);

		free(fpath);
	}

	if (resp) {
		return resp;
	} else {
		char *page = strdup(PAGE_NOT_FOUND);
		*rp_code = MHD_HTTP_NOT_FOUND;

		return MHD_create_response_from_data
			(strlen(page), page, MHD_YES, MHD_NO);
	}
}

static int
cbk_http_request(void *cls,
		 struct MHD_Connection *connection,
		 const char *url,
		 const char *method,
		 const char *version,
		 const char *upload_data,
		 size_t *upload_data_size, void **ptr)
{
	static int dummy;
	struct MHD_Response *response;
	int ret;
	char *nurl;
	unsigned int resp_code;

	if (strcmp(method, "GET"))
		return MHD_NO;

	if (&dummy != *ptr) {
		/* The first time only the headers are valid, do not
		   respond in the first round... */
		*ptr = &dummy;
		return MHD_YES;
	}

	if (*upload_data_size)
		return MHD_NO;

	*ptr = NULL;		/* clear context pointer */

	log_debug(_("HTTP Request: %s"), url);

	nurl = url_normalize(url);

	pthread_mutex_lock(&mutex);
	response = create_response(nurl, method, &resp_code);
	pthread_mutex_unlock(&mutex);

	ret = MHD_queue_response(connection, resp_code, response);
	MHD_destroy_response(response);

	free(nurl);

	return ret;
}

int main(int argc, char *argv[])
{
	struct MHD_Daemon *d;
	int port = DEFAULT_PORT;
	int optc;
	int cmdok = 1;
	char *log_file, *slog_file;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	server_data.www_dir = NULL;
	server_data.psysinfo.interfaces = NULL;
	log_file = NULL;
	slog_file = NULL;

	while ((optc = getopt_long(argc, argv,
				   "vhp:w:d:l:s:", long_options, NULL)) != -1) {
		switch (optc) {
		case 'w':
			if (optarg)
				server_data.www_dir = strdup(optarg);
			break;
		case 'p':
			if (optarg)
				port = atoi(optarg);
			break;
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'd':
			log_level = atoi(optarg);
			log_info(_("Enables debug mode: %d"), log_level);
			break;
		case 'l':
			if (optarg)
				log_file = strdup(optarg);
			break;
		case 's':
			if (optarg)
				slog_file = strdup(optarg);
			break;
		default:
			cmdok = 0;
			break;
		}
	}

	if (!server_data.www_dir)
		server_data.www_dir = strdup(DEFAULT_WWW_DIR);

	if (!log_file)
		log_file = strdup(DEFAULT_LOG_FILE);

	if (!cmdok || optind != argc) {
		fprintf(stderr, _("Try `%s --help' for more information.\n"),
			program_name);
		exit(EXIT_FAILURE);
	}

	log_open(log_file);

	psensor_init();

	server_data.sensors = get_all_sensors(0, 600);

#ifdef HAVE_GTOP
	server_data.cpu_usage = create_cpu_usage_sensor(600);
#endif

	if (!*server_data.sensors)
		log_err(_("No sensors detected."));

	d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
			     port,
			     NULL, NULL, &cbk_http_request, server_data.sensors,
			     MHD_OPTION_END);
	if (!d) {
		log_err(_("Failed to create Web server."));
		exit(EXIT_FAILURE);
	}

	log_info(_("Web server started on port: %d"), port);
	log_info(_("WWW directory: %s"), server_data.www_dir);
	log_info(_("URL: http://localhost:%d"), port);

	if (slog_file)
		slog_init(slog_file, server_data.sensors);

	while (!server_stop_requested) {
		pthread_mutex_lock(&mutex);

#ifdef HAVE_GTOP
		sysinfo_update(&server_data.psysinfo);
		cpu_usage_sensor_update(server_data.cpu_usage);
#endif
		psensor_list_update_measures(server_data.sensors);

		psensor_log_measures(server_data.sensors);

		slog_write_sensors(server_data.sensors);

		pthread_mutex_unlock(&mutex);
		sleep(5);
	}

	slog_close();

	MHD_stop_daemon(d);

	/* sanity cleanup for valgrind */
	psensor_list_free(server_data.sensors);
#ifdef HAVE_GTOP
	psensor_free(server_data.cpu_usage);
#endif
	free(server_data.www_dir);
	sensors_cleanup();

#ifdef HAVE_GTOP
	sysinfo_cleanup();
	cpu_cleanup();
#endif

	if (log_file != DEFAULT_LOG_FILE)
		free(log_file);

	return EXIT_SUCCESS;
}
