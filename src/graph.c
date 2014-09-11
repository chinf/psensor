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
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <cfg.h>
#include <plog.h>
#include <psensor.h>

/* horizontal padding */
#define GRAPH_H_PADDING 4
/* vertical padding */
#define GRAPH_V_PADDING 4

bool is_smooth_curves_enabled;

static time_t get_graph_end_time_s()
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0)
		return tv.tv_sec;

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

static double
compute_y(double value, double min, double max, int height, int off)
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
		      int g_xoff, int g_yoff,
		      int g_width, int g_height,
		      int width, int height, struct config *config,
		      GtkWidget *widget,
		      GtkWidget *window)
{
	GtkStyleContext *style_ctx;
	struct color *bgcolor;
	GdkRGBA rgba;

	bgcolor = config->graph_bgcolor;

	style_ctx = gtk_widget_get_style_context(window);
	gtk_style_context_get_background_color(style_ctx,
					       GTK_STATE_FLAG_NORMAL,
					       &rgba);

	if (config->alpha_channel_enabled)
		cairo_set_source_rgba(cr,
				      rgba.red,
				      rgba.green,
				      rgba.blue,
				      config->graph_bg_alpha);
	else
		cairo_set_source_rgb(cr,
				     rgba.red,
				     rgba.green,
				     rgba.blue);

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
	if (config->alpha_channel_enabled)
		cairo_set_source_rgba(cr,
				      bgcolor->red,
				      bgcolor->green,
				      bgcolor->blue,
				      config->graph_bg_alpha);
	else
		cairo_set_source_rgb(cr,
				     bgcolor->red,
				     bgcolor->green,
				     bgcolor->blue);

	cairo_rectangle(cr, g_xoff, g_yoff, g_width, g_height);
	cairo_fill(cr);
}

