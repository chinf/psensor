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
#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <getopt.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <microhttpd.h>
#include <json/json.h>

#ifdef HAVE_GTOP
#include <glibtop/cpu.h>
#endif

#define lua_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "plib/url.h"
#include "httpd.h"
#include "psensor.h"
#include "hdd.h"
#include "nvidia.h"
#include "lmsensor.h"

#define DEFAULT_PORT 3131


#define PAGE_NOT_FOUND \
"<html><body><p>Page not found - Go to <a href='/index.lua'>Main page</a>\
</p></body>"


#define PSENSOR_SERVER_OPTIONS "  --help    display this help and exit\n\
  --version   output version information and exit\n\
  --port=PORT webserver port\n\
  --wdir=DIR  directory containing webserver pages"

#define PSENSOR_SERVER_USAGE "Usage: psensor-server [OPTION]"

#define PSENSOR_SERVER_DESCRIPTION \
"HTTP server for retrieving hardware temperatures."

#define PSENSOR_SERVER_CONTACT "Report bugs to: psensor@wpitchoune.net\n\
Psensor home page: <http://wpitchoune.net/psensor/>"

#define PSENSOR_SERVER_COPYRIGHT \
"Copyright (C) 2010-2011 wpitchoune@gmail.com\n\
License GPLv2: GNU GPL version 2 or later \
<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law."

#define OPT_PORT "port"
#define OPT_HELP "help"
#define OPT_VERSION "version"
#define OPT_WWW_DIR "wdir"

static struct option long_options[] = {
	{OPT_VERSION, no_argument, 0, 'v'},
	{OPT_HELP, no_argument, 0, 'h'},
	{OPT_PORT, required_argument, 0, 'p'},
	{OPT_WWW_DIR, required_argument, 0, 'w'},
	{0, 0, 0, 0}
};

static char *www_dir = DEFAULT_WWW_DIR;

#ifdef HAVE_GTOP
static float cpu_rate;
static glibtop_cpu *cpu;
#endif

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

json_object *sensor_to_json_object(struct psensor *s)
{
	json_object *mo;
	json_object *obj = json_object_new_object();
	struct measure *m;

	json_object_object_add(obj, "id", json_object_new_string(s->id));
	json_object_object_add(obj, "name", json_object_new_string(s->name));
	json_object_object_add(obj, "type", json_object_new_int(s->type));
	json_object_object_add(obj, "min", json_object_new_double(s->min));
	json_object_object_add(obj, "max", json_object_new_double(s->max));

	m = psensor_get_current_measure(s);
	mo = json_object_new_object();
	json_object_object_add(mo, "value", json_object_new_double(m->value));
	json_object_object_add(mo, "time",
			       json_object_new_int((m->time).tv_sec));
	json_object_object_add(obj, "last_measure", mo);

	return obj;
}

char *sensor_to_json_string(struct psensor *s)
{
	char *str;
	json_object *obj = sensor_to_json_object(s);

	str = strdup(json_object_to_json_string(obj));

	json_object_put(obj);

	return str;
}

char *sensors_to_json_string(struct psensor **sensors)
{
	struct psensor **sensors_cur;
	char *str;

	json_object *obj = json_object_new_array();

	sensors_cur = sensors;

	while (*sensors_cur) {
		struct psensor *s = *sensors_cur;

		json_object_array_add(obj, sensor_to_json_object(s));

		sensors_cur++;
	}

	str = strdup(json_object_to_json_string(obj));

	json_object_put(obj);

	return str;
}

char *lua_to_html_page(struct psensor **sensors, const char *url)
{
	struct psensor **s_cur;
	char *page = NULL;
	int i;
	char *lua_fpath;

	lua_State *L = lua_open();	/* create state */

	if (!L)
		return NULL;

	luaL_openlibs(L);

#ifdef HAVE_GTOP
	lua_newtable(L);
	lua_pushstring(L, "load");
	lua_pushnumber(L, cpu_rate);
	lua_settable(L, -3);
	lua_setglobal(L, "cpu");
#endif

	lua_newtable(L);

	s_cur = sensors;
	i = 0;
	while (*s_cur) {

		lua_pushinteger(L, i);

		lua_newtable(L);

		lua_pushstring(L, "name");
		lua_pushstring(L, (*s_cur)->name);
		lua_settable(L, -3);

		lua_pushstring(L, "measure_last");
		lua_pushnumber(L, psensor_get_current_value(*s_cur));
		lua_settable(L, -3);

		lua_pushstring(L, "measure_min");
		lua_pushnumber(L, (*s_cur)->min);
		lua_settable(L, -3);

		lua_pushstring(L, "measure_max");
		lua_pushnumber(L, (*s_cur)->max);
		lua_settable(L, -3);

		lua_pushstring(L, "id");
		lua_pushstring(L, (*s_cur)->id);
		lua_settable(L, -3);

		lua_settable(L, -3);

		s_cur++;
		i++;
	}

	lua_setglobal(L, "sensors");

	lua_fpath = malloc(strlen(www_dir) + strlen(url) + 1);
	strcpy(lua_fpath, www_dir);
	strcat(lua_fpath, url);

	if (!luaL_loadfile(L, lua_fpath) && !lua_pcall(L, 0, 1, 0))
		page = strdup(lua_tostring(L, -1));

	lua_close(L);

	free(lua_fpath);

	return page;
}

