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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>


#include "psensor.h"

Display *nvidia_sensors_dpy;

int nvidia_get_sensor_temp(struct psensor *sensor)
{
	int temp;
	Bool res;

	res = XNVCTRLQueryTargetAttribute(nvidia_sensors_dpy,
					  NV_CTRL_TARGET_TYPE_GPU,
					  sensor->nvidia_id,
					  0,
					  NV_CTRL_GPU_CORE_TEMPERATURE, &temp);

	if (res == True) {
		return temp;
	} else {
		fprintf(stderr,
			_("ERROR: failed to retrieve nvidia temperature\n"));
		return 0;
	}
}

struct psensor *nvidia_create_sensor(int id, int values_max_length)
{
	char name[200];
	char *sid;
	struct psensor *s;

	sprintf(name, "GPU%d", id);

	sid = malloc(strlen("nvidia") + 1 + strlen(name) + 1);
	sprintf(sid, "nvidia %s", name);

	s = psensor_create(sid, strdup(name),
			   SENSOR_TYPE_NVIDIA, values_max_length);

	s->nvidia_id = id;

	return s;
}

int nvidia_init()
{
	int event_base, error_base;
	int num_gpus;

	nvidia_sensors_dpy = XOpenDisplay(NULL);

	if (!nvidia_sensors_dpy) {
		fprintf(stderr, _("ERROR: nvidia initialization failure\n"));
		return 0;
	}

	if (XNVCTRLQueryExtension(nvidia_sensors_dpy, &event_base,
				  &error_base)) {
		if (XNVCTRLQueryTargetCount(nvidia_sensors_dpy,
					    NV_CTRL_TARGET_TYPE_GPU,
					    &num_gpus)) {
			return num_gpus;
		}

	}

	fprintf(stderr, _("ERROR: nvidia initialization failure: %d\n"),
		error_base);

	return 0;
}

void nvidia_psensor_list_update(struct psensor **sensors)
{
	struct psensor **s_ptr = sensors;

	while (*s_ptr) {
		struct psensor *sensor = *s_ptr;

		if (sensor->type == SENSOR_TYPE_NVIDIA) {
			int val = nvidia_get_sensor_temp(sensor);

			psensor_set_current_value(sensor, (double)val);
		}

		s_ptr++;
	}
}

struct psensor **nvidia_psensor_list_add(struct psensor **sensors,
					 int values_max_length)
{
	int i;
	int nvidia_gpus_count = nvidia_init();
	struct psensor **res = sensors;


	if (!nvidia_gpus_count) {
		fprintf(stderr,
			_("ERROR: "
			  "no nvidia chips or initialization failure\n"));
	}

	for (i = 0; i < nvidia_gpus_count; i++) {
		struct psensor *sensor
			= nvidia_create_sensor(i, values_max_length);

		struct psensor **tmp_psensors = psensor_list_add(res,
								 sensor);

		if (res != sensors)
			free(res);

		res = tmp_psensors;
	}

	return res;
}
