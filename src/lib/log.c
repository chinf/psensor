/*
 * Copyright (C) 2010-2013 jeanfi@gmail.com
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "log.h"
#include "ptime.h"

static FILE *file;
int log_level =  LOG_WARN;

void log_open(const char *path)
{
	file = fopen(path, "a");

	if (!file)
		log_printf(LOG_ERR, _("Cannot open log file: %s"), path);
}

void log_close()
{
	if (!file)
		return ;

	fclose(file);

	file = NULL;
}


#define LOG_BUFFER 4096
static void vlogf(int lvl, const char *fmt, va_list ap)
{
	char buffer[1 + LOG_BUFFER];
	char *lvl_str, *t;
	FILE *stdf;

	if (lvl > LOG_INFO && (!file || lvl > log_level))
		return ;

	vsnprintf(buffer, LOG_BUFFER, fmt, ap);
	buffer[LOG_BUFFER] = '\0';

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

	t = get_time_str();
	if (!t)
		return ;

	if (file && lvl <= log_level) {
		fprintf(file, "[%s] %s %s\n", t, lvl_str, buffer);
		fflush(file);
	} else {
		t = NULL;
	}

	if (lvl <= LOG_INFO) {
		if (lvl == LOG_WARN || lvl == LOG_ERR)
			stdf = stderr;
		else
			stdf = stdout;


		fprintf(stdf, "[%s] %s %s\n", t, lvl_str, buffer);
	}

	free(t);
}

void log_printf(int lvl, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlogf(lvl, fmt, ap);
	va_end(ap);
}

void log_debug(const char *fmt, ...)
{
	va_list ap;

	if (log_level < LOG_DEBUG)
		return ;

	va_start(ap, fmt);
	vlogf(LOG_DEBUG, fmt, ap);
	va_end(ap);
}

void log_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlogf(LOG_ERR, fmt, ap);
	va_end(ap);
}

void log_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlogf(LOG_WARN, fmt, ap);
	va_end(ap);
}

void log_info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlogf(LOG_INFO, fmt, ap);
	va_end(ap);
}
