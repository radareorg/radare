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

#include "renderer.h"
#include "graph.h"

static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);


#line 24 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge) {
	double dx;
	double dy;
#line 24 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	g_return_if_fail (edge == NULL || GRAVA_IS_EDGE (edge));
#line 26 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	dx = 0;
#line 27 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	dy = 0;
#line 29 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_save (ctx);
#line 30 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_set_color (ctx, edge->data);
#line 32 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_line_width (ctx, 3);
#line 33 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	if (edge->orig->y + edge->orig->h < (edge->dest->y)) {
		/*-edge.dest.h)) {
		 up to bottom */
#line 35 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + edge->orig->h);
#line 36 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		dx = edge->dest->x - edge->orig->x;
		/* + edge.orig.w/2; //-edge.orig.x;*/
#line 37 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		dy = edge->dest->y - edge->orig->y - edge->orig->h;
#line 38 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_move_to (ctx, 30, 30);
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		grava_renderer_line (ctx, 0, 0, dx, dy);
	} else {
		double ox;
		/* bottom to up */
#line 42 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + edge->orig->h);
#line 43 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_move_to (ctx, 0, 0);
#line 44 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		dx = edge->dest->x - edge->orig->x;
#line 45 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		dy = edge->dest->y - edge->orig->y - edge->orig->h;
		/* or 80 or so depending if > or < ???*/
#line 46 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		ox = dx;
#line 47 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		if (ox == 0) {
#line 47 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
			ox = 150;
		}
#line 48 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_curve_to (ctx, 0, 100, ox, 120, dx, dy);
#line 49 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_stroke (ctx);
	}
#line 52 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_source_rgb (ctx, 0.0, 0.0, 0.0);
#line 53 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_rectangle (ctx, dx, dy, 2, 2);
#line 54 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_stroke (ctx);
#line 55 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_restore (ctx);
#line 56 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_source_rgb (ctx, 0.6, 0.6, 0.6);
}


#line 59 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
void grava_renderer_set_color (cairo_t* ctx, GHashTable* ht) {
	const char* color;
#line 61 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	color = g_hash_table_lookup (ht, "color");
#line 62 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_set_color_str (ctx, color);
}


#line 65 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
void grava_renderer_set_color_str (cairo_t* ctx, const char* color) {
#line 67 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	if (color != NULL) {
#line 68 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		if (g_utf8_collate (color, "black") == 0) {
#line 69 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
			cairo_set_source_rgba (ctx, 0.0, 0.0, 0.0, 0.7);
		} else {
#line 71 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
			if (g_utf8_collate (color, "white") == 0) {
#line 72 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
				cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.4);
			} else {
#line 74 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
				if (g_utf8_collate (color, "green") == 0) {
#line 75 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
					cairo_set_source_rgba (ctx, 0.1, 0.7, 0.1, 0.8);
				} else {
#line 77 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
					if (g_utf8_collate (color, "red") == 0) {
#line 78 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
						cairo_set_source_rgba (ctx, 1.0, 0.1, 0.1, 0.6);
					} else {
#line 80 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
						if (g_utf8_collate (color, "blue") == 0) {
#line 81 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
							cairo_set_source_rgba (ctx, 0.2, 0.2, 1.0, 0.9);
						} else {
#line 83 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
							if (g_utf8_collate (color, "yellow") == 0) {
#line 84 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
								cairo_set_source_rgba (ctx, 0.7, 0.7, 0.0, 0.8);
							} else {
#line 86 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
								cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.4);
							}
						}
					}
				}
			}
		}
	} else {
#line 88 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.4);
	}
}


#line 91 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node) {
	gint y;
	const char* _tmp0;
	char* body;
#line 91 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	g_return_if_fail (node == NULL || GRAVA_IS_NODE (node));
#line 93 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_save (ctx);
#line 97 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_tolerance (ctx, 0.1);
#line 98 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_line_join (ctx, CAIRO_LINE_JOIN_ROUND);
#line 99 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_line_width (ctx, 1);
#line 100 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_translate (ctx, node->x, node->y);
	/*ctx.set_source_rgba (1, 1, 1, 0.7);*/
#line 103 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_set_color (ctx, node->data);
#line 104 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_square (ctx, node->w, node->h);
#line 105 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_fill (ctx);
	/* title rectangle */
