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

#include "../src/lib/pio.h"

static int test_empty_dir(void)
{
	int ret;
	char **paths;

	paths = dir_list("data/empty_dir", NULL);

	ret = 0;
	if (paths) {
		if (*paths) {
			ret = 1;
			fprintf(stderr, "ERROR: list not empty %s\n", *paths);
		}

		paths_free(paths);
	}

	return ret;
}

static int test_2files_dir(void)
{
	int ret, one, two;
	char **paths, **cur;

	paths = dir_list("data/2files_dir", NULL);

	one = two = ret = 0;

	if (!paths) {
		fprintf(stderr, "ERROR: list is NULL\n");
		return 1;
	}

	cur = paths;
	while(*cur) {
		if (!strcmp(*cur, "data/2files_dir/one")) {
			one++;
		} else if (!strcmp(*cur, "data/2files_dir/two")) {
			two++;
		} else {
			fprintf(stderr, "ERROR: wrong item: %s\n", *cur);

			ret = 1;
		}

		cur++;
	}

	if (!ret && one == 1 && two == 1)
		ret = 0;
	else
		ret = 1;

	paths_free(paths);

	return ret;
}

static int tests_dir_list(void) {
	int failures;

	failures = test_empty_dir();

	failures += test_2files_dir();

	return failures;
}

int main(int argc, char **argv)
{
	int failures;

	failures = tests_dir_list();

	if (failures)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
