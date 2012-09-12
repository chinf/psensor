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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sensors/sensors.h>
#include <sensors/error.h>

#include "psensor.h"

static int init_done;

static double get_value(const sensors_chip_name *name,
			const sensors_subfeature *sub)
{
	double val;
	int err;

	err = sensors_get_value(name, sub->number, &val);
	if (err) {
		log_err(_("lmsensor: cannot get value of subfeature %s: %s."),
			sub->name, sensors_strerror(err));
		val = UNKNOWN_DBL_VALUE;
	}
	return val;
}

static double get_temp_input(struct psensor *sensor)
{
	const sensors_chip_name *chip = sensor->iname;
	const sensors_feature *feature = sensor->feature;

	const sensors_subfeature *sf;

	sf = sensors_get_subfeature(chip,
				    feature, SENSORS_SUBFEATURE_TEMP_INPUT);
	if (sf)
		return get_value(chip, sf);
	else
		return UNKNOWN_DBL_VALUE;
}

static double get_fan_input(struct psensor *sensor)
{
	const sensors_chip_name *chip = sensor->iname;
	const sensors_feature *feature = sensor->feature;

	const sensors_subfeature *sf;

	sf = sensors_get_subfeature(chip,
				    feature, SENSORS_SUBFEATURE_FAN_INPUT);
	if (sf)
		return get_value(chip, sf);
	else
		return UNKNOWN_DBL_VALUE;
}

void lmsensor_psensor_list_update(struct psensor **sensors)
{
	struct psensor **s_ptr = sensors;

	if (!init_done)
		return ;

	while (*s_ptr) {
		struct psensor *sensor = *s_ptr;

		if (sensor->type == SENSOR_TYPE_LMSENSOR_TEMP)
			psensor_set_current_value
			    (sensor, get_temp_input(sensor));
		else if (sensor->type == SENSOR_TYPE_LMSENSOR_FAN)
			psensor_set_current_value(sensor,
						  get_fan_input(sensor));

		s_ptr++;
	}
}

static struct psensor *
lmsensor_psensor_create(const sensors_chip_name *chip,
			const sensors_feature *feature,
			int values_max_length)
{
	char name[200];
	const sensors_subfeature *sf;
	int type;
	char *id, *label, *cname;
	struct psensor *psensor;
	sensors_subfeature_type fault_subfeature;

	if (sensors_snprintf_chip_name(name, 200, chip) < 0)
		return NULL;

	if (feature->type == SENSORS_FEATURE_TEMP) {
		fault_subfeature = SENSORS_SUBFEATURE_TEMP_FAULT;

	} else if (feature->type == SENSORS_FEATURE_FAN) {
		fault_subfeature = SENSORS_SUBFEATURE_FAN_FAULT;

	} else {
		log_err(_(
"lmsensor: lmsensor_psensor_create failure: wrong feature type."));
		return NULL;
	}

	sf = sensors_get_subfeature(chip, feature, fault_subfeature);
	if (sf && get_value(chip, sf))
		return NULL;

	label = sensors_get_label(chip, feature);
	if (!label)
		return NULL;

	type = 0;
	if (feature->type == SENSORS_FEATURE_TEMP)
		type = SENSOR_TYPE_LMSENSOR_TEMP;
	else if (feature->type == SENSORS_FEATURE_FAN)
		type = SENSOR_TYPE_LMSENSOR_FAN;
	else
		return NULL;

	id = malloc(strlen("lmsensor ") + 1 + strlen(name) + 1 + strlen(label) +
		    1);
	sprintf(id, "lmsensor %s %s", name, label);

	if (!strcmp(chip->prefix, "coretemp"))
		cname = strdup("Intel CPU");
	else if (!strcmp(chip->prefix, "k10temp")
		 || !strcmp(chip->prefix, "k8temp")
		 || !strcmp(chip->prefix, "fam15h_power"))
		cname = strdup("AMD CPU");
	else if (!strcmp(chip->prefix, "nouveau"))
		cname = strdup("Nvidia GPU");
	else if (!strcmp(chip->prefix, "via-cputemp"))
		cname = strdup("VIA CPU");
	else
		cname = strdup(chip->prefix);

	psensor = psensor_create(id, label, cname, type, values_max_length);

	psensor->iname = chip;
	psensor->feature = feature;

	if (feature->type == SENSORS_FEATURE_TEMP
	    && (get_temp_input(psensor) == UNKNOWN_DBL_VALUE)) {
		free(psensor);
		return NULL;
	}

	return psensor;
}

struct psensor **lmsensor_psensor_list_add(struct psensor **sensors,
					   int vn)
{
	const sensors_chip_name *chip;
	int chip_nr = 0;
	struct psensor **tmp, **result;
	const sensors_feature *feature;
	struct psensor *s;
	int i;

	if (!init_done)
		return NULL;

	result = sensors;
	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {

		i = 0;
		while ((feature = sensors_get_features(chip, &i))) {

			if (feature->type == SENSORS_FEATURE_TEMP
			    || feature->type == SENSORS_FEATURE_FAN) {

				s = lmsensor_psensor_create(chip, feature, vn);

				if (s) {
					tmp = psensor_list_add(result, s);

					if (tmp != sensors)
						free(result);

					result = tmp;
				}
			}
		}
	}

	return result;
}

void lmsensor_init()
{
	int err = sensors_init(NULL);

	if (err) {
		log_err(_("lmsensor: initialization failure: %s."),
			sensors_strerror(err));
		init_done = 0;
	} else {
		init_done = 1;
	}
}

void lmsensor_cleanup()
{
	if (init_done)
		sensors_cleanup();
}
