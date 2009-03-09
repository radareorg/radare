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

#ifndef __EDGE_H__
#define __EDGE_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "node.h"

G_BEGIN_DECLS


#define GRAVA_TYPE_EDGE (grava_edge_get_type ())
#define GRAVA_EDGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_EDGE, GravaEdge))
#define GRAVA_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_EDGE, GravaEdgeClass))
#define GRAVA_IS_EDGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_EDGE))
#define GRAVA_IS_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_EDGE))
#define GRAVA_EDGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_EDGE, GravaEdgeClass))

typedef struct _GravaEdge GravaEdge;
typedef struct _GravaEdgeClass GravaEdgeClass;
typedef struct _GravaEdgePrivate GravaEdgePrivate;

struct _GravaEdge {
	GObject parent_instance;
	GravaEdgePrivate * priv;
	GHashTable* data;
	GravaNode* orig;
	GravaNode* dest;
	gboolean visible;
	gboolean jmpcnd;
};

struct _GravaEdgeClass {
	GObjectClass parent_class;
};


char* grava_edge_get_s (GravaEdge* self, const char* val);
void grava_edge_set_s (GravaEdge* self, const char* val, const char* key);
GravaEdge* grava_edge_with (GravaEdge* self, GravaNode* a, GravaNode* b);
double grava_edge_distance (GravaEdge* self);
GravaEdge* grava_edge_construct (GType object_type);
GravaEdge* grava_edge_new (void);
GType grava_edge_get_type (void);


G_END_DECLS

#endif