#line 108 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_source_rgb (ctx, 0.8, 0.8, 0.8);
#line 109 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_square (ctx, node->w, 15);
#line 110 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_fill (ctx);
#line 111 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_line (ctx, 0, 15, node->w, 0);
#line 113 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_select_font_face (ctx, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
#line 116 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_font_size (ctx, 10);
	/* set label */
#line 119 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_set_source_rgb (ctx, 0.1, 0.1, 0.1);
#line 120 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_move_to (ctx, 5, 10);
#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_show_text (ctx, g_hash_table_lookup (node->data, "label"));
	/* set body */
#line 124 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	y = 25;
#line 125 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	_tmp0 = NULL;
#line 125 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	body = (_tmp0 = g_hash_table_lookup (node->data, "body"), (_tmp0 == NULL ? NULL : g_strdup (_tmp0)));
#line 126 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	if (body != NULL) {
		{
			char** str_collection;
			char** str_it;
			int str_collection_length1;
#line 127 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
			str_collection = g_strsplit (body, "\n", 0);
			str_collection_length1 = -1;
			for (str_it = str_collection; *str_it != NULL; str_it = str_it + 1) {
				const char* _tmp1;
				char* str;
				_tmp1 = NULL;
				str = (_tmp1 = *str_it, (_tmp1 == NULL ? NULL : g_strdup (_tmp1)));
				{
#line 128 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
					cairo_move_to (ctx, 5, y = y + (10));
#line 129 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
					if ((strstr (str, "call ") != NULL) || (strstr (str, "bl ") != NULL)) {
#line 131 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
						grava_renderer_set_color_str (ctx, "blue");
					} else {
#line 133 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
						if (strstr (str, "goto") != NULL) {
#line 134 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
							grava_renderer_set_color_str (ctx, "green");
						} else {
#line 136 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
							if (strstr (str, " j") != NULL) {
#line 137 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
								grava_renderer_set_color_str (ctx, "green");
							} else {
#line 139 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
								if (g_str_has_suffix (str, ":")) {
#line 140 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
									grava_renderer_set_color_str (ctx, "red");
								} else {
#line 142 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
									grava_renderer_set_color_str (ctx, "black");
								}
							}
						}
					}
#line 143 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
					cairo_show_text (ctx, str);
					(str = (g_free (str), NULL));
				}
			}
#line 127 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
			(str_collection = (_vala_array_free (str_collection, str_collection_length1, ((GDestroyNotify) g_free)), NULL));
		}
	}
	/*set_color(ctx, node.data);
	 box square */
#line 148 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	if (grava_graph_selected == node) {
#line 149 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_set_source_rgba (ctx, 0.9, 0.1, 0.1, 0.7);
#line 150 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_set_line_width (ctx, 5);
	} else {
#line 152 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_set_source_rgba (ctx, 0.2, 0.2, 0.2, 0.4);
#line 153 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
		cairo_set_line_width (ctx, 1);
	}
#line 155 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	grava_renderer_square (ctx, node->w, node->h);
#line 156 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_stroke (ctx);
#line 158 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_restore (ctx);
	(body = (g_free (body), NULL));
}


#line 161 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
void grava_renderer_square (cairo_t* ctx, double w, double h) {
#line 162 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_move_to (ctx, 0, 0);
#line 163 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_rel_line_to (ctx, w, 0);
#line 164 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_rel_line_to (ctx, 0, h);
#line 165 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_rel_line_to (ctx, -w, 0);
#line 166 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_close_path (ctx);
}


#line 169 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
void grava_renderer_line (cairo_t* ctx, double x, double y, double w, double h) {
#line 170 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_move_to (ctx, x, y);
#line 171 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_rel_line_to (ctx, w, h);
#line 172 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
	cairo_stroke (ctx);
}


#line 22 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/renderer.vala"
GravaRenderer* grava_renderer_new (void) {
	GravaRenderer* self;
	self = g_slice_new0 (GravaRenderer);
	return self;
}


void grava_renderer_free (GravaRenderer* self) {
	g_slice_free (GravaRenderer, self);
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if (array != NULL && destroy_func != NULL) {
		int i;
		if (array_length >= 0)
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) array)[i] != NULL)
			destroy_func (((gpointer*) array)[i]);
		}
		else
		for (i = 0; ((gpointer*) array)[i] != NULL; i = i + 1) {
			destroy_func (((gpointer*) array)[i]);
		}
	}
	g_free (array);
}




