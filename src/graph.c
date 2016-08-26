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
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <math.h>

#include <cfg.h>
#include <graph.h>
#include <parray.h>
#include <plog.h>
#include <psensor.h>

/* horizontal padding */
static const int GRAPH_H_PADDING = 4;
/* vertical padding */
static const int GRAPH_V_PADDING = 4;

bool is_smooth_curves_enabled;

struct graph_info {
	/* Horizontal position of the central area (curves) */
	int g_xoff;
	/* Vertical position of the central area (curves) */
	int g_yoff;

	/* Width of the central area (curves) */
	int g_width;
	/* Height of the central area (curves) */
	int g_height;

	/* Height of the drawing canvas */
	int height;
	/* Width of the drawing canvas */
	int width;
};

static GtkStyleContext *style;
/* Foreground color of the current desktop theme */
static GdkRGBA theme_fg_color;
/* Background color of the current desktop theme */
static GdkRGBA theme_bg_color;

static void update_theme(GtkWidget *w)
{
	style = gtk_widget_get_style_context(w);

	gtk_style_context_get_background_color(style,
					       GTK_STATE_FLAG_NORMAL,
					       &theme_bg_color);
	gtk_style_context_get_color(style,
				    GTK_STATE_FLAG_NORMAL,
				    &theme_fg_color);
}

static struct psensor **list_filter_graph_enabled(struct psensor **sensors)
{
	int n, i;
	struct psensor **result, **cur, *s;

	if (!sensors)
		return NULL;

	n = psensor_list_size(sensors);
	result = malloc((n+1) * sizeof(struct psensor *));

	for (cur = sensors, i = 0; *cur; cur++) {
		s = *cur;

		if (config_is_sensor_graph_enabled(s->id))
			result[i++] = s;
	}

	result[i] = NULL;

	return result;
}

/* Return the end time of the graph i.e. the more recent measure.  If
 * no measure are available, return 0.
 * If Bezier curves are used return the measure n-3 to avoid to
 * display a part of the curve outside the graph area.
 */
static time_t get_graph_end_time_s(struct psensor **sensors)
{
	time_t ret, t;
	struct psensor *s;
	struct measure *measures;
	int i, n;

	ret = 0;
	while (*sensors) {
		s = *sensors;
		measures = s->measures;

		if (is_smooth_curves_enabled)
			n = 2;
		else
			n = 0;

		for (i = s->values_max_length - 1; i >= 0; i--) {
			if (measures[i].value != UNKNOWN_DBL_VALUE) {
				if (!n) {
					t = measures[i].time.tv_sec;

					if (t > ret) {
						ret = t;
						break;
					}
				} else {
					n--;
				}
			}
			i--;
		}

		sensors++;
	}

	return ret;
}

