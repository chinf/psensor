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

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>

#include <nvidia.h>
#include <psensor.h>

static Display *display;

static const char *PROVIDER_NAME = "nvctrl";

static void set_nvidia_id(struct psensor *s, int id)
{
	*(int *)s->provider_data = id;
}

static int get_nvidia_id(struct psensor *s)
{
	return *(int *)s->provider_data;
}

static char *get_product_name(int id, int type)
{
	char *name;
	Bool res;

	if (type & SENSOR_TYPE_FAN)
		return strdup("NVIDIA");

	res = XNVCTRLQueryTargetStringAttribute(display,
						NV_CTRL_TARGET_TYPE_GPU,
						id,
						0,
						NV_CTRL_STRING_PRODUCT_NAME,
						&name);
	if (res == True) {
		if (strcmp(name, "Unknown"))
			return name;

		log_err(_("%s: Unknown NVIDIA product name for GPU %d"),
			PROVIDER_NAME,
			id);
		free(name);
	} else {
		log_err(_("%s: "
			  "Failed to retrieve NVIDIA product name for GPU %d"),
			PROVIDER_NAME,
			id);
	}

	return strdup("NVIDIA");
}

static double get_att(int target, int id, int att)
{
	Bool res;
	int temp;

	res = XNVCTRLQueryTargetAttribute(display, target, id, 0, att, &temp);

	if (res == True)
		return temp;

	return UNKNOWN_DBL_VALUE;
}

static double get_usage_att(char *atts, const char *att)
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

static const char *get_nvidia_type_str(int type)
{
	if (type & SENSOR_TYPE_GRAPHICS)
		return "graphics";

	if (type & SENSOR_TYPE_VIDEO)
		return "video";

	if (type & SENSOR_TYPE_MEMORY)
		return "memory";

	if (type & SENSOR_TYPE_PCIE)
		return "PCIe";

	if (type & SENSOR_TYPE_AMBIENT)
		return "ambient";

	if (type & SENSOR_TYPE_TEMP)
		return "temp";

	if (type & SENSOR_TYPE_FAN) {
		if (type & SENSOR_TYPE_RPM)
			return "fan rpm";

		return "fan level";
	}

	return "unknown";
}

