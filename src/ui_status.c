/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
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

#include <gtk/gtk.h>

#include "log.h"
#include "ui_status.h"

void ui_status_create()
{
	GtkStatusIcon *status;
	
	log_printf(LOG_DEBUG, "ui_status_create()");

        status = gtk_status_icon_new();
        gtk_status_icon_set_from_icon_name(status, "psensor");
        gtk_status_icon_set_tooltip(status, 
                                    "Psensor - Hardware Sensors Monitoring");
        gtk_status_icon_set_visible(status, TRUE);
}
