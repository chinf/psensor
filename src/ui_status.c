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
#include <plog.h>
#include "ui_status.h"

static const char *ICON = "psensor_normal";
static const char *ATTENTION_ICON = "psensor_hot";

static GtkStatusIcon *status;
static unsigned int status_attention;

static void cb_activate(GtkStatusIcon *icon,
			gpointer data)
{
	log_debug("cb_activate()");
	ui_window_show((struct ui_psensor *)data);
}

static void cb_popup_menu(GtkStatusIcon *icon,
			  guint button,
			  guint activate_time,
			  gpointer data)
{
	log_debug("cb_popup_menu()");
}

void ui_status_init(struct ui_psensor *ui)
{
	if (status)
		return;

	log_debug("ui_status_create()");

	status = gtk_status_icon_new();
	gtk_status_icon_set_from_icon_name(status, ICON);
	ui_status_set_visible(0);

	g_signal_connect(G_OBJECT(status),
			 "popup-menu",
			 G_CALLBACK(cb_popup_menu),
			 NULL);

	g_signal_connect(G_OBJECT(status),
			 "activate",
			 G_CALLBACK(cb_activate),
			 ui);
}

int is_status_supported(void)
{
	return status && gtk_status_icon_is_embedded(status);
}

void ui_status_cleanup(void)
{
	log_debug("ui_status_cleanup()");

	if (status) {
		g_object_unref(G_OBJECT(status));
		status = NULL;
	}
}

void ui_status_update(struct ui_psensor *ui, unsigned int attention)
{
	log_debug("ui_status_update()");

	if (status_attention && !attention)
		gtk_status_icon_set_from_icon_name(status, ICON);
	else if (!status_attention && attention)
		gtk_status_icon_set_from_icon_name(status, ATTENTION_ICON);

	status_attention = attention;
}

GtkStatusIcon *ui_status_get_icon(struct ui_psensor *ui)
{
	if (!status)
		ui_status_init(ui);

	return status;
}

void ui_status_set_visible(unsigned int visible)
{
	log_debug("ui_status_set_visible(%d)", visible);

	if (status) {
		if (visible)
			gtk_status_icon_set_visible(status, TRUE);
		else
			gtk_status_icon_set_visible(status, FALSE);
	}
}