static double get_usage(int id, int type)
{
	const char *stype;
	char *atts;
	double v;
	Bool res;

	stype = get_nvidia_type_str(type);

	if (!stype)
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

static double get_value(int id, int type)
{
	int att;

	if (type & SENSOR_TYPE_TEMP) {
		if (type & SENSOR_TYPE_AMBIENT)
			att = NV_CTRL_AMBIENT_TEMPERATURE;
		else
			att = NV_CTRL_GPU_CORE_TEMPERATURE;

		return get_att(NV_CTRL_TARGET_TYPE_GPU, id, att);
	} else if (type & SENSOR_TYPE_FAN) {
		if (type & SENSOR_TYPE_RPM)
			return get_att(NV_CTRL_TARGET_TYPE_COOLER,
				       id,
				       NV_CTRL_THERMAL_COOLER_SPEED);
		else /* SENSOR_TYPE_PERCENT */
			return get_att(NV_CTRL_TARGET_TYPE_COOLER,
				       id,
				       NV_CTRL_THERMAL_COOLER_LEVEL);
	} else { /* SENSOR_TYPE_PERCENT */
		return get_usage(id, type);
	}
}

static void update(struct psensor *sensor)
{
	double v;
	int id;

	id = get_nvidia_id(sensor);

	v = get_value(id, sensor->type);

	if (v == UNKNOWN_DBL_VALUE)
		log_err(_("%s: Failed to retrieve measure of type %x "
			  "for NVIDIA GPU %d"),
			PROVIDER_NAME,
			sensor->type,
			id);
	psensor_set_current_value(sensor, v);
}

static int check_sensor(int id, int type)
{
	return get_value(id, type) != UNKNOWN_DBL_VALUE;
}

static char *i2str(int i)
{
	char *str;
	size_t n;

	/* second +1 to avoid issue about the conversion of a double
	 * to a lower int
	 */
	n = 1 + (ceil(log10(INT_MAX)) + 1) + 1;

	str = malloc(n);
	snprintf(str, n, "%d", i);

	return str;
}

static struct psensor *create_nvidia_sensor(int id, int subtype, int value_len)
{
	char *pname, *name, *strnid, *sid;
	const char *stype;
	int type;
	size_t n;
	struct psensor *s;
	double v;

	type = SENSOR_TYPE_NVCTRL | subtype;

	if (!check_sensor(id, type))
		return NULL;

	pname = get_product_name(id, type);
	strnid = i2str(id);
	stype = get_nvidia_type_str(type);

	n = strlen(pname) + 1 + strlen(strnid) + 1 + strlen(stype) + 1;
	name = malloc(n);
	sprintf(name, "%s %s %s", pname, strnid, stype);

	sid = malloc(strlen(PROVIDER_NAME) + 1 + strlen(name) + 1);
	sprintf(sid, "%s %s", PROVIDER_NAME, name);

	s = psensor_create(sid, name, pname, type, value_len);
	s->provider_data = malloc(sizeof(int));
	set_nvidia_id(s, id);

	if ((type & SENSOR_TYPE_GPU) && (type & SENSOR_TYPE_TEMP)) {
		v = get_att(NV_CTRL_TARGET_TYPE_GPU,
			    id,
			    NV_CTRL_GPU_CORE_THRESHOLD);
		s->max = v;
	}

	free(strnid);

	return s;
}

static int init(void)
{
	int evt, err;

	display = XOpenDisplay(NULL);

	if (!display) {
		log_err(_("%s: Cannot open connection to X11 server."),
			PROVIDER_NAME);
		return 0;
	}

	if (XNVCTRLQueryExtension(display, &evt, &err))
		return 1;

	log_err(_("%s: Failed to retrieve NVIDIA information."),
		PROVIDER_NAME);

	return 0;
}

void nvidia_psensor_list_update(struct psensor **sensors)
{
	struct psensor *s;

	while (*sensors) {
		s = *sensors;

		if (!(s->type & SENSOR_TYPE_REMOTE)
		    && s->type & SENSOR_TYPE_NVCTRL)
			update(s);

		sensors++;
	}
}

static void add(struct psensor ***sensors, int id, int type, int values_len)
{
	struct psensor *s;

	s = create_nvidia_sensor(id, type, values_len);

	if (s)
		psensor_list_append(sensors, s);
}

void nvidia_psensor_list_append(struct psensor ***ss, int values_len)
{
	int i, n, utype;
	Bool ret;

	if (!init())
		return;

	ret = XNVCTRLQueryTargetCount(display, NV_CTRL_TARGET_TYPE_GPU, &n);
	if (ret == True) {
		for (i = 0; i < n; i++) {
			add(ss,
			    i,
			    SENSOR_TYPE_GPU | SENSOR_TYPE_TEMP,
			    values_len);

			utype = SENSOR_TYPE_GPU | SENSOR_TYPE_PERCENT;
			add(ss, i, utype | SENSOR_TYPE_AMBIENT, values_len);
			add(ss, i, utype | SENSOR_TYPE_GRAPHICS, values_len);
			add(ss, i, utype | SENSOR_TYPE_VIDEO, values_len);
			add(ss, i, utype | SENSOR_TYPE_MEMORY, values_len);
			add(ss, i, utype | SENSOR_TYPE_PCIE, values_len);
		}
	}

	ret = XNVCTRLQueryTargetCount(display, NV_CTRL_TARGET_TYPE_COOLER, &n);
	if (ret == True) {
		log_fct("%s: Number of fans: %d", PROVIDER_NAME, n);
		for (i = 0; i < n; i++) {
			utype = SENSOR_TYPE_FAN | SENSOR_TYPE_RPM;
			if (check_sensor(i, utype))
				add(ss, i, utype, values_len);

			utype = SENSOR_TYPE_FAN | SENSOR_TYPE_PERCENT;
			if (check_sensor(i, utype))
				add(ss, i, utype, values_len);
		}
	} else {
		log_err(_("%s: Failed to retrieve number of fans."),
			PROVIDER_NAME);
	}
}

void nvidia_cleanup(void)
{
	if (display) {
		XCloseDisplay(display);
		display = NULL;
	}
}
