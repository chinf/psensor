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
#ifndef _PSENSOR_UI_SENSORLIST_H_
#define _PSENSOR_UI_SENSORLIST_H_

#include <gtk/gtk.h>

#include "psensor.h"

struct ui_sensorlist {
	/* The GtkTreeView widget containing the list of sensors */
	GtkTreeView *treeview;

	/* Scrolled widget containing the GtkTreeView */
	GtkWidget *widget;

	struct psensor **sensors;
};

void ui_sensorlist_create(struct ui_psensor *);

/* Update values current/min/max */
void ui_sensorlist_update(struct ui_psensor *ui);

void ui_sensorlist_update_sensors_preferences(struct ui_psensor *);

#endif
