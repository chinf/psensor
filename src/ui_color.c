/*
 * Copyright (C) 2010-2014 jeanfi@gmail.com
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

int ui_change_color(const char *title, struct color *col, GtkWindow *win)
{
	GdkRGBA color;
	int res;
	GtkColorChooserDialog *colordlg;
	double r, g, b;

	color.red = col->red;
	color.green = col->green;
	color.blue = col->blue;
	color.alpha = 1;

	colordlg = GTK_COLOR_CHOOSER_DIALOG
		(gtk_color_chooser_dialog_new(title, win));

	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(colordlg), 0);

	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colordlg), &color);

	res = gtk_dialog_run(GTK_DIALOG(colordlg));

	if (res == GTK_RESPONSE_OK) {
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colordlg), &color);

		/* GdkRGBA defines rgb as double 0..1 but chooser returns
		 * values > 1 when selecting a custom undefined color.
		 * Not sure whether that's a gtk/gdk bug. */

		if (color.red > 1)
			r = 1;
		else
			r = color.red;

		if (color.green > 1)
			g = 1;
		else
			g = color.green;

		if (color.blue > 1)
			b = 1;
		else
			b = color.blue;

		color_set(col, 65535*r, 65535*g, 65535*b);
	}

	gtk_widget_destroy(GTK_WIDGET(colordlg));

	return res == GTK_RESPONSE_OK;
}
