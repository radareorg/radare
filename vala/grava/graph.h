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

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <glib.h>
#include <glib-object.h>
#include <float.h>
#include <math.h>
#include <cairo.h>
#include "layout.h"
#include "node.h"
#include "edge.h"

G_BEGIN_DECLS


#define GRAVA_TYPE_GRAPH (grava_graph_get_type ())
#define GRAVA_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_GRAPH, GravaGraph))
#define GRAVA_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_GRAPH, GravaGraphClass))
#define GRAVA_IS_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_GRAPH))
#define GRAVA_IS_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_GRAPH))
#define GRAVA_GRAPH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_GRAPH, GravaGraphClass))

typedef struct _GravaGraphPrivate GravaGraphPrivate;

struct _GravaGraph {
	GObject parent;
	GravaGraphPrivate * priv;
	GravaLayout* layout;
	GSList* nodes;
	GSList* edges;
	double zoom;
	double panx;
	double pany;
	double angle;
};
struct _GravaGraphClass {
	GObjectClass parent;
};

GravaNode* grava_graph_selected;
void grava_graph_select_next (GravaGraph* self);
gboolean grava_graph_is_selected (GravaNode* n);
void grava_graph_update (GravaGraph* self);
void grava_graph_add_node (GravaGraph* self, GravaNode* n);
void grava_graph_add_edge (GravaGraph* self, GravaEdge* e);
GSList* grava_graph_unlinked_nodes (GravaGraph* self);
GSList* grava_graph_outer_nodes (GravaGraph* self, GravaNode* n);
GSList* grava_graph_inner_nodes (GravaGraph* self, GravaNode* n);
GravaNode* grava_graph_click (GravaGraph* self, double x, double y);
gboolean grava_graph_overlaps (GravaGraph* self, GravaNode* n);
void grava_graph_draw (GravaGraph* self, cairo_t* ctx);
void grava_graph_add (GravaGraph* self, GravaNode* n);
void grava_graph_link (GravaGraph* self, GravaNode* n, GravaNode* n2);
GravaGraph* grava_graph_new (void);
GType grava_graph_get_type (void);

G_END_DECLS

#endif
