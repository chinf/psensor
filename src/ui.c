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

#include "cfg.h"
#include "ui.h"
#include "ui_graph.h"
#include "ui_pref.h"
#include "ui_sensorlist.h"

static gboolean
on_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
	gtk_widget_hide(((struct ui_psensor *)data)->main_window);
#else
	ui_psensor_quit();
#endif

	return TRUE;
}

static void
cb_menu_quit(gpointer data, guint cb_action, GtkWidget *item)
{
	ui_psensor_quit();
}

static void
cb_menu_preferences(gpointer data, guint cb_action, GtkWidget *item)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

void ui_psensor_quit()
{
	gtk_main_quit();
}

static GtkItemFactoryEntry menu_items[] = {
	{"/Psensor", NULL, NULL, 0, "<Branch>"},
	{"/Psensor/Preferences",
	 NULL, cb_menu_preferences, 0, "<Item>"},
	{"/Psensor/sep1",
	 NULL, NULL, 0, "<Separator>"},
	{"/Psensor/Quit",
	 "", cb_menu_quit, 0, "<StockItem>", GTK_STOCK_QUIT},
};

static gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);

static GtkWidget *get_menu(struct ui_psensor *ui)
{
	GtkItemFactory *item_factory;

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", NULL);

	gtk_item_factory_create_items(item_factory,
				      nmenu_items, menu_items, ui);
	return gtk_item_factory_get_widget(item_factory, "<main>");
}

void ui_window_create(struct ui_psensor *ui)
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GdkScreen *screen;
	GdkColormap *colormap;
	GdkPixbuf *icon;
	GtkIconTheme *icon_theme;
	GtkWidget *menubar;

	gtk_window_set_default_size(GTK_WINDOW(window), 800, 200);

	gtk_window_set_title(GTK_WINDOW(window),
			     _("Psensor - Temperature Monitor"));
	gtk_window_set_role(GTK_WINDOW(window), "psensor");

	screen = gtk_widget_get_screen(window);

	if (ui->config->alpha_channel_enabled
	    && gdk_screen_is_composited(screen)) {

		colormap = gdk_screen_get_rgba_colormap(screen);
		if (colormap)
			gtk_widget_set_colormap(window, colormap);
		else
			ui->config->alpha_channel_enabled = 0;
	} else {
		ui->config->alpha_channel_enabled = 0;
	}

	icon_theme = gtk_icon_theme_get_default();
	icon = gtk_icon_theme_load_icon(icon_theme, "psensor", 48, 0, NULL);
	if (icon)
		gtk_window_set_icon(GTK_WINDOW(window), icon);
	else
		fprintf(stderr, _("ERROR: Failed to load psensor icon.\n"));

	g_signal_connect(window,
			 "delete_event", G_CALLBACK(on_delete_event_cb), ui);

	gtk_window_set_decorated(GTK_WINDOW(window),
				 ui->config->window_decoration_enabled);

	gtk_window_set_keep_below(GTK_WINDOW(window),
				  ui->config->window_keep_below_enabled);

	/* main box */
	menubar = get_menu(ui);

	ui->main_box = gtk_vbox_new(FALSE, 1);

	gtk_box_pack_start(GTK_BOX(ui->main_box), menubar,
			   FALSE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(window), ui->main_box);

	ui->main_window = window;
	ui->menu_bar = menubar;

	gtk_widget_show_all(ui->main_window);
}

static void menu_bar_show(unsigned int show, struct ui_psensor *ui)
{
	if (show)
		gtk_widget_show(ui->menu_bar);
	else
		gtk_widget_hide(ui->menu_bar);
}

void ui_window_update(struct ui_psensor *ui)
{
	struct config *cfg;
	GtkWidget *w_sensorlist;

	cfg = ui->config;

	if (ui->sensor_box) {
		ui_sensorlist_create_widget(ui->ui_sensorlist);

		gtk_container_remove(GTK_CONTAINER(ui->main_box),
				     ui->sensor_box);

		ui->w_graph = ui_graph_create(ui);
		ui->w_sensorlist = ui->ui_sensorlist->widget;
	}

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_LEFT)
		ui->sensor_box = gtk_hpaned_new();
	else
		ui->sensor_box = gtk_vpaned_new();

	w_sensorlist = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w_sensorlist),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(w_sensorlist),
			  ui->ui_sensorlist->widget);

	gtk_box_pack_end(GTK_BOX(ui->main_box), ui->sensor_box, TRUE, TRUE, 2);

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_BOTTOM) {
		gtk_paned_pack1(GTK_PANED(ui->sensor_box),
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
		gtk_paned_pack2(GTK_PANED(ui->sensor_box),
				w_sensorlist, FALSE, TRUE);
	} else {
		gtk_paned_pack1(GTK_PANED(ui->sensor_box),
				w_sensorlist, FALSE, TRUE);
		gtk_paned_pack2(GTK_PANED(ui->sensor_box),
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
	}

	gtk_widget_show_all(ui->sensor_box);

	if (cfg->menu_bar_disabled)
		menu_bar_show(0, ui);
	else
		menu_bar_show(1, ui);
}
