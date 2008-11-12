/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007,2008  pancake <youterm.com>
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

#ifndef __NODE_H__
#define __NODE_H__

#include <glib.h>
#include <glib-object.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define GRAVA_TYPE_NODE (grava_node_get_type ())
#define GRAVA_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_NODE, GravaNode))
#define GRAVA_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_NODE, GravaNodeClass))
#define GRAVA_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_NODE))
#define GRAVA_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_NODE))
#define GRAVA_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_NODE, GravaNodeClass))

typedef struct _GravaNode GravaNode;
typedef struct _GravaNodeClass GravaNodeClass;
typedef struct _GravaNodePrivate GravaNodePrivate;

struct _GravaNode {
	GObject parent_instance;
	GravaNodePrivate * priv;
	GHashTable* data;
	guint baseaddr;
	GSList* calls;
	GSList* xrefs;
	gboolean visible;
	gboolean selected;
	gboolean has_body;
	gint shape;
	double x;
	double y;
	double w;
	double h;
};

struct _GravaNodeClass {
	GObjectClass parent_class;
};


void grava_node_set (GravaNode* self, const char* key, const char* val);
void grava_node_set_i (GravaNode* self, const char* key, guint64 val);
char* grava_node_get (GravaNode* self, const char* key);
void grava_node_add_call (GravaNode* self, guint64 addr);
void grava_node_add_xref (GravaNode* self, guint64 addr);
gboolean grava_node_overlaps (GravaNode* self, GravaNode* n);
void grava_node_fit (GravaNode* self);
GravaNode* grava_node_construct (GType object_type);
GravaNode* grava_node_new (void);
GType grava_node_get_type (void);


G_END_DECLS

#endif
