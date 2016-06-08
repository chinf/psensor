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
#include <stdlib.h>
#include <string.h>

#include <ptime.h>

const int P_TIME_VER = 3;

static const int ISO8601_TIME_LENGTH = 19; /* YYYY-MM-DDThh:mm:ss */
static const int ISO8601_DATE_LENGTH = 10; /* YYYY-MM-DD */

char *time_to_ISO8601_time(time_t *t)
{
	struct tm lt;

	memset(&lt, 0, sizeof(struct tm));
	if (!gmtime_r(t, &lt))
		return NULL;

	return tm_to_ISO8601_time(&lt);
}

char *time_to_ISO8601_date(time_t *t)
{
	struct tm lt;

	memset(&lt, 0, sizeof(struct tm));
	if (!gmtime_r(t, &lt))
		return NULL;

	return tm_to_ISO8601_date(&lt);
}

char *tm_to_ISO8601_date(struct tm *tm)
{
	char *str;

	str = malloc(ISO8601_DATE_LENGTH + 1);

	if (strftime(str, ISO8601_DATE_LENGTH + 1, "%F", tm))
		return str;

	free(str);
	return NULL;
}

char *tm_to_ISO8601_time(struct tm *tm)
{
	char *str;

	str = malloc(ISO8601_TIME_LENGTH + 1);

	if (strftime(str, ISO8601_TIME_LENGTH + 1, "%FT%T", tm))
		return str;

	free(str);
	return NULL;
}

char *get_current_ISO8601_time(void)
{
	time_t t;

	t = time(NULL);
	return time_to_ISO8601_time(&t);
}
