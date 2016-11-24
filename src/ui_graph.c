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
#include "graph.h"
#include "ui_graph.h"

static int
on_graph_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	gtk_menu_popup(GTK_MENU(((struct ui_psensor *)data)->popup_menu),
		       NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

static gboolean
on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	struct ui_psensor *ui_psensor = (struct ui_psensor *)data;

	graph_update(ui_psensor->sensors,
		     ui_get_graph(),
		     ui_psensor->config,
		     ui_psensor->main_window);

	return FALSE;
}

static void smoothing_changed_cbk(void *data)
{
	smoothing = config_get_graph_smoothing();
}

static void yaxis_rightside_enabled_changed_cbk(void *data)
{
	is_yaxis_rightside_enabled = config_is_yaxis_rightside_enabled();
}

static void yaxis_tags_enabled_changed_cbk(void *data)
{
	is_yaxis_tags_enabled = config_is_yaxis_tags_enabled();
}

void ui_graph_create(struct ui_psensor *ui)
{
	GtkWidget *w_graph;

	log_debug("ui_graph_create()");

	w_graph = ui_get_graph();

	smoothing = config_get_graph_smoothing();
	g_signal_connect_after(config_get_GSettings(),
			       "changed::graph-smoothing",
			       G_CALLBACK(smoothing_changed_cbk),
			       NULL);

	is_yaxis_rightside_enabled = config_is_yaxis_rightside_enabled();
	g_signal_connect_after(config_get_GSettings(),
			       "changed::graph-yaxis-rightside-enabled",
			       G_CALLBACK(yaxis_rightside_enabled_changed_cbk),
			       NULL);

	is_yaxis_tags_enabled = config_is_yaxis_tags_enabled();
	g_signal_connect_after(config_get_GSettings(),
			       "changed::graph-yaxis-tags-enabled",
			       G_CALLBACK(yaxis_tags_enabled_changed_cbk),
			       NULL);

	g_signal_connect(GTK_WIDGET(w_graph),
			 "draw",
			 G_CALLBACK(on_expose_event),
			 ui);

	gtk_widget_add_events(w_graph, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(GTK_WIDGET(w_graph),
			 "button_press_event",
			 (GCallback) on_graph_clicked, ui);

	log_debug("ui_graph_create() ends");
}
