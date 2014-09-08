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

/*
 * Following code is based on GNOME sensors applet code
 * hddtemp-plugin.c see http://sensors-applet.sourceforge.net/
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "hdd.h"
#include "psensor.h"

#define HDDTEMP_SERVER_IP_ADDRESS "127.0.0.1"
#define HDDTEMP_PORT_NUMBER 7634
#define HDDTEMP_OUTPUT_BUFFER_LENGTH 4048

struct hdd_info {
	char *name;
	int temp;
};

static char *fetch()
{
	int sockfd;
	ssize_t n = 1;
	int output_length = 0;
	char *pc;
	char *buffer;
	struct sockaddr_in address;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		log_err(_("hddtemp: failed to open socket."));
		return NULL;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HDDTEMP_SERVER_IP_ADDRESS);
	address.sin_port = htons(HDDTEMP_PORT_NUMBER);

	buffer = NULL;

	if (connect(sockfd,
		    (struct sockaddr *)&address,
		    (socklen_t) sizeof(address)) == -1) {
		log_err(_("hddtemp: failed to open connection."));
	} else {
		buffer = malloc(HDDTEMP_OUTPUT_BUFFER_LENGTH);

		pc = buffer;
		while ((n = read(sockfd,
				 pc,
				 HDDTEMP_OUTPUT_BUFFER_LENGTH -
				 output_length)) > 0) {

			output_length += n;
			pc = &pc[n];
		}

		buffer[output_length] = '\0';
	}

	close(sockfd);

	return buffer;
}

static int str_index(char *str, char d)
{
	char *c;
	int i;

	if (!str || *str == '\0')
		return -1;

	c = str;

	i = 0;
	while (*c) {
		if (*c == d)
			return i;
		i++;
		c++;
	}

	return -1;
}

static struct psensor *
create_sensor(char *id, char *name, int values_max_length)
{
	int t;

	t = SENSOR_TYPE_HDD | SENSOR_TYPE_HDDTEMP | SENSOR_TYPE_TEMP;

	return psensor_create(id, name, strdup(_("Disk")),
			      t,
			      values_max_length);
}

static char *next_hdd_info(char *string, struct hdd_info *info)
{
	char *c;
	int idx_name_n, i, temp;

	if (!string || strlen(string) <= 5	/* at least 5 pipes */
	    || string[0] != '|')
		return NULL;

	/* skip first pipe */
	c = string + 1;

	/* name */
	idx_name_n = str_index(c, '|');

	if (idx_name_n == -1)
		return NULL;
	c = c + idx_name_n + 1;

	/* skip label */
	i = str_index(c, '|');
	if (i == -1)
		return NULL;
	c = c + i + 1;

	/* temp */
	i = str_index(c, '|');
	if (i == -1)
		return NULL;
	temp = atoi(c);
	c = c + i + 1;

	/* skip unit  */
	i = str_index(c, '|');
	if (i == -1)
		return NULL;
	c = c + i + 1;

	info->name = malloc(idx_name_n + 1);
	strncpy(info->name, string + 1, idx_name_n);
	info->name[idx_name_n] = '\0';

	info->temp = temp;

	return c;
}

struct psensor **hddtemp_psensor_list_add(struct psensor **sensors,
					  int values_max_length)
{
	char *hddtemp_output = fetch();
	char *c;
	struct hdd_info info;
	struct psensor **result;

	if (!hddtemp_output)
		return sensors;

	if (hddtemp_output[0] != '|') {
		log_err(_("hddtemp: wrong string: %s."), hddtemp_output);

		free(hddtemp_output);

		return sensors;
	}

	c = hddtemp_output;

	result = sensors;

	while (c && (c = next_hdd_info(c, &info))) {
		struct psensor *sensor;
		struct psensor **tmp_sensors;

		char *id = malloc(strlen("hddtemp ") + strlen(info.name) + 1);
		strcpy(id, "hddtemp ");
		strcat(id, info.name);

		sensor = create_sensor(id, info.name, values_max_length);

		tmp_sensors = psensor_list_add(result, sensor);

		if (result != sensors)
			free(result);

		result = tmp_sensors;
	}

	free(hddtemp_output);

	return result;
}

static void update(struct psensor **sensors, struct hdd_info *info)
{
	struct psensor **sensor_cur = sensors;

	while (*sensor_cur) {
		if (!((*sensor_cur)->type & SENSOR_TYPE_REMOTE)
		    && (*sensor_cur)->type & SENSOR_TYPE_HDDTEMP
		    && !strcmp((*sensor_cur)->id + 8, info->name))
			psensor_set_current_value(*sensor_cur,
						  (float)info->temp);

		sensor_cur++;
	}
}

void hddtemp_psensor_list_update(struct psensor **sensors)
{
	char *hddtemp_output = fetch();

	if (!hddtemp_output)
		return;

	if (hddtemp_output[0] == '|') {

		char *c = hddtemp_output;
		struct hdd_info info;
		info.name = NULL;
		info.temp = 0;

		while (c && (c = next_hdd_info(c, &info))) {

			update(sensors, &info);

			free(info.name);
		}
	} else {
		log_err(_("hddtemp: wrong string: %s."), hddtemp_output);
	}

	free(hddtemp_output);
}
