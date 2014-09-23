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

#include "../src/lib/url.h"

static int test_url_normalize(const char *url, const char *ref_url)
{
	int ret;
	char *tmp = url_normalize(url);

	if (!strcmp(tmp, ref_url)) {
		ret = 1;
	} else {
		fprintf(stderr,
			"FAILURE: "
			"url_normalize(%s) returns %s instead of %s\n",
			url,
			tmp,
			ref_url);
		ret = 0;
	}

	free(tmp);

	return ret;
}

static int tests_url_normalize(void)
{
	int failures;

	failures = 0;

	if (!test_url_normalize("http://test/test", "http://test/test"))
		failures++;

	if (!test_url_normalize("http://test/test/", "http://test/test"))
		failures++;

	return failures;
}

int main(int argc, char **argv)
{
	int failures;

	failures = 0;

	failures += tests_url_normalize();

	if (failures)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
