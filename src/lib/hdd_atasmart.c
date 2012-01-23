/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
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

#include <stdlib.h>
#include <string.h>

#include <atasmart.h>

#include "pio.h"
#include "hdd.h"
#include "log.h"

static int filter_sd(const char *p)
{
	return strlen(p) == 8 && !strncmp(p, "/dev/sd", 7);
}

static struct psensor *
create_sensor(char *id, char *name, SkDisk *disk, int values_max_length)
{
	struct psensor *s;
	s = psensor_create(id,
			   strdup(name),
			   SENSOR_TYPE_HDD_TEMP_ATASMART,
			   values_max_length);

	s->disk = disk;

	return s;
}

struct psensor **hdd_psensor_list_add(struct psensor **sensors,
				      int values_max_length)
{
	char **paths, **tmp, *id;
	SkDisk *disk;
	struct psensor *sensor, **tmp_sensors, **result;

	log_debug("hdd_psensor_list_add");

	paths = dir_list("/dev", filter_sd);

	result = sensors;
	tmp = paths;
	while (*tmp) {
		log_debug("hdd_psensor_list_add open %s", *tmp);

		if (!sk_disk_open(*tmp, &disk)) {
			id = malloc(strlen("hdd at") + strlen(*tmp) + 1);
			strcpy(id, "hdd at");
			strcat(id, *tmp);

			sensor = create_sensor(id,
					       *tmp,
					       disk,
					       values_max_length);

			tmp_sensors = psensor_list_add(result, sensor);

			if (result != sensors)
				free(result);

			result = tmp_sensors;
		} else {
			log_err("Failed to open %s", *tmp);
		}

		tmp++;
	}

	paths_free(paths);

	return result;
}

void hdd_psensor_list_update(struct psensor **sensors)
{
	struct psensor **cur, *s;
	uint64_t kelvin;
	int ret;
	double c;

	cur = sensors;
	while (*cur) {
		s = *cur;
		if (s->type == SENSOR_TYPE_HDD_TEMP_ATASMART) {
			ret = sk_disk_smart_read_data(s->disk);

			if (!ret) {
				ret = sk_disk_smart_get_temperature(s->disk,
								    &kelvin);

				if (!ret) {
					c = (kelvin - 273150) / 1000;
					psensor_set_current_value(s, c);
					log_debug("hdd atasmart: %s %.2f",
						  s->id,
						  c);
				}
			}
		}

		cur++;
	}
}
