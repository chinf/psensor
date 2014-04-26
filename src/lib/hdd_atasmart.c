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
#define _LARGEFILE_SOURCE 1
#include "config.h"

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atasmart.h>
#include <linux/fs.h>

#include "pio.h"
#include "hdd.h"
#include <plog.h>

static int filter_sd(const char *p)
{
	return strlen(p) == 8 && !strncmp(p, "/dev/sd", 7);
}

static struct psensor *
create_sensor(char *id, char *name, SkDisk *disk, int values_max_length)
{
	struct psensor *s;
	int t;

	t = SENSOR_TYPE_ATASMART | SENSOR_TYPE_HDD | SENSOR_TYPE_TEMP;

	s = psensor_create(id,
			   strdup(name),
			   strdup(_("Disk")),
			   t,
			   values_max_length);

	s->disk = disk;

	return s;
}

/*
 * Performs the same tests than sk_disk_open and outputs the result.
 */
static void analyze_disk(const char *dname)
{
	int f;
	struct stat st;
	uint64_t size;

	log_debug("analyze_disk(hdd_atasmart): %s", dname);

	f = open(dname, O_RDONLY|O_NOCTTY|O_NONBLOCK|O_CLOEXEC);

	if (f < 0) {
		log_debug("analyze_disk(hdd_atasmart): Could not open file %s: %s",
			  dname,
			  strerror(errno));
		goto fail;
	}

	if (fstat(f, &st) < 0) {
		log_debug("analyze_disk(hdd_atasmart): fstat fails %s: %s",
			  dname,
			  strerror(errno));
		goto fail;
	}

	if (!S_ISBLK(st.st_mode)) {
		log_debug("analyze_disk(hdd_atasmart): !S_ISBLK fails %s",
			  dname);
		goto fail;
	}

	size = (uint64_t)-1;
	/* So, it's a block device. Let's make sure the ioctls work */
	if (ioctl(f, BLKGETSIZE64, &size) < 0) {
		log_debug("analyze_disk(hdd_atasmart): ioctl fails %s: %s",
			  dname,
			  strerror(errno));
		goto fail;
	}

	if (size <= 0 || size == (uint64_t) -1) {
		log_debug("analyze_disk(hdd_atasmart): ioctl wrong size %s: %ld",
			  dname,
			  size);
		goto fail;
	}

 fail:
	close(f);
}

struct psensor **hdd_psensor_list_add(struct psensor **sensors,
				      int values_max_length)
{
	char **paths, **tmp, *id;
	SkDisk *disk;
	struct psensor *sensor, **tmp_sensors, **result;

	log_debug("hdd_psensor_list_add(hdd_atasmart)");

	paths = dir_list("/dev", filter_sd);

	result = sensors;
	tmp = paths;
	while (*tmp) {
		log_debug("hdd_psensor_list_add(hdd_atasmart) open %s", *tmp);

		if (!sk_disk_open(*tmp, &disk)) {
			id = malloc(strlen("hdd at") + strlen(*tmp) + 1);
			strcpy(id, "hdd at");
			strcat(id, *tmp);

			sensor = create_sensor(id,
					       *tmp,
					       disk,
					       values_max_length);

			tmp_sensors = psensor_list_add(result, sensor);

			if (result != sensors)
				free(result);

			result = tmp_sensors;
		} else {
			log_err(_("atasmart: sk_disk_open() failure: %s."),
				*tmp);
			analyze_disk(*tmp);
		}

		tmp++;
	}

	paths_free(paths);

	return result;
}

void hdd_psensor_list_update(struct psensor **sensors)
{
	struct psensor **cur, *s;
	uint64_t kelvin;
	int ret;
	double c;

	cur = sensors;
	while (*cur) {
		s = *cur;
		if (!(s->type & SENSOR_TYPE_REMOTE)
		    && s->type & SENSOR_TYPE_ATASMART) {
			ret = sk_disk_smart_read_data(s->disk);

			if (!ret) {
				ret = sk_disk_smart_get_temperature(s->disk,
								    &kelvin);

				if (!ret) {
					c = (kelvin - 273150) / 1000;
					psensor_set_current_value(s, c);
					log_debug("hdd_psensor_list_update(hdd_atasmart): %s %.2f",
						  s->id,
						  c);
				}
			}
		}

		cur++;
	}
}
