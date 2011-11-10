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

#ifndef _PSENSOR_UI_APPINDICATOR_H_
#define _PSENSOR_UI_APPINDICATOR_H_

#include "config.h"
#include "ui.h"

#if defined(HAVE_APPINDICATOR) || defined(HAVE_APPINDICATOR_029)
void ui_appindicator_init(struct ui_psensor *ui);
void ui_appindicator_update(struct ui_psensor *ui);
int is_appindicator_supported();
void ui_appindicator_cleanup();
#else
#define is_appindicator_supported() 0
#endif

#endif
