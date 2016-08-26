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
#ifndef _PSENSOR_RSENSOR_H_
#define _PSENSOR_RSENSOR_H_

#include <psensor.h>

#if defined(HAVE_REMOTE_SUPPORT) && HAVE_REMOTE_SUPPORT

static inline bool rsensor_is_supported(void) { return true; }

struct psensor **get_remote_sensors(const char *, int);
void remote_psensor_list_update(struct psensor **);
void rsensor_init(void);
void rsensor_cleanup(void);

#else

static inline bool rsensor_is_supported(void) { return false; }

static inline struct psensor **
get_remote_sensors(const char *url, int n) { return NULL; }
static inline void remote_psensor_list_update(struct psensor **s) {}
static inline void rsensor_init(void) {}
static inline void rsensor_cleanup(void) {}

#endif

#endif
