/*
    Copyright (C) 2010-2011 jeanfi@gmail.com

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
#include <sys/time.h>
#include <gtk/gtk.h>

#include "cfg.h"
#include "psensor.h"

/* horizontal padding */
#define GRAPH_H_PADDING 4
/* vertical padding */
#define GRAPH_V_PADDING 4

static time_t get_graph_end_time_s()
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0)
		return tv.tv_sec;
	else
		return 0;
}

static time_t get_graph_begin_time_s(struct config *cfg)
{
	int ct;

	ct = get_graph_end_time_s();

	if (!ct)
		return 0;

	return ct - cfg->graph_monitoring_duration * 60;
}

static int compute_y(double value, double min, double max, int height, int off)
{
	double t = value - min;
	return height - ((double)height * (t / (max - min))) + off;
}

static char *time_to_str(time_t s)
{
	char *str;
	/* note: localtime returns a static field, no free required */
	struct tm *tm = localtime(&s);

	if (!tm)
		return NULL;

	str = malloc(6);
	strftime(str, 6, "%H:%M", tm);

	return str;
}

static void
draw_graph_background(cairo_t *cr,
		      int width, int height, struct config *config)
{
	struct color *bgcolor = config->graph_bgcolor;

	/* draw background */
	if (config->alpha_channel_enabled)
		cairo_set_source_rgba(cr,
				      bgcolor->f_red,
				      bgcolor->f_green,
				      bgcolor->f_blue, config->graph_bg_alpha);
	else
		cairo_set_source_rgb(cr,
				     bgcolor->f_red,
				     bgcolor->f_green, bgcolor->f_blue);

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
}

/* setup dash style */
static double dashes[] = {
	1.0,		/* ink */
	1.0,		/* skip */
	1.0,		/* ink */
	1.0		/* skip */
};
static int ndash = sizeof(dashes) / sizeof(dashes[0]);

static void draw_background_lines(cairo_t *cr,
				  struct color *color,
				  int g_width, int g_height,
				  int g_xoff, int g_yoff,
				  int min, int max)
{
	int i;

	/* draw background lines */
	cairo_set_dash(cr, dashes, ndash, 0);
	cairo_set_line_width(cr, 1);
	cairo_set_source_rgb(cr,
			     color->f_red, color->f_green, color->f_blue);

	/* vertical lines representing time steps */
	for (i = 0; i <= 5; i++) {
		int x = i * (g_width / 5) + g_xoff;
		cairo_move_to(cr, x, g_yoff);
		cairo_line_to(cr, x, g_yoff + g_height);
		cairo_stroke(cr);
	}

	/* horizontal lines draws a line for each 10C step */
	for (i = min; i < max; i++) {
		if (i % 10 == 0) {
			int y = compute_y(i, min, max, g_height, g_yoff);

			cairo_move_to(cr, g_xoff, y);
			cairo_line_to(cr, g_xoff + g_width, y);
			cairo_stroke(cr);
		}
	}

	/* back to normal line style */
	cairo_set_dash(cr, 0, 0, 0);

}

static void draw_sensor_curve(struct psensor *s,
			      cairo_t *cr,
			      double min,
			      double max,
			      int bt,
			      int et,
			      int g_width,
			      int g_height,
			      int g_xoff,
			      int g_yoff)
{
	int first = 1;
	int i;

	cairo_set_source_rgb(cr,
			     s->color->f_red,
			     s->color->f_green,
			     s->color->f_blue);
	cairo_set_line_width(cr, 1);

	for (i = 0; i < s->values_max_length; i++) {
		int x, y, t;
		double v;

		t = s->measures[i].time.tv_sec;
		v = s->measures[i].value;

		if (v == UNKNOWN_VALUE || !t || (t - bt) < 0)
			continue;

		x = (t - bt) * g_width / (et - bt) + g_xoff;

		y = compute_y(v, min, max, g_height, g_yoff);

		if (first) {
			cairo_move_to(cr, x, y);
			first = 0;
		} else {
			cairo_line_to(cr, x, y);
		}

	}
	cairo_stroke(cr);
}


