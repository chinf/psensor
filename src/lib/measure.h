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
#ifndef _PSENSOR_MEASURE_H_
#define _PSENSOR_MEASURE_H_

#include <sys/time.h>
#include <float.h>
#include <stdint.h>

#define UNKNOWN_DBL_VALUE DBL_MIN

struct measure {
	double value;

	struct timeval time;
};

void measure_copy(struct measure *src, struct measure *dst);

struct measure *measures_dbl_create(int size);

void measures_free(struct measure *measures);

#endif
