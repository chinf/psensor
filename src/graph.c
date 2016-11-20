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
 */
static time_t get_graph_end_time_s(struct psensor **sensors)
{
	time_t end_time, t;
	struct psensor *s;
	struct measure *measures;
	int i;

	end_time = 0;
	while (*sensors) {
		s = *sensors;
		measures = s->measures;

		for (i = (s->values_max_length - 1); i >= 0; i--) {
			if (measures[i].value != UNKNOWN_DBL_VALUE) {
				t = measures[i].time.tv_sec;

				if (t > end_time) {
					end_time = t;
					break;
				}
			}
			i--;
		}

		sensors++;
	}

	return end_time;
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

static void
draw_graph_background(cairo_t *cr,
		      struct config *config,
		      struct graph_info *info)
{
	struct color *bgcolor;

	/* draw graph pane background */
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

	cairo_rectangle(cr, 0, 0, info->width, info->height);
	cairo_fill(cr);

	/* draw plot area background */
	bgcolor = config->graph_bgcolor;

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

static void draw_sensor_curve(struct psensor *s,
			      cairo_t *plotarea,
			      double min,
			      double max,
			      int bt,
			      int et,
			      struct graph_info *info,
				  int filter)
{
	int stage, i, samp, t;
	double v, x, y, p_x, p_y, pp_x, pp_y;
	double dx, dy, m, p_m, t_m, curve_factor;
	double x_cp1, y_cp1, x_cp2, y_cp2;
	GdkRGBA *color;
	double rx[s->values_max_length], ry[s->values_max_length], f[3];

	color = config_get_sensor_color(s->id);
	cairo_set_source_rgb(plotarea,
			     color->red,
			     color->green,
			     color->blue);
	gdk_rgba_free(color);

	samp = 0;
	for (i = 0; i < s->values_max_length; i++) {
		t = s->measures[i].time.tv_sec;
		v = s->measures[i].value;
		if (v == UNKNOWN_DBL_VALUE || !t)
			continue;

		rx[samp] = ((double)(t - bt) * info->g_width) / (et - bt);
		ry[samp] = compute_y(v, min, max, info->g_height, info->g_yoff);
		samp++;
	}

	stage = 0;
	m = 0;
	p_m = 0;
	p_x = 0;
	p_y = 0;
	pp_x = 0;
	pp_y = 0;
	dx = 0;
	dy = 0;
	curve_factor = 0.4; /* sensible range: 0.1 to 0.7 */
	f[0] = 0.25;
	f[1] = 0.5;
	f[2] = 0.25;

	for (i = 0; i < samp; i++) {
		x = rx[i];
		if (filter) {
			if (i == 0) { /* first sample */
				y = (ry[i] * (f[0] + f[1])) + (ry[i+1] * f[2]);
			} else if (i >= (samp - 1)) { /* last sample */
				y = (ry[i-1] * f[0]) + (ry[i] * (f[1] + f[2]));
			} else {
				y = (ry[i-1] * f[0]) +
					(ry[i] * f[1]) +
					(ry[i+1] * f[2]);
			}
		} else {
			y = ry[i];
		}

		switch (stage) {
		case 0:
			log_debug("Curve start: #%d, x=%f, y=%f", i, x, y);
			cairo_move_to(plotarea, x, y);
			pp_x = x;
			pp_y = y;
			stage++;
			break;
		case 1:
			if (x > pp_x)
				p_m = (y - pp_y) / (x - pp_x);

			dx = (x - pp_x) * curve_factor;
			p_x = x;
			p_y = y;
			stage++;
			break;
		default:
			if (x > p_x)
				m = (y - p_y) / (x - p_x);
			else
				m = 0;

			/* calculate control point 1 */
			x_cp1 = pp_x + dx;
			y_cp1 = pp_y + dy;

			/* calculate control point 2 */
			log_debug("#%d: pp_y, p_y, y: %f, %f, %f",
					  i, pp_y, p_y, y);
			if (((p_y > pp_y) && (y > p_y)) ||
				((p_y < pp_y) && (y < p_y))) {
				/* local strong monotonicity, so use
				 * shallower slope for tangent
				 */
				if (fabs(m) < fabs(p_m)) {
					t_m = m;
					log_debug("#%d: using next slope %f, previous was %f",
							  i, m, p_m);
				} else {
					t_m = p_m;
					log_debug("#%d: using prev slope %f, next is %f",
							  i, p_m, m);
				}
			} else {
				t_m = 0;
			}
			dy = t_m * dx;
			x_cp2 = p_x - dx;
			y_cp2 = p_y - dy;

			log_debug("#%d: cp1=%f, %f, cp2=%f, %f, p_x=%f, p_y=%f",
					  i,
					  x_cp1, y_cp1,
					  x_cp2, y_cp2,
					  p_x, p_y);
			cairo_curve_to(plotarea,
						   x_cp1, y_cp1,
						   x_cp2, y_cp2,
						   p_x, p_y);

			dx = (x - p_x) * curve_factor;
			dy = t_m * dx;
			pp_x = p_x;
			pp_y = p_y;
			p_x = x;
			p_y = y;
			p_m = m;
			break;
		}
		if ((i >= (samp - 1)) && (stage > 1)) {
			x_cp1 = pp_x + dx;
			y_cp1 = pp_y + dy;

			x_cp2 = p_x - dx;
			y_cp2 = p_y;

			log_debug("Last segment: cp1=%f, %f, cp2=%f, %f, p_x=%f, p_y=%f",
					  x_cp1, y_cp1, x_cp2, y_cp2, p_x, p_y);
			cairo_curve_to(plotarea,
						   x_cp1, y_cp1,
						   x_cp2, y_cp2,
						   p_x, p_y);
		}
	}

	cairo_stroke(plotarea);
}

static void
display_no_graphs_warning(cairo_t *cr,
		      struct graph_info *info)
{
	char *msg;
	int x, y;
	cairo_text_extents_t te_msg;

	msg = strdup(_("No graphs to show"));

	cairo_select_font_face(cr,
			       "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 18.0);

	cairo_text_extents(cr, msg, &te_msg);
	x = (info->width / 2) - (te_msg.width / 2);
	y = info->height / 2;

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
	int et, bt, width, height, g_width, g_height, g_xoff, g_yoff;
	double min_rpm, max_rpm, mint, maxt, min, max;
	char *strmin, *strmax;
	int graphs_drawn, use_celsius;
	cairo_surface_t *cst, *plotsurface;
	cairo_t *cr, *cr_pixmap, *plotarea;
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

	plotsurface = cairo_surface_create_for_rectangle(cst,
						(double)g_xoff,
						(double)0,
						(double)g_width,
						(double)height);
	plotarea = cairo_create(plotsurface);

	draw_graph_background(cr, config, &info);
	draw_background_lines(cr, mint, maxt, config, &info);

	/* draw the enabled graphs */
	graphs_drawn = 0;
	if (bt && et) {
		sensor_cur = enabled_sensors;

		cairo_set_line_join(plotarea, CAIRO_LINE_JOIN_ROUND);
		while (*sensor_cur) {
			struct psensor *s = *sensor_cur;

			log_debug("sensor graph enabled: %s", s->name);
			graphs_drawn = 1;
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

			cairo_set_line_width(plotarea, 1);
			draw_sensor_curve(s, plotarea,
							 min, max,
							 bt, et,
							 &info,
						     is_smooth_curves_enabled);

			sensor_cur++;
		}
	}
	free(enabled_sensors);
	cairo_destroy(plotarea);
	cairo_surface_destroy(plotsurface);

	/* Set the color for text drawing */
	cairo_set_source_rgb(cr,
			     theme_fg_color.red,
			     theme_fg_color.green,
			     theme_fg_color.blue);

	if (graphs_drawn) {
		/* draw x-axis: begin and end times */
		cairo_move_to(cr, g_xoff, height - GRAPH_V_PADDING);
		cairo_show_text(cr, str_btime);

		cairo_move_to(cr,
				width - te_etime.width - GRAPH_H_PADDING,
				height - GRAPH_V_PADDING);
		cairo_show_text(cr, str_etime);

		/* draw primary y-axis: min and max temp */
		cairo_move_to(cr, GRAPH_H_PADDING,
				te_max.height + GRAPH_V_PADDING);
		cairo_show_text(cr, strmax);

		cairo_move_to(cr, GRAPH_H_PADDING,
				height - (te_min.height / 2) - g_yoff);
		cairo_show_text(cr, strmin);
	} else {
		display_no_graphs_warning(cr, &info);
	}
	free(str_btime);
	free(str_etime);
	free(strmax);
	free(strmin);

	cr_pixmap = gdk_cairo_create(gtk_widget_get_window(w_graph));

	if (cr_pixmap) {
		if (config->alpha_channel_enabled)
			cairo_set_operator(cr_pixmap, CAIRO_OPERATOR_SOURCE);

		cairo_set_source_surface(cr_pixmap, cst, 0, 0);
		cairo_paint(cr_pixmap);
	}

	cairo_destroy(cr_pixmap);
	cairo_destroy(cr);
	cairo_surface_destroy(cst);
}

int compute_values_max_length(struct config *c)
{
	int n, duration, interval;

	duration = c->graph_monitoring_duration * 60;
	interval = c->sensor_update_interval;

	n = 3 + ceil((((double)duration) / interval) + 0.5) + 3;

	return n;
}
