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

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GravaLayout GravaLayout;
typedef struct _GravaLayoutClass GravaLayoutClass;
typedef struct _GravaGraph GravaGraph;
typedef struct _GravaGraphClass GravaGraphClass;

#define GRAVA_TYPE_LAYOUT (grava_layout_get_type ())
#define GRAVA_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_LAYOUT, GravaLayout))
#define GRAVA_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_LAYOUT, GravaLayoutClass))
#define GRAVA_IS_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_LAYOUT))
#define GRAVA_IS_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_LAYOUT))
#define GRAVA_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_LAYOUT, GravaLayoutClass))

typedef struct _GravaLayoutPrivate GravaLayoutPrivate;

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


void grava_layout_run (GravaLayout* self, GravaGraph* graph);
void grava_layout_set_graph (GravaLayout* self, GravaGraph* graph);
void grava_layout_reset (GravaLayout* self);
GType grava_layout_get_type (void);


G_END_DECLS

#endif
