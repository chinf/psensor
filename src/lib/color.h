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
#ifndef _PSENSOR_COLOR_H_
#define _PSENSOR_COLOR_H_

/*
 * Represents a RGB color.
 *
 * Contains integer and floating RGB representation to avoid useless
 * conversion.
 *
 * Uses color_set to maintain the coherence of the both
 * representation.
 */
struct color {
	/* rgb 0..65535 */
	unsigned int red;
	unsigned int green;
	unsigned int blue;

	/* rgb floating 0..1 */
	double f_red;
	double f_green;
	double f_blue;
};

/** rgb 0..65535 */
struct color *color_new(unsigned int r, unsigned int g, unsigned int b);

struct color *color_dup(struct color *);

/** rgb 0..65535 */
void color_set(struct color *,
	       unsigned int r,
	       unsigned int g,
	       unsigned int b);

/** rgb 0..1 */
void color_set_f(struct color *, double r, double g, double b);

int is_color(const char *str);

struct color *str_to_color(const char *str);

char *color_to_str(const struct color *color);

#endif
