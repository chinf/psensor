/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include "../src/lib/psensor.h"

static int
test_fct(unsigned int type, int use_celsius, const char *ref)
{
	const char *u;

	u = psensor_type_to_unit_str(type, use_celsius);
	if (strcmp(ref, u)) {
		fprintf(stderr, "returns: %s expected: %s\n", u, ref);
		return 0;
	}

	return 1;
}

static int test(void) {
	int failures;

	failures = 0;

	if (!test_fct(SENSOR_TYPE_TEMP, 1, "\302\260C"))
		failures++;

	if (!test_fct(SENSOR_TYPE_TEMP, 0, "\302\260F"))
		failures++;

	if (!test_fct(SENSOR_TYPE_LMSENSOR | SENSOR_TYPE_TEMP, 1, "\302\260C"))
		failures++;

	if (!test_fct(SENSOR_TYPE_LMSENSOR | SENSOR_TYPE_TEMP, 0, "\302\260F"))
		failures++;

	if (!test_fct(SENSOR_TYPE_RPM, 0, _("RPM")))
		failures++;

	return failures;
}

int main(int argc, char **argv)
{
	int failures;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	failures = test();

	if (failures)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
