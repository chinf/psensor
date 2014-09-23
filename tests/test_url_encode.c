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

static int test_url_encode(char *url, char *ref_url)
{
	char *res_url;
	int ret;

	res_url = url_encode(url);

	if (strcmp(ref_url, res_url)) {
		fprintf(stderr,
			"FAILURE: url_encode(%s) returns %s instead of %s\n",
			url, res_url, ref_url);
		ret = 0;
	} else {
		ret = 1;
	}

	free(res_url);

	return ret;
}

static int tests_url_encode(void)
{
	int failures;

	failures = 0;

	if (!test_url_encode("abcdef12345", "abcdef12345"))
		failures++;

	if (!test_url_encode("a b", "a%20b"))
		failures++;

	if (!test_url_encode("ab-_.~", "ab-_.~"))
		failures++;

	return failures;
}


int main(int argc, char **argv)
{
	int failures;

	failures = 0;

	failures += tests_url_encode();

	if (failures) 
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
