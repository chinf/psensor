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
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "color.h"

void color_set(struct color *c, double r, double g, double b)
{
	c->red = r;
	c->green = g;
	c->blue = b;
}

struct color *color_new(double r, double g, double b)
{
	struct color *color;

	color = malloc(sizeof(struct color));

	color_set(color, r, g, b);

	return color;
}

struct color *color_dup(struct color *color)
{
	return color_new(color->red, color->green, color->blue);
}

int is_color(const char *str)
{
	int n = strlen(str);
	int i;

	if (n != 13 || str[0] != '#')
		return 0;

	for (i = 1; i < n; i++)
		if (isxdigit(str[i]) == 0)
			return 0;

	return 1;
}

struct color *str_to_color(const char *str)
{
	char tmp[5];
	unsigned int red, green, blue;

	if (!is_color(str))
		return NULL;

	strncpy(tmp, str + 1, 4);
	tmp[4] = '\0';
	red = strtol(tmp, NULL, 16);

	strncpy(tmp, str + 5, 4);
	tmp[4] = '\0';
	green = strtol(tmp, NULL, 16);

	strncpy(tmp, str + 9, 4);
	tmp[4] = '\0';
	blue = strtol(tmp, NULL, 16);

	return color_new(((double)red)/65535,
			 ((double)green)/65535,
			 ((double)blue)/65535);
}

char *color_to_str(const struct color *color)
{
	char *str = malloc(1 + 12 + 1);

	sprintf(str, "#%.4x%.4x%.4x",
		(int)(65535 * color->red),
		(int)(65535 * color->green),
		(int)(65535 * color->blue));

	return str;
}
