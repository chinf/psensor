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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <pio.h>
#include <plog.h>

static char *get_user_autostart_dir()
{
	const char *xdg_cfg_dir;

	xdg_cfg_dir = g_get_user_config_dir();

	log_fct("g_user_config_dir(): %s", xdg_cfg_dir);

	return path_append(xdg_cfg_dir, "autostart");
}

static char *get_user_desktop_file()
{
	char *dir, *path;

	dir = get_user_autostart_dir();
	path = path_append(dir, "psensor.desktop");

	free(dir);

	return path;
}

static const char *get_desktop_file()
{
	return DATADIR"/applications/psensor.desktop";
}

static int is_file_exists(const char *path)
{
	struct stat st;

	return stat(path, &st) == 0;
}

static GKeyFile *get_key_file(const char *path)
{
	GKeyFile *kfile;
	int ret;

	kfile = g_key_file_new();
	ret = g_key_file_load_from_file(kfile,
					path,
					G_KEY_FILE_KEEP_COMMENTS
					| G_KEY_FILE_KEEP_TRANSLATIONS,
					NULL);

	if (ret) {
		return kfile;
	} else {
		log_err("Failed to parse: %s", path);

		g_key_file_free(kfile);
		return NULL;
	}
}

static int is_user_desktop_autostarted(GKeyFile *f)
{
	return (!g_key_file_has_key(f,
				    G_KEY_FILE_DESKTOP_GROUP,
				    "X-GNOME-Autostart-enabled",
				    NULL))
		|| g_key_file_get_boolean(f,
					  G_KEY_FILE_DESKTOP_GROUP,
					  "X-GNOME-Autostart-enabled",
					  NULL);
}

int pxdg_is_autostarted()
{
	char *user_desktop;
	unsigned int ret;
	GKeyFile *kfile;

	log_fct_enter();

	user_desktop = get_user_desktop_file();

	log_fct("user desktop file: %s", user_desktop);

	ret = is_file_exists(user_desktop);

	if (!ret) {
		log_fct("user desktop file does not exist.");
	} else {
		log_fct("user desktop file exist.");
		if (ret) {
			kfile = get_key_file(user_desktop);
			if (kfile)
				ret = is_user_desktop_autostarted(kfile);
			else
				ret = -1;
		}
		g_key_file_free(kfile);
	}

	free(user_desktop);

	log_fct_exit();

	return ret;
}

void pxdg_set_autostart(unsigned int enable)
{
	char *user_desktop, *data;
	GKeyFile *f;

	log_fct_enter();

	user_desktop = get_user_desktop_file();

	log_fct("user desktop file: %s", user_desktop);

	log_fct("desktop file: %s", get_desktop_file());

	if (enable) {
		if (!is_file_exists(user_desktop))
			file_copy(get_desktop_file(), user_desktop);

		f = get_key_file(user_desktop);
		if (f) {
			if (g_key_file_has_key(f,
					       G_KEY_FILE_DESKTOP_GROUP,
					       "X-GNOME-Autostart-enabled",
					       NULL))
				g_key_file_set_boolean
					(f,
					 G_KEY_FILE_DESKTOP_GROUP,
					 "X-GNOME-Autostart-enabled",
					 TRUE);
			data = g_key_file_to_data(f, NULL, NULL);
			g_file_set_contents(user_desktop,
					    data,
					    -1,
					    NULL);

			g_key_file_free(f);
		}
	} else {
		/* because X-GNOME-Autostart-enabled does not turn off
		 * autostart on all Desktop Envs. */
		remove(user_desktop);
	}

	log_fct_exit();

	free(user_desktop);
}
