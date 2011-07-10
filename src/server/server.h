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

#ifndef _PSENSOR_SERVER_H_
#define _PSENSOR_SERVER_H_

#include "psensor.h"
#include "sysinfo.h"

#define URL_BASE_API_1_0 "/api/1.0"
#define URL_BASE_API_1_0_SENSORS "/api/1.0/sensors"
#define URL_API_1_0_SERVER_STOP "/api/1.0/server/stop"
#define URL_API_1_0_SYSINFO "/api/1.0/sysinfo"

struct server_data {
	struct psensor **sensors;
	struct psysinfo psysinfo;
	char *www_dir;
};

#endif
