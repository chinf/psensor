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
#ifndef _PSENSOR_PSENSOR_H_
#define _PSENSOR_PSENSOR_H_

#include <config.h>

#include <bool.h>
#include <measure.h>
#include <plog.h>

enum psensor_type {
	/* type of sensor values */
	SENSOR_TYPE_TEMP = 0x00001,
	SENSOR_TYPE_RPM = 0x00002,
	SENSOR_TYPE_PERCENT = 0x00004,

	/* Whether the sensor is remote */
	SENSOR_TYPE_REMOTE = 0x00008,

	/* Libraries used for retrieving sensor information */
	SENSOR_TYPE_LMSENSOR = 0x00100,
	SENSOR_TYPE_NVCTRL = 0x00200,
	SENSOR_TYPE_GTOP = 0x00400,
	SENSOR_TYPE_ATIADL = 0x00800,
	SENSOR_TYPE_ATASMART = 0x01000,
	SENSOR_TYPE_HDDTEMP = 0x02000,
	SENSOR_TYPE_UDISKS2 = 0x800000,

	/* Type of HW component */
	SENSOR_TYPE_HDD = 0x04000,
	SENSOR_TYPE_CPU = 0x08000,
	SENSOR_TYPE_GPU = 0x10000,
	SENSOR_TYPE_FAN = 0x20000,

	SENSOR_TYPE_GRAPHICS = 0x40000,
	SENSOR_TYPE_VIDEO = 0x80000,
	SENSOR_TYPE_PCIE = 0x100000,
	SENSOR_TYPE_MEMORY = 0x200000,
	SENSOR_TYPE_AMBIENT = 0x400000,

	/* Combinations */
	SENSOR_TYPE_HDD_TEMP = (SENSOR_TYPE_HDD | SENSOR_TYPE_TEMP),
	SENSOR_TYPE_CPU_USAGE = (SENSOR_TYPE_CPU | SENSOR_TYPE_PERCENT)
};

struct psensor {
	/* Human readable name of the sensor.  It may not be uniq. */
	char *name;

	/* Uniq id of the sensor */
	char *id;

	/* Name of the chip. */
	char *chip;

	/* Maximum length of 'values' */
	int values_max_length;

	/*
	 * Last registered measures of the sensor.  Index 0 for the
	 * oldest measure.
	 */
	struct measure *measures;

	/* see psensor_type */
	unsigned int type;

	double max;

	double min;

	/* The highest value detected during this session. */
	double sess_highest;

	/* The lowest value detected during this session. */
	double sess_lowest;

	double alarm_high_threshold;
	double alarm_low_threshold;

	/* Whether an alarm is raised for this sensor */
	bool alarm_raised;

	void (*cb_alarm_raised)(struct psensor *, void *);
	void *cb_alarm_raised_data;

#ifdef HAVE_LIBATIADL
	/* AMD id for the aticonfig */
	int amd_id;
#endif

	void *provider_data;
	void (*provider_data_free_fct)(void *);
};

struct psensor *psensor_create(char *id,
			       char *name,
			       char *chip,
			       unsigned int type,
			       int values_max_length);

void psensor_values_resize(struct psensor *s, int new_size);

void psensor_free(struct psensor *sensor);

void psensor_list_free(struct psensor **sensors);
int psensor_list_size(struct psensor **sensors);

struct psensor *psensor_list_get_by_id(struct psensor **sensors,
				       const char *id);

int is_temp_type(unsigned int type);

double get_min_temp(struct psensor **sensors);
double get_max_temp(struct psensor **sensors);

double get_min_rpm(struct psensor **sensors);
double get_max_rpm(struct psensor **sensors);

/*
 * Converts the value of a sensor to a string.
 *
 * parameter 'type' is SENSOR_TYPE_LMSENSOR_TEMP, SENSOR_TYPE_NVIDIA,
 * or SENSOR_TYPE_LMSENSOR_FAN
 */
char *psensor_value_to_str(unsigned int type,
			   double value,
			   int use_celsius);

char *psensor_measure_to_str(const struct measure *m,
			     unsigned int type,
			     unsigned int use_celsius);

struct psensor **psensor_list_add(struct psensor **sensors,
				  struct psensor *sensor);

void psensor_list_append(struct psensor ***sensors, struct psensor *sensor);

struct psensor **psensor_list_copy(struct psensor **);

void psensor_set_current_value(struct psensor *sensor, double value);
void psensor_set_current_measure(struct psensor *sensor, double value,
				 struct timeval tv);

double psensor_get_current_value(const struct psensor *);

struct measure *psensor_get_current_measure(struct psensor *sensor);

/* Returns a string representation of a psensor type. */
const char *psensor_type_to_str(unsigned int type);

const char *psensor_type_to_unit_str(unsigned int type, int use_celsius);

double get_max_value(struct psensor **sensors, int type);

char *psensor_current_value_to_str(const struct psensor *, unsigned int);

void psensor_log_measures(struct psensor **sensors);

#endif
