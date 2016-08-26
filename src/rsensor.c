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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include <psensor_json.h>
#include <rsensor.h>
#include <server/server.h>
#include <url.h>

struct ucontent {
	char *data;
	size_t len;
};

static CURL *curl;

static const char *PROVIDER_NAME = "rsensor";

static const char *get_url(struct psensor *s)
{
	return (char *)s->provider_data;
}

static size_t cbk_curl(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t realsize;
	struct ucontent *mem;

	realsize = size * nmemb;
	mem = (struct ucontent *)userp;

	mem->data = realloc(mem->data, mem->len + realsize + 1);

	memcpy(&(mem->data[mem->len]), buffer, realsize);
	mem->len += realsize;
	mem->data[mem->len] = 0;

	return realsize;
}

static char *create_api_1_1_sensors_url(const char *base_url)
{
	char *nurl, *ret;
	int n;

	nurl = url_normalize(base_url);
	n = strlen(nurl) + strlen(URL_BASE_API_1_1_SENSORS) + 1;
	ret = malloc(n);

	strcpy(ret, nurl);
	strcat(ret, URL_BASE_API_1_1_SENSORS);

	free(nurl);

	return ret;
}

void rsensor_init(void)
{
	curl = curl_easy_init();
}

void rsensor_cleanup(void)
{
	curl_easy_cleanup(curl);
}

static json_object *get_json_object(const char *url)
{
	struct ucontent chunk;
	json_object *obj;

	obj = NULL;

	if (!curl)
		return NULL;

	chunk.data = malloc(1);
	chunk.len = 0;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cbk_curl);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	log_fct("%s: HTTP request %s", PROVIDER_NAME, url);

	if (curl_easy_perform(curl) == CURLE_OK)
		obj = json_tokener_parse(chunk.data);
	else
		log_err(_("%s: Fail to connect to: %s"), PROVIDER_NAME, url);

	free(chunk.data);

	return obj;
}

struct psensor **get_remote_sensors(const char *server_url,
				    int values_max_length)
{
	struct psensor **sensors, *s;
	char *url;
	json_object *obj;
	int i, n;

	sensors = NULL;

	url = create_api_1_1_sensors_url(server_url);

	obj = get_json_object(url);

	if (obj && !is_error(obj)) {
		n = json_object_array_length(obj);
		sensors = malloc((n + 1) * sizeof(struct psensor *));

		for (i = 0; i < n; i++) {
			s = psensor_new_from_json
				(json_object_array_get_idx(obj, i),
				 url,
				 values_max_length);
			sensors[i] = s;
		}

		sensors[n] = NULL;

		json_object_put(obj);
	} else {
		log_err(_("%s: Invalid content: %s"), PROVIDER_NAME, url);
	}

	free(url);

	if (!sensors) {
		sensors = malloc(sizeof(struct psensor *));
		*sensors = NULL;
	}

	return sensors;
}

static void remote_psensor_update(struct psensor *s)
{
	json_object *obj;

	obj = get_json_object(get_url(s));

	if (obj && !is_error(obj)) {
		json_object *om;

		json_object_object_get_ex(obj, "last_measure", &om);

		if (!is_error(obj)) {
			json_object *ov, *ot;
			struct timeval tv;

			json_object_object_get_ex(om, "value", &ov);
			json_object_object_get_ex(om, "time", &ot);

			tv.tv_sec = json_object_get_int(ot);
			tv.tv_usec = 0;

			psensor_set_current_measure
			    (s, json_object_get_double(ov), tv);
		}

		json_object_put(obj);
	} else {
		log_err(_("%s: Invalid JSON: %s"), PROVIDER_NAME, get_url(s));
	}

}

void remote_psensor_list_update(struct psensor **sensors)
{
	struct psensor **cur;

	cur = sensors;
	while (*cur) {
		struct psensor *s = *cur;

		if (s->type & SENSOR_TYPE_REMOTE)
			remote_psensor_update(s);

		cur++;
	}
}
