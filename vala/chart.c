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

#include "chart.h"
#include <stdlib.h>
#include <string.h>


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



enum  {
	GRAVA_CHART_DUMMY_PROPERTY
};
static void grava_chart_setColor (GravaChart* self, const char* color);
static void grava_chart_setLimits (GravaChart* self, gint from, gint to);
static void grava_chart_setBar (GravaChart* self, const char* name, gint val);
static void grava_chart_setColumn (GravaChart* self, const char* name, gint val);
static void grava_chart_setPoint (GravaChart* self, gint x, gint y, const char* color);
static void grava_chart_draw (GravaChart* self);
static GravaChart* grava_chart_construct (GType object_type);
static GravaChart* grava_chart_new (void);
static GObject * grava_chart_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_chart_parent_class = NULL;
static GType grava_chart_get_type (void);



/*bars = new SList<Bar>();*/
#line 48 "chart.vala"
static void grava_chart_setColor (GravaChart* self, const char* color) {
#line 48 "chart.vala"
	g_return_if_fail (self != NULL);
#line 48 "chart.vala"
	g_return_if_fail (color != NULL);
}


#line 52 "chart.vala"
static void grava_chart_setLimits (GravaChart* self, gint from, gint to) {
#line 52 "chart.vala"
	g_return_if_fail (self != NULL);
}


#line 56 "chart.vala"
static void grava_chart_setBar (GravaChart* self, const char* name, gint val) {
#line 56 "chart.vala"
	g_return_if_fail (self != NULL);
#line 56 "chart.vala"
	g_return_if_fail (name != NULL);
}


#line 60 "chart.vala"
static void grava_chart_setColumn (GravaChart* self, const char* name, gint val) {
#line 60 "chart.vala"
	g_return_if_fail (self != NULL);
#line 60 "chart.vala"
	g_return_if_fail (name != NULL);
}


#line 64 "chart.vala"
static void grava_chart_setPoint (GravaChart* self, gint x, gint y, const char* color) {
#line 64 "chart.vala"
	g_return_if_fail (self != NULL);
#line 64 "chart.vala"
	g_return_if_fail (color != NULL);
}


#line 68 "chart.vala"
static void grava_chart_draw (GravaChart* self) {
#line 68 "chart.vala"
	g_return_if_fail (self != NULL);
}


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
#line 39 "chart.vala"
static GravaChart* grava_chart_construct (GType object_type) {
	GravaChart * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 39 "chart.vala"
static GravaChart* grava_chart_new (void) {
#line 39 "chart.vala"
	return grava_chart_construct (GRAVA_TYPE_CHART);
}


/*public SList<Bar> bars;
 constructor */
static GObject * grava_chart_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GravaChartClass * klass;
	GObjectClass * parent_class;
	GravaChart * self;
	klass = GRAVA_CHART_CLASS (g_type_class_peek (GRAVA_TYPE_CHART));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GRAVA_CHART (obj);
	{
	}
	return obj;
}


static void grava_chart_class_init (GravaChartClass * klass) {
	grava_chart_parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->constructor = grava_chart_constructor;
}


static void grava_chart_instance_init (GravaChart * self) {
}


static GType grava_chart_get_type (void) {
	static GType grava_chart_type_id = 0;
	if (grava_chart_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaChartClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_chart_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaChart), 0, (GInstanceInitFunc) grava_chart_instance_init, NULL };
		grava_chart_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaChart", &g_define_type_info, 0);
	}
	return grava_chart_type_id;
}




