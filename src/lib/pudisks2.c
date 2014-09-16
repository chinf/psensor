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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <string.h>

#include <udisks/udisks.h>

#include <pudisks2.h>
#include <temperature.h>

const char *PROVIDER_NAME = "udisks2";

static GDBusObjectManager *manager;

void udisks2_psensor_list_update(struct psensor **sensors)
{
	struct psensor *s;
	GDBusObject *o;
	UDisksDriveAta *drive_ata;
	double v;
	GVariant *variant;

	for (; *sensors; sensors++) {
		s = *sensors;

		if (s->type & SENSOR_TYPE_UDISKS2) {
			o = g_dbus_object_manager_get_object(manager,
							     s->udisks2_path);

			if (!o)
				continue;

			g_object_get(o, "drive-ata", &drive_ata, NULL);

			variant = g_variant_new_parsed
				("{'nowakeup': %v}",
				 g_variant_new_boolean(TRUE));

			udisks_drive_ata_call_smart_update_sync(drive_ata,
								variant,
								NULL,
								NULL);

			v = udisks_drive_ata_get_smart_temperature(drive_ata);

			psensor_set_current_value(s, kelvin_to_celsius(v));

			g_object_unref(G_OBJECT(o));
		}
	}
}

void udisks2_psensor_list_add(struct psensor ***sensors, int values_length)
{
	UDisksClient *client;
	GList *objects, *cur;
	UDisksDrive *drive;
	UDisksDriveAta *drive_ata;
	int i, type;
	char *id, *name, *chip;
	const char *path, *drive_id, *drive_model;
	struct psensor *s;

	log_fct_enter();

	client = udisks_client_new_sync(NULL, NULL);

	if (!client) {
		log_err(_("%s: cannot get the udisks2 client"), PROVIDER_NAME);
		log_fct_exit();
		return;
	}

	manager = udisks_client_get_object_manager(client);

	objects = g_dbus_object_manager_get_objects(manager);

	i = 0;
	for (cur = objects; cur; cur = cur->next) {
		path = g_dbus_object_get_object_path(cur->data);

		g_object_get(cur->data,
			     "drive", &drive,
			     "drive-ata", &drive_ata,
			     NULL);

		if (!drive) {
			log_fct("Not a drive: %s", path);
			continue;
		}

		if (!drive_ata) {
			log_fct("Not an ATA drive: %s", path);
			continue;
		}

		if (!udisks_drive_ata_get_smart_enabled(drive_ata)) {
			log_fct("SMART not enabled: %s", path);
			continue;
		}

		if (!udisks_drive_ata_get_smart_temperature(drive_ata)) {
			log_fct("No temperature available: %s", path);
			continue;
		}

		drive_id = udisks_drive_get_id(drive);
		if (drive_id) {
			id = g_strdup_printf("%s %s", PROVIDER_NAME, drive_id);
		} else {
			id = g_strdup_printf("%s %d", PROVIDER_NAME, i);
			i++;
		}

		drive_model = udisks_drive_get_model(drive);
		if (drive_model) {
			name = strdup(drive_model);
			chip = strdup(drive_model);
		} else {
			name = strdup(_("Disk"));
			chip = strdup(_("Disk"));
		}

		type = SENSOR_TYPE_TEMP | SENSOR_TYPE_UDISKS2 | SENSOR_TYPE_HDD;

		s = psensor_create(id, name, chip, type, values_length);
		s->udisks2_path = strdup(path);

		psensor_list_append(sensors, s);

		g_object_unref(G_OBJECT(cur->data));
	}

	g_list_free(objects);

	log_fct_exit();
}
