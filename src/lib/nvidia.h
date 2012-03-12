/*
 * Copyright (C) 2010-2012 jeanfi@gmail.com
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
#ifndef _PSENSOR_NVIDIA_H_
#define _PSENSOR_NVIDIA_H_

#include "psensor.h"

/*
  Updates temperatures of NVidia sensors.
*/
void nvidia_psensor_list_update(struct psensor **sensors);

/*
  Adds NVIDIA sensors to a given list of sensors.

  Returns the new allocated list of sensors if sensors have been added
  otherwise returns 'sensors'. The list is 'NULL' terminated.
 */
struct psensor **nvidia_psensor_list_add(struct psensor **sensors,
					 int values_max_length);

void nvidia_cleanup();

#endif
