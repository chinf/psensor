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
#ifndef _PSENSOR_UI_APPINDICATOR_H_
#define _PSENSOR_UI_APPINDICATOR_H_

#include <config.h>

#include <bool.h>
#include <ui.h>

#if defined(HAVE_APPINDICATOR) && HAVE_APPINDICATOR

bool is_appindicator_supported(void);

void ui_appindicator_init(struct ui_psensor *ui);
void ui_appindicator_update(struct ui_psensor *ui, bool alert);
void ui_appindicator_update_menu(struct ui_psensor *ui);
void ui_appindicator_cleanup(void);
void ui_appindicator_menu_show_cb(GtkMenuItem *, gpointer);

#else

static inline bool is_appindicator_supported(void) { return false; }

static inline void ui_appindicator_init(struct ui_psensor *ui) {}
static inline void ui_appindicator_update(struct ui_psensor *ui, bool alert) {}
static inline void ui_appindicator_update_menu(struct ui_psensor *ui) {}
static inline void ui_appindicator_cleanup(void) {}
static inline void ui_appindicator_menu_show_cb(GtkMenuItem *m, gpointer d) {}

#endif

#endif
