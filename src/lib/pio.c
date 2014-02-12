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

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include "pio.h"

static char *path_append(const char *dir, const char *path)
{
	char *result;

	result = malloc(strlen(dir) + 1 + strlen(path) + 1);

	strcpy(result, dir);
	strcat(result, "/");
	strcat(result, path);

	return result;
}

static char **paths_add(char **paths, int n, char *path)
{
	char **result;

	result = malloc((n+1) * sizeof(void *));

	memcpy(result + 1, paths, n * sizeof(void *));

	*result = path;

	return result;
}

char **dir_list(const char *dpath, int (*filter) (const char *))
{
	struct dirent *ent;
	DIR *dir;
	char **paths, *path, *name, **tmp;
	int n;

	dir = opendir(dpath);

	if (!dir)
		return NULL;

	n = 1;
	paths = malloc(sizeof(void *));
	*paths = NULL;

	while ((ent = readdir(dir)) != NULL) {
		name = ent->d_name;

		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;

		path = path_append(dpath, name);

		if (!filter || filter(path)) {
			tmp = paths_add(paths, n, path);
			free(paths);
			paths = tmp;

			n++;
		} else {
			free(path);
		}
	}

	closedir(dir);

	return paths;
}

void paths_free(char **paths)
{
	char **paths_cur;

	paths_cur = paths;
	while (*paths_cur) {
		free(*paths_cur);

		paths_cur++;
	}

	free(paths);
}
