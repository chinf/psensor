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

#include <stdio.h>

#include <plog.h>
#include <pmutex.h>
#include <string.h>

int pmutex_lock(pthread_mutex_t *m)
{
	int ret;

	ret = pthread_mutex_lock(m);

	if (ret)
		log_err("pmutex_lock: %p %d %s", m, ret, strerror(ret));

	return ret;
}

int pmutex_unlock(pthread_mutex_t *m)
{
	int ret;

	ret = pthread_mutex_unlock(m);

	if (ret)
		log_err("pmutex_unlock: %p %d", m, ret);

	return ret;
}

int pmutex_init(pthread_mutex_t *m)
{
	pthread_mutexattr_t attr;
	int ret;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

	ret = pthread_mutex_init(m, &attr);

	if (ret)
		log_err("pmutex_init: %p %d", m, ret);

	return ret;
}
