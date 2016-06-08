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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atasmart.h>
#include <linux/fs.h>

#include <pio.h>
#include <hdd.h>
#include <plog.h>

static const char *PROVIDER_NAME = "atasmart";

static int filter_sd(const char *p)
{
	return strlen(p) == 8 && !strncmp(p, "/dev/sd", 7);
}

static void provider_data_free(void *data)
{
	sk_disk_free((SkDisk *)data);
}

static SkDisk *get_disk(struct psensor *s)
{
	return (SkDisk *)s->provider_data;
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

	s->provider_data = disk;
	s->provider_data_free_fct = &provider_data_free;

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

	log_fct("Analyze %s", dname);

	f = open(dname, O_RDONLY|O_NOCTTY|O_NONBLOCK|O_CLOEXEC);

	if (f < 0) {
		log_fct("Could not open file %s: %s", dname, strerror(errno));
		goto fail;
	}

	if (fstat(f, &st) < 0) {
		log_fct("fstat fails %s: %s", dname, strerror(errno));
		goto fail;
	}

	if (!S_ISBLK(st.st_mode)) {
		log_fct("!S_ISBLK fails %s", dname);
		goto fail;
	}

	size = (uint64_t)-1;
	/* So, it's a block device. Let's make sure the ioctls work */
	if (ioctl(f, BLKGETSIZE64, &size) < 0) {
		log_fct("ioctl fails %s: %s", dname, strerror(errno));
		goto fail;
	}

	if (size <= 0 || size == (uint64_t) -1) {
		log_fct("ioctl wrong size %s: %ld", dname, size);
		goto fail;
	}

 fail:
	close(f);
}

void
atasmart_psensor_list_append(struct psensor ***sensors, int values_max_length)
{
	char **paths, **tmp, *id;
	SkDisk *disk;
	struct psensor *sensor;

	log_fct_enter();

	paths = dir_list("/dev", filter_sd);

	tmp = paths;
	while (*tmp) {
		log_fct("Open %s", *tmp);

		if (!sk_disk_open(*tmp, &disk)) {
			id = malloc(strlen(PROVIDER_NAME)
				    + 1
				    + strlen(*tmp)
				    + 1);
			sprintf(id, "%s %s", PROVIDER_NAME, *tmp);

			sensor = create_sensor(id,
					       *tmp,
					       disk,
					       values_max_length);

			psensor_list_append(sensors, sensor);
		} else {
			log_err(_("%s: sk_disk_open() failure: %s."),
				PROVIDER_NAME,
				*tmp);
			analyze_disk(*tmp);
		}

		tmp++;
	}

	paths_free(paths);

	log_fct_exit();
}

void atasmart_psensor_list_update(struct psensor **sensors)
{
	struct psensor **cur, *s;
	uint64_t kelvin;
	int ret;
	double c;
	SkDisk *disk;

	if (!sensors)
		return;

	cur = sensors;
	while (*cur) {
		s = *cur;
		if (!(s->type & SENSOR_TYPE_REMOTE)
		    && s->type & SENSOR_TYPE_ATASMART) {
			disk = get_disk(s);

			ret = sk_disk_smart_read_data(disk);

			if (!ret) {
				ret = sk_disk_smart_get_temperature(disk,
								    &kelvin);

				if (!ret) {
					c = (kelvin - 273150) / 1000;
					psensor_set_current_value(s, c);
					log_fct("%s %.2f", s->id, c);
				}
			}
		}

		cur++;
	}
}
