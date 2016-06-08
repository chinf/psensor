/*
 * Copyright (C) 2010-2011 thgreasi@gmail.com, jeanfi@gmail.com
 * Copyright (C) 2012-2014 jeanfi@gmail.com
 *
 * GPU usage is a contribution of MestreLion
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
#ifndef LINUX
#define LINUX 1
#endif

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <adl_sdk.h>

#include <psensor.h>

typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_MAIN_CONTROL_DESTROY)();
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int *);
typedef int (*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int (*ADL_ADAPTER_ACTIVE_GET) (int, int*);
typedef int (*ADL_OD5_TEMPERATURE_GET) (int, int, ADLTemperature*);
typedef int (*ADL_OD5_FANSPEED_GET) (int, int, ADLFanSpeedValue*);
typedef int (*ADL_OD5_CURRENTACTIVITY_GET) (int, ADLPMActivity*);

static ADL_MAIN_CONTROL_CREATE adl_main_control_create;
static ADL_MAIN_CONTROL_DESTROY adl_main_control_destroy;
static ADL_ADAPTER_NUMBEROFADAPTERS_GET adl_adapter_numberofadapters_get;
static ADL_ADAPTER_ADAPTERINFO_GET adl_adapter_adapterinfo_get;
static ADL_ADAPTER_ACTIVE_GET adl_adapter_active_get;
static ADL_OD5_TEMPERATURE_GET adl_od5_temperature_get;
static ADL_OD5_FANSPEED_GET adl_od5_fanspeed_get;
static ADL_OD5_CURRENTACTIVITY_GET adl_od5_currentactivity_get;

static void *hdll;
static int adl_main_control_done;
static int *active_adapters;

static void __stdcall *adl_main_memory_alloc(int isize)
{
	void *lpbuffer = malloc(isize);
	return lpbuffer;
}

static void *getprocaddress(void *plibrary, const char *name)
{
	return dlsym(plibrary, name);
}

/* Returns the temperature (Celsius) of an AMD/ATI GPU. */
static double get_temp(struct psensor *sensor)
{
	ADLTemperature v;

	v.iSize = sizeof(ADLTemperature);
	v.iTemperature = -273;

	if (adl_od5_temperature_get(sensor->amd_id, 0, &v) == ADL_OK)
		return v.iTemperature/1000;

	return UNKNOWN_DBL_VALUE;
}

static double get_fanspeed(struct psensor *sensor)
{
	ADLFanSpeedValue v;

	v.iSize = sizeof(ADLFanSpeedValue);
	v.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_RPM;
	v.iFanSpeed = -1;

	if (adl_od5_fanspeed_get(sensor->amd_id, 0, &v) == ADL_OK)
		return v.iFanSpeed;

	return UNKNOWN_DBL_VALUE;
}

static double get_usage(struct psensor *sensor)
{
	ADLPMActivity v;

	v.iSize = sizeof(ADLPMActivity);

	if (adl_od5_currentactivity_get(sensor->amd_id, &v) == ADL_OK)
		return v.iActivityPercent;

	return UNKNOWN_DBL_VALUE;
}

static struct psensor *create_sensor(int id, int type, int values_len)
{
	char name[200];
	char *sid;
	int sensor_type;
	struct psensor *s;

	sensor_type = SENSOR_TYPE_ATIADL;
	switch (type) {
	/* Fan rotation speed */
	case 0:
		sprintf(name, "AMD GPU%d Fan", id);
		sensor_type |= SENSOR_TYPE_FAN | SENSOR_TYPE_RPM;
		break;

	/* Temperature */
	case 1:
		sprintf(name, "AMD GPU%d Temperature", id);
		sensor_type |= SENSOR_TYPE_GPU | SENSOR_TYPE_TEMP;
		break;

	/* GPU Usage (Activity/Load %) */
	case 2:
		sprintf(name, "AMD GPU%d Usage", id);
		sensor_type |= SENSOR_TYPE_GPU | SENSOR_TYPE_PERCENT;
		break;
	}

	sid = malloc(strlen("amd") + 1 + strlen(name) + 1);
	sprintf(sid, "amd %s", name);

	s = psensor_create(sid,
			   strdup(name),
			   strdup("AMD/ATI GPU"),
			   sensor_type,
			   values_len);

	s->amd_id = active_adapters[id];

	return s;
}

/*
 * Returns the number of active AMD/ATI GPU adapters
 *
 * Return 0 if no AMD/ATI GPUs or cannot get information.
 */
