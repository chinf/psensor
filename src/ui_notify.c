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
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <libnotify/notify.h>

/* Macro defined since libnotify 0.5.2 */
#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x, y, z) 0
#endif

#include "cfg.h"
#include "ui.h"
#include "ui_notify.h"

/* Time of the last notification. */
static struct timeval last_notification_tv;

void ui_notify(struct psensor *sensor, struct ui_psensor *ui)
{
	struct timeval t;
	char *body, *svalue;
	const char *summary;
	NotifyNotification *notif;
	unsigned int use_celsius;

	log_debug("last_notification %d", last_notification_tv.tv_sec);

	if (gettimeofday(&t, NULL) != 0) {
		log_err(_("gettimeofday failed."));
		return;
	}

	if (!last_notification_tv.tv_sec
	    || t.tv_sec - last_notification_tv.tv_sec >= 60)
		last_notification_tv = t;
	else
		return;

	if (notify_is_initted() == FALSE)
		notify_init("psensor");

	if (notify_is_initted() == TRUE) {
		if (config_get_temperature_unit() == CELSIUS)
			use_celsius = 1;
		else
			use_celsius = 0;

		svalue = psensor_measure_to_str
			(psensor_get_current_measure(sensor),
			 sensor->type,
			 use_celsius);

		body = malloc(strlen(sensor->name) + 3 + strlen(svalue) + 1);
		sprintf(body, "%s : %s", sensor->name, svalue);
		free(svalue);

		if (is_temp_type(sensor->type))
			summary = _("Temperature alert");
		else if (sensor->type & SENSOR_TYPE_RPM)
			summary = _("Fan speed alert");
		else
			summary = _("N/A");

		/*
		 * Since libnotify 0.7 notify_notification_new has
		 * only 3 parameters.
		 */
#if NOTIFY_CHECK_VERSION(0, 7, 0)
		notif = notify_notification_new(summary, body, PSENSOR_ICON);
#else
		notif = notify_notification_new(summary,
						body,
						PSENSOR_ICON,
						GTK_WIDGET(ui->main_window));
#endif
		log_debug("notif_notification_new %s", body);

		notify_notification_show(notif, NULL);

		g_object_unref(notif);
	} else {
		log_err("notify not initialized");
	}
}
