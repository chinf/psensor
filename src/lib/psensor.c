/*
 * Copyright (C) 2010-2014 jeanfi@gmail.com
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
#include <stdlib.h>
#include <string.h>

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <hdd.h>
#include <psensor.h>
#include <lmsensor.h>
#include <temperature.h>

#ifdef HAVE_GTOP
#include <cpu.h>
#include <pmem.h>
#endif

struct psensor *psensor_create(char *id,
			       char *name,
			       char *chip,
			       unsigned int type,
			       int values_max_length)
{
	struct psensor *psensor;

	psensor = (struct psensor *)malloc(sizeof(struct psensor));

	psensor->id = id;
	psensor->name = name;
	psensor->chip = chip;
	psensor->min = UNKNOWN_DBL_VALUE;
	psensor->max = UNKNOWN_DBL_VALUE;

	psensor->type = type;

	psensor->values_max_length = values_max_length;
	psensor->measures = measures_dbl_create(values_max_length);

	psensor->alarm_enabled = 0;
	psensor->alarm_high_threshold = 0;
	psensor->alarm_low_threshold = 0;

	psensor->cb_alarm_raised = NULL;
	psensor->cb_alarm_raised_data = NULL;
	psensor->alarm_raised = 0;

	psensor->url = NULL;

	psensor->color = NULL;

	psensor->graph_enabled = 1;
	psensor->appindicator_enabled = 0;

	return psensor;
}

void psensor_values_resize(struct psensor *s, int new_size)
{
	struct measure *new_ms, *cur_ms;
	int cur_size;

	cur_size = s->values_max_length;
	cur_ms = s->measures;
	new_ms = measures_dbl_create(new_size);

	if (cur_ms) {
		int i;
		for (i = 0; i < new_size - 1 && i < cur_size - 1; i++)
			measure_copy(&cur_ms[cur_size - i - 1],
				     &new_ms[new_size - i - 1]);

		measures_free(s->measures);
	}

	s->values_max_length = new_size;
	s->measures = new_ms;
}

void psensor_free(struct psensor *sensor)
{
	if (sensor) {
		log_debug("Cleanup %s", sensor->id);

		free(sensor->name);
		free(sensor->id);

		if (sensor->chip)
			free(sensor->chip);

		if (sensor->color)
			free(sensor->color);

		measures_free(sensor->measures);

		free(sensor->url);

		free(sensor);
	}
}

void psensor_list_free(struct psensor **sensors)
{
	struct psensor **sensor_cur;

	if (sensors) {
		sensor_cur = sensors;

		while (*sensor_cur) {
			psensor_free(*sensor_cur);

			sensor_cur++;
		}

		free(sensors);

		sensors = NULL;
	}
}

int psensor_list_size(struct psensor **sensors)
{
	int size;
	struct psensor **sensor_cur;

	if (!sensors)
		return 0;

	size = 0;
	sensor_cur = sensors;

	while (*sensor_cur) {
		size++;
		sensor_cur++;
	}
	return size;
}

int psensor_list_contains_type(struct psensor **sensors, unsigned int type)
{
	struct psensor **s;

	if (!sensors)
		return 0;

	s = sensors;
	while (*s) {
		if ((*s)->type & type)
			return 1;
		s++;
	}

	return 0;
}

struct psensor **psensor_list_add(struct psensor **sensors,
				  struct psensor *sensor)
{
	int size;

	size = psensor_list_size(sensors);

	struct psensor **result
	    = malloc((size + 1 + 1) * sizeof(struct psensor *));

	if (sensors)
		memcpy(result, sensors, size * sizeof(struct psensor *));

	result[size] = sensor;
	result[size + 1] = NULL;

	return result;
}

void psensor_list_append(struct psensor ***sensors, struct psensor *sensor)
{
	struct psensor **tmp;

	if (!sensor)
		return ;

	tmp = psensor_list_add(*sensors, sensor);

	if (tmp != *sensors) {
		free(*sensors);
		*sensors = tmp;
	}
}


struct psensor *psensor_list_get_by_id(struct psensor **sensors, const char *id)
{
	struct psensor **sensors_cur = sensors;

	while (*sensors_cur) {
		if (!strcmp((*sensors_cur)->id, id))
			return *sensors_cur;

		sensors_cur++;
	}

	return NULL;
}

int is_temp_type(unsigned int type)
{
	return type & SENSOR_TYPE_TEMP;
}

int is_fan_type(unsigned int type)
{
	return type & SENSOR_TYPE_FAN;
}

char *
psensor_value_to_str(unsigned int type, double value, int use_celsius)
{
	char *str;
	const char *unit;

	/*
	 * should not be possible to exceed 20 characters with temp or
	 * rpm values the .x part is never displayed
	 */
	str = malloc(20);

	unit = psensor_type_to_unit_str(type, use_celsius);

	if (is_temp_type(type) && !use_celsius)
		value = celsius_to_fahrenheit(value);

	sprintf(str, "%.0f%s", value, unit);

	return str;
}

