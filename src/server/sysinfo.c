/*
    Copyright (C) 2010-2011 jeanfi@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include <stdlib.h>
#include <glibtop/cpu.h>
#include <sys/sysinfo.h>

#include "sysinfo.h"

static glibtop_cpu *cpu;
static float last_used;
static float last_total;

void sysinfo_update(struct psysinfo *info)
{
	unsigned long int used = 0;
	unsigned long int dt;

	/* cpu */
	if (!cpu)
		cpu = malloc(sizeof(glibtop_cpu));

	glibtop_get_cpu(cpu);

	used = cpu->user + cpu->nice + cpu->sys;

	dt = cpu->total - last_total;

	if (dt)
		info->cpu_rate = (used - last_used) / dt;

	last_used = used;
	last_total = cpu->total;

	/* memory */
	sysinfo(&info->sysinfo);
}

void sysinfo_cleanup()
{
	if (cpu)
		free(cpu);
}
