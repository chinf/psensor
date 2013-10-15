/*
 * Copyright (C) 2010-2013 jeanfi@gmail.com
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
#include <stdlib.h>

#include "ptime.h"

char *time_to_str(time_t *t)
{
	struct tm lt;
	char *str;

	if (!localtime_r(t, &lt))
		return NULL;

	str = malloc(64);

	if (strftime(str, 64, "%s", &lt)) {
		return str;
	} else {
		free(str);
		return NULL;
	}
}

char *get_time_str()
{
	time_t t;

	t = time(NULL);
	return time_to_str(&t);
}
