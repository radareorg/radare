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

#include "layout.h"
#include "graph.h"




enum  {
	GRAVA_LAYOUT_DUMMY_PROPERTY
};
static void grava_layout_real_run (GravaLayout* self, GravaGraph* graph);
static void grava_layout_real_set_graph (GravaLayout* self, GravaGraph* graph);
static void grava_layout_real_reset (GravaLayout* self);
static gpointer grava_layout_parent_class = NULL;



#line 23 "layout.vala"
static void grava_layout_real_run (GravaLayout* self, GravaGraph* graph) {
#line 23 "layout.vala"
	g_return_if_fail (self != NULL);
#line 23 "layout.vala"
	g_return_if_fail (graph != NULL);
}


#line 23 "layout.vala"
void grava_layout_run (GravaLayout* self, GravaGraph* graph) {
#line 23 "layout.vala"
	GRAVA_LAYOUT_GET_CLASS (self)->run (self, graph);
}


#line 24 "layout.vala"
static void grava_layout_real_set_graph (GravaLayout* self, GravaGraph* graph) {
#line 24 "layout.vala"
	g_return_if_fail (self != NULL);
#line 24 "layout.vala"
	g_return_if_fail (graph != NULL);
}


#line 24 "layout.vala"
void grava_layout_set_graph (GravaLayout* self, GravaGraph* graph) {
#line 24 "layout.vala"
	GRAVA_LAYOUT_GET_CLASS (self)->set_graph (self, graph);
}


#line 25 "layout.vala"
static void grava_layout_real_reset (GravaLayout* self) {
#line 25 "layout.vala"
	g_return_if_fail (self != NULL);
}


#line 25 "layout.vala"
void grava_layout_reset (GravaLayout* self) {
#line 25 "layout.vala"
	GRAVA_LAYOUT_GET_CLASS (self)->reset (self);
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




