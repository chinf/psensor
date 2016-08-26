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
#include <gtk/gtk.h>

#include <ui_color.h>

/*
 * UI to change a given color.
 *
 * Returns 1 if the color has been modified.
 */
bool ui_change_color(const char *title, GdkRGBA *color, GtkWindow *win)
{
	int res;
	GtkColorChooserDialog *colordlg;

	colordlg = GTK_COLOR_CHOOSER_DIALOG
		(gtk_color_chooser_dialog_new(title, win));

	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(colordlg), 0);

	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colordlg), color);

	res = gtk_dialog_run(GTK_DIALOG(colordlg));

	if (res == GTK_RESPONSE_OK)
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colordlg), color);

	gtk_widget_destroy(GTK_WIDGET(colordlg));

	return res == GTK_RESPONSE_OK;
}
