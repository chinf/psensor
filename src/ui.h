/*
    Copyright (C) 2010-2011 jeanfi@gmail.com

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

#ifndef _PSENSOR_UI_H_
#define _PSENSOR_UI_H_

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
#include <libappindicator/app-indicator.h>
#endif

#include "psensor.h"

struct ui_psensor {
	struct psensor **sensors;

	GtkWidget *w_graph;

	struct ui_sensorlist *ui_sensorlist;

	struct config *config;

	GtkWidget *main_window;

	GtkWidget *menu_bar;

	/*
	   The main vertical box, top contains the menubar, bottom
	   contains the sensor_box.
	*/
	GtkWidget *main_box;

	/*
	   The box which contains the sensors graph and the sensors
	   information list.
	*/
	GtkWidget *sensor_box;

	int graph_update_interval;

	GMutex *sensors_mutex;

#ifdef HAVE_LIBNOTIFY
	/*
	   Time of the last notification
	 */
	struct timeval *notification_last_time;
#endif

#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
	AppIndicator *indicator;
#endif
};

/*
  Update the window according to the configuration.

  Creates or re-creates the sensor_box according to the position of
  the list of sensors in the configuration.

  Show or hide the menu bar.
*/
void ui_window_update(struct ui_psensor *);

/*
  Must be called to terminate Psensor UI.
*/
void ui_psensor_quit(struct ui_psensor *ui);

/*
  Creates the main GTK window
*/
void ui_window_create(struct ui_psensor *ui);

void ui_menu_bar_show(unsigned int show, struct ui_psensor *ui);

void ui_show_about_dialog();

#endif
