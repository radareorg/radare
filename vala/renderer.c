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
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <gobject/gvaluecollector.h>


#define GRAVA_TYPE_RENDERER (grava_renderer_get_type ())
#define GRAVA_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_RENDERER, GravaRenderer))
#define GRAVA_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_RENDERER, GravaRendererClass))
#define GRAVA_IS_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_RENDERER))
#define GRAVA_IS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_RENDERER))
#define GRAVA_RENDERER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_RENDERER, GravaRendererClass))

typedef struct _GravaRenderer GravaRenderer;
typedef struct _GravaRendererClass GravaRendererClass;
typedef struct _GravaRendererPrivate GravaRendererPrivate;

#define GRAVA_TYPE_EDGE (grava_edge_get_type ())
#define GRAVA_EDGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_EDGE, GravaEdge))
#define GRAVA_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_EDGE, GravaEdgeClass))
#define GRAVA_IS_EDGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_EDGE))
#define GRAVA_IS_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_EDGE))
#define GRAVA_EDGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_EDGE, GravaEdgeClass))

typedef struct _GravaEdge GravaEdge;
typedef struct _GravaEdgeClass GravaEdgeClass;
typedef struct _GravaEdgePrivate GravaEdgePrivate;

#define GRAVA_TYPE_NODE (grava_node_get_type ())
#define GRAVA_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_NODE, GravaNode))
#define GRAVA_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_NODE, GravaNodeClass))
#define GRAVA_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_NODE))
#define GRAVA_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_NODE))
#define GRAVA_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_NODE, GravaNodeClass))

typedef struct _GravaNode GravaNode;
typedef struct _GravaNodeClass GravaNodeClass;
typedef struct _GravaNodePrivate GravaNodePrivate;

#define GRAVA_TYPE_SHAPE (grava_shape_get_type ())
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

typedef enum  {
	GRAVA_SHAPE_RECTANGLE = 0,
	GRAVA_SHAPE_CIRCLE
} GravaShape;

struct _GravaParamSpecRenderer {
	GParamSpec parent_instance;
};



gpointer grava_renderer_ref (gpointer instance);
void grava_renderer_unref (gpointer instance);
GParamSpec* grava_param_spec_renderer (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags);
void grava_value_set_renderer (GValue* value, gpointer v_object);
gpointer grava_value_get_renderer (const GValue* value);
GType grava_renderer_get_type (void);
enum  {
	GRAVA_RENDERER_DUMMY_PROPERTY
};
GType grava_edge_get_type (void);
GType grava_node_get_type (void);
extern GravaNode* grava_graph_selected;
void grava_renderer_set_color (cairo_t* ctx, GHashTable* ht);
void grava_renderer_triangle (cairo_t* ctx, double w, double h, gboolean down);
void grava_renderer_line (cairo_t* ctx, double x, double y, double w, double h);
void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge);
gboolean grava_renderer_set_color_str (cairo_t* ctx, const char* color);
GType grava_shape_get_type (void);
void grava_renderer_circle (cairo_t* ctx, double w, double h);
void grava_renderer_square (cairo_t* ctx, double w, double h);
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node);
GravaRenderer* grava_renderer_new (void);
GravaRenderer* grava_renderer_construct (GType object_type);
GravaRenderer* grava_renderer_new (void);
static gpointer grava_renderer_parent_class = NULL;
static void grava_renderer_finalize (GravaRenderer* obj);
static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func);
static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);
static gint _vala_array_length (gpointer array);
static int _vala_strcmp0 (const char * str1, const char * str2);



