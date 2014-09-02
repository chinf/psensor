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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>

#include "psensor.h"

Display *display;

/* Returns the temperature (Celsius) of a NVIDIA GPU. */
static int get_temp(struct psensor *sensor)
{
	int temp;
	Bool res;

	res = XNVCTRLQueryTargetAttribute(display,
					  NV_CTRL_TARGET_TYPE_GPU,
					  sensor->nvidia_id,
					  0,
					  NV_CTRL_GPU_CORE_TEMPERATURE,
					  &temp);

	if (res == True)
		return temp;

	log_debug(_("NVIDIA proprietary driver not used or cannot "
		    "retrieve NVIDIA GPU temperature."));
	return 0;
}

static double get_usage_att(char *atts, char *att)
{
	char *c, *key, *strv, *s;
	size_t n;
	double v;

	c = atts;

	v = UNKNOWN_DBL_VALUE;
	while (*c) {
		s = c;
		n = 0;
		while (*c) {
			if (*c == '=')
				break;
			c++;
			n++;
		}

		key = strndup(s, n);

		if (*c)
			c++;

		n = 0;
		s = c;
		while (*c) {
			if (*c == ',')
				break;
			c++;
			n++;
		}

		strv = strndup(s, n);
		if (!strcmp(key, att))
			v = atoi(strv);

		free(key);
		free(strv);

		if (v != UNKNOWN_DBL_VALUE)
			break;

		while (*c && (*c == ' ' || *c == ','))
			c++;
	}

	return v;
}

static int get_usage(struct psensor *sensor)
{
	char *temp;
	Bool res;

	res = XNVCTRLQueryTargetStringAttribute(display,
					  NV_CTRL_TARGET_TYPE_GPU,
					  sensor->nvidia_id,
					  0,
					  NV_CTRL_STRING_GPU_UTILIZATION,
					  &temp);

	if (res == True)
		return get_usage_att(temp, "graphics");

	log_debug(_("NVIDIA proprietary driver not used or cannot "
		    "retrieve NVIDIA GPU usage."));
	return 0;
}

static struct psensor *create_temp_sensor(int id, int values_len)
{
	char name[200];
	char *sid;
	struct psensor *s;
	int t;

	sprintf(name, "GPU%d", id);

	sid = malloc(strlen("NVIDIA") + 1 + strlen(name) + 1);
	sprintf(sid, "NVIDIA %s", name);

	t = SENSOR_TYPE_NVCTRL | SENSOR_TYPE_GPU | SENSOR_TYPE_TEMP;

	s = psensor_create(sid,
			   strdup(name),
			   strdup(_("NVIDIA GPU")),
			   t,
			   values_len);

	s->nvidia_id = id;

	return s;
}

static struct psensor *create_usage_sensor(int id, int values_len)
{
	char name[200];
	char *sid;
	struct psensor *s;
	int t;

	sprintf(name, "GPU%d graphics", id);

	sid = malloc(strlen("NVIDIA") + 1 + strlen(name) + 1);
	sprintf(sid, "NVIDIA %s", name);

	t = SENSOR_TYPE_NVCTRL | SENSOR_TYPE_GPU | SENSOR_TYPE_USAGE;

	s = psensor_create(sid,
			   strdup(name),
			   strdup(_("NVIDIA GPU")),
			   t,
			   values_len);

	s->nvidia_id = id;

	return s;
}

/*
  Opens connection to X server and returns the number
  of NVIDIA GPUs.

  Return 0 if no NVIDIA gpus or cannot get information.
*/
static int init()
{
	int evt, err, n;

	display = XOpenDisplay(NULL);

	if (!display) {
		log_err(_("Cannot open connection to X11 server."));
		return 0;
	}

	if (XNVCTRLQueryExtension(display, &evt, &err) &&
	    XNVCTRLQueryTargetCount(display, NV_CTRL_TARGET_TYPE_GPU, &n))
		return n;

	log_err(_("Failed to retrieve NVIDIA information."));

	return 0;
}

void nvidia_psensor_list_update(struct psensor **sensors)
{
	struct psensor **ss, *s;

	ss = sensors;
	while (*ss) {
		s = *ss;

		if (s->type & SENSOR_TYPE_NVCTRL) {
			if (s->type & SENSOR_TYPE_TEMP)
				psensor_set_current_value(s, get_temp(s));
			else if (s->type & SENSOR_TYPE_USAGE)
				psensor_set_current_value(s, get_usage(s));
		}

		ss++;
	}
}

struct psensor **nvidia_psensor_list_add(struct psensor **sensors,
					 int values_len)
{
	int i, n;
	struct psensor **tmp, **ss, *s;

	n = init();

	ss = sensors;
	for (i = 0; i < n; i++) {
		s = create_temp_sensor(i, values_len);

		tmp = psensor_list_add(ss, s);

		if (ss != tmp)
			free(ss);

		ss = tmp;

		s = create_usage_sensor(i, values_len);

		tmp = psensor_list_add(ss, s);

		if (ss != tmp)
			free(ss);

		ss = tmp;
	}

	return ss;
}

void nvidia_cleanup()
{
	if (display) {
		XCloseDisplay(display);
		display = NULL;
	}
}
