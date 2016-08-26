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
#ifndef _PSENSOR_UI_NOTIFY_H_
#define _PSENSOR_UI_NOTIFY_H_

#include <ui.h>

#if defined(HAVE_LIBNOTIFY) && HAVE_LIBNOTIFY

void ui_notify(struct psensor *, struct ui_psensor *);

#else

static inline void ui_notify(struct psensor *s, struct ui_psensor *u) {}

#endif

#endif
