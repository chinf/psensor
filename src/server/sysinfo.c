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
#include <glibtop/cpu.h>
#include <glibtop/netlist.h>
#include <glibtop/netload.h>

#include "config.h"

#ifdef HAVE_JSON_0
#include <json/json.h>
#else
#include <json-c/json.h>
#endif

#include "sysinfo.h"

static glibtop_cpu *cpu;
static float last_used;
static float last_total;

void sysinfo_update(struct psysinfo *info)
{
	unsigned long int used = 0;
	unsigned long int dt;
	glibtop_netlist buf;

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

	glibtop_get_loadavg(&info->loadavg);
	glibtop_get_mem(&info->mem);
	glibtop_get_swap(&info->swap);
	glibtop_get_uptime(&info->uptime);

	/* network */
	if (!info->interfaces)
		info->interfaces = glibtop_get_netlist(&buf);
}

void sysinfo_cleanup(void)
{
	if (cpu)
		g_free(cpu);
}

static json_object *ram_to_json_object(const struct psysinfo *s)
{
	json_object *obj = json_object_new_object();

	json_object_object_add(obj, "total",
			       json_object_new_double(s->mem.total));

	json_object_object_add(obj, "free",
			       json_object_new_double(s->mem.free));

	json_object_object_add(obj, "shared",
			       json_object_new_double(s->mem.shared));

	json_object_object_add(obj, "buffer",
			       json_object_new_double(s->mem.buffer));

	return obj;
}

static json_object *swap_to_json_object(const struct psysinfo *s)
{
	json_object *obj = json_object_new_object();

	json_object_object_add(obj, "total",
			       json_object_new_double(s->swap.total));

	json_object_object_add(obj, "free",
			       json_object_new_double(s->swap.free));

	return obj;
}

static json_object *netif_to_json_object(const char *netif)
{
	glibtop_netload buf;
	json_object *obj = json_object_new_object();

	json_object_object_add(obj, "name", json_object_new_string(netif));

	glibtop_get_netload(&buf, netif);

	json_object_object_add(obj, "bytes_in",
			       json_object_new_double(buf.bytes_in));

	json_object_object_add(obj, "bytes_out",
			       json_object_new_double(buf.bytes_out));

	return obj;
}

static json_object *net_to_json_object(const struct psysinfo *s)
{
	char **netif = s->interfaces;
	json_object *net = json_object_new_array();

	while (*netif) {
		json_object_array_add(net, netif_to_json_object(*netif));

		netif++;
	}

	return net;
}

static json_object *sysinfo_to_json_object(const struct psysinfo *s)
{
	json_object *obj;

	obj = json_object_new_object();

	json_object_object_add(obj, "load",
			       json_object_new_double(s->cpu_rate));

	json_object_object_add
		(obj, "load_1",
		 json_object_new_double(s->loadavg.loadavg[0]));

	json_object_object_add
		(obj, "load_5",
		 json_object_new_double(s->loadavg.loadavg[1]));

	json_object_object_add
		(obj, "load_15",
		 json_object_new_double(s->loadavg.loadavg[2]));

	json_object_object_add
		(obj, "uptime", json_object_new_double(s->uptime.uptime));

	json_object_object_add
		(obj, "mem_unit", json_object_new_double(1));

	json_object_object_add(obj, "ram", ram_to_json_object(s));
	json_object_object_add(obj, "swap", swap_to_json_object(s));
	json_object_object_add(obj, "net", net_to_json_object(s));

	return obj;
}

char *sysinfo_to_json_string(const struct psysinfo *s)
{
	char *str;
	json_object *obj = sysinfo_to_json_object(s);

	str = strdup(json_object_to_json_string(obj));

	json_object_put(obj);

	return str;
}
