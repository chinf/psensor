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
#ifndef _PSENSOR_UI_H_
#define _PSENSOR_UI_H_

#include "config.h"

#include <pthread.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#if defined(HAVE_APPINDICATOR)
#include <libappindicator/app-indicator.h>
#endif

#include "psensor.h"

#define PSENSOR_ICON "psensor"

struct ui_psensor {
	struct psensor **sensors;
	/* mutex which MUST be used for accessing sensors.*/
	pthread_mutex_t sensors_mutex;

	struct config *config;

	GtkWidget *main_window;

	GtkWidget *popup_menu;

	GtkListStore *sensors_store;
	GtkTreeView *sensors_tree;

	int graph_update_interval;
};

/*
 * Update the window according to the configuration.
 *
 * Creates or re-creates the sensor_box according to the position of
 * the list of sensors in the configuration.
 *
 * Show or hide the menu bar.
 */
void ui_window_update(struct ui_psensor *);

/* Show the main psensor window. */
void ui_window_show(struct ui_psensor *);

/* Must be called to terminate Psensor UI. */
void ui_psensor_quit(struct ui_psensor *ui);

/* Creates the main GTK window */
void ui_window_create(struct ui_psensor *ui);

void ui_menu_bar_show(unsigned int show, struct ui_psensor *ui);

void ui_enable_alpha_channel(struct ui_psensor *ui);

void ui_cb_preferences(GtkMenuItem *mi, gpointer data);
void ui_cb_menu_quit(GtkMenuItem *mi, gpointer data);
void ui_cb_sensor_preferences(GtkMenuItem *mi, gpointer data);

GtkWidget *ui_get_graph(void);

struct psensor **ui_get_sensors_ordered_by_position(struct psensor **);
#endif
