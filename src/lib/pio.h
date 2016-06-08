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

#ifndef _P_IO_H
#define _P_IO_H

#define P_IO_VER 6

/* Returns '1' if a given 'path' denotates a directory else returns
 * 0
 */
int is_dir(const char *path);

/* Returns '1' if a given 'path' denotates a file else returns
 * 0
 */
int is_file(const char *path);

/* Returns a normalized path */
char *path_normalize(const char *dpath);

/* Returns the null-terminated entries of a directory */
char **dir_list(const char *dpath, int (*filter) (const char *path));
void paths_free(char **paths);

char *path_append(const char *dir, const char *path);

/*
 * Returns the size of a file.
 * Returns '-1' if the size cannot be retrieved or not a file.
 */
long file_get_size(const char *path);

/*
 * Returns the content of a file.
 * Returns 'NULL' if the file cannot be read or failed to allocate
 * enough memory.
 * Returns an empty string if the file exists but is empty.
 */
char *file_get_content(const char *path);

enum file_copy_error {
	FILE_COPY_ERROR_OPEN_SRC = 1,
	FILE_COPY_ERROR_OPEN_DST,
	FILE_COPY_ERROR_READ,
	FILE_COPY_ERROR_WRITE,
	FILE_COPY_ERROR_ALLOC_BUFFER
};

void file_copy_print_error(int code, const char *src, const char *dst);

/*
 * Copy a file.
 *
 * Returns '0' if sucessfull, otherwise return the error code.
 */
int file_copy(const char *src, const char *dst);

int dir_rcopy(const char *, const char *);

void mkdirs(const char *dirs, mode_t mode);

#endif
