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

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include "plib/url.h"
#include "server/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json/json.h>

#include "rsensor.h"

struct ucontent {
	char *data;
	size_t len;
};

static CURL *curl;

size_t cbk_curl(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct ucontent *mem = (struct ucontent *)userp;

	mem->data = realloc(mem->data, mem->len + realsize + 1);

	memcpy(&(mem->data[mem->len]), buffer, realsize);
	mem->len += realsize;
	mem->data[mem->len] = 0;

	return realsize;
}

char *create_api_1_0_sensors_url(const char *base_url)
{
	char *nurl = url_normalize(base_url);
	int n = strlen(nurl) + strlen(URL_BASE_API_1_0_SENSORS) + 1;
	char *ret = malloc(n);

	strcpy(ret, nurl);
	strcat(ret, URL_BASE_API_1_0_SENSORS);

	free(nurl);

	return ret;
}

struct psensor *json_object_to_psensor(json_object * o,
				       const char *sensors_url,
				       int values_max_length)
{
	json_object *oid;
	json_object *oname;
	json_object *otype;
	struct psensor *s;
	char *eid;
	char *url;

	oid = json_object_object_get(o, "id");
	oname = json_object_object_get(o, "name");
	otype = json_object_object_get(o, "type");

	eid = url_encode(json_object_get_string(oid));
	url = malloc(strlen(sensors_url) + 1 + strlen(eid) + 1);
	sprintf(url, "%s/%s", sensors_url, eid);

	s = psensor_create(strdup(url),
			   strdup(json_object_get_string(oname)),
			   json_object_get_int(otype) | SENSOR_TYPE_REMOTE,
			   values_max_length);
	s->url = url;

	free(eid);

	return s;
}

void rsensor_init()
{
	curl = curl_easy_init();
}

void rsensor_end()
{
	curl_easy_cleanup(curl);
}

json_object *get_json_object(const char *url)
{

	struct ucontent chunk;
	json_object *obj = NULL;

	if (!curl)
		return NULL;

	chunk.data = malloc(1);
	chunk.len = 0;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cbk_curl);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	if (curl_easy_perform(curl) == CURLE_OK)
		obj = json_tokener_parse(chunk.data);
	else
		fprintf(stderr, _("ERROR: Fail to connect to: %s\n"), url);

	free(chunk.data);

	return obj;
}

struct psensor **get_remote_sensors(const char *server_url,
				    int values_max_length)
{
	struct psensor **sensors = NULL;
	char *url;
	json_object *obj;

	url = create_api_1_0_sensors_url(server_url);

	obj = get_json_object(url);

	if (obj && !is_error(obj)) {
		int i;
		int n = json_object_array_length(obj);
		sensors = malloc((n + 1) * sizeof(struct psensor *));

		for (i = 0; i < n; i++) {
			struct psensor *s = json_object_to_psensor
			    (json_object_array_get_idx(obj, i),
			     url,
			     values_max_length);
			sensors[i] = s;
		}

		sensors[n] = NULL;

		json_object_put(obj);
	} else {
		fprintf(stderr, _("ERROR: Invalid content: %s\n"), url);
	}

	free(url);

	if (!sensors) {
		sensors = malloc(sizeof(struct psensor *));
		*sensors = NULL;
	}

	return sensors;
}

void remote_psensor_update(struct psensor *s)
{
	json_object *obj = get_json_object(s->url);

	if (obj && !is_error(obj)) {
		json_object *om;

		om = json_object_object_get(obj, "last_measure");

		if (!is_error(obj)) {
			json_object *ov, *ot;
			struct timeval tv;

			ov = json_object_object_get(om, "value");
			ot = json_object_object_get(om, "time");

			tv.tv_sec = json_object_get_int(ot);
			tv.tv_usec = 0;

			psensor_set_current_measure
			    (s, json_object_get_double(ov), tv);;
		}

		json_object_put(obj);
	} else {
		printf(_("ERROR: Invalid JSON: %s\n"), s->url);
	}

}

void remote_psensor_list_update(struct psensor **sensors)
{
	struct psensor **cur = sensors;

	while (*cur) {
		struct psensor *s = *cur;

		if (s->type & SENSOR_TYPE_REMOTE)
			remote_psensor_update(s);

		cur++;
	}
}
