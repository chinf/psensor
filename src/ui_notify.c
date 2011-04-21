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

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <libnotify/notify.h>

#include "ui.h"
#include "ui_notify.h"

void ui_notify(struct psensor *sensor, struct ui_psensor *ui)
{
	struct timeval *t = malloc(sizeof(struct timeval));
	char *name;
	NotifyNotification *notif;

	if (gettimeofday(t, NULL) != 0) {
		fprintf(stderr, _("ERROR: failed gettimeofday\n"));
		free(t);

		return;
	}

	if (!ui->notification_last_time) {
		/* first notification */
		ui->notification_last_time = t;
	} else {

		if (t->tv_sec - ui->notification_last_time->tv_sec < 60) {
			/* last notification less than 1mn ago */
			free(t);
			return;
		} else {
			/* last notification more than 1mn ago */
			free(ui->notification_last_time);
			ui->notification_last_time = t;
		}
	}

	if (notify_is_initted() == FALSE)
		notify_init("psensor");

	if (notify_is_initted() == TRUE) {
		name = strdup(sensor->name);

#ifdef NOTIFY_VERSION_MAJOR
		notif = notify_notification_new(_("Temperature alert"),
						name,
						NULL);
#else
		notif = notify_notification_new(_("Temperature alert"),
						name,
						NULL,
						GTK_WIDGET(ui->main_window));
#endif


		notify_notification_show(notif, NULL);

		g_object_unref(notif);
	}
}
