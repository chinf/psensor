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
#ifndef _PSENSOR_SERVER_H_
#define _PSENSOR_SERVER_H_

#include "config.h"

#include "psensor.h"

#ifdef HAVE_GTOP
#include "sysinfo.h"
#endif

#define URL_BASE_API_1_1 "/api/1.1"
#define URL_BASE_API_1_1_SENSORS "/api/1.1/sensors"
#define URL_API_1_1_SERVER_STOP "/api/1.1/server/stop"
#define URL_API_1_1_SYSINFO "/api/1.1/sysinfo"
#define URL_API_1_1_CPU_USAGE "/api/1.1/cpu/usage"

struct server_data {
	struct psensor *cpu_usage;
	struct psensor **sensors;
#ifdef HAVE_GTOP
	struct psysinfo psysinfo;
#endif
	char *www_dir;
};

#endif
