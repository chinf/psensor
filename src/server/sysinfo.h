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
#ifndef _PSENSOR_SYSINFO_H_
#define _PSENSOR_SYSINFO_H_

#include <config.h>
#include <glibtop/loadavg.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/uptime.h>

struct psysinfo {
	glibtop_loadavg loadavg;
	glibtop_mem mem;
	glibtop_swap swap;
	glibtop_uptime uptime;

	float cpu_rate;

	char **interfaces;
};

void sysinfo_update(struct psysinfo *sysinfo);
void sysinfo_cleanup(void);

char *sysinfo_to_json_string(const struct psysinfo *sysinfo);

#endif
