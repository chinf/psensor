/*
 *  Copyright (C) 2010-2016 jeanfi@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301 USA
 */
#ifndef _P_LOG_H_
#define _P_LOG_H_

#define P_LOG_VER 3

enum log_level {
	LOG_ERR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
};

void log_open(const char *path);
void log_close(void);

void log_printf(int lvl, const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_err(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);

void _log(const char *fct, const char *fmt, ...);

#define log_fct(...) _log(__func__, __VA_ARGS__)
#define log_fct_enter() log_fct("ENTER")
#define log_fct_exit() log_fct("EXIT")

/* level of the log file. */
extern int log_level;

#endif