void
graph_update(struct psensor **sensors,
	     GtkWidget *w_graph, struct config *config)
{
	struct color *fgcolor = config->graph_fgcolor;
	int et, bt;
	double min_rpm = get_min_rpm(sensors);
	double max_rpm = get_max_rpm(sensors);

	double mint = get_min_temp(sensors);
	char *strmin = psensor_value_to_string(SENSOR_TYPE_TEMP, mint);

	double maxt = get_max_temp(sensors);
	char *strmax = psensor_value_to_string(SENSOR_TYPE_TEMP, maxt);

	int width, height, g_width, g_height;

	/* horizontal and vertical offset of the graph */
	int g_xoff, g_yoff;

	cairo_surface_t *cst;
	cairo_t *cr, *cr_pixmap;

	char *str_btime = time_to_str(get_graph_begin_time_s(config));
	char *str_etime = time_to_str(get_graph_end_time_s());

	cairo_text_extents_t te_btime, te_etime, te_max, te_min;

	struct psensor **sensor_cur;
	GtkAllocation galloc;

	gtk_widget_get_allocation(w_graph, &galloc);
	width = galloc.width;
	height = galloc.height;

	cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cr = cairo_create(cst);

	draw_graph_background(cr, width, height, config);

	cairo_select_font_face(cr,
			       "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 10.0);

	cairo_text_extents(cr, str_etime, &te_etime);
	cairo_text_extents(cr, str_btime, &te_btime);
	cairo_text_extents(cr, strmax, &te_max);
	cairo_text_extents(cr, strmin, &te_min);

	g_yoff = GRAPH_V_PADDING;

	g_height = height - GRAPH_V_PADDING;
	if (te_etime.height > te_btime.height)
		g_height -= GRAPH_V_PADDING + te_etime.height + GRAPH_V_PADDING;
	else
		g_height -= GRAPH_V_PADDING + te_btime.height + GRAPH_V_PADDING;

	if (te_min.width > te_max.width)
		g_xoff = (2 * GRAPH_H_PADDING) + te_max.width;
	else
		g_xoff = (2 * GRAPH_H_PADDING) + te_min.width;

	g_width = width - g_xoff - GRAPH_H_PADDING;

	cairo_set_source_rgb(cr,
			     fgcolor->f_red, fgcolor->f_green, fgcolor->f_blue);

	/* draw graph begin time */
	cairo_move_to(cr, g_xoff, height - GRAPH_V_PADDING);
	cairo_show_text(cr, str_btime);
	free(str_btime);

	/* draw graph end time */
	cairo_move_to(cr,
		      width - te_etime.width - GRAPH_H_PADDING,
		      height - GRAPH_V_PADDING);
	cairo_show_text(cr, str_etime);
	free(str_etime);

	/* draw min and max temp */
	cairo_move_to(cr, GRAPH_H_PADDING, te_max.height + GRAPH_V_PADDING);
	cairo_show_text(cr, strmax);
	free(strmax);

	cairo_move_to(cr,
		      GRAPH_H_PADDING, height - (te_min.height / 2) - g_yoff);
	cairo_show_text(cr, strmin);
	free(strmin);

	draw_background_lines(cr, fgcolor,
			      g_width, g_height,
			      g_xoff, g_yoff,
			      mint, maxt);

	/* .. and finaly draws the temperature graphs */
	bt = get_graph_begin_time_s(config);
	et = get_graph_end_time_s();

	if (bt && et) {
		sensor_cur = sensors;
		while (*sensor_cur) {
			struct psensor *s = *sensor_cur;

			if (s->enabled) {
				double min, max;

				if (is_fan_type(s->type)) {
					min = min_rpm;
					max = max_rpm;
				} else {
					min = mint;
					max = maxt;
				}

				draw_sensor_curve(s, cr,
						  min, max,
						  bt, et,
						  g_width, g_height,
						  g_xoff, g_yoff);
			}

			sensor_cur++;
		}
	}

	cr_pixmap = gdk_cairo_create(gtk_widget_get_window(w_graph));

	if (cr_pixmap) {

		if (config->alpha_channel_enabled)
			cairo_set_operator(cr_pixmap, CAIRO_OPERATOR_SOURCE);

		cairo_set_source_surface(cr_pixmap, cst, 0, 0);
		cairo_paint(cr_pixmap);
	}

	cairo_destroy(cr_pixmap);
	cairo_surface_destroy(cst);
	cairo_destroy(cr);
}
