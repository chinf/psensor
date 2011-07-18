/*
    Copyright (C) 2010-2011 thgreasi@gmail.com

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
#ifndef LINUX
#define LINUX 1
#endif
#ifdef HAVE_LIBATIADL
	/* AMD id for the aticonfig */
	int amd_id;
#endif

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <adl_sdk.h>
#include <dlfcn.h>	/* dyopen, dlsym, dlclose */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	/* memeset */

#include "psensor.h"

/* Definitions of the used function pointers.
	 Add more if you use other ADL APIs */
typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_MAIN_CONTROL_DESTROY)();
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int *);
typedef int (*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int (*ADL_ADAPTER_ACTIVE_GET) (int, int*);
typedef int (*ADL_OVERDRIVE5_TEMPERATURE_GET) (int, int, ADLTemperature*);
typedef int (*ADL_OVERDRIVE5_FANSPEED_GET) (int, int, ADLFanSpeedValue*);

static ADL_MAIN_CONTROL_CREATE            adl_main_control_create;
static ADL_MAIN_CONTROL_DESTROY           adl_main_control_destroy;
static ADL_ADAPTER_NUMBEROFADAPTERS_GET   adl_adapter_numberofadapters_get;
static ADL_ADAPTER_ADAPTERINFO_GET        adl_adapter_adapterinfo_get;
static ADL_ADAPTER_ACTIVE_GET             adl_adapter_active_get;
static ADL_OVERDRIVE5_TEMPERATURE_GET     adl_overdrive5_temperature_get;
static ADL_OVERDRIVE5_FANSPEED_GET        adl_overdrive5_fanspeed_get;

static void *hdll;
static int adl_main_control_done;
static int *active_amd_adapters;

/* Memory allocation function */
static void __stdcall *adl_main_memory_alloc(int isize)
{
    void *lpbuffer = malloc(isize);
    return lpbuffer;
}

/* Optional Memory de-allocation function */
static void __stdcall adl_main_memory_free(void** lpbuffer)
{
	if (*lpbuffer) {
		free(*lpbuffer);
		*lpbuffer = NULL;
	}
}

static void *getprocaddress(void * plibrary, const char * name)
{
    return dlsym(plibrary, name);
}

/*
  Returns the temperature (Celcius) of an AMD/Ati GPU.
*/
static int get_temp(struct psensor *sensor)
{
	ADLTemperature temperature;

	temperature.iSize = sizeof(ADLTemperature);
	temperature.iTemperature = -273;
	if (ADL_OK != adl_overdrive5_temperature_get(sensor->amd_id,
		 0, &temperature))
		return -273;

	return temperature.iTemperature/1000;
}

static int get_fanspeed(struct psensor *sensor)
{
	ADLFanSpeedValue fanspeedvalue;

	fanspeedvalue.iSize = sizeof(ADLFanSpeedValue);
	fanspeedvalue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_RPM;
	fanspeedvalue.iFanSpeed = -1;
	if (ADL_OK != adl_overdrive5_fanspeed_get(sensor->amd_id,
		 0, &fanspeedvalue))
		return -1;

	return fanspeedvalue.iFanSpeed;
}

static struct psensor *create_sensor(int id, int values_len)
{
	char name[200];
	char *sid;
	int sensor_type;

	struct psensor *s;

	if (id & 1) {/* odd number ids represent fan sensors */
		id = id >> 1;
		sprintf(name, "GPU%dfan", id);
		sensor_type = SENSOR_TYPE_AMD_FAN;
	} else {/* even number ids represent temperature sensors */
		id = id >> 1;
		sprintf(name, "GPU%dtemp", id);
		sensor_type = SENSOR_TYPE_AMD_TEMP;
	}

	sid = malloc(strlen("AMD") + 1 + strlen(name) + 1);
	sprintf(sid, "AMD %s", name);

	s = psensor_create(sid, strdup(name),
			   sensor_type, values_len);

	s->amd_id = active_amd_adapters[id];

	return s;
}

/*
  Returns the number of AMD/Ati GPU sensors (temperature and fan speed).

  Return 0 if no AMD/Ati gpus or cannot get information.
*/
static int init()
{
	LPAdapterInfo lpadapterinfo = NULL;
	int  i, inumberadapters, inumberadaptersactive = 0;
	int lpstatus, iadapterindex;

	hdll = NULL;
	adl_main_control_done = 0;
	active_amd_adapters = NULL;
	hdll = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);


	if (!hdll) {
		fprintf(stderr,
			_("ERROR: ADL library not found!\n"));
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
	adl_overdrive5_temperature_get = (ADL_OVERDRIVE5_TEMPERATURE_GET)
		 getprocaddress(hdll, "ADL_Overdrive5_Temperature_Get");
	adl_overdrive5_fanspeed_get = (ADL_OVERDRIVE5_FANSPEED_GET)
		 getprocaddress(hdll, "ADL_Overdrive5_FanSpeed_Get");
	if (!adl_main_control_create ||
		!adl_main_control_destroy ||
		!adl_adapter_numberofadapters_get ||
		!adl_adapter_adapterinfo_get ||
		!adl_overdrive5_temperature_get ||
		!adl_overdrive5_fanspeed_get) {
		fprintf(stderr,
			_("ERROR: ADL's API is missing!\n"));
		return 0;
	}

	/* Initialize ADL. The second parameter is 1, which means:
	   retrieve adapter information only for adapters that
	   are physically present and enabled in the system */
	if (ADL_OK != adl_main_control_create(adl_main_memory_alloc, 1)) {
		fprintf(stderr,
			_("ERROR: ADL Initialization Error!\n"));
		return 0;
	}
	adl_main_control_done = 1;

	/* Obtain the number of adapters for the system */
	if (ADL_OK != adl_adapter_numberofadapters_get(&inumberadapters)) {
		fprintf(stderr,
			_("ERROR: Cannot get the number of adapters!\n"));
		return 0;
	}

	if (!inumberadapters)
		return 0;

    lpadapterinfo = malloc(sizeof(AdapterInfo) * inumberadapters);
    memset(lpadapterinfo, '\0', sizeof(AdapterInfo) * inumberadapters);

    /* Get the AdapterInfo structure for all adapters in the system */
    adl_adapter_adapterinfo_get(lpadapterinfo,
		 sizeof(AdapterInfo) * inumberadapters);

    /* Repeat for all available adapters in the system */
    for (i = 0; i < inumberadapters; i++) {
		iadapterindex = lpadapterinfo[i].iAdapterIndex;
		if (ADL_OK != adl_adapter_active_get(iadapterindex, &lpstatus))
			continue;
		if (lpstatus != ADL_TRUE)
			/* count only if the adapter is active */
			continue;

		if (!active_amd_adapters) {
			active_amd_adapters = (int *) malloc(sizeof(int));
			inumberadaptersactive = 1;
		} else {
			++inumberadaptersactive;
			active_amd_adapters =
				 (int *) realloc(active_amd_adapters,
				 sizeof(int)*inumberadaptersactive);
		}
		active_amd_adapters[inumberadaptersactive-1] = iadapterindex;
    }

    adl_main_memory_free((void **) &lpadapterinfo);

	/* Each Adapter has one GPU temperature
	 sensor and one fan control sensor */
	return 2*inumberadaptersactive;
}

void amd_psensor_list_update(struct psensor **sensors)
{
	struct psensor **ss, *s;

	ss = sensors;
	while (*ss) {
		s = *ss;

		if (s->type == SENSOR_TYPE_AMD_TEMP)
			psensor_set_current_value(s, get_temp(s));
		else if (s->type == SENSOR_TYPE_AMD_FAN)
			psensor_set_current_value(s, get_fanspeed(s));

		ss++;
	}
}

struct psensor * *
amd_psensor_list_add(struct psensor **sensors, int values_len)
{
	int i, n;
	struct psensor **tmp, **ss, *s;

	n = init();

	ss = sensors;
	for (i = 0; i < n; i++) {
		s = create_sensor(i, values_len);

		tmp = psensor_list_add(ss, s);

		if (ss != tmp)
			free(ss);

		ss = tmp;
	}

	if (active_amd_adapters) {
		free(active_amd_adapters);
		active_amd_adapters = NULL;
	}

	return ss;
}

void amd_cleanup()
{
	if (hdll) {
		if (adl_main_control_done)
			adl_main_control_destroy();
		dlclose(hdll);
	}
	if (active_amd_adapters) {
		free(active_amd_adapters);
		active_amd_adapters = NULL;
	}
}
