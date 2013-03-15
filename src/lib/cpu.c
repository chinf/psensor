/*
 * Copyright (C) 2010-2013 jeanfi@gmail.com
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

#include <string.h>

#include <glibtop/cpu.h>

#include "cpu.h"

static glibtop_cpu *cpu;
static float last_used;
static float last_total;

struct psensor *create_cpu_usage_sensor(int measures_len)
{
	char *label;
	int type;
	char *id;
	struct psensor *psensor;

	id = strdup("cpu usage");
	label = strdup("cpu usage");
	type = SENSOR_TYPE_CPU_USAGE;

	psensor = psensor_create(id, label, strdup("CPU"), type, measures_len);

	return psensor;
}

struct psensor * *
cpu_psensor_list_add(struct psensor **sensors, int measures_len)
{
	struct psensor *s;

	s = create_cpu_usage_sensor(measures_len);

	return psensor_list_add(sensors, s);
}

static double get_usage()
{
	unsigned long int used = 0;
	unsigned long int dt;
	double cpu_rate = UNKNOWN_DBL_VALUE;

	if (!cpu)
		cpu = malloc(sizeof(glibtop_cpu));

	glibtop_get_cpu(cpu);

	used = cpu->user + cpu->nice + cpu->sys;

	dt = cpu->total - last_total;

	if (dt)
		cpu_rate = 100 * (used - last_used) / dt;

	last_used = used;
	last_total = cpu->total;

	return cpu_rate;
}

void cpu_usage_sensor_update(struct psensor *s)
{
	psensor_set_current_value(s, get_usage());
}

void cpu_psensor_list_update(struct psensor **sensors)
{
	struct psensor **ss, *s;

	ss = sensors;
	while (*ss) {
		s = *ss;

		if (s->type == SENSOR_TYPE_CPU_USAGE)
			cpu_usage_sensor_update(s);

		ss++;
	}
}

void cpu_cleanup()
{
	if (cpu)
		free(cpu);
}