#line 25 "renderer.vala"
void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge) {
#line 158 "renderer.c"
	double dx;
	double dy;
	double oh;
#line 25 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 25 "renderer.vala"
	g_return_if_fail (edge != NULL);
#line 166 "renderer.c"
	dx = (double) 0;
	dy = (double) 0;
	oh = edge->orig->h;
#line 31 "renderer.vala"
	if (edge->orig->has_body == FALSE) {
#line 32 "renderer.vala"
		oh = (double) 16;
#line 174 "renderer.c"
	}
#line 34 "renderer.vala"
	cairo_save (ctx);
#line 178 "renderer.c"
	/* triangle dest*/
#line 37 "renderer.vala"
	if (grava_graph_selected == edge->orig) {
#line 38 "renderer.vala"
		cairo_set_line_width (ctx, (double) 6);
#line 184 "renderer.c"
	} else {
#line 39 "renderer.vala"
		cairo_set_line_width (ctx, (double) 1);
#line 188 "renderer.c"
	}
#line 40 "renderer.vala"
	grava_renderer_set_color (ctx, edge->data);
#line 41 "renderer.vala"
	cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 42 "renderer.vala"
	cairo_translate (ctx, dx - 4, dy);
#line 43 "renderer.vala"
	grava_renderer_triangle (ctx, (double) 8, (double) 8, TRUE);
#line 44 "renderer.vala"
	cairo_stroke (ctx);
#line 46 "renderer.vala"
	cairo_restore (ctx);
#line 47 "renderer.vala"
	cairo_save (ctx);
#line 49 "renderer.vala"
	grava_renderer_set_color (ctx, edge->data);
#line 51 "renderer.vala"
	cairo_set_line_width (ctx, (double) 2);
#line 208 "renderer.c"
	/* oxymoroon! */
#line 53 "renderer.vala"
	if (edge->orig == edge->dest) {
#line 212 "renderer.c"
		double ox;
		/* bottom to up */
#line 55 "renderer.vala"
		cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 56 "renderer.vala"
		cairo_move_to (ctx, (double) 0, (double) 0);
#line 219 "renderer.c"
		/*dx = edge.dest.x-edge.orig.x;*/
#line 58 "renderer.vala"
		dx = ((edge->dest->x - edge->orig->x) - (edge->orig->w / 1.3)) + (edge->dest->w / 1.3);
#line 223 "renderer.c"
		/*-edge.orig.x;*/
#line 59 "renderer.vala"
		dy = (edge->dest->y - edge->orig->y) - oh;
#line 227 "renderer.c"
		/* or 80 or so depending if > or < ???*/
		ox = dx;
		/*if (ox == 0){ ox = 150; }
		arrow*/
#line 63 "renderer.vala"
		if (grava_graph_selected == edge->orig) {
#line 64 "renderer.vala"
			cairo_set_line_width (ctx, (double) 6);
#line 236 "renderer.c"
		}
#line 66 "renderer.vala"
		dx = edge->dest->w;
#line 240 "renderer.c"
		/*dy += edge.dest.h/2;
		////ctx.curve_to(0, 0, 200, 100, dx-edge.orig.w/2, dy);*/
#line 69 "renderer.vala"
		cairo_curve_to (ctx, (double) 0, (double) 0, (edge->dest->x - edge->orig->x) / 2, (edge->dest->y - edge->orig->y) / 2, (edge->dest->x - edge->orig->x) / 2, (edge->dest->y - edge->orig->y) / 2);
#line 245 "renderer.c"
	} else {
#line 93 "renderer.vala"
		if ((edge->orig->y + oh) < edge->dest->y) {
#line 249 "renderer.c"
			/*-edge.dest.h)) {
			 up to bottom 
			ctx.translate (edge.orig.x+(edge.orig.w/1.3),edge.orig.y+oh);*/
#line 96 "renderer.vala"
			cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 97 "renderer.vala"
			cairo_move_to (ctx, (double) 0, (double) 0);
#line 257 "renderer.c"
			/*dx = edge.dest.x-edge.orig.x-(edge.orig.w/1.3) + edge.dest.w/1.3; //-edge.orig.x;*/
#line 99 "renderer.vala"
			dx = ((edge->dest->x - edge->orig->x) + (edge->dest->w / 2)) - (edge->orig->w / 2);
#line 100 "renderer.vala"
			dy = (edge->dest->y - edge->orig->y) - oh;
#line 263 "renderer.c"
			/*ctx.move_to(30,30);
			ctx.set_source_rgb (0.0, 0.0, 0.0);*/
#line 103 "renderer.vala"
			if (grava_graph_selected == edge->orig) {
#line 104 "renderer.vala"
				cairo_set_line_width (ctx, (double) 6);
#line 270 "renderer.c"
			}
#line 105 "renderer.vala"
			grava_renderer_line (ctx, (double) 0, (double) 0, dx, dy);
#line 274 "renderer.c"
		} else {
			double ox;
			double _x;
			double _y;
			/* bottom to up */
#line 108 "renderer.vala"
			cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 109 "renderer.vala"
			cairo_move_to (ctx, (double) 0, (double) 0);
#line 284 "renderer.c"
			/*dx = edge.dest.x-edge.orig.x;*/
#line 111 "renderer.vala"
			dx = ((edge->dest->x - edge->orig->x) + (edge->dest->w / 2)) - (edge->orig->w / 2);
#line 112 "renderer.vala"
			dy = (edge->dest->y - edge->orig->y) - oh;
#line 290 "renderer.c"
			/* or 80 or so depending if > or < ???*/
			ox = dx;
			/*if (ox == 0){ ox = 150; }
			arrow*/
#line 116 "renderer.vala"
			if (grava_graph_selected == edge->orig) {
#line 117 "renderer.vala"
				cairo_set_line_width (ctx, (double) 6);
#line 299 "renderer.c"
			}
			_x = (-(edge->orig->x - edge->dest->x)) / 1.5;
			_y = (-(edge->orig->y - edge->dest->y)) / 3;
#line 120 "renderer.vala"
			cairo_curve_to (ctx, _x, _y, _x, _y, dx, dy);
#line 305 "renderer.c"
		}
	}
	/*ctx.stroke();*/
#line 124 "renderer.vala"
	cairo_stroke (ctx);
#line 125 "renderer.vala"
	if (grava_graph_selected == edge->orig) {
#line 126 "renderer.vala"
		cairo_set_line_width (ctx, (double) 6);
#line 315 "renderer.c"
	} else {
#line 127 "renderer.vala"
		cairo_set_line_width (ctx, (double) 1);
#line 319 "renderer.c"
	}
	/* triangle dest*/
#line 130 "renderer.vala"
	cairo_translate (ctx, dx - 4, dy - 8);
#line 131 "renderer.vala"
	grava_renderer_triangle (ctx, (double) 8, (double) 8, TRUE);
#line 132 "renderer.vala"
	cairo_stroke (ctx);
#line 134 "renderer.vala"
	cairo_restore (ctx);
#line 135 "renderer.vala"
	cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, (double) 1);
