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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "bool.h"
#include "config.h"
#include "log.h"
#include "slog.h"

static FILE *file;
static struct timeval stv;
static double *last_values;

int slog_init(const char *path, struct psensor **sensors)
{
	file = fopen(path, "a");

	if (!file) {
		log_err(_("Cannot open sensor log file: %s"), path);
		return 0;
	}

	if (gettimeofday(&stv, NULL)) {
		log_err(_("slog_init: gettimeofday failed."));
		return 0;
	}

	fprintf(file, "I,%ld,%s\n", stv.tv_sec, VERSION);

	while (*sensors) {
		fprintf(file, "S,%s,%x\n", (*sensors)->id,  (*sensors)->type);
		sensors++;
	}

	fflush(file);

	return 1;
}



void slog_write_sensors(struct psensor **sensors)
{
	int count, i;
	double v;
	struct timeval tv;
	bool first_call;

	if (!file)
		return ;

	if (gettimeofday(&tv, NULL)) {
		log_err(_("slog_write_sensors: gettimeofday failed."));
		return ;
	}

	count = psensor_list_size(sensors);

	if (last_values) {
		first_call = 0;
	} else {
		first_call = 1;
		last_values = malloc(count * sizeof(double));
	}

	fprintf(file, "%ld", tv.tv_sec - stv.tv_sec);
	for (i = 0; i < count; i++) {
		v = psensor_get_current_value(sensors[i]);

		if (!first_call && last_values[i] == v)
			fputc(',', file);
		else
			fprintf(file, ",%.1f", v);

		last_values[i] = v;
	}

	fputc('\n', file);

	fflush(file);
}

void slog_close()
{
	if (file) {
		fclose(file);
		file = NULL;
		free(last_values);
		last_values = NULL;
	}
}
