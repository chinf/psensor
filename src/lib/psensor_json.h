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
#ifndef _PSENSOR_PSENSOR_JSON_H_
#define _PSENSOR_PSENSOR_JSON_H_

#include "config.h"

#ifdef HAVE_JSON_0
#include <json/json.h>
#else
#include <json-c/json.h>
#endif

#include "psensor.h"

char *sensor_to_json_string(struct psensor *s);
char *sensors_to_json_string(struct psensor **sensors);

/*
 * Creates a new allocated psensor corresponding to a given json
 * representation.
 */
struct psensor *psensor_new_from_json(json_object *o,
				      const char *sensors_url,
				      int values_max_length);
#endif