/* setup dash style */
static double dashes[] = {
	1.0,		/* ink */
	2.0,		/* skip */
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
	cairo_set_line_width(cr, 1);
	cairo_set_dash(cr, dashes, ndash, 0);
	cairo_set_source_rgb(cr,
			     color->red, color->green, color->blue);

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

/* Keys: sensor identifier.
 *
 * Values: array of time_t. Each time_t is corresponding to a sensor
 * measure which has been used as the start point of a Bezier curve.
 */
static GHashTable *times;

static void draw_sensor_smooth_curve(struct psensor *s,
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
	int i, dt, vdt, j, k, found;
	double x[4], y[4], v;
	time_t t, t0, *stimes;

	if (!times)
		times = g_hash_table_new_full(g_str_hash,
					      g_str_equal,
					      free,
					      free);

	stimes = g_hash_table_lookup(times, s->id);

	cairo_set_source_rgb(cr,
			     s->color->red,
			     s->color->green,
			     s->color->blue);

	/* search the index of the first measure used as a start point
	 * of a Bezier curve. The start and end points of the Bezier
	 * curves must be preserved to ensure the same overall shape
	 * of the graph. */
	i = 0;
	if (stimes) {
		while (i < s->values_max_length) {
			t = s->measures[i].time.tv_sec;
			v = s->measures[i].value;

			found = 0;
			if (v != UNKNOWN_DBL_VALUE && t) {
				k = 0;
				while (stimes[k]) {
					if (t == stimes[k]) {
						found = 1;
						break;
					}
					k++;
				}
			}

			if (found)
				break;

			i++;
		}
	}

	stimes = malloc((s->values_max_length + 1) * sizeof(time_t));
	memset(stimes, 0, (s->values_max_length + 1) * sizeof(time_t));
	g_hash_table_insert(times, strdup(s->id), stimes);

	if (i == s->values_max_length)
		i = 0;

	k = 0;
	dt = et - bt;
	while (i < s->values_max_length) {
		j = 0;
		t = 0;
		while (i < s->values_max_length && j < 4) {
			t = s->measures[i].time.tv_sec;
			v = s->measures[i].value;

			if (v == UNKNOWN_DBL_VALUE || !t) {
				i++;
				continue;
			}

			vdt = t - bt;

			x[0 + j] = ((double)vdt * g_width) / dt + g_xoff;
			y[0 + j] = compute_y(v, min, max, g_height, g_yoff);

			if (j == 0)
				t0 = t;

			i++;
			j++;
		}

		if (j == 4) {
			cairo_move_to(cr, x[0], y[0]);
			cairo_curve_to(cr, x[1], y[1], x[2], y[3], x[3], y[3]);
			stimes[k++] = t0;
			i--;
		}
	}

	cairo_stroke(cr);
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
	int first, i, t, dt, vdt;
	double v, x, y;

	cairo_set_source_rgb(cr,
			     s->color->red,
			     s->color->green,
			     s->color->blue);

	dt = et - bt;
	first = 1;
	for (i = 0; i < s->values_max_length; i++) {
		t = s->measures[i].time.tv_sec;
		v = s->measures[i].value;

		if (v == UNKNOWN_DBL_VALUE || !t)
			continue;

		vdt = t - bt;
		if (vdt < 0)
			continue;

		x = ((double)vdt * g_width) / dt + g_xoff;

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

static void display_no_graphs_warning(cairo_t *cr, int x, int y)
{
	char *msg;

	msg = strdup(_("No graphs enabled"));

	cairo_select_font_face(cr,
			       "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 18.0);

	cairo_move_to(cr, x, y);
	cairo_show_text(cr, msg);

	free(msg);
}

void
graph_update(struct psensor **sensors,
	     GtkWidget *w_graph,
	     struct config *config,
	     GtkWidget *window)
{
	struct color *fgcolor = config->graph_fgcolor;
	int et, bt, width, height, g_width, g_height;
	double min_rpm, max_rpm, mint, maxt, min, max;
	char *strmin, *strmax;
	/* horizontal and vertical offset of the graph */
	int g_xoff, g_yoff, no_graphs;
	cairo_surface_t *cst;
	cairo_t *cr, *cr_pixmap;
	char *str_btime, *str_etime;
	cairo_text_extents_t te_btime, te_etime, te_max, te_min;
	struct psensor **sensor_cur, **enabled_sensors;
	GtkAllocation galloc;
	GtkStyleContext *style_ctx;
	GdkRGBA rgba;

	if (!gtk_widget_is_drawable(w_graph))
		return;

	enabled_sensors = psensor_list_filter_graph_enabled(sensors);

	min_rpm = get_min_rpm(enabled_sensors);
	max_rpm = get_max_rpm(enabled_sensors);

	mint = get_min_temp(enabled_sensors);
	strmin = psensor_value_to_str(SENSOR_TYPE_TEMP,
				      mint,
				      config->temperature_unit == CELSIUS);

	maxt = get_max_temp(enabled_sensors);
	strmax = psensor_value_to_str(SENSOR_TYPE_TEMP,
				      maxt,
				      config->temperature_unit == CELSIUS);

	str_btime = time_to_str(get_graph_begin_time_s(config));
	str_etime = time_to_str(get_graph_end_time_s());

	gtk_widget_get_allocation(w_graph, &galloc);
	width = galloc.width;
	height = galloc.height;

	cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cr = cairo_create(cst);

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

	draw_graph_background(cr,
			      g_xoff, g_yoff, g_width, g_height,
			      width, height, config,
			      w_graph,
			      window);

	/** Set the color for text drawing */
	style_ctx = gtk_widget_get_style_context(window);
	gtk_style_context_get_color(style_ctx, GTK_STATE_FLAG_NORMAL, &rgba);
	cairo_set_source_rgb(cr, rgba.red, rgba.green, rgba.blue);

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
		sensor_cur = enabled_sensors;

		cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
		cairo_set_line_width(cr, 1);
		no_graphs = 1;
		while (*sensor_cur) {
			struct psensor *s = *sensor_cur;

			no_graphs = 0;
			if (s->type & SENSOR_TYPE_RPM) {
				min = min_rpm;
				max = max_rpm;
			} else if (s->type & SENSOR_TYPE_PERCENT) {
				min = 0;
				max = get_max_value(enabled_sensors,
						    SENSOR_TYPE_PERCENT);
			} else {
				min = mint;
				max = maxt;
			}

			if (is_smooth_curves_enabled)
				draw_sensor_smooth_curve(s, cr,
							 min, max,
							 bt, et,
							 g_width, g_height,
							 g_xoff, g_yoff);
			else
				draw_sensor_curve(s, cr,
						  min, max,
						  bt, et,
						  g_width, g_height,
						  g_xoff, g_yoff);

			sensor_cur++;
		}

		if (no_graphs)
			display_no_graphs_warning(cr,
						  g_xoff + 12,
						  g_height / 2);
	}

	cr_pixmap = gdk_cairo_create(gtk_widget_get_window(w_graph));

	if (cr_pixmap) {
		if (config->alpha_channel_enabled)
			cairo_set_operator(cr_pixmap, CAIRO_OPERATOR_SOURCE);

		cairo_set_source_surface(cr_pixmap, cst, 0, 0);
		cairo_paint(cr_pixmap);
	}

	free(enabled_sensors);

	cairo_destroy(cr_pixmap);
	cairo_surface_destroy(cst);
	cairo_destroy(cr);
}
