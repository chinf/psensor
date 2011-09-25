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

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdio.h>

#include "log.h"

static FILE *file;
static int log_level =  LOG_DEBUG;

void log_open(const char *path, int lvl)
{
	file = fopen(path, "a");

	if (file)
		log_puts(LOG_INFO, "Start logging");
	else
		fprintf(stderr, _("Cannot open log file: %s\n"), path);
}

void log_puts(int lvl, const char *msg)
{
	if (!file || lvl > log_level)
		return ;

	switch (lvl) {
	case LOG_WARN:
		fputs("[WARN] ", file);
		break;
	case LOG_ERR:
		fputs("[ERR] ", file);
		break;
	case LOG_DEBUG:
		fputs("[DEBUG] ", file);
		break;
	case LOG_INFO:
		fputs("[INFO] ", file);
		break;
	default:
		fputs("[??] ", file);
	}

	fputs(msg, file);
	fputc('\n', file);

	fflush(file);
}

void log_close()
{
	if (!file)
		return ;

	fclose(file);

	file = NULL;
}

