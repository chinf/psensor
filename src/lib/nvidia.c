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

#include <psensor.h>

Display *display;

static char *get_product_name(int id)
{
	char *name;
	Bool res;

	res = XNVCTRLQueryTargetStringAttribute(display,
						NV_CTRL_TARGET_TYPE_GPU,
						id,
						0,
						NV_CTRL_STRING_PRODUCT_NAME,
						&name);
	if (res == True) {
		if (strcmp(name, "Unknown")) {
			return name;
		} else {
			log_err(_("Unknown NVIDIA product name for GPU %d"),
				id);
			free(name);
		}
	} else {
		log_err(_("Failed to retrieve NVIDIA product name for GPU %d"),
			id);
	}

	return strdup("NVIDIA");
}

static double get_temp(int id)
{
	Bool res;
	int temp;

	res = XNVCTRLQueryTargetAttribute(display,
					  NV_CTRL_TARGET_TYPE_GPU,
					  id,
					  0,
					  NV_CTRL_GPU_CORE_TEMPERATURE,
					  &temp);

	if (res == True)
		return temp;
	else
		return UNKNOWN_DBL_VALUE;
}

static double get_ambient_temp(int id)
{
	Bool res;
	int temp;

	res = XNVCTRLQueryTargetAttribute(display,
					  NV_CTRL_TARGET_TYPE_GPU,
					  id,
					  0,
					  NV_CTRL_AMBIENT_TEMPERATURE,
					  &temp);

	if (res == True)
		return temp;
	else
		return UNKNOWN_DBL_VALUE;
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

static double get_usage(int id, int type)
{
	char *stype, *atts;
	double v;
	Bool res;

	if (type & SENSOR_TYPE_GRAPHICS)
		stype = "graphics";
	else if (type & SENSOR_TYPE_VIDEO)
		stype = "video";
	else if (type & SENSOR_TYPE_MEMORY)
		stype = "memory";
	else if (type & SENSOR_TYPE_PCIE)
		stype = "PCIe";
	else
		return UNKNOWN_DBL_VALUE;

	res = XNVCTRLQueryTargetStringAttribute(display,
						NV_CTRL_TARGET_TYPE_GPU,
						id,
						0,
						NV_CTRL_STRING_GPU_UTILIZATION,
						&atts);

	if (res != True)
		return UNKNOWN_DBL_VALUE;

	v = get_usage_att(atts, stype);

	free(atts);

	return v;
}

static void update(struct psensor *sensor)
{
	double v;

	if (sensor->type & SENSOR_TYPE_TEMP) {
		if (sensor->type & SENSOR_TYPE_AMBIENT)
			v = get_ambient_temp(sensor->nvidia_id);
		else
			v = get_temp(sensor->nvidia_id);
	} else { /* SENSOR_TYPE_USAGE */
		v = get_usage(sensor->nvidia_id, sensor->type);
	}

	if (v == UNKNOWN_DBL_VALUE)
		log_err(_("Failed to retrieve measure of type %x "
			  "for NVIDIA GPU %d"),
			sensor->type,
			sensor->nvidia_id);
	psensor_set_current_value(sensor, v);
}

static struct psensor *create_temp_sensor(int id, int subtype, int values_len)
{
	char name[200];
	char *sid, *pname;
	struct psensor *s;
	int t;

	pname = get_product_name(id);

	if (subtype & SENSOR_TYPE_AMBIENT)
		sprintf(name, "%s %d ambient", pname, id);
	else
		sprintf(name, "%s %d", pname, id);
	free(pname);

	sid = malloc(strlen("NVIDIA") + 1 + strlen(name) + 1);
	sprintf(sid, "NVIDIA %s", name);

	t = SENSOR_TYPE_NVCTRL | SENSOR_TYPE_GPU | SENSOR_TYPE_TEMP | subtype;

	s = psensor_create(sid,
			   strdup(name),
			   strdup(_("NVIDIA GPU")),
			   t,
			   values_len);

	s->nvidia_id = id;

	return s;
}

static struct psensor *create_usage_sensor(int id,
					   int subtype,
					   int values_len)
{
	char name[200];
	char *sid;
	struct psensor *s;
	int t;

	if (subtype & SENSOR_TYPE_GRAPHICS)
		sprintf(name, "GPU%d graphics", id);
	else if (subtype & SENSOR_TYPE_MEMORY)
		sprintf(name, "GPU%d memory", id);
	else if (subtype & SENSOR_TYPE_VIDEO)
		sprintf(name, "GPU%d video", id);
	else /* if (subtype & SENSOR_TYPE_PCIE) */
		sprintf(name, "GPU%d PCIe", id);


	sid = malloc(strlen("NVIDIA") + 1 + strlen(name) + 1);
	sprintf(sid, "NVIDIA %s", name);

	t = SENSOR_TYPE_NVCTRL | SENSOR_TYPE_GPU | SENSOR_TYPE_USAGE | subtype;

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

		if (s->type & SENSOR_TYPE_NVCTRL)
			update(s);

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
		s = create_temp_sensor(i, 0, values_len);
		tmp = psensor_list_add(ss, s);
		if (ss != tmp)
			free(ss);

		ss = tmp;
		s = create_temp_sensor(i, SENSOR_TYPE_AMBIENT, values_len);
		tmp = psensor_list_add(ss, s);
		if (ss != tmp)
			free(ss);

		ss = tmp;
		s = create_usage_sensor(i, SENSOR_TYPE_GRAPHICS, values_len);
		tmp = psensor_list_add(ss, s);
		if (ss != tmp)
			free(ss);

		ss = tmp;
		s = create_usage_sensor(i, SENSOR_TYPE_VIDEO, values_len);
		tmp = psensor_list_add(ss, s);
		if (ss != tmp)
			free(ss);

		ss = tmp;
		s = create_usage_sensor(i, SENSOR_TYPE_MEMORY, values_len);
		tmp = psensor_list_add(ss, s);
		if (ss != tmp)
			free(ss);

		ss = tmp;
		s = create_usage_sensor(i, SENSOR_TYPE_PCIE, values_len);
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
