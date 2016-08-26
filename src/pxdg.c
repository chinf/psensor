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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <pio.h>
#include <plog.h>
#include <pxdg.h>

static const char *KEY_GNOME_AUTOSTART = "X-GNOME-Autostart-enabled";

static char *get_user_autostart_dir(void)
{
	const char *xdg_cfg_dir;

	xdg_cfg_dir = g_get_user_config_dir();

	log_fct("g_user_config_dir(): %s", xdg_cfg_dir);

	return path_append(xdg_cfg_dir, "autostart");
}

static char *get_user_desktop_file(void)
{
	char *dir, *path;

	dir = get_user_autostart_dir();
	path = path_append(dir, PSENSOR_DESKTOP_FILE);

	free(dir);

	return path;
}

static const char *get_desktop_file(void)
{
	return DATADIR"/applications/"PSENSOR_DESKTOP_FILE;
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

	if (ret)
		return kfile;

	log_err("Failed to parse: %s", path);

	g_key_file_free(kfile);
	return NULL;
}

static int is_user_desktop_autostarted(GKeyFile *f)
{
	return (!g_key_file_has_key(f,
				    G_KEY_FILE_DESKTOP_GROUP,
				    KEY_GNOME_AUTOSTART,
				    NULL))
		|| g_key_file_get_boolean(f,
					  G_KEY_FILE_DESKTOP_GROUP,
					  KEY_GNOME_AUTOSTART,
					  NULL);
}

int pxdg_is_autostarted(void)
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
			g_key_file_free(kfile);
		}
	}

	free(user_desktop);

	log_fct_exit();

	return ret;
}

static void enable_gnome_autostart(const char *path)
{
	GKeyFile *f;
	char *data;

	f = get_key_file(path);
	if (f) {
		if (g_key_file_has_key(f,
				       G_KEY_FILE_DESKTOP_GROUP,
				       KEY_GNOME_AUTOSTART,
				       NULL))
			g_key_file_set_boolean(f,
					       G_KEY_FILE_DESKTOP_GROUP,
					       KEY_GNOME_AUTOSTART,
					       TRUE);
		data = g_key_file_to_data(f, NULL, NULL);
		g_file_set_contents(path, data, -1, NULL);

		g_key_file_free(f);
	} else {
		log_err("Fail to enable %s", KEY_GNOME_AUTOSTART);
	}
}

void pxdg_set_autostart(unsigned int enable)
{
	char *user_desktop, *dir;

	log_fct_enter();

	user_desktop = get_user_desktop_file();

	log_fct("user desktop file: %s", user_desktop);

	log_fct("desktop file: %s", get_desktop_file());

	if (enable) {
		if (!is_file_exists(user_desktop)) {
			dir = get_user_autostart_dir();
			mkdirs(dir, 0700);
			free(dir);
			file_copy(get_desktop_file(), user_desktop);
		}
		enable_gnome_autostart(user_desktop);
	} else {
		/* because X-GNOME-Autostart-enabled does not turn off
		 * autostart on all Desktop Envs.
		 */
		remove(user_desktop);
	}

	log_fct_exit();

	free(user_desktop);
}
