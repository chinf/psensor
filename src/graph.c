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
	/* Horizontal position of the plot area (curves) */
	int g_xoff;
	/* Vertical position of the plot area (curves) */
	int g_yoff;

	/* Width of the plot area (curves) */
	int g_width;
	/* Height of the plot area (curves) */
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
	int stage, i, samp;
	double x, y, p_x, p_y, pp_x, pp_y, c1_x, c1_y, c2_x, c2_y, dx, dy;
	double m, p_m, t_m, curve_factor, a1, b0, b1, c0;
	double rx[s->values_max_length], ry[s->values_max_length];
	GdkRGBA *color;

	color = config_get_sensor_color(s->id);
	cairo_set_source_rgb(plotarea,
			     color->red,
			     color->green,
			     color->blue);
	gdk_rgba_free(color);

	samp = 0;
	for (i = 0; i < s->values_max_length; i++) {
		if (s->measures[i].value == UNKNOWN_DBL_VALUE ||
			!s->measures[i].time.tv_sec)
			continue;

		rx[samp] = ((double)(s->measures[i].time.tv_sec - bt) *
					info->g_width) / (et - bt);
		ry[samp] = compute_y(s->measures[i].value, min, max,
					info->g_height, info->g_yoff);
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
	/* filter coefficients must sum to 1 */
	if (s->type & SENSOR_TYPE_PERCENT) {
		a1 = 0.2;	/* t - 1 feedback */
		b1 = 0.15;	/* t - 1 */
		b0 = 0.6;	/* t     */
		c0 = 0.05;	/* t + 1 */
	} else {
		a1 = 0.6;	/* t - 1 feedback */
		b1 = 0.15;	/* t - 1 */
		b0 = 0.15;	/* t     */
		c0 = 0.1;	/* t + 1 */
	}

	for (i = 0; i < samp; i++) {
		x = rx[i];
		if (filter) { /* modified first order IIR filter */
			if (i == 0) { /* first sample */
				y = ((b0 + a1) * ry[i]) +
					((c0 + b1) * ry[i + 1]);
			} else if (i >= (samp - 1)) { /* last sample */
				y = (a1 * p_y) +
					(b1 * ry[i - 1]) +
					((b0 + c0) * ry[i]);
			} else {
				y = (a1 * p_y) +
					(b1 * ry[i - 1]) +
					(b0 * ry[i]) +
					(c0 * ry[i + 1]);
			}
		} else {
			y = ry[i];
		}

		switch (stage) {
		case 0:
			pp_x = x;
			pp_y = y;
			p_x = x;
			p_y = y;
			stage++;
			break;
		case 1:
			log_debug("[%d] curve start: %f,%f", i, p_x, p_y);
			cairo_move_to(plotarea, p_x, p_y);

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
			c1_x = pp_x + dx;
			c1_y = pp_y + dy;

			/* calculate control point 2 */
			log_debug("[%d] pp_y, p_y, y: %f, %f, %f",
					  i, pp_y, p_y, y);
			if (((p_y > pp_y) && (y > p_y)) ||
				((p_y < pp_y) && (y < p_y))) {
				/* local strong monotonicity, so use
				 * shallower slope for tangent
				 */
				if (fabs(m) < fabs(p_m)) {
					t_m = m;
					log_debug("[%d] using next slope %f, previous was %f",
							  i, m, p_m);
				} else {
					t_m = p_m;
					log_debug("[%d] using prev slope %f, next is %f",
							  i, p_m, m);
				}
			} else {
				t_m = 0;
			}
			dy = t_m * dx;
			c2_x = p_x - dx;
			c2_y = p_y - dy;

			log_debug("[%d] cp1=%f,%f cp2=%f,%f end=%f,%f",
					  i,
					  c1_x, c1_y,
					  c2_x, c2_y,
					  p_x, p_y);
			cairo_curve_to(plotarea,
						   c1_x, c1_y,
						   c2_x, c2_y,
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
			c1_x = pp_x + dx;
			c1_y = pp_y + dy;

			c2_x = p_x - dx;
			c2_y = p_y;

			log_debug("[finish] cp1=%f,%f, cp2=%f,%f, end=%f,%f",
					  c1_x, c1_y, c2_x, c2_y, p_x, p_y);
			cairo_curve_to(plotarea,
						   c1_x, c1_y,
						   c2_x, c2_y,
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
	int use_celsius, et, bt, width, height;
	int yaxis_width, yaxis_tx, yaxis_sx, yaxis_ux, yaxis_y1, yaxis_y0;
	int t_graphs, s_graphs, u_graphs;
	double min_t, max_t, min_s, max_s, min_u, max_u, min, max;
	char *min_t_str, *max_t_str, *min_s_str, *max_s_str;
	char *min_u_str, *max_u_str, *bt_str, *et_str;
	cairo_surface_t *cst, *plotsurface;
	cairo_t *cr, *cr_pixmap, *plotarea;
	cairo_font_face_t *labelfont;
	cairo_text_extents_t et_te, bt_te, min_t_te, max_t_te;
	cairo_text_extents_t min_s_te, max_s_te, min_u_te, max_u_te;
	struct psensor **sensor_cur, **enabled_sensors;
	struct graph_info info;
	GtkAllocation galloc;

	if (!gtk_widget_is_drawable(w_graph))
		return;

	if (!style)
		update_theme(window);

	if (config_get_temperature_unit() == CELSIUS)
		use_celsius = 1;
	else
		use_celsius = 0;

	enabled_sensors = list_filter_graph_enabled(sensors);

	et = get_graph_end_time_s(enabled_sensors);
	bt = get_graph_begin_time_s(config, et);
	et_str = time_to_str(et);
	bt_str = time_to_str(bt);

	t_graphs = 0;
	s_graphs = 0;
	u_graphs = 0;
	if (bt && et) {
		sensor_cur = enabled_sensors;
		while (*sensor_cur) {
			struct psensor *s = *sensor_cur;

			if (s->type & SENSOR_TYPE_RPM)
				s_graphs++;
			else if (s->type & SENSOR_TYPE_PERCENT)
				u_graphs++;
			else
				t_graphs++;
			sensor_cur++;
		}
	}

	min_t = get_min_value(enabled_sensors, SENSOR_TYPE_TEMP);
	max_t = get_max_value(enabled_sensors, SENSOR_TYPE_TEMP);
	min_t_str = psensor_value_to_str(SENSOR_TYPE_TEMP, min_t, use_celsius);
	max_t_str = psensor_value_to_str(SENSOR_TYPE_TEMP, max_t, use_celsius);

	min_s = get_min_value(enabled_sensors, SENSOR_TYPE_RPM);
	max_s = get_max_value(enabled_sensors, SENSOR_TYPE_RPM);
	min_s_str = psensor_value_to_str(SENSOR_TYPE_RPM, min_s, 0);
	max_s_str = psensor_value_to_str(SENSOR_TYPE_RPM, max_s, 0);

	min_u = 0;
	max_u = 100;
	min_u_str = psensor_value_to_str(SENSOR_TYPE_PERCENT, min_u, 0);
	max_u_str = psensor_value_to_str(SENSOR_TYPE_PERCENT, max_u, 0);

	gtk_widget_get_allocation(w_graph, &galloc);
	width = galloc.width;
	info.width = width;
	height = galloc.height;
	info.height = height;

	cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cr = cairo_create(cst);

	labelfont = cairo_toy_font_face_create("sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_face(cr, labelfont);
	cairo_set_font_size(cr, 10.0);

	cairo_text_extents(cr, et_str, &et_te);
	cairo_text_extents(cr, bt_str, &bt_te);
	cairo_text_extents(cr, min_t_str, &min_t_te);
	cairo_text_extents(cr, max_t_str, &max_t_te);
	cairo_text_extents(cr, min_s_str, &min_s_te);
	cairo_text_extents(cr, max_s_str, &max_s_te);
	cairo_text_extents(cr, min_u_str, &min_u_te);
	cairo_text_extents(cr, max_u_str, &max_u_te);

	/* calculate vertical metrics */
	info.g_yoff = GRAPH_V_PADDING;
	if (et_te.height > bt_te.height)
		info.g_height = height - et_te.height - (3 * GRAPH_V_PADDING);
	else
		info.g_height = height - et_te.height - (3 * GRAPH_V_PADDING);
	yaxis_y1 = max_t_te.height + GRAPH_V_PADDING;
	yaxis_y0 = height - min_t_te.height - GRAPH_V_PADDING;

	/* calculate horizontal metrics */
	info.g_xoff = GRAPH_H_PADDING;
	yaxis_width = GRAPH_H_PADDING;
	if (u_graphs) {
		if (max_u_te.width > min_u_te.width)
			yaxis_width += GRAPH_H_PADDING + max_u_te.width;
		else
			yaxis_width += GRAPH_H_PADDING + min_u_te.width;
	}
	yaxis_ux = width - yaxis_width + GRAPH_H_PADDING;
	if (s_graphs) {
		if (max_s_te.width > min_s_te.width)
			yaxis_width += GRAPH_H_PADDING + max_s_te.width;
		else
			yaxis_width += GRAPH_H_PADDING + min_s_te.width;
	}
	yaxis_sx = width - yaxis_width + GRAPH_H_PADDING;
	if (t_graphs) {
		if (max_t_te.width > min_t_te.width)
			yaxis_width += GRAPH_H_PADDING + max_t_te.width;
		else
			yaxis_width += GRAPH_H_PADDING + min_t_te.width;
	}
	yaxis_tx = width - yaxis_width + GRAPH_H_PADDING;
	info.g_width = width - info.g_xoff - yaxis_width;

	draw_graph_background(cr, config, &info);
	draw_background_lines(cr, min_t, max_t, config, &info);

	/* draw the enabled graphs */
	plotsurface = cairo_surface_create_for_rectangle(cst,
						(double)info.g_xoff,
						(double)0,
						(double)info.g_width,
						(double)height);
	plotarea = cairo_create(plotsurface);

	if (bt && et) {
		sensor_cur = enabled_sensors;
		cairo_set_line_join(plotarea, CAIRO_LINE_JOIN_ROUND);

		while (*sensor_cur) {
			struct psensor *s = *sensor_cur;

			log_debug("sensor graph enabled: %s", s->name);
			if (s->type & SENSOR_TYPE_RPM) {
				min = min_s;
				max = max_s;
			} else if (s->type & SENSOR_TYPE_PERCENT) {
				min = min_u;
				max = max_u;
			} else {
				min = min_t;
				max = max_t;
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

	if (t_graphs) {
		/* draw y-axis: max and min temp */
		cairo_move_to(cr, yaxis_tx, yaxis_y1);
		cairo_show_text(cr, max_t_str);

		cairo_move_to(cr, yaxis_tx, yaxis_y0);
		cairo_show_text(cr, min_t_str);
	}
	if (s_graphs) {
		/* draw y-axis: max and min speeds */
		cairo_move_to(cr, yaxis_sx, yaxis_y1);
		cairo_show_text(cr, max_s_str);

		cairo_move_to(cr, yaxis_sx, yaxis_y0);
		cairo_show_text(cr, min_s_str);
	}
	if (u_graphs) {
		/* draw y-axis: max and min usage */
		cairo_move_to(cr, yaxis_ux, yaxis_y1);
		cairo_show_text(cr, max_u_str);

		cairo_move_to(cr, yaxis_ux, yaxis_y0);
		cairo_show_text(cr, min_u_str);
	}
	if (t_graphs + s_graphs + u_graphs) {
		/* draw x-axis: begin and end times */
		cairo_move_to(cr, info.g_xoff, height - GRAPH_V_PADDING);
		cairo_show_text(cr, bt_str);

		cairo_move_to(cr,
				info.g_width - et_te.width,
				height - GRAPH_V_PADDING);
		cairo_show_text(cr, et_str);
	} else {
		display_no_graphs_warning(cr, &info);
	}
	free(et_str);
	free(bt_str);
	free(min_t_str);
	free(max_t_str);
	free(min_s_str);
	free(max_s_str);
	free(min_u_str);
	free(max_u_str);
	cairo_font_face_destroy(labelfont);

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
