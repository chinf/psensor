/*
 * Copyright (C) 2010-2012 jeanfi@gmail.com
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
#include <unity.h>

#include "psensor.h"

static int initialized;
static UnityLauncherEntry *psensor_entry;
static unsigned int last_visible = -1;

void ui_unity_launcher_entry_update(struct psensor **sensors,
				    unsigned int show)
{
	if (!initialized) {
		psensor_entry = unity_launcher_entry_get_for_desktop_file
			("psensor.desktop");

		unity_launcher_entry_set_count(psensor_entry, 0);
		initialized = 1;
	}

	if (last_visible != show) {
		if (show)
			unity_launcher_entry_set_count_visible(psensor_entry,
							       TRUE);
		else
			unity_launcher_entry_set_count_visible(psensor_entry,
							       FALSE);
		last_visible = show;
	}

	if (sensors && *sensors) {
		double v;

		v = psensor_get_max_current_value(sensors, SENSOR_TYPE_TEMP);

		unity_launcher_entry_set_count(psensor_entry, v);
	}
}
