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

#include "graph.h"
#include "ui_graph.h"
#include "ui_pref.h"

static void
cb_preferences(gpointer data, guint callback_action, GtkWidget *item)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

static GtkItemFactoryEntry menu_items[] = {
	{N_("/Preferences"),
	 NULL, cb_preferences, 0, "<Item>"},

	{"/sep1",
	 NULL, NULL, 0, "<Separator>"},

	{N_("/Quit"),
	 "", ui_psensor_exit, 0, "<StockItem>", GTK_STOCK_QUIT},
};

static gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);

GtkWidget *ui_graph_create_popupmenu(struct ui_psensor *ui)
{
	GtkItemFactory *item_factory;

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
	gtk_item_factory_create_items(item_factory,
				      nmenu_items, menu_items, ui);
	return gtk_item_factory_get_widget(item_factory, "<main>");
}

int on_graph_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkWidget *menu;

	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	menu = ui_graph_create_popupmenu((struct ui_psensor *)data);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}

gboolean
on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	struct ui_psensor *ui_psensor = (struct ui_psensor *)data;

	graph_update(ui_psensor->sensors,
		     ui_psensor->w_graph, ui_psensor->config);

	return FALSE;
}

GtkWidget *ui_graph_create(struct ui_psensor * ui)
{
	GtkWidget *w_graph;

	w_graph = gtk_drawing_area_new();

	g_signal_connect(G_OBJECT(w_graph),
			 "expose-event", G_CALLBACK(on_expose_event), ui);

	gtk_widget_add_events(w_graph, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(w_graph),
			   "button_press_event",
			   (GCallback) on_graph_clicked, ui);

	return w_graph;
}
