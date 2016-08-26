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
#ifndef _PSENSOR_NVIDIA_H_
#define _PSENSOR_NVIDIA_H_

#include <bool.h>
#include <psensor.h>


#if defined(HAVE_NVIDIA) && HAVE_NVIDIA

static inline bool nvidia_is_supported(void) { return true; }

void nvidia_psensor_list_update(struct psensor **);
void nvidia_psensor_list_append(struct psensor ***, int);
void nvidia_cleanup(void);

#else

static inline bool nvidia_is_supported(void) { return false; }

static inline void nvidia_psensor_list_update(struct psensor **s) {}
static inline void nvidia_psensor_list_append(struct psensor ***s, int n) {}
static inline void nvidia_cleanup(void) {}

#endif

#endif
