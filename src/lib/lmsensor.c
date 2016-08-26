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

#include <sensors/sensors.h>
#include <sensors/error.h>

#include <lmsensor.h>

static int init_done;

static const char *PROVIDER_NAME = "lmsensor";

struct lmsensor_data {
	const sensors_chip_name *chip;

	const sensors_feature *feature;
};

static const sensors_chip_name *get_chip_name(struct psensor *s)
{
	return ((struct lmsensor_data *)s->provider_data)->chip;
}

static const sensors_feature *get_feature(struct psensor *s)
{
	return ((struct lmsensor_data *)s->provider_data)->feature;
}

static void lmsensor_data_set(struct psensor *s,
			      const struct sensors_chip_name *chip,
			      const struct sensors_feature *feature)
{
	struct lmsensor_data *data;

	data = malloc(sizeof(struct lmsensor_data));
	data->chip = chip;
	data->feature = feature;

	s->provider_data = data;
}

static double get_value(const sensors_chip_name *name,
			const sensors_subfeature *sub)
{
	double val;
	int err;

	err = sensors_get_value(name, sub->number, &val);
	if (err) {
		log_err(_("%s: Cannot get value of subfeature %s: %s."),
			PROVIDER_NAME,
			sub->name,
			sensors_strerror(err));
		val = UNKNOWN_DBL_VALUE;
	}
	return val;
}

static double get_temp_input(struct psensor *sensor)
{
	const sensors_subfeature *sf;

	const sensors_chip_name *chip;

	const sensors_feature *feature;

	chip = get_chip_name(sensor);
	feature = get_feature(sensor);

	sf = sensors_get_subfeature(chip,
				    feature,
				    SENSORS_SUBFEATURE_TEMP_INPUT);
	if (sf)
		return get_value(chip, sf);

	return UNKNOWN_DBL_VALUE;
}

static double get_fan_input(struct psensor *sensor)
{
	const sensors_chip_name *chip;
	const sensors_feature *feature;

	const sensors_subfeature *sf;

	chip = get_chip_name(sensor);
	feature = get_feature(sensor);

	sf = sensors_get_subfeature(chip,
				    feature,
				    SENSORS_SUBFEATURE_FAN_INPUT);

	if (sf)
		return get_value(chip, sf);

	return UNKNOWN_DBL_VALUE;
}

void lmsensor_psensor_list_update(struct psensor **sensors)
{
	struct psensor *s;
	double v;

	if (!init_done || !sensors)
		return;

	while (*sensors) {
		s = *sensors;

		if (!(s->type & SENSOR_TYPE_REMOTE)
		    && s->type & SENSOR_TYPE_LMSENSOR) {

			if (s->type & SENSOR_TYPE_TEMP)
				v = get_temp_input(s);
			else /* s->type & SENSOR_TYPE_RPM */
				v = get_fan_input(s);

			if (v != UNKNOWN_DBL_VALUE)
				psensor_set_current_value(s, v);
		}

		sensors++;
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
	sensors_subfeature_type fault_subfeature, min_subfeature,
		max_subfeature;

	if (sensors_snprintf_chip_name(name, 200, chip) < 0)
		return NULL;

	if (feature->type == SENSORS_FEATURE_TEMP) {
		fault_subfeature = SENSORS_SUBFEATURE_TEMP_FAULT;
		max_subfeature = SENSORS_SUBFEATURE_TEMP_MAX;
		min_subfeature = SENSORS_SUBFEATURE_TEMP_MIN;
	} else if (feature->type == SENSORS_FEATURE_FAN) {
		fault_subfeature = SENSORS_SUBFEATURE_FAN_FAULT;
		max_subfeature = SENSORS_SUBFEATURE_FAN_MAX;
		min_subfeature = SENSORS_SUBFEATURE_FAN_MIN;
	} else {
		log_err(_("%s: Wrong feature type."), PROVIDER_NAME);
		return NULL;
	}

	sf = sensors_get_subfeature(chip, feature, fault_subfeature);
	if (sf && get_value(chip, sf))
		return NULL;

	label = sensors_get_label(chip, feature);
	if (!label)
		return NULL;

	type = SENSOR_TYPE_LMSENSOR;
	if (feature->type == SENSORS_FEATURE_TEMP)
		type |= SENSOR_TYPE_TEMP;
	else if (feature->type == SENSORS_FEATURE_FAN)
		type |= (SENSOR_TYPE_RPM|SENSOR_TYPE_FAN);
	else
		return NULL;

	id = malloc(strlen(PROVIDER_NAME)
		    + 1
		    + strlen(name)
		    + 1
		    + strlen(label)
		    + 1);
	sprintf(id, "%s %s %s", PROVIDER_NAME, name, label);

	if (!strcmp(chip->prefix, "coretemp"))
		cname = strdup(_("Intel CPU"));
	else if (!strcmp(chip->prefix, "k10temp")
		 || !strcmp(chip->prefix, "k8temp")
		 || !strcmp(chip->prefix, "fam15h_power"))
		cname = strdup(_("AMD CPU"));
	else if (!strcmp(chip->prefix, "nouveau"))
		cname = strdup(_("NVIDIA GPU"));
	else if (!strcmp(chip->prefix, "via-cputemp"))
		cname = strdup(_("VIA CPU"));
	else if (!strcmp(chip->prefix, "acpitz"))
		cname = strdup(_("ACPI"));
	else
		cname = strdup(chip->prefix);

	psensor = psensor_create(id, label, cname, type, values_max_length);

	sf = sensors_get_subfeature(chip, feature, max_subfeature);
	if (sf)
		psensor->max = get_value(chip, sf);

	sf = sensors_get_subfeature(chip, feature, min_subfeature);
	if (sf)
		psensor->min = get_value(chip, sf);

	lmsensor_data_set(psensor, chip, feature);

	if (feature->type == SENSORS_FEATURE_TEMP
	    && (get_temp_input(psensor) == UNKNOWN_DBL_VALUE)) {
		free(psensor);
		return NULL;
	}

	return psensor;
}

static void lmsensor_init(void)
{
	int err;

	err = sensors_init(NULL);

	if (err) {
		log_err(_("%s: initialization failure: %s."),
			PROVIDER_NAME,
			sensors_strerror(err));
		init_done = 0;
	} else {
		init_done = 1;
	}
}

void lmsensor_psensor_list_append(struct psensor ***sensors, int vn)
{
	const sensors_chip_name *chip;
	int chip_nr, i;
	const sensors_feature *feature;
	struct psensor *s;

	if (!init_done)
		lmsensor_init();

	if (!init_done)
		return;

	chip_nr = 0;
	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {

		i = 0;
		while ((feature = sensors_get_features(chip, &i))) {
			if (feature->type == SENSORS_FEATURE_TEMP
			    || feature->type == SENSORS_FEATURE_FAN) {

				s = lmsensor_psensor_create(chip, feature, vn);

				if (s)
					psensor_list_append(sensors, s);
			}
		}
	}
}

void lmsensor_cleanup(void)
{
	if (init_done)
		sensors_cleanup();
}
