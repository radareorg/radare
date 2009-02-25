/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007  pancake <youterm.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CHART_H__
#define __CHART_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define GRAVA_TYPE_CHART (grava_chart_get_type ())
#define GRAVA_CHART(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_CHART, GravaChart))
#define GRAVA_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_CHART, GravaChartClass))
#define GRAVA_IS_CHART(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_CHART))
#define GRAVA_IS_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_CHART))
#define GRAVA_CHART_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_CHART, GravaChartClass))

typedef struct _GravaChart GravaChart;
typedef struct _GravaChartClass GravaChartClass;
typedef struct _GravaChartPrivate GravaChartPrivate;

/*
Charting api for Vala
=====================
 - integrated with grava
 - support for colors
 - support for:
   - bars
   - 2D dot space
   - 2D lines
 Chart class
   setColor("#ff0000");
   setLimits(0,5000);
   setBar("code", 100);
   setColumn("food", 30);
   setPoint(x, y, "#ff0000")
*/
struct _GravaChart {
	GObject parent_instance;
	GravaChartPrivate * priv;
};

struct _GravaChartClass {
	GObjectClass parent_class;
};


void grava_chart_setColor (GravaChart* self, const char* color);
void grava_chart_setLimits (GravaChart* self, gint from, gint to);
void grava_chart_setBar (GravaChart* self, const char* name, gint val);
void grava_chart_setColumn (GravaChart* self, const char* name, gint val);
void grava_chart_setPoint (GravaChart* self, gint x, gint y, const char* color);
void grava_chart_draw (GravaChart* self);
GravaChart* grava_chart_construct (GType object_type);
GravaChart* grava_chart_new (void);
GType grava_chart_get_type (void);


G_END_DECLS

#endif