char *
psensor_measure_to_str(const struct measure *m,
		       unsigned int type,
		       unsigned int use_celsius)
{
	return psensor_value_to_str(type, m->value, use_celsius);
}

void psensor_set_current_value(struct psensor *sensor, double value)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		timerclear(&tv);

	psensor_set_current_measure(sensor, value, tv);
}

void psensor_set_current_measure(struct psensor *s, double v, struct timeval tv)
{
	memmove(s->measures,
		&s->measures[1],
		(s->values_max_length - 1) * sizeof(struct measure));

	s->measures[s->values_max_length - 1].value = v;
	s->measures[s->values_max_length - 1].time = tv;

	if (s->min == UNKNOWN_DBL_VALUE || v < s->min)
		s->min = v;

	if (s->max == UNKNOWN_DBL_VALUE || v > s->max)
		s->max = v;

	if (s->alarm_enabled) {
		if (v > s->alarm_high_threshold || v < s->alarm_low_threshold) {
			if (!s->alarm_raised && s->cb_alarm_raised)
				s->cb_alarm_raised(s, s->cb_alarm_raised_data);

			s->alarm_raised = 1;
		} else {
			s->alarm_raised = 0;
		}
	}
}

double psensor_get_current_value(const struct psensor *sensor)
{
	return sensor->measures[sensor->values_max_length - 1].value;
}

struct measure *psensor_get_current_measure(struct psensor *sensor)
{
	return &sensor->measures[sensor->values_max_length - 1];
}

/*
  Returns the minimal value of a given 'type' (SENSOR_TYPE_TEMP or
  SENSOR_TYPE_FAN)
 */
static double get_min_value(struct psensor **sensors, int type)
{
	double m = UNKNOWN_DBL_VALUE;
	struct psensor **s = sensors;

	while (*s) {
		struct psensor *sensor = *s;

		if (sensor->type & type) {
			int i;
			double t;

			for (i = 0; i < sensor->values_max_length; i++) {
				t = sensor->measures[i].value;

				if (t == UNKNOWN_DBL_VALUE)
					continue;

				if (m == UNKNOWN_DBL_VALUE || t < m)
					m = t;
			}
		}
		s++;
	}

	return m;
}

/*
  Returns the maximal value of a given 'type' (SENSOR_TYPE_TEMP or
  SENSOR_TYPE_FAN)
 */
double get_max_value(struct psensor **sensors, int type)
{
	double m = UNKNOWN_DBL_VALUE;
	struct psensor **s = sensors;

	while (*s) {
		struct psensor *sensor = *s;

		if (sensor->type & type) {
			int i;
			double t;
			for (i = 0; i < sensor->values_max_length; i++) {
				t = sensor->measures[i].value;

				if (t == UNKNOWN_DBL_VALUE)
					continue;

				if (m == UNKNOWN_DBL_VALUE || t > m)
					m = t;
			}
		}
		s++;
	}

	return m;
}

double
psensor_get_max_current_value(struct psensor **sensors, unsigned int type)
{
	double m = UNKNOWN_DBL_VALUE;
	struct psensor **s_cur = sensors;

	while (*s_cur) {
		struct psensor *s = *s_cur;

		if (s->graph_enabled && (s->type & type)) {
			double v = psensor_get_current_value(s);

			if (m == UNKNOWN_DBL_VALUE || v > m)
				m = v;
		}

		s_cur++;
	}

	return m;
}

double get_min_temp(struct psensor **sensors)
{
	return get_min_value(sensors, SENSOR_TYPE_TEMP);
}

double get_min_rpm(struct psensor **sensors)
{
	return get_min_value(sensors, SENSOR_TYPE_FAN);
}

double get_max_rpm(struct psensor **sensors)
{
	return get_max_value(sensors, SENSOR_TYPE_FAN);
}

double get_max_temp(struct psensor **sensors)
{
	return get_max_value(sensors, SENSOR_TYPE_TEMP);
}

struct psensor **get_all_sensors(int use_libatasmart, int values_max_length)
{
	struct psensor **psensors;
	struct psensor **tmp_psensors;

	psensors = lmsensor_psensor_list_add(NULL, values_max_length);

