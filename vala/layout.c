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

#include <glib.h>
#include <glib-object.h>


#define GRAVA_TYPE_LAYOUT (grava_layout_get_type ())
#define GRAVA_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_LAYOUT, GravaLayout))
#define GRAVA_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_LAYOUT, GravaLayoutClass))
#define GRAVA_IS_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_LAYOUT))
#define GRAVA_IS_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_LAYOUT))
#define GRAVA_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_LAYOUT, GravaLayoutClass))

typedef struct _GravaLayout GravaLayout;
typedef struct _GravaLayoutClass GravaLayoutClass;
typedef struct _GravaLayoutPrivate GravaLayoutPrivate;

#define GRAVA_TYPE_GRAPH (grava_graph_get_type ())
#define GRAVA_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_GRAPH, GravaGraph))
#define GRAVA_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_GRAPH, GravaGraphClass))
#define GRAVA_IS_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_GRAPH))
#define GRAVA_IS_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_GRAPH))
#define GRAVA_GRAPH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_GRAPH, GravaGraphClass))

typedef struct _GravaGraph GravaGraph;
typedef struct _GravaGraphClass GravaGraphClass;

struct _GravaLayout {
	GObject parent_instance;
	GravaLayoutPrivate * priv;
};

struct _GravaLayoutClass {
	GObjectClass parent_class;
	void (*run) (GravaLayout* self, GravaGraph* graph);
	void (*set_graph) (GravaLayout* self, GravaGraph* graph);
	void (*reset) (GravaLayout* self);
};



GType grava_layout_get_type (void);
GType grava_graph_get_type (void);
enum  {
	GRAVA_LAYOUT_DUMMY_PROPERTY
};
void grava_layout_run (GravaLayout* self, GravaGraph* graph);
static void grava_layout_real_run (GravaLayout* self, GravaGraph* graph);
void grava_layout_set_graph (GravaLayout* self, GravaGraph* graph);
static void grava_layout_real_set_graph (GravaLayout* self, GravaGraph* graph);
void grava_layout_reset (GravaLayout* self);
static void grava_layout_real_reset (GravaLayout* self);
static gpointer grava_layout_parent_class = NULL;



#line 23 "layout.vala"
static void grava_layout_real_run (GravaLayout* self, GravaGraph* graph) {
#line 23 "layout.vala"
	g_return_if_fail (self != NULL);
#line 23 "layout.vala"
	g_return_if_fail (graph != NULL);
#line 80 "layout.c"
}


#line 23 "layout.vala"
void grava_layout_run (GravaLayout* self, GravaGraph* graph) {
#line 23 "layout.vala"
	GRAVA_LAYOUT_GET_CLASS (self)->run (self, graph);
#line 88 "layout.c"
}


#line 24 "layout.vala"
static void grava_layout_real_set_graph (GravaLayout* self, GravaGraph* graph) {
#line 24 "layout.vala"
	g_return_if_fail (self != NULL);
#line 24 "layout.vala"
	g_return_if_fail (graph != NULL);
#line 98 "layout.c"
}


#line 24 "layout.vala"
void grava_layout_set_graph (GravaLayout* self, GravaGraph* graph) {
#line 24 "layout.vala"
	GRAVA_LAYOUT_GET_CLASS (self)->set_graph (self, graph);
#line 106 "layout.c"
}


#line 25 "layout.vala"
static void grava_layout_real_reset (GravaLayout* self) {
#line 25 "layout.vala"
	g_return_if_fail (self != NULL);
#line 114 "layout.c"
}


#line 25 "layout.vala"
void grava_layout_reset (GravaLayout* self) {
#line 25 "layout.vala"
	GRAVA_LAYOUT_GET_CLASS (self)->reset (self);
#line 122 "layout.c"
}


static void grava_layout_class_init (GravaLayoutClass * klass) {
	grava_layout_parent_class = g_type_class_peek_parent (klass);
	GRAVA_LAYOUT_CLASS (klass)->run = grava_layout_real_run;
	GRAVA_LAYOUT_CLASS (klass)->set_graph = grava_layout_real_set_graph;
	GRAVA_LAYOUT_CLASS (klass)->reset = grava_layout_real_reset;
}


static void grava_layout_instance_init (GravaLayout * self) {
}


GType grava_layout_get_type (void) {
	static GType grava_layout_type_id = 0;
	if (grava_layout_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaLayoutClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_layout_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaLayout), 0, (GInstanceInitFunc) grava_layout_instance_init, NULL };
		grava_layout_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaLayout", &g_define_type_info, G_TYPE_FLAG_ABSTRACT);
	}
	return grava_layout_type_id;
}




