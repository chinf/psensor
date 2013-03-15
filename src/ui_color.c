/*
 * Copyright (C) 2010-2013 jeanfi@gmail.com
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

#include "ui_color.h"

int ui_change_color(const char *title, struct color *col)
{
	GdkColor color;
	GtkColorSelection *colorsel;
	int res;
	GtkColorSelectionDialog *colordlg;

	color.red = col->red;
	color.green = col->green;
	color.blue = col->blue;

	colordlg = GTK_COLOR_SELECTION_DIALOG
		(gtk_color_selection_dialog_new(title));

	colorsel = GTK_COLOR_SELECTION
		(gtk_color_selection_dialog_get_color_selection(colordlg));

	gtk_color_selection_set_current_color(colorsel, &color);

	res = gtk_dialog_run(GTK_DIALOG(colordlg));

	if (res == GTK_RESPONSE_OK) {
		gtk_color_selection_get_current_color(colorsel, &color);

		color_set(col, color.red, color.green, color.blue);
	}

	gtk_widget_destroy(GTK_WIDGET(colordlg));

	return res == GTK_RESPONSE_OK;
}
