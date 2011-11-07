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

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#include "log.h"

static FILE *file;
int log_level =  LOG_WARN;

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
	log_printf(lvl, msg);
}

void log_close()
{
	if (!file)
		return ;

	fclose(file);

	file = NULL;
}

#define LOG_BUFFER 4096
void log_printf(int lvl, const char *fmt, ...)
{
	struct timeval tv;
	static char buffer[1 + LOG_BUFFER];
	va_list ap;
	char *lvl_str;

	if (!file || lvl > log_level)
		return ;

	va_start(ap, fmt);
	vsnprintf(buffer, LOG_BUFFER, fmt, ap);
	buffer[LOG_BUFFER] = '\0';
	va_end(ap);

	if (gettimeofday(&tv, NULL) != 0)
		timerclear(&tv);

	switch (lvl) {
	case LOG_WARN:
		lvl_str = "[WARN]";
		break;
	case LOG_ERR:
		lvl_str = "[ERR]";
		break;
	case LOG_DEBUG:
		lvl_str = "[DEBUG]";
		break;
	case LOG_INFO:
		lvl_str = "[INFO]";
		break;
	default:
		lvl_str = "[??]";
	}

	fprintf(file, "[%ld] %s %s\n", tv.tv_sec, lvl_str, buffer);
	fflush(file);
}

