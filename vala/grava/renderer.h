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
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "edge.h"
#include "node.h"

G_BEGIN_DECLS

typedef struct _GravaRenderer GravaRenderer;

struct _GravaRenderer {
};

void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge);
void grava_renderer_set_color (cairo_t* ctx, GHashTable* ht);
void grava_renderer_set_color_str (cairo_t* ctx, const char* color);
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node);
void grava_renderer_square (cairo_t* ctx, double w, double h);
void grava_renderer_line (cairo_t* ctx, double x, double y, double w, double h);
GravaRenderer* grava_renderer_new (void);
void grava_renderer_free (GravaRenderer* self);

G_END_DECLS

#endif
