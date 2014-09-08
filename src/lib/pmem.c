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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <glibtop/mem.h>

#include <pmem.h>

static const char *ID_MEM_FREE = "sys memory free";

void mem_psensor_list_update(struct psensor **sensors)
{
	struct psensor *s;
	glibtop_mem mem;
	double v;

	while (*sensors) {
		s = *sensors;

		if (!strcmp(s->id, ID_MEM_FREE)) {
			glibtop_get_mem(&mem);
			v = mem.free * 100 / mem.total;

			psensor_set_current_value(s, v);
		}

		sensors++;
	}
}

void mem_psensor_list_add(struct psensor ***sensors, int values_max_len)
{
	struct psensor *s;

	s = psensor_create(strdup(ID_MEM_FREE),
			   _("free memory"),
			   _("memory"),
			   SENSOR_TYPE_MEMORY | SENSOR_TYPE_PERCENT,
			   values_max_len);

	psensor_list_append(sensors, s);
}
