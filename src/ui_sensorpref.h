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
#ifndef _PSENSOR_UI_SENSORPREF_H_
#define _PSENSOR_UI_SENSORPREF_H_

#include <ui.h>

void ui_sensorpref_dialog_run(struct psensor *sensor, struct ui_psensor *ui);

void ui_sensorpref_name_changed_cb(GtkEntry *, gpointer);
void ui_sensorpref_draw_toggled_cb(GtkToggleButton *, gpointer);
void ui_sensorpref_display_toggled_cb(GtkToggleButton *, gpointer);
void ui_sensorpref_alarm_toggled_cb(GtkToggleButton *, gpointer);
void ui_sensorpref_appindicator_menu_toggled_cb(GtkToggleButton *, gpointer);
void ui_sensorpref_appindicator_label_toggled_cb(GtkToggleButton *, gpointer);
void ui_sensorpref_color_set_cb(GtkColorButton *, gpointer);
void ui_sensorpref_alarm_high_threshold_changed_cb(GtkSpinButton *, gpointer);
void ui_sensorpref_alarm_low_threshold_changed_cb(GtkSpinButton *, gpointer);
void ui_sensorpref_tree_selection_changed_cb(GtkTreeSelection *, gpointer);
void ui_sensorpref_close_clicked_cb(GtkButton *, gpointer);

#endif