static int init(void)
{
	LPAdapterInfo lpadapterinfo;
	int i, inumberadapters, inumberadaptersactive, lpstatus, iadapterindex;

	adl_main_control_done = 0;
	inumberadaptersactive = 0;
	active_adapters = NULL;
	lpadapterinfo = NULL;

	hdll = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
	if (!hdll) {
		log_debug(_("AMD: cannot found ADL library."));
		return 0;
	}

	adl_main_control_create = (ADL_MAIN_CONTROL_CREATE)
		getprocaddress(hdll, "ADL_Main_Control_Create");
	adl_main_control_destroy = (ADL_MAIN_CONTROL_DESTROY)
		getprocaddress(hdll, "ADL_Main_Control_Destroy");
	adl_adapter_numberofadapters_get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)
		getprocaddress(hdll, "ADL_Adapter_NumberOfAdapters_Get");
	adl_adapter_adapterinfo_get = (ADL_ADAPTER_ADAPTERINFO_GET)
		getprocaddress(hdll, "ADL_Adapter_AdapterInfo_Get");
	adl_adapter_active_get = (ADL_ADAPTER_ACTIVE_GET)
		getprocaddress(hdll, "ADL_Adapter_Active_Get");
	adl_od5_temperature_get = (ADL_OD5_TEMPERATURE_GET)
		getprocaddress(hdll, "ADL_Overdrive5_Temperature_Get");
	adl_od5_fanspeed_get = (ADL_OD5_FANSPEED_GET)
		getprocaddress(hdll, "ADL_Overdrive5_FanSpeed_Get");
	adl_od5_currentactivity_get = (ADL_OD5_CURRENTACTIVITY_GET)
		getprocaddress(hdll, "ADL_Overdrive5_CurrentActivity_Get");
	if (!adl_main_control_create
	    || !adl_main_control_destroy
	    || !adl_adapter_numberofadapters_get
	    || !adl_adapter_adapterinfo_get
	    || !adl_od5_temperature_get
	    || !adl_od5_fanspeed_get
	    || !adl_od5_currentactivity_get) {
		log_err(_("AMD: missing ADL's API."));
		return 0;
	}

	/*
	 * 1 in 2nd parameter means retrieve adapter information only
	 * for adapters that are physically present and enabled in the
	 * system
	 */
	if (adl_main_control_create(adl_main_memory_alloc, 1) != ADL_OK) {
		log_err(_("AMD: failed to initialize ADL."));
		return 0;
	}
	adl_main_control_done = 1;

	if (adl_adapter_numberofadapters_get(&inumberadapters) != ADL_OK) {
		log_err(_("AMD: cannot get the number of adapters."));
		return 0;
	}

	if (!inumberadapters)
		return 0;

	lpadapterinfo = malloc(sizeof(AdapterInfo) * inumberadapters);
	memset(lpadapterinfo, '\0', sizeof(AdapterInfo) * inumberadapters);

	adl_adapter_adapterinfo_get(lpadapterinfo,
				    sizeof(AdapterInfo) * inumberadapters);

	for (i = 0; i < inumberadapters; i++) {

		iadapterindex = lpadapterinfo[i].iAdapterIndex;

		if (adl_adapter_active_get(iadapterindex, &lpstatus) != ADL_OK)
			continue;
		if (lpstatus != ADL_TRUE)
			continue;

		if (!active_adapters) {
			active_adapters = (int *) malloc(sizeof(int));
			inumberadaptersactive = 1;
		} else {
			++inumberadaptersactive;
			active_adapters = (int *)realloc
				(active_adapters,
				 sizeof(int)*inumberadaptersactive);

			if (!active_adapters)
				exit(EXIT_FAILURE);
		}
		active_adapters[inumberadaptersactive-1] = iadapterindex;
	}

	free(lpadapterinfo);

	log_debug(_("Number of AMD/ATI adapters: %d"), inumberadapters);
	log_debug(_("Number of active AMD/ATI adapters: %d"),
		  inumberadaptersactive);

	return inumberadaptersactive;
}

/* Called regularly to update sensors values */
void amd_psensor_list_update(struct psensor **sensors)
{
	struct psensor **ss, *s;

	ss = sensors;
	while (*ss) {
		s = *ss;

		if (s->type & SENSOR_TYPE_ATIADL) {
			if (s->type & SENSOR_TYPE_TEMP)
				psensor_set_current_value(s, get_temp(s));
			else if (s->type & SENSOR_TYPE_RPM)
				psensor_set_current_value(s, get_fanspeed(s));
			else if (s->type & SENSOR_TYPE_PERCENT)
				psensor_set_current_value(s, get_usage(s));
		}

		ss++;
	}
}

/* Entry point for AMD sensors */
void amd_psensor_list_append(struct psensor ***sensors, int values_len)
{
	int i, j, n;
	struct psensor *s;

	n = init();

	for (i = 0; i < n; i++)
		/* Each GPU Adapter has 3 sensors: temp, fan speed and usage */
		for (j = 0; j < 3; j++) {
			s = create_sensor(i, j, values_len);
			psensor_list_append(sensors, s);
		}
}

void amd_cleanup(void)
{
	if (hdll) {
		if (adl_main_control_done)
			adl_main_control_destroy();
		dlclose(hdll);
	}

	if (active_adapters) {
		free(active_adapters);
		active_adapters = NULL;
	}
}