static time_t get_graph_begin_time_s(struct config *cfg, time_t etime)
{
	if (!etime)
		return 0;

	return etime - cfg->graph_monitoring_duration * 60;
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

static void draw_left_region(cairo_t *cr, struct graph_info *info)
{
	cairo_set_source_rgb(cr,
			     theme_bg_color.red,
			     theme_bg_color.green,
			     theme_bg_color.blue);

	cairo_rectangle(cr, 0, 0, info->g_xoff, info->height);
	cairo_fill(cr);
}

static void draw_right_region(cairo_t *cr, struct graph_info *info)
{
	cairo_set_source_rgb(cr,
			     theme_bg_color.red,
			     theme_bg_color.green,
			     theme_bg_color.blue);


	cairo_rectangle(cr,
			info->g_xoff + info->g_width,
			0,
			info->g_xoff + info->g_width + GRAPH_H_PADDING,
			info->height);
	cairo_fill(cr);
}

static void
draw_graph_background(cairo_t *cr,
		      struct config *config,
		      struct graph_info *info)
{
	struct color *bgcolor;

	bgcolor = config->graph_bgcolor;

	if (config->alpha_channel_enabled)
		cairo_set_source_rgba(cr,
				      theme_bg_color.red,
				      theme_bg_color.green,
				      theme_bg_color.blue,
				      config->graph_bg_alpha);
	else
		cairo_set_source_rgb(cr,
				     theme_bg_color.red,
				     theme_bg_color.green,
				     theme_bg_color.blue);

	cairo_rectangle(cr, info->g_xoff, 0, info->g_width, info->height);
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

	cairo_rectangle(cr,
			info->g_xoff,
			info->g_yoff,
			info->g_width,
			info->g_height);
	cairo_fill(cr);
}

/* setup dash style */
static double dashes[] = {
	1.0,		/* ink */
	2.0,		/* skip */
};
static int ndash = ARRAY_SIZE(dashes);

static void draw_background_lines(cairo_t *cr,
				  int min, int max,
				  struct config *config,
				  struct graph_info *info)
{
	int i;
	double x, y;
	struct color *color;

	color = config->graph_fgcolor;

	/* draw background lines */
	cairo_set_line_width(cr, 1);
	cairo_set_dash(cr, dashes, ndash, 0);
	cairo_set_source_rgb(cr, color->red, color->green, color->blue);

	/* vertical lines representing time steps */
	for (i = 0; i <= 5; i++) {
		x = i * ((double)info->g_width / 5) + info->g_xoff;
		cairo_move_to(cr, x, info->g_yoff);
		cairo_line_to(cr, x, info->g_yoff + info->g_height);
	}

	/* horizontal lines draws a line for each 10C step */
	for (i = min; i < max; i++) {
		if (i % 10 == 0) {
			y = compute_y(i,
				      min,
				      max,
				      info->g_height,
				      info->g_yoff);

			cairo_move_to(cr, info->g_xoff, y);
			cairo_line_to(cr, info->g_xoff + info->g_width, y);
		}
	}

	cairo_stroke(cr);

	/* back to normal line style */
	cairo_set_dash(cr, NULL, 0, 0);
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
				     struct graph_info *info)
{
	int i, dt, vdt, j, k, found;
	double x[4], y[4], v;
	time_t t, t0, *stimes;
	GdkRGBA *color;

	if (!times)
		times = g_hash_table_new_full(g_str_hash,
					      g_str_equal,
					      free,
					      free);

	stimes = g_hash_table_lookup(times, s->id);

	color = config_get_sensor_color(s->id);

	cairo_set_source_rgb(cr,
			     color->red,
			     color->green,
			     color->blue);
	gdk_rgba_free(color);

	/* search the index of the first measure used as a start point
	 * of a Bezier curve. The start and end points of the Bezier
	 * curves must be preserved to ensure the same overall shape
	 * of the graph.
	 */
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

			x[0 + j] = ((double)vdt * info->g_width)
				/ dt + info->g_xoff;
			y[0 + j] = compute_y(v,
					     min,
					     max,
					     info->g_height,
					     info->g_yoff);

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
			      struct graph_info *info)
{
	int first, i, t, dt, vdt;
	double v, x, y;
	GdkRGBA *color;

	color = config_get_sensor_color(s->id);
	cairo_set_source_rgb(cr,
			     color->red,
			     color->green,
			     color->blue);
	gdk_rgba_free(color);

	dt = et - bt;
	first = 1;
	for (i = 0; i < s->values_max_length; i++) {
		t = s->measures[i].time.tv_sec;
		v = s->measures[i].value;

		if (v == UNKNOWN_DBL_VALUE || !t)
			continue;

		vdt = t - bt;

		x = ((double)vdt * info->g_width) / dt + info->g_xoff;

		y = compute_y(v, min, max, info->g_height, info->g_yoff);

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
	int et, bt, width, height, g_width, g_height;
	double min_rpm, max_rpm, mint, maxt, min, max;
	char *strmin, *strmax;
	/* horizontal and vertical offset of the graph */
	int g_xoff, g_yoff, no_graphs, use_celsius;
	cairo_surface_t *cst;
	cairo_t *cr, *cr_pixmap;
	char *str_btime, *str_etime;
	cairo_text_extents_t te_btime, te_etime, te_max, te_min;
	struct psensor **sensor_cur, **enabled_sensors;
	GtkAllocation galloc;
	struct graph_info info;

	if (!gtk_widget_is_drawable(w_graph))
		return;

	if (!style)
		update_theme(window);

	enabled_sensors = list_filter_graph_enabled(sensors);

	min_rpm = get_min_rpm(enabled_sensors);
	max_rpm = get_max_rpm(enabled_sensors);

	if (config_get_temperature_unit() == CELSIUS)
		use_celsius = 1;
	else
		use_celsius = 0;

	mint = get_min_temp(enabled_sensors);
	strmin = psensor_value_to_str(SENSOR_TYPE_TEMP, mint, use_celsius);

	maxt = get_max_temp(enabled_sensors);
	strmax = psensor_value_to_str(SENSOR_TYPE_TEMP, maxt, use_celsius);

	et = get_graph_end_time_s(enabled_sensors);
	bt = get_graph_begin_time_s(config, et);

	str_btime = time_to_str(bt);
	str_etime = time_to_str(et);

	gtk_widget_get_allocation(w_graph, &galloc);
	width = galloc.width;
	info.width = galloc.width;
	height = galloc.height;
	info.height = height;


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
	info.g_yoff = g_yoff;

	g_height = height - GRAPH_V_PADDING;
	if (te_etime.height > te_btime.height)
		g_height -= GRAPH_V_PADDING + te_etime.height + GRAPH_V_PADDING;
	else
		g_height -= GRAPH_V_PADDING + te_btime.height + GRAPH_V_PADDING;

	info.g_height = g_height;

	if (te_min.width > te_max.width)
		g_xoff = (2 * GRAPH_H_PADDING) + te_max.width;
	else
		g_xoff = (2 * GRAPH_H_PADDING) + te_min.width;

	info.g_xoff = g_xoff;

	g_width = width - g_xoff - GRAPH_H_PADDING;
	info.g_width = g_width;

	draw_graph_background(cr, config, &info);

	/* Set the color for text drawing */
	cairo_set_source_rgb(cr,
			     theme_fg_color.red,
			     theme_fg_color.green,
			     theme_fg_color.blue);

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

	draw_background_lines(cr, mint, maxt, config, &info);

	/* .. and finaly draws the temperature graphs */
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
							 &info);
			else
				draw_sensor_curve(s, cr,
						  min, max,
						  bt, et,
						  &info);

			sensor_cur++;
		}

		if (no_graphs)
			display_no_graphs_warning(cr,
						  g_xoff + 12,
						  g_height / 2);
	}

	draw_left_region(cr, &info);
	draw_right_region(cr, &info);

	/* draw min and max temp */
	cairo_set_source_rgb(cr,
			     theme_fg_color.red,
			     theme_fg_color.green,
			     theme_fg_color.blue);

	cairo_move_to(cr, GRAPH_H_PADDING, te_max.height + GRAPH_V_PADDING);
	cairo_show_text(cr, strmax);
	free(strmax);

	cairo_move_to(cr,
		      GRAPH_H_PADDING, height - (te_min.height / 2) - g_yoff);
	cairo_show_text(cr, strmin);
	free(strmin);

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

int compute_values_max_length(struct config *c)
{
	int n, duration, interval;

	duration = c->graph_monitoring_duration * 60;
	interval = c->sensor_update_interval;

	n = 3 + ceil((((double)duration) / interval) + 0.5) + 3;

	return n;
}
