/*
 * Copyright (C) 2010-2011 thgreasi@gmail.com, jeanfi@gmail.com
 * Copyright (C) 2012-2016 jeanfi@gmail.com
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
#ifndef _PSENSOR_AMD_H_
#define _PSENSOR_AMD_H_

#include <bool.h>
#include <psensor.h>

#if defined(HAVE_LIBATIADL) && HAVE_LIBATIADL

static inline bool amd_is_supported(void) { return true; }

void amd_psensor_list_update(struct psensor **s);
void amd_psensor_list_append(struct psensor ***s, int n);
void amd_cleanup(void);

#else

static inline bool amd_is_supported(void) { return false; }

static inline void amd_psensor_list_update(struct psensor **s) {}
static inline void amd_psensor_list_append(struct psensor ***s, int n) {}
static inline void amd_cleanup(void) {}

#endif

#endif
