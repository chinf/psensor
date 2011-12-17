/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include "pio.h"

char **dir_list(const char *dpath, int (*filter) (const char *path))
{
	struct dirent *ent;
	DIR *dir;
	char **paths;
	int n;

	dir = opendir(dpath);

	if (!dir)
		return NULL;

	n = 1;
	paths = malloc(sizeof(void *));
	*paths = NULL;

	while ((ent = readdir(dir)) != NULL) {
		char *fpath;
		char *name = ent->d_name;

		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;

		fpath = malloc(strlen(dpath) + 1 + strlen(name) + 1);

		strcpy(fpath, dpath);
		strcat(fpath, "/");
		strcat(fpath, name);

		if (!filter || filter(fpath)) {
			char **npaths;

			n++;
			npaths = malloc(n * sizeof(void *));
			memcpy(npaths + 1, paths, (n - 1) * sizeof(void *));
			free(paths);
			paths = npaths;
			*npaths = fpath;

		} else {
			free(fpath);
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
