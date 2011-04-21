/*
    Copyright (C) 2010-2011 jeanfi@gmail.com

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

#include <string.h>

#include "psensor_json.h"

#define ATT_SENSOR_ID "id"
#define ATT_SENSOR_NAME "name"
#define ATT_SENSOR_TYPE "type"
#define ATT_SENSOR_MIN "min"
#define ATT_SENSOR_MAX "max"
#define ATT_SENSOR_LAST_MEASURE "last_measure"
#define ATT_MEASURE_VALUE "value"
#define ATT_MEASURE_TIME "time"

json_object *sensor_to_json_object(struct psensor *s)
{
	json_object *mo;
	json_object *obj = json_object_new_object();
	struct measure *m;

	json_object_object_add(obj,
			       ATT_SENSOR_ID,
			       json_object_new_string(s->id));
	json_object_object_add(obj,
			       ATT_SENSOR_NAME,
			       json_object_new_string(s->name));
	json_object_object_add(obj,
			       ATT_SENSOR_TYPE, json_object_new_int(s->type));
	json_object_object_add(obj,
			       ATT_SENSOR_MIN, json_object_new_double(s->min));
	json_object_object_add(obj,
			       ATT_SENSOR_MAX, json_object_new_double(s->max));

	m = psensor_get_current_measure(s);
	mo = json_object_new_object();
	json_object_object_add(mo,
			       ATT_MEASURE_VALUE,
			       json_object_new_double(m->value));
	json_object_object_add(mo, ATT_MEASURE_TIME,
			       json_object_new_int((m->time).tv_sec));
	json_object_object_add(obj, ATT_SENSOR_LAST_MEASURE, mo);

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

