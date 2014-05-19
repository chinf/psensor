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

#include "../src/lib/psensor.h"

#define CELSIUS "\302\260C"
#define FAHRENHEIT "\302\260F"

static int
test_psensor_value_to_str(unsigned int type,
			  double value,
			  int celsius,
			  const char *ref)
{
	char *str;

	str = psensor_value_to_str(type, value, celsius);
	if (strcmp(ref, str)) {
		fprintf(stderr, "returns: %s expected: %s\n", str, ref);
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char **argv)
{
	int errs;

	errs = test_psensor_value_to_str(SENSOR_TYPE_TEMP, 13, 1,
					 "13"CELSIUS);
	errs += test_psensor_value_to_str(SENSOR_TYPE_TEMP, 13, 0,
					  "55"FAHRENHEIT);
	errs += test_psensor_value_to_str(SENSOR_TYPE_TEMP, 13.4, 1,
					  "13"CELSIUS);
	errs += test_psensor_value_to_str(SENSOR_TYPE_TEMP, 13.5, 1,
					  "14"CELSIUS);

	if (errs) 
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
