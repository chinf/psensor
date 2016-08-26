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
#ifndef _PSENSOR_UI_PREF_H_
#define _PSENSOR_UI_PREF_H_

#include "ui.h"

void ui_pref_dialog_run(struct ui_psensor *);
GdkRGBA color_to_GdkRGBA(struct color *color);

void ui_pref_decoration_toggled_cbk(GtkToggleButton *, gpointer);
void ui_pref_keep_below_toggled_cbk(GtkToggleButton *, gpointer);

#endif
