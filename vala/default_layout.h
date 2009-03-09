/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007, 2008   pancake <youterm.com>
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

#ifndef __DEFAULT_LAYOUT_H__
#define __DEFAULT_LAYOUT_H__

#include <glib.h>
#include <glib-object.h>
#include <float.h>
#include <math.h>
#include "layout.h"
#include "node.h"
#include "graph.h"

G_BEGIN_DECLS


#define GRAVA_TYPE_DEFAULT_LAYOUT (grava_default_layout_get_type ())
#define GRAVA_DEFAULT_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayout))
#define GRAVA_DEFAULT_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayoutClass))
#define GRAVA_IS_DEFAULT_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_DEFAULT_LAYOUT))
#define GRAVA_IS_DEFAULT_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_DEFAULT_LAYOUT))
#define GRAVA_DEFAULT_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_DEFAULT_LAYOUT, GravaDefaultLayoutClass))

typedef struct _GravaDefaultLayout GravaDefaultLayout;
typedef struct _GravaDefaultLayoutClass GravaDefaultLayoutClass;
typedef struct _GravaDefaultLayoutPrivate GravaDefaultLayoutPrivate;

struct _GravaDefaultLayout {
	GravaLayout parent_instance;
	GravaDefaultLayoutPrivate * priv;
	double y_offset;
	double x_offset;
	GHashTable* data;
};

struct _GravaDefaultLayoutClass {
	GravaLayoutClass parent_class;
};


void grava_default_layout_reset_layout (GravaDefaultLayout* self);
void grava_default_layout_reset_real (GravaDefaultLayout* self);
void grava_default_layout_setxy (GravaDefaultLayout* self, GravaNode* n);
gboolean grava_default_layout_getxy (GravaDefaultLayout* self, GravaNode** n);
void grava_default_layout_walkChild (GravaDefaultLayout* self, GravaNode* node, gint level);
GravaNode* grava_default_layout_get_parent (GravaDefaultLayout* self, GravaNode* node);
GravaDefaultLayout* grava_default_layout_construct (GType object_type);
GravaDefaultLayout* grava_default_layout_new (void);
GType grava_default_layout_get_type (void);


G_END_DECLS

#endif
