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
#include <unity.h>

#include <cfg.h>
#include <temperature.h>
#include <ui_unity.h>

static UnityLauncherEntry *psensor_entry;
static bool count_visible;

static void
count_visible_changed_cbk(GSettings *settings, gchar *key, gpointer data)
{
	count_visible = config_is_count_visible();

	if (count_visible) {
		unity_launcher_entry_set_count(psensor_entry, 0);
		unity_launcher_entry_set_count_visible(psensor_entry, TRUE);
	} else {
		unity_launcher_entry_set_count_visible(psensor_entry, FALSE);
	}
}

static double get_max_current_value(struct psensor **sensors, unsigned int type)
{
	double m, v;
	struct psensor *s;

	m = UNKNOWN_DBL_VALUE;
	while (*sensors) {
		s = *sensors;

		if ((s->type & type) && config_is_sensor_graph_enabled(s->id)) {
			v = psensor_get_current_value(s);

			if (m == UNKNOWN_DBL_VALUE || v > m)
				m = v;
		}

		sensors++;
	}

	return m;
}

void ui_unity_launcher_entry_update(struct psensor **sensors)
{
	double v;

	if (!count_visible || !sensors || !*sensors)
		return;

	v = get_max_current_value(sensors, SENSOR_TYPE_TEMP);

	if (v != UNKNOWN_DBL_VALUE) {
		if (config_get_temperature_unit() == FAHRENHEIT)
			v = celsius_to_fahrenheit(v);

		unity_launcher_entry_set_count(psensor_entry, v);
	}
}

void ui_unity_init(void)
{
	psensor_entry = unity_launcher_entry_get_for_desktop_file
		(PSENSOR_DESKTOP_FILE);

	count_visible = config_is_count_visible();

	if (count_visible) {
		unity_launcher_entry_set_count(psensor_entry, 0);
		unity_launcher_entry_set_count_visible(psensor_entry, TRUE);
	} else {
		unity_launcher_entry_set_count_visible(psensor_entry, FALSE);
	}

	g_signal_connect_after(config_get_GSettings(),
			       "changed::interface-unity-launcher-count-disabled",
			       G_CALLBACK(count_visible_changed_cbk),
			       NULL);
}
