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

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "edge.h"
#include "node.h"

G_BEGIN_DECLS


#define GRAVA_TYPE_RENDERER (grava_renderer_get_type ())
#define GRAVA_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_RENDERER, GravaRenderer))
#define GRAVA_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_RENDERER, GravaRendererClass))
#define GRAVA_IS_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_RENDERER))
#define GRAVA_IS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_RENDERER))
#define GRAVA_RENDERER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_RENDERER, GravaRendererClass))

typedef struct _GravaRenderer GravaRenderer;
typedef struct _GravaRendererClass GravaRendererClass;
typedef struct _GravaRendererPrivate GravaRendererPrivate;
typedef struct _GravaParamSpecRenderer GravaParamSpecRenderer;

struct _GravaRenderer {
	GTypeInstance parent_instance;
	volatile int ref_count;
	GravaRendererPrivate * priv;
};

struct _GravaRendererClass {
	GTypeClass parent_class;
	void (*finalize) (GravaRenderer *self);
};

struct _GravaParamSpecRenderer {
	GParamSpec parent_instance;
};


void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge);
void grava_renderer_set_color (cairo_t* ctx, GHashTable* ht);
gboolean grava_renderer_set_color_str (cairo_t* ctx, const char* color);
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node);
void grava_renderer_circle (cairo_t* ctx, double w, double h);
void grava_renderer_square (cairo_t* ctx, double w, double h);
void grava_renderer_line (cairo_t* ctx, double x, double y, double w, double h);
GravaRenderer* grava_renderer_construct (GType object_type);
GravaRenderer* grava_renderer_new (void);
GParamSpec* grava_param_spec_renderer (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
gpointer grava_value_get_renderer (const GValue* value);
void grava_value_set_renderer (GValue* value, gpointer v_object);
GType grava_renderer_get_type (void);
gpointer grava_renderer_ref (gpointer instance);
void grava_renderer_unref (gpointer instance);


G_END_DECLS

#endif
