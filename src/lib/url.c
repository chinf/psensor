/*
 * Copyright (C) 2010-2014 jeanfi@gmail.com
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

/*
 * Part of the following code is based on:
 * http://www.geekhideout.com/urlcode.shtml
 */
#include "url.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *url_normalize(const char *url)
{
	int n = strlen(url);
	char *ret = strdup(url);

	if (url[n - 1] == '/')
		ret[n - 1] = '\0';

	return ret;
}

static char to_hex(char code)
{
	static const char hex[] = "0123456789abcdef";
	return hex[code & 0x0f];
}

/*
 * Returns a url-encoded version of str.
 */
char *url_encode(const char *str)
{
	char *c, *buf, *pbuf;

	buf = malloc(strlen(str) * 3 + 1);
	pbuf = buf;

	c = (char *)str;

	while (*c) {

		if (isalnum(*c) ||
		    *c == '.' || *c == '_' || *c == '-' || *c == '~')
			*pbuf++ = *c;
		else {
			*pbuf++ = '%';
			*pbuf++ = to_hex(*c >> 4);
			*pbuf++ = to_hex(*c & 0x0f);
		}
		c++;
	}
	*pbuf = '\0';

	return buf;
}
