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

#ifndef _PSENSOR_UI_STATUS_H_
#define _PSENSOR_UI_STATUS_H_

#include <gtk/gtk.h>
#include <ui.h>

void ui_status_init(struct ui_psensor *ui);
void ui_status_cleanup(void);
void ui_status_update(struct ui_psensor *ui, unsigned int attention);
/* Whether status icon is supported i.e. visible. */
int is_status_supported(void);
GtkStatusIcon *ui_status_get_icon(struct ui_psensor *ui);
/* Whether the statuc icon should be visible.*/
void ui_status_set_visible(unsigned int visible);

#endif