	if (!use_libatasmart) {
		tmp_psensors = hddtemp_psensor_list_add(psensors,
							values_max_length);
		if (tmp_psensors != psensors) {
			free(psensors);
			psensors = tmp_psensors;
		}
	}
#ifdef HAVE_ATASMART
		else {
			tmp_psensors = hdd_psensor_list_add(psensors,
							    values_max_length);
			if (tmp_psensors != psensors) {
				free(psensors);
				psensors = tmp_psensors;
			}
		}
#endif

#ifdef HAVE_GTOP
	mem_psensor_list_add(&psensors, values_max_length);
#endif

	if (!psensors) {	/* there is no detected sensors */
		psensors = malloc(sizeof(struct psensor *));
		*psensors = NULL;
	}

	return psensors;
}

const char *psensor_type_to_str(unsigned int type)
{
	if (type & SENSOR_TYPE_NVCTRL) {
		if (type & SENSOR_TYPE_TEMP)
			return "Temperature";
		else if (type & SENSOR_TYPE_GRAPHICS)
			return "Graphics usage";
		else if (type & SENSOR_TYPE_VIDEO)
			return "Video usage";
		else if (type & SENSOR_TYPE_MEMORY)
			return "Memory usage";
		else if (type & SENSOR_TYPE_PCIE)
			return "PCIe usage";
		else
			return "NVIDIA GPU";
	}

	if (type & SENSOR_TYPE_ATIADL) {
		if (type & SENSOR_TYPE_TEMP)
			return "AMD GPU Temperature";
		else if (type & SENSOR_TYPE_RPM)
			return "AMD GPU Fan Speed";
		else /* type & SENSOR_TYPE_USAGE */
			return "AMD GPU Usage";
	}

	if ((type & SENSOR_TYPE_HDD_TEMP) == SENSOR_TYPE_HDD_TEMP)
		return "HDD Temperature";

	if ((type & SENSOR_TYPE_CPU_USAGE) == SENSOR_TYPE_CPU_USAGE)
		return "CPU Usage";

	if (type & SENSOR_TYPE_TEMP)
		return "Temperature";

	if (type & SENSOR_TYPE_RPM)
		return "Fan";

	if (type & SENSOR_TYPE_CPU)
		return "CPU";

	if (type & SENSOR_TYPE_REMOTE)
		return "Remote";

	if (type & SENSOR_TYPE_MEMORY)
		return "Memory";

	return "N/A";
}


const char *psensor_type_to_unit_str(unsigned int type, int use_celsius)
{
	if (is_temp_type(type)) {
		if (use_celsius)
			return "\302\260C";
		else
			return "\302\260F";
	} else if (is_fan_type(type)) {
		return _("RPM");
	} else if (type & SENSOR_TYPE_CPU_USAGE) {
		return _("%");
	} else {
		return _("N/A");
	}
}

void psensor_list_update_measures(struct psensor **sensors)
{
	lmsensor_psensor_list_update(sensors);

#ifdef HAVE_GTOP
	cpu_psensor_list_update(sensors);
	mem_psensor_list_update(sensors);
#endif

	if (psensor_list_contains_type(sensors, SENSOR_TYPE_HDDTEMP))
		hddtemp_psensor_list_update(sensors);

#ifdef HAVE_ATASMART
	if (psensor_list_contains_type(sensors, SENSOR_TYPE_ATASMART))
		hdd_psensor_list_update(sensors);
#endif
}

void psensor_log_measures(struct psensor **sensors)
{
	if (log_level == LOG_DEBUG)
		while (*sensors) {
			log_debug("Measure: %s %.2f",
				   (*sensors)->name,
				   psensor_get_current_value(*sensors));

			sensors++;
		}
}

void psensor_init()
{
	lmsensor_init();
}

void psensor_cleanup()
{
	lmsensor_cleanup();
}

struct psensor **psensor_list_copy(struct psensor **sensors)
{
	struct psensor **result;
	int n, i;

	n = psensor_list_size(sensors);
	result = malloc((n+1) * sizeof(struct psensor *));
	for (i = 0; i < n; i++)
		result[i] = sensors[i];
	result[n] = NULL;

	return result;
}

char *
psensor_current_value_to_str(const struct psensor *s, unsigned int use_celsius)
{
	return psensor_value_to_str(s->type,
				    psensor_get_current_value(s),
				    use_celsius);
}

struct psensor **psensor_list_filter_graph_enabled(struct psensor **sensors)
{
	int n, i;
	struct psensor **result, **cur, *s;

	if (!sensors)
		return NULL;

	n = psensor_list_size(sensors);
	result = malloc((n+1) * sizeof(struct psensor *));

	for (cur = sensors, i = 0; *cur; cur++) {
		s = *cur;

		if (s->graph_enabled)
			result[i++] = s;
	}

	result[i] = NULL;

	return result;
}