static int cbk_http_request(void *cls,
			    struct MHD_Connection *connection,
			    const char *url,
			    const char *method,
			    const char *version,
			    const char *upload_data,
			    size_t *upload_data_size, void **ptr)
{
	struct psensor **sensors = cls;
	static int dummy;
	struct MHD_Response *response;
	int ret;
	char *nurl;
	unsigned int resp_code;
	char *page = NULL;

	if (strcmp(method, "GET"))
		return MHD_NO;	/* unexpected method */

	if (&dummy != *ptr) {
		/* The first time only the headers are valid, do not
		   respond in the first round... */
		*ptr = &dummy;
		return MHD_YES;
	}

	if (*upload_data_size)
		return MHD_NO;	/* upload data in a GET!? */

	*ptr = NULL;		/* clear context pointer */

	nurl = url_normalize(url);

	pthread_mutex_lock(&mutex);

	if (!strcmp(nurl, URL_BASE_API_1_0_SENSORS)) {
		page = sensors_to_json_string(sensors);

	} else if (!strncmp(nurl, URL_BASE_API_1_0_SENSORS,
			    strlen(URL_BASE_API_1_0_SENSORS))
		   && nurl[strlen(URL_BASE_API_1_0_SENSORS)] == '/') {

		char *sid = nurl + strlen(URL_BASE_API_1_0_SENSORS) + 1;
		struct psensor *s = psensor_list_get_by_id(sensors, sid);

		if (s)
			page = sensor_to_json_string(s);

	} else {
		page = lua_to_html_page(sensors, nurl);
	}

	if (page) {
		resp_code = MHD_HTTP_OK;
	} else {
		page = strdup(PAGE_NOT_FOUND);
		resp_code = MHD_HTTP_NOT_FOUND;
	}

	pthread_mutex_unlock(&mutex);

	response = MHD_create_response_from_data(strlen(page),
						 (void *)page, MHD_YES, MHD_NO);

	ret = MHD_queue_response(connection, resp_code, response);
	MHD_destroy_response(response);

	free(nurl);

	return ret;
}

int main(int argc, char *argv[])
{
	float last_used = 0;
	float last_total = 0;
	struct MHD_Daemon *d;
	struct psensor **sensors;
	int port = DEFAULT_PORT;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "vhpw:", long_options,
				&option_index);

		if (c == -1)
			break;

		switch (c) {

		case 0:
			break;

		case 'w':
			if (optarg)
				www_dir = strdup(optarg);
			break;

		case 'p':
			if (optarg)
				port = atoi(optarg);

			break;

		case 'h':
			printf("%s\n%s\n\n%s\n\n%s\n",
			       PSENSOR_SERVER_USAGE,
			       PSENSOR_SERVER_DESCRIPTION,
			       PSENSOR_SERVER_OPTIONS, PSENSOR_SERVER_CONTACT);
			exit(EXIT_SUCCESS);

		case 'v':
			printf("psensor-server %s\n", VERSION);
			printf("%s\n", PSENSOR_SERVER_COPYRIGHT);

			exit(EXIT_SUCCESS);

		case '?':
			break;

		default:
			abort();
		}
	}

	if (optind != argc) {
		fprintf(stderr, "Invalid usage.\n");
		exit(EXIT_FAILURE);
	}

	if (!lmsensor_init()) {
		fprintf(stderr, "failed to init lm-sensors\n");
		exit(EXIT_FAILURE);
	}

	sensors = get_all_sensors(1);

	if (!sensors || !*sensors) {
		fprintf(stderr, "no sensors detected\n");
		exit(EXIT_FAILURE);
	}

	d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
			     port,
			     NULL, NULL, &cbk_http_request, sensors,
			     MHD_OPTION_END);
	if (!d) {
		fprintf(stderr, "Fail to create web server\n");
		exit(EXIT_FAILURE);
	}

	printf("Web server started on port: %d\n", port);
	printf("Psensor URL: http://localhost:%d\n", port);

	while (1) {
		unsigned long int used = 0;
		unsigned long int dt;

		pthread_mutex_lock(&mutex);

#ifdef HAVE_GTOP
		if (!cpu)
			cpu = malloc(sizeof(glibtop_cpu));

		glibtop_get_cpu(cpu);

		used = cpu->user + cpu->nice + cpu->sys;

		dt = cpu->total - last_total;

		if (dt)
			cpu_rate = (used - last_used) / dt;

		last_used = used;
		last_total = cpu->total;

		psensor_list_update_measures(sensors);
#endif

		pthread_mutex_unlock(&mutex);
		sleep(5);
	}

	MHD_stop_daemon(d);

#ifdef HAVE_GTOP
	free(cpu);
#endif

	return 0;
}
