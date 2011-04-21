/*
    Copyright (C) 2010-2011 wpitchoune@gmail.com

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

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include "plib_io.h"

int is_dir(const char *path)
{
	struct stat st;

	int ret = lstat(path, &st);

	if (ret == 0 && S_ISDIR(st.st_mode))
		return 1;

	return 0;
}

int is_file(const char *path)
{
	struct stat st;

	int ret = lstat(path, &st);

	if (ret == 0 && S_ISREG(st.st_mode))
		return 1;

	return 0;
}

char *dir_normalize(const char *dpath)
{
	char *npath;
	int n;

	if (!dpath || !strlen(dpath))
		return NULL;

	npath = strdup(dpath);

	n = strlen(npath);

	if (n > 1 && npath[n - 1] == '/')
		npath[n - 1] = '\0';

	return npath;
}

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

char *file_get_content(const char *fpath)
{
	long size;

	char *page;

	size = file_get_size(fpath);

	if (size == -1) {
		page = NULL;

	} else if (size == 0) {
		page = malloc(1);
		*page = '\0';

	} else {
		FILE *fp = fopen(fpath, "rb");
		if (fp) {
			page = malloc(size + 1);
			if (!page || size != fread(page, 1, size, fp)) {
				free(page);
				return NULL;
			}

			*(page + size) = '\0';

			fclose(fp);
		} else {
			page = NULL;
		}
	}

	return page;
}

long file_get_size(const char *path)
{
	FILE *fp;

	if (!is_file(path))
		return -1;

	fp = fopen(path, "rb");
	if (fp) {
		long size;

		if (fseek(fp, 0, SEEK_END) == -1)
			return -1;

		size = ftell(fp);

		fclose(fp);

		return size;
	}

	return -1;
}

#define FCOPY_BUF_SZ 4096
static int FILE_copy(FILE *src, FILE *dst)
{
	int ret = 0;
	char *buf = malloc(FCOPY_BUF_SZ);
	int n;

	if (!buf)
		return FILE_COPY_ERROR_ALLOC_BUFFER;

	while (!ret) {
		n = fread(buf, 1, FCOPY_BUF_SZ, src);
		if (n) {
			if (fwrite(buf, 1, n, dst) != n)
				ret = FILE_COPY_ERROR_WRITE;
		} else {
			if (!feof(src))
				ret = FILE_COPY_ERROR_READ;
			else
				break;
		}
	}

	free(buf);

	return ret;
}

int
file_copy(const char *src, const char *dst)
{
	FILE *fsrc, *fdst;
	int ret = 0;

	fsrc = fopen(src, "r");

	if (fsrc) {
		fdst = fopen(dst, "w+");

		if (fdst) {
			ret = FILE_copy(fsrc, fdst);
			fclose(fdst);
		} else {
			ret = FILE_COPY_ERROR_OPEN_DST;
		}

		fclose(fsrc);
	} else {
		ret = FILE_COPY_ERROR_OPEN_SRC;
	}

	return ret;
}

char *path_append(const char *dir, const char *path)
{
	char *ret, *ndir;

	ndir = dir_normalize(dir);

	if (!ndir && (!path || !strlen(path)))
		ret = NULL;

	else if (!ndir) {
		ret = strdup(path);

	} else if (!path || !strlen(path)) {
		return ndir;

	} else {
		ret = malloc(strlen(ndir) + 1 + strlen(path) + 1);
		strcpy(ret, ndir);
		strcat(ret, "/");
		strcat(ret, path);
	}

	free(ndir);

	return ret;
}

void
file_copy_print_error(int code, const char *src, const char *dst)
{
	switch (code) {
	case 0:
		break;
	case FILE_COPY_ERROR_OPEN_SRC:
		printf("File copy error: failed to open %s.\n", src);
		break;
	case FILE_COPY_ERROR_OPEN_DST:
		printf("File copy error: failed to open %s.\n", dst);
		break;
	case FILE_COPY_ERROR_READ:
		printf("File copy error: failed to read %s.\n", src);
		break;
	case FILE_COPY_ERROR_WRITE:
		printf("File copy error: failed to write %s.\n", src);
		break;
	case FILE_COPY_ERROR_ALLOC_BUFFER:
		printf("File copy error: failed to allocate buffer.\n");
		break;
	default:
		printf("File copy error: unknown error %d.\n", code);
	}
}
