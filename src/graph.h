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
#ifndef _PSENSOR_GRAPH_H_
#define _PSENSOR_GRAPH_H_

#include <gtk/gtk.h>

#include <cfg.h>
#include <psensor.h>

extern bool is_smooth_curves_enabled;

void graph_update(struct psensor **sensors,
		  GtkWidget *w_graph,
		  struct config *config,
		  GtkWidget *window);

/* Compute the number of measures which must be kept. */
int compute_values_max_length(struct config *);

#endif