#line 332 "renderer.c"
}


#line 139 "renderer.vala"
void grava_renderer_set_color (cairo_t* ctx, GHashTable* ht) {
#line 338 "renderer.c"
	const char* _tmp0_;
	char* color;
#line 139 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 139 "renderer.vala"
	g_return_if_fail (ht != NULL);
#line 141 "renderer.vala"
	_tmp0_ = NULL;
#line 347 "renderer.c"
	color = (_tmp0_ = (const char*) g_hash_table_lookup (ht, "color"), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 142 "renderer.vala"
	grava_renderer_set_color_str (ctx, color);
#line 351 "renderer.c"
	color = (g_free (color), NULL);
}


#line 145 "renderer.vala"
gboolean grava_renderer_set_color_str (cairo_t* ctx, const char* color) {
#line 145 "renderer.vala"
	g_return_val_if_fail (ctx != NULL, FALSE);
#line 147 "renderer.vala"
	if (color != NULL) {
#line 148 "renderer.vala"
		if (_vala_strcmp0 (color, "black") == 0) {
#line 149 "renderer.vala"
			cairo_set_source_rgba (ctx, 0.0, 0.0, 0.0, 0.7);
#line 366 "renderer.c"
		} else {
#line 151 "renderer.vala"
			if (_vala_strcmp0 (color, "white") == 0) {
#line 152 "renderer.vala"
				cairo_set_source_rgba (ctx, 0.9, 0.9, 0.9, 0.8);
#line 372 "renderer.c"
			} else {
#line 154 "renderer.vala"
				if (_vala_strcmp0 (color, "green") == 0) {
#line 155 "renderer.vala"
					cairo_set_source_rgba (ctx, 0.1, 0.7, 0.1, 0.8);
#line 378 "renderer.c"
				} else {
#line 157 "renderer.vala"
					if (_vala_strcmp0 (color, "red") == 0) {
#line 158 "renderer.vala"
						cairo_set_source_rgba (ctx, 1.0, 0.1, 0.1, 0.9);
#line 384 "renderer.c"
					} else {
#line 160 "renderer.vala"
						if (_vala_strcmp0 (color, "turqoise") == 0) {
#line 161 "renderer.vala"
							cairo_set_source_rgba (ctx, 0.6, 0.9, 1.0, 0.8);
#line 390 "renderer.c"
						} else {
#line 163 "renderer.vala"
							if (_vala_strcmp0 (color, "blue") == 0) {
#line 164 "renderer.vala"
								cairo_set_source_rgba (ctx, 0.2, 0.5, 0.9, 0.8);
#line 396 "renderer.c"
							} else {
#line 166 "renderer.vala"
								if (_vala_strcmp0 (color, "yellow") == 0) {
#line 167 "renderer.vala"
									cairo_set_source_rgba (ctx, 0.9, 0.9, 0.0, 0.6);
#line 402 "renderer.c"
								} else {
#line 169 "renderer.vala"
									if (_vala_strcmp0 (color, "gray") == 0) {
#line 170 "renderer.vala"
										cairo_set_source_rgba (ctx, 0.8, 0.8, 0.8, 0.8);
#line 408 "renderer.c"
									} else {
#line 172 "renderer.vala"
										if (_vala_strcmp0 (color, "beige") == 0) {
#line 173 "renderer.vala"
											cairo_set_source_rgba (ctx, 0.9, 0.9, 0.6, 0.7);
#line 414 "renderer.c"
										} else {
#line 175 "renderer.vala"
											if (_vala_strcmp0 (color, "darkgray") == 0) {
#line 176 "renderer.vala"
												cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, 0.8);
#line 420 "renderer.c"
											} else {
#line 178 "renderer.vala"
												cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.4);
#line 424 "renderer.c"
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	} else {
#line 180 "renderer.vala"
		cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.8);
#line 438 "renderer.c"
	}
#line 182 "renderer.vala"
	return color != NULL;
#line 442 "renderer.c"
}


#line 185 "renderer.vala"
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node) {
#line 185 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 185 "renderer.vala"
	g_return_if_fail (node != NULL);
#line 187 "renderer.vala"
	cairo_save (ctx);
#line 189 "renderer.vala"
	cairo_set_tolerance (ctx, 0.1);
#line 190 "renderer.vala"
	cairo_set_line_join (ctx, CAIRO_LINE_JOIN_ROUND);
#line 191 "renderer.vala"
	cairo_set_line_width (ctx, (double) 1);
#line 192 "renderer.vala"
	cairo_translate (ctx, node->x, node->y);
#line 462 "renderer.c"
	/*ctx.set_source_rgb (1, 1, 1);*/
#line 195 "renderer.vala"
	cairo_set_source_rgb (ctx, 0.8, 0.8, 0.8);
#line 466 "renderer.c"
	/*#if 0
	if (node.calls.length() >0) 
	set_color(ctx, "red");
	else
	set_color(ctx, "blue");
	#endif
	*/
#line 203 "renderer.vala"
	grava_renderer_set_color (ctx, node->data);
#line 204 "renderer.vala"
	grava_renderer_set_color_str (ctx, (const char*) g_hash_table_lookup (node->data, "bgcolor"));
#line 206 "renderer.vala"
	if (node->has_body) {
#line 207 "renderer.vala"
		switch (node->shape) {
#line 482 "renderer.c"
			case GRAVA_SHAPE_CIRCLE:
			{
#line 209 "renderer.vala"
				grava_renderer_circle (ctx, node->w, node->h);
#line 210 "renderer.vala"
				cairo_fill (ctx);
#line 211 "renderer.vala"
				break;
#line 491 "renderer.c"
			}
			default:
			{
#line 214 "renderer.vala"
				grava_renderer_square (ctx, node->w, node->h);
#line 215 "renderer.vala"
				cairo_fill (ctx);
#line 216 "renderer.vala"
				break;
#line 501 "renderer.c"
			}
		}
	}
	/* title rectangle */
#line 221 "renderer.vala"
	if (((const char*) g_hash_table_lookup (node->data, "color")) != NULL) {
#line 222 "renderer.vala"
		grava_renderer_set_color_str (ctx, (const char*) g_hash_table_lookup (node->data, "color"));
#line 510 "renderer.c"
	} else {
#line 224 "renderer.vala"
		if (g_slist_length (node->calls) == 1) {
#line 225 "renderer.vala"
			cairo_set_source_rgba (ctx, 0.2, 0.2, 0.4, 0.7);
#line 516 "renderer.c"
		} else {
#line 227 "renderer.vala"
			if (g_slist_length (node->calls) > 0) {
#line 228 "renderer.vala"
				cairo_set_source_rgba (ctx, 0.3, 0.3, (double) 1, 0.7);
#line 522 "renderer.c"
			} else {
#line 230 "renderer.vala"
				cairo_set_source_rgba (ctx, 0.8, 0.8, 0.8, 0.8);
#line 526 "renderer.c"
			}
		}
	}
#line 231 "renderer.vala"
	grava_renderer_square (ctx, node->w, (double) 15);
#line 232 "renderer.vala"
	cairo_fill (ctx);
#line 233 "renderer.vala"
	grava_renderer_line (ctx, (double) 0, (double) 15, node->w, (double) 0);
#line 536 "renderer.c"
	/* draw minimize button */
#line 236 "renderer.vala"
	cairo_save (ctx);
#line 540 "renderer.c"
	/*ctx.set_source_rgba (0.7, 0.0, 0.0, 1);*/
#line 238 "renderer.vala"
	cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, 0.8);
#line 239 "renderer.vala"
	cairo_translate (ctx, node->w - 13, (double) 3);
#line 240 "renderer.vala"
	grava_renderer_square (ctx, (double) 10, (double) 10);
#line 241 "renderer.vala"
	cairo_fill (ctx);
#line 242 "renderer.vala"
	cairo_restore (ctx);
#line 244 "renderer.vala"
	cairo_select_font_face (ctx, "Sans Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
#line 554 "renderer.c"
	/*Courier", */
#line 247 "renderer.vala"
	cairo_set_font_size (ctx, (double) 10);
#line 558 "renderer.c"
	/* set label */
#line 250 "renderer.vala"
	cairo_set_source_rgb (ctx, 0.1, 0.1, 0.1);
#line 251 "renderer.vala"
	cairo_move_to (ctx, (double) 5, (double) 10);
#line 252 "renderer.vala"
	cairo_show_text (ctx, (const char*) g_hash_table_lookup (node->data, "label"));
#line 566 "renderer.c"
	/* set body */
#line 255 "renderer.vala"
	if (node->has_body) {
#line 570 "renderer.c"
		gint y;
		const char* _tmp0_;
		char* body;
		y = 25;
#line 257 "renderer.vala"
		_tmp0_ = NULL;
#line 577 "renderer.c"
		body = (_tmp0_ = (const char*) g_hash_table_lookup (node->data, "body"), (_tmp0_ == NULL) ? NULL : g_strdup (_tmp0_));
#line 258 "renderer.vala"
		if (body != NULL) {
#line 581 "renderer.c"
			{
				char** _tmp1_;
				char** str_collection;
				int str_collection_length1;
				int str_it;
				_tmp1_ = NULL;
#line 259 "renderer.vala"
				str_collection = _tmp1_ = g_strsplit (body, "\n", 0);
#line 590 "renderer.c"
				str_collection_length1 = _vala_array_length (_tmp1_);
				for (str_it = 0; str_it < _vala_array_length (_tmp1_); str_it = str_it + 1) {
					const char* _tmp3_;
					char* str;
#line 822 "glib-2.0.vapi"
					_tmp3_ = NULL;
#line 597 "renderer.c"
					str = (_tmp3_ = str_collection[str_it], (_tmp3_ == NULL) ? NULL : g_strdup (_tmp3_));
					{
						gboolean _tmp2_ = {0};
#line 260 "renderer.vala"
						y = y + 10;
#line 261 "renderer.vala"
						cairo_move_to (ctx, (double) 5, (double) y);
#line 262 "renderer.vala"
						if (strstr (str, "call ") != NULL) {
#line 262 "renderer.vala"
							_tmp2_ = TRUE;
#line 609 "renderer.c"
						} else {
#line 263 "renderer.vala"
							_tmp2_ = strstr (str, "bl ") != NULL;
#line 613 "renderer.c"
						}
#line 262 "renderer.vala"
						if (_tmp2_) {
#line 264 "renderer.vala"
							grava_renderer_set_color_str (ctx, "blue");
#line 619 "renderer.c"
						} else {
#line 266 "renderer.vala"
							if (strstr (str, "goto") != NULL) {
#line 267 "renderer.vala"
								grava_renderer_set_color_str (ctx, "green");
#line 625 "renderer.c"
							} else {
#line 269 "renderer.vala"
								if (strstr (str, " j") != NULL) {
#line 270 "renderer.vala"
									grava_renderer_set_color_str (ctx, "green");
#line 631 "renderer.c"
								} else {
#line 272 "renderer.vala"
									if (g_str_has_suffix (str, ":")) {
#line 273 "renderer.vala"
										grava_renderer_set_color_str (ctx, "red");
#line 637 "renderer.c"
									} else {
#line 275 "renderer.vala"
										grava_renderer_set_color_str (ctx, "black");
#line 641 "renderer.c"
									}
								}
							}
						}
#line 276 "renderer.vala"
						cairo_show_text (ctx, str);
#line 648 "renderer.c"
						str = (g_free (str), NULL);
					}
				}
#line 259 "renderer.vala"
				str_collection = (_vala_array_free (str_collection, str_collection_length1, (GDestroyNotify) g_free), NULL);
#line 654 "renderer.c"
			}
		}
		/*set_color(ctx, node.data);
		 box square */
#line 281 "renderer.vala"
		if (grava_graph_selected == node) {
#line 661 "renderer.c"
			/*ctx.set_source_rgba (1, 0.8, 0.0, 0.9);*/
#line 283 "renderer.vala"
			cairo_set_source_rgba (ctx, (double) 0, 0.0, 0.0, 1.0);
#line 284 "renderer.vala"
			cairo_set_line_width (ctx, (double) 2);
#line 667 "renderer.c"
		} else {
#line 286 "renderer.vala"
			cairo_set_source_rgba (ctx, 0.2, 0.2, 0.2, 0.4);
#line 287 "renderer.vala"
			cairo_set_line_width (ctx, (double) 1);
#line 673 "renderer.c"
		}
#line 290 "renderer.vala"
		if (node->shape == GRAVA_SHAPE_CIRCLE) {
#line 291 "renderer.vala"
			grava_renderer_circle (ctx, node->w, node->h);
#line 679 "renderer.c"
		} else {
#line 292 "renderer.vala"
			grava_renderer_square (ctx, node->w, node->h);
#line 683 "renderer.c"
		}
		body = (g_free (body), NULL);
	}
#line 295 "renderer.vala"
	cairo_stroke (ctx);
#line 297 "renderer.vala"
	cairo_restore (ctx);
#line 691 "renderer.c"
}


#line 300 "renderer.vala"
void grava_renderer_circle (cairo_t* ctx, double w, double h) {
#line 697 "renderer.c"
	double _tmp0_ = {0};
#line 300 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 301 "renderer.vala"
	cairo_save (ctx);
#line 302 "renderer.vala"
	cairo_scale (ctx, (double) 1, h / w);
#line 303 "renderer.vala"
	cairo_move_to (ctx, w, h / 2.5);
#line 304 "renderer.vala"
	if (w < h) {
#line 304 "renderer.vala"
		_tmp0_ = h;
#line 711 "renderer.c"
	} else {
#line 304 "renderer.vala"
		_tmp0_ = w;
#line 715 "renderer.c"
	}
#line 304 "renderer.vala"
	cairo_arc (ctx, w / 2, h / 2.5, _tmp0_ * 0.7, (double) 0, 2 * G_PI);
#line 305 "renderer.vala"
	cairo_restore (ctx);
#line 721 "renderer.c"
	/*ctx.arc(100,250, 50, 0, 2*Math.PI); //w/2, h/2.5, ((w<h)?h:w)*0.7, 0, 2*Math.PI);
	
	ctx.rel_line_to (w, 0);
	ctx.rel_line_to (0, h);
	ctx.rel_line_to (-w, 0);
	*/
#line 312 "renderer.vala"
	cairo_close_path (ctx);
#line 730 "renderer.c"
}


#line 315 "renderer.vala"
void grava_renderer_triangle (cairo_t* ctx, double w, double h, gboolean down) {
#line 315 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 316 "renderer.vala"
	cairo_move_to (ctx, (double) 0, (double) 0);
#line 740 "renderer.c"
	/*ctx.set_line_width (1);*/
#line 318 "renderer.vala"
	if (down) {
#line 319 "renderer.vala"
		cairo_rel_line_to (ctx, w / 2, h);
#line 320 "renderer.vala"
		cairo_rel_line_to (ctx, w / 2, -h);
#line 321 "renderer.vala"
		cairo_rel_line_to (ctx, -w, (double) 0);
#line 750 "renderer.c"
	} else {
#line 323 "renderer.vala"
		cairo_rel_line_to (ctx, w / 2, -h);
#line 324 "renderer.vala"
		cairo_rel_line_to (ctx, w / 2, h);
#line 325 "renderer.vala"
		cairo_rel_line_to (ctx, -w, (double) 0);
#line 758 "renderer.c"
	}
#line 327 "renderer.vala"
	cairo_close_path (ctx);
#line 762 "renderer.c"
}


#line 330 "renderer.vala"
void grava_renderer_square (cairo_t* ctx, double w, double h) {
#line 330 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 331 "renderer.vala"
	cairo_move_to (ctx, (double) 0, (double) 0);
#line 332 "renderer.vala"
	cairo_rel_line_to (ctx, w, (double) 0);
#line 333 "renderer.vala"
	cairo_rel_line_to (ctx, (double) 0, h);
#line 334 "renderer.vala"
	cairo_rel_line_to (ctx, -w, (double) 0);
#line 335 "renderer.vala"
	cairo_close_path (ctx);
#line 780 "renderer.c"
}


#line 338 "renderer.vala"
void grava_renderer_line (cairo_t* ctx, double x, double y, double w, double h) {
#line 338 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 339 "renderer.vala"
	cairo_move_to (ctx, x, y);
#line 340 "renderer.vala"
	cairo_rel_line_to (ctx, w, h);
#line 341 "renderer.vala"
	cairo_stroke (ctx);
#line 794 "renderer.c"
}


#line 23 "renderer.vala"
GravaRenderer* grava_renderer_construct (GType object_type) {
#line 800 "renderer.c"
	GravaRenderer* self;
	self = (GravaRenderer*) g_type_create_instance (object_type);
	return self;
}


#line 23 "renderer.vala"
GravaRenderer* grava_renderer_new (void) {
#line 23 "renderer.vala"
	return grava_renderer_construct (GRAVA_TYPE_RENDERER);
#line 811 "renderer.c"
}


static void grava_value_renderer_init (GValue* value) {
	value->data[0].v_pointer = NULL;
}


static void grava_value_renderer_free_value (GValue* value) {
	if (value->data[0].v_pointer) {
		grava_renderer_unref (value->data[0].v_pointer);
	}
}


static void grava_value_renderer_copy_value (const GValue* src_value, GValue* dest_value) {
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = grava_renderer_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}


static gpointer grava_value_renderer_peek_pointer (const GValue* value) {
	return value->data[0].v_pointer;
}


static gchar* grava_value_renderer_collect_value (GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	if (collect_values[0].v_pointer) {
		GravaRenderer* object;
		object = collect_values[0].v_pointer;
		if (object->parent_instance.g_class == NULL) {
			return g_strconcat ("invalid unclassed object pointer for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		} else if (!g_value_type_compatible (G_TYPE_FROM_INSTANCE (object), G_VALUE_TYPE (value))) {
			return g_strconcat ("invalid object type `", g_type_name (G_TYPE_FROM_INSTANCE (object)), "' for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		}
		value->data[0].v_pointer = grava_renderer_ref (object);
	} else {
		value->data[0].v_pointer = NULL;
	}
	return NULL;
}


static gchar* grava_value_renderer_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	GravaRenderer** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
		return g_strdup_printf ("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME (value));
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags && G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = grava_renderer_ref (value->data[0].v_pointer);
	}
	return NULL;
}


GParamSpec* grava_param_spec_renderer (const gchar* name, const gchar* nick, const gchar* blurb, GType object_type, GParamFlags flags) {
	GravaParamSpecRenderer* spec;
	g_return_val_if_fail (g_type_is_a (object_type, GRAVA_TYPE_RENDERER), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}


gpointer grava_value_get_renderer (const GValue* value) {
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, GRAVA_TYPE_RENDERER), NULL);
	return value->data[0].v_pointer;
}


void grava_value_set_renderer (GValue* value, gpointer v_object) {
	GravaRenderer* old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, GRAVA_TYPE_RENDERER));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, GRAVA_TYPE_RENDERER));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		grava_renderer_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		grava_renderer_unref (old);
	}
}


static void grava_renderer_class_init (GravaRendererClass * klass) {
	grava_renderer_parent_class = g_type_class_peek_parent (klass);
	GRAVA_RENDERER_CLASS (klass)->finalize = grava_renderer_finalize;
}


static void grava_renderer_instance_init (GravaRenderer * self) {
	self->ref_count = 1;
}


static void grava_renderer_finalize (GravaRenderer* obj) {
	GravaRenderer * self;
	self = GRAVA_RENDERER (obj);
}


GType grava_renderer_get_type (void) {
	static GType grava_renderer_type_id = 0;
	if (grava_renderer_type_id == 0) {
		static const GTypeValueTable g_define_type_value_table = { grava_value_renderer_init, grava_value_renderer_free_value, grava_value_renderer_copy_value, grava_value_renderer_peek_pointer, "p", grava_value_renderer_collect_value, "p", grava_value_renderer_lcopy_value };
		static const GTypeInfo g_define_type_info = { sizeof (GravaRendererClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_renderer_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaRenderer), 0, (GInstanceInitFunc) grava_renderer_instance_init, &g_define_type_value_table };
		static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
		grava_renderer_type_id = g_type_register_fundamental (g_type_fundamental_next (), "GravaRenderer", &g_define_type_info, &g_define_type_fundamental_info, 0);
	}
	return grava_renderer_type_id;
}


gpointer grava_renderer_ref (gpointer instance) {
	GravaRenderer* self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}


void grava_renderer_unref (gpointer instance) {
	GravaRenderer* self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		GRAVA_RENDERER_GET_CLASS (self)->finalize (self);
		g_type_free_instance ((GTypeInstance *) self);
	}
}


static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if ((array != NULL) && (destroy_func != NULL)) {
		int i;
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) array)[i] != NULL) {
				destroy_func (((gpointer*) array)[i]);
			}
		}
	}
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	_vala_array_destroy (array, array_length, destroy_func);
	g_free (array);
}


static gint _vala_array_length (gpointer array) {
	int length;
	length = 0;
	if (array) {
		while (((gpointer*) array)[length]) {
			length++;
		}
	}
	return length;
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return str1 != str2;
	}
	return strcmp (str1, str2);
}




