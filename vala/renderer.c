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
#include "shape.h"
#include <gobject/gvaluecollector.h>




enum  {
	GRAVA_RENDERER_DUMMY_PROPERTY
};
static gpointer grava_renderer_parent_class = NULL;
static void grava_renderer_finalize (GravaRenderer* obj);
static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);
static int _vala_strcmp0 (const char * str1, const char * str2);



#line 25 "renderer.vala"
void grava_renderer_draw_edge (cairo_t* ctx, GravaEdge* edge) {
	double dx;
	double dy;
	double oh;
#line 25 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 25 "renderer.vala"
	g_return_if_fail (GRAVA_IS_EDGE (edge));
	dx = ((double) (0));
	dy = ((double) (0));
	oh = edge->orig->h;
#line 31 "renderer.vala"
	if (edge->orig->has_body == FALSE) {
#line 32 "renderer.vala"
		oh = ((double) (16));
	}
#line 34 "renderer.vala"
	cairo_save (ctx);
#line 37 "renderer.vala"
	grava_renderer_set_color (ctx, edge->data);
#line 39 "renderer.vala"
	cairo_set_line_width (ctx, ((double) (2)));
	/* oxymoroon! */
#line 41 "renderer.vala"
	if (edge->orig == edge->dest) {
		double ox;
		/* bottom to up */
#line 43 "renderer.vala"
		cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 44 "renderer.vala"
		cairo_move_to (ctx, ((double) (0)), ((double) (0)));
		/*dx = edge.dest.x-edge.orig.x;*/
#line 46 "renderer.vala"
		dx = edge->dest->x - edge->orig->x - (edge->orig->w / 1.3) + edge->dest->w / 1.3;
		/*-edge.orig.x;*/
#line 47 "renderer.vala"
		dy = edge->dest->y - edge->orig->y - oh;
		/* or 80 or so depending if > or < ???*/
		ox = dx;
		/*if (ox == 0){ ox = 150; }
		arrow*/
#line 51 "renderer.vala"
		if (grava_graph_selected == edge->orig) {
#line 52 "renderer.vala"
			cairo_set_line_width (ctx, ((double) (6)));
		}
#line 54 "renderer.vala"
		dx = edge->dest->w;
		/*dy += edge.dest.h/2;*/
#line 56 "renderer.vala"
		cairo_curve_to (ctx, ((double) (0)), ((double) (0)), ((double) (200)), ((double) (100)), dx - edge->orig->w / 2, dy);
	} else {
#line 75 "renderer.vala"
		if (edge->orig->y + oh < (edge->dest->y)) {
			/*-edge.dest.h)) {
			 up to bottom 
			ctx.translate (edge.orig.x+(edge.orig.w/1.3),edge.orig.y+oh);*/
#line 78 "renderer.vala"
			cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 79 "renderer.vala"
			cairo_move_to (ctx, ((double) (0)), ((double) (0)));
#line 80 "renderer.vala"
			dx = edge->dest->x - edge->orig->x - (edge->orig->w / 1.3) + edge->dest->w / 1.3;
			/*-edge.orig.x;*/
#line 81 "renderer.vala"
			dy = edge->dest->y - edge->orig->y - oh;
			/*ctx.move_to(30,30);
			ctx.set_source_rgb (0.0, 0.0, 0.0);*/
#line 84 "renderer.vala"
			if (grava_graph_selected == edge->orig) {
#line 85 "renderer.vala"
				cairo_set_line_width (ctx, ((double) (6)));
			}
#line 86 "renderer.vala"
			grava_renderer_line (ctx, ((double) (0)), ((double) (0)), dx, dy);
		} else {
			double ox;
			/* bottom to up */
#line 89 "renderer.vala"
			cairo_translate (ctx, edge->orig->x + (edge->orig->w / 2), edge->orig->y + oh);
#line 90 "renderer.vala"
			cairo_move_to (ctx, ((double) (0)), ((double) (0)));
			/*dx = edge.dest.x-edge.orig.x;*/
#line 92 "renderer.vala"
			dx = edge->dest->x - edge->orig->x - (edge->orig->w / 1.3) + edge->dest->w / 1.3;
			/*-edge.orig.x;*/
#line 93 "renderer.vala"
			dy = edge->dest->y - edge->orig->y - oh;
			/* or 80 or so depending if > or < ???*/
			ox = dx;
			/*if (ox == 0){ ox = 150; }
			arrow*/
#line 97 "renderer.vala"
			if (grava_graph_selected == edge->orig) {
#line 98 "renderer.vala"
				cairo_set_line_width (ctx, ((double) (6)));
			}
#line 99 "renderer.vala"
			cairo_curve_to (ctx, dx, ((double) (100)), ox, ((double) (((ox > 0) ? 200 : -200))), dx, dy);
		}
	}
	/*ctx.stroke();*/
#line 102 "renderer.vala"
	cairo_stroke (ctx);
#line 103 "renderer.vala"
	cairo_restore (ctx);
#line 104 "renderer.vala"
	cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, ((double) (1)));
}


#line 108 "renderer.vala"
void grava_renderer_set_color (cairo_t* ctx, GHashTable* ht) {
	const char* _tmp0;
	char* color;
#line 108 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 108 "renderer.vala"
	g_return_if_fail (ht != NULL);
#line 110 "renderer.vala"
	_tmp0 = NULL;
	color = (_tmp0 = ((const char*) (g_hash_table_lookup (ht, "color"))), (_tmp0 == NULL ? NULL : g_strdup (_tmp0)));
#line 111 "renderer.vala"
	grava_renderer_set_color_str (ctx, color);
	color = (g_free (color), NULL);
}


#line 114 "renderer.vala"
gboolean grava_renderer_set_color_str (cairo_t* ctx, const char* color) {
#line 114 "renderer.vala"
	g_return_val_if_fail (ctx != NULL, FALSE);
#line 116 "renderer.vala"
	if (color != NULL) {
#line 117 "renderer.vala"
		if (_vala_strcmp0 (color, "black") == 0) {
#line 118 "renderer.vala"
			cairo_set_source_rgba (ctx, 0.0, 0.0, 0.0, 0.7);
		} else {
#line 120 "renderer.vala"
			if (_vala_strcmp0 (color, "white") == 0) {
#line 121 "renderer.vala"
				cairo_set_source_rgba (ctx, 0.9, 0.9, 0.9, 0.8);
			} else {
#line 123 "renderer.vala"
				if (_vala_strcmp0 (color, "green") == 0) {
#line 124 "renderer.vala"
					cairo_set_source_rgba (ctx, 0.1, 0.7, 0.1, 0.8);
				} else {
#line 126 "renderer.vala"
					if (_vala_strcmp0 (color, "red") == 0) {
#line 127 "renderer.vala"
						cairo_set_source_rgba (ctx, 1.0, 0.1, 0.1, 0.9);
					} else {
#line 129 "renderer.vala"
						if (_vala_strcmp0 (color, "turqoise") == 0) {
#line 130 "renderer.vala"
							cairo_set_source_rgba (ctx, 0.6, 0.9, 1.0, 0.8);
						} else {
#line 132 "renderer.vala"
							if (_vala_strcmp0 (color, "blue") == 0) {
#line 133 "renderer.vala"
								cairo_set_source_rgba (ctx, 0.2, 0.5, 0.9, 0.8);
							} else {
#line 135 "renderer.vala"
								if (_vala_strcmp0 (color, "yellow") == 0) {
#line 136 "renderer.vala"
									cairo_set_source_rgba (ctx, 0.9, 0.9, 0.0, 0.6);
								} else {
#line 138 "renderer.vala"
									if (_vala_strcmp0 (color, "gray") == 0) {
#line 139 "renderer.vala"
										cairo_set_source_rgba (ctx, 0.8, 0.8, 0.8, 0.8);
									} else {
#line 141 "renderer.vala"
										if (_vala_strcmp0 (color, "beige") == 0) {
#line 142 "renderer.vala"
											cairo_set_source_rgba (ctx, 0.9, 0.9, 0.6, 0.7);
										} else {
#line 144 "renderer.vala"
											if (_vala_strcmp0 (color, "darkgray") == 0) {
#line 145 "renderer.vala"
												cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, 0.8);
											} else {
#line 147 "renderer.vala"
												cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.4);
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
#line 149 "renderer.vala"
		cairo_set_source_rgba (ctx, 1.0, 1.0, 1.0, 0.8);
	}
#line 151 "renderer.vala"
	return (color != NULL);
}


#line 154 "renderer.vala"
void grava_renderer_draw_node (cairo_t* ctx, GravaNode* node) {
#line 154 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 154 "renderer.vala"
	g_return_if_fail (GRAVA_IS_NODE (node));
#line 156 "renderer.vala"
	cairo_save (ctx);
#line 158 "renderer.vala"
	cairo_set_tolerance (ctx, 0.1);
#line 159 "renderer.vala"
	cairo_set_line_join (ctx, CAIRO_LINE_JOIN_ROUND);
#line 160 "renderer.vala"
	cairo_set_line_width (ctx, ((double) (1)));
#line 161 "renderer.vala"
	cairo_translate (ctx, node->x, node->y);
	/*ctx.set_source_rgb (1, 1, 1);*/
#line 164 "renderer.vala"
	cairo_set_source_rgb (ctx, 0.8, 0.8, 0.8);
	/*#if 0
	if (node.calls.length() >0) 
	set_color(ctx, "red");
	else
	set_color(ctx, "blue");
	#endif
	*/
#line 172 "renderer.vala"
	grava_renderer_set_color (ctx, node->data);
#line 173 "renderer.vala"
	grava_renderer_set_color_str (ctx, ((const char*) (g_hash_table_lookup (node->data, "bgcolor"))));
#line 175 "renderer.vala"
	if (node->has_body) {
		gint _tmp0;
		_tmp0 = node->shape;
		if (_tmp0 == GRAVA_SHAPE_CIRCLE)
		do {
#line 178 "renderer.vala"
			grava_renderer_circle (ctx, node->w, node->h);
#line 179 "renderer.vala"
			cairo_fill (ctx);
#line 180 "renderer.vala"
			break;
		} while (0); else
		do {
#line 183 "renderer.vala"
			grava_renderer_square (ctx, node->w, node->h);
#line 184 "renderer.vala"
			cairo_fill (ctx);
#line 185 "renderer.vala"
			break;
		} while (0);
	}
	/* title rectangle */
#line 190 "renderer.vala"
	if (((const char*) (g_hash_table_lookup (node->data, "color"))) != NULL) {
#line 191 "renderer.vala"
		grava_renderer_set_color_str (ctx, ((const char*) (g_hash_table_lookup (node->data, "color"))));
	} else {
#line 193 "renderer.vala"
		if (g_slist_length (node->calls) == 1) {
#line 194 "renderer.vala"
			cairo_set_source_rgba (ctx, 0.2, 0.2, 0.4, 0.7);
		} else {
#line 196 "renderer.vala"
			if (g_slist_length (node->calls) > 0) {
#line 197 "renderer.vala"
				cairo_set_source_rgba (ctx, 0.3, 0.3, ((double) (1)), 0.7);
			} else {
#line 199 "renderer.vala"
				cairo_set_source_rgba (ctx, 0.8, 0.8, 0.8, 0.8);
			}
		}
	}
#line 200 "renderer.vala"
	grava_renderer_square (ctx, node->w, ((double) (15)));
#line 201 "renderer.vala"
	cairo_fill (ctx);
#line 202 "renderer.vala"
	grava_renderer_line (ctx, ((double) (0)), ((double) (15)), node->w, ((double) (0)));
	/* draw minimize button */
#line 205 "renderer.vala"
	cairo_save (ctx);
	/*ctx.set_source_rgba (0.7, 0.0, 0.0, 1);*/
#line 207 "renderer.vala"
	cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, 0.8);
#line 208 "renderer.vala"
	cairo_translate (ctx, node->w - 16, ((double) (0)));
#line 209 "renderer.vala"
	grava_renderer_square (ctx, ((double) (16)), ((double) (16)));
#line 210 "renderer.vala"
	cairo_fill (ctx);
#line 211 "renderer.vala"
	cairo_restore (ctx);
#line 213 "renderer.vala"
	cairo_select_font_face (ctx, "Sans Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	/*Courier", */
#line 216 "renderer.vala"
	cairo_set_font_size (ctx, ((double) (10)));
	/* set label */
#line 219 "renderer.vala"
	cairo_set_source_rgb (ctx, 0.1, 0.1, 0.1);
#line 220 "renderer.vala"
	cairo_move_to (ctx, ((double) (5)), ((double) (10)));
#line 221 "renderer.vala"
	cairo_show_text (ctx, ((const char*) (g_hash_table_lookup (node->data, "label"))));
	/* set body */
#line 224 "renderer.vala"
	if (node->has_body) {
		gint y;
		const char* _tmp1;
		char* body;
		y = 25;
#line 226 "renderer.vala"
		_tmp1 = NULL;
		body = (_tmp1 = ((const char*) (g_hash_table_lookup (node->data, "body"))), (_tmp1 == NULL ? NULL : g_strdup (_tmp1)));
#line 227 "renderer.vala"
		if (body != NULL) {
			{
				char** str_collection;
				int str_collection_length1;
				char** str_it;
#line 228 "renderer.vala"
				str_collection = g_strsplit (body, "\n", 0);
				str_collection_length1 = -1;
				for (str_it = str_collection; *str_it != NULL; str_it = str_it + 1) {
					const char* _tmp2;
					char* str;
#line 716 "glib-2.0.vapi"
					_tmp2 = NULL;
					str = (_tmp2 = *str_it, (_tmp2 == NULL ? NULL : g_strdup (_tmp2)));
					{
#line 229 "renderer.vala"
						y = y + (10);
#line 230 "renderer.vala"
						cairo_move_to (ctx, ((double) (5)), ((double) (y)));
#line 231 "renderer.vala"
						if ((strstr (str, "call ") != NULL) || (strstr (str, "bl ") != NULL)) {
#line 233 "renderer.vala"
							grava_renderer_set_color_str (ctx, "blue");
						} else {
#line 235 "renderer.vala"
							if (strstr (str, "goto") != NULL) {
#line 236 "renderer.vala"
								grava_renderer_set_color_str (ctx, "green");
							} else {
#line 238 "renderer.vala"
								if (strstr (str, " j") != NULL) {
#line 239 "renderer.vala"
									grava_renderer_set_color_str (ctx, "green");
								} else {
#line 241 "renderer.vala"
									if (g_str_has_suffix (str, ":")) {
#line 242 "renderer.vala"
										grava_renderer_set_color_str (ctx, "red");
									} else {
#line 244 "renderer.vala"
										grava_renderer_set_color_str (ctx, "black");
									}
								}
							}
						}
#line 245 "renderer.vala"
						cairo_show_text (ctx, str);
						str = (g_free (str), NULL);
					}
				}
#line 228 "renderer.vala"
				str_collection = (_vala_array_free (str_collection, str_collection_length1, ((GDestroyNotify) (g_free))), NULL);
			}
		}
		/*set_color(ctx, node.data);
		 box square */
#line 250 "renderer.vala"
		if (grava_graph_selected == node) {
			/*ctx.set_source_rgba (1, 0.8, 0.0, 0.9);*/
#line 252 "renderer.vala"
			cairo_set_source_rgba (ctx, ((double) (0)), 0.0, 0.0, 1.0);
#line 253 "renderer.vala"
			cairo_set_line_width (ctx, ((double) (2)));
		} else {
#line 255 "renderer.vala"
			cairo_set_source_rgba (ctx, 0.2, 0.2, 0.2, 0.4);
#line 256 "renderer.vala"
			cairo_set_line_width (ctx, ((double) (1)));
		}
#line 259 "renderer.vala"
		if (node->shape == GRAVA_SHAPE_CIRCLE) {
#line 260 "renderer.vala"
			grava_renderer_circle (ctx, node->w, node->h);
		} else {
#line 261 "renderer.vala"
			grava_renderer_square (ctx, node->w, node->h);
		}
		body = (g_free (body), NULL);
	}
#line 264 "renderer.vala"
	cairo_stroke (ctx);
#line 266 "renderer.vala"
	cairo_restore (ctx);
}


#line 269 "renderer.vala"
void grava_renderer_circle (cairo_t* ctx, double w, double h) {
#line 269 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 270 "renderer.vala"
	cairo_save (ctx);
#line 271 "renderer.vala"
	cairo_scale (ctx, ((double) (1)), h / w);
#line 272 "renderer.vala"
	cairo_move_to (ctx, w, h / 2.5);
#line 273 "renderer.vala"
	cairo_arc (ctx, w / 2, h / 2.5, (((w < h) ? h : w)) * 0.7, ((double) (0)), 2 * G_PI);
#line 274 "renderer.vala"
	cairo_restore (ctx);
	/*ctx.arc(100,250, 50, 0, 2*Math.PI); //w/2, h/2.5, ((w<h)?h:w)*0.7, 0, 2*Math.PI);
	
	ctx.rel_line_to (w, 0);
	ctx.rel_line_to (0, h);
	ctx.rel_line_to (-w, 0);
	*/
#line 281 "renderer.vala"
	cairo_close_path (ctx);
}


#line 284 "renderer.vala"
void grava_renderer_square (cairo_t* ctx, double w, double h) {
#line 284 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 285 "renderer.vala"
	cairo_move_to (ctx, ((double) (0)), ((double) (0)));
#line 286 "renderer.vala"
	cairo_rel_line_to (ctx, w, ((double) (0)));
#line 287 "renderer.vala"
	cairo_rel_line_to (ctx, ((double) (0)), h);
#line 288 "renderer.vala"
	cairo_rel_line_to (ctx, -w, ((double) (0)));
#line 289 "renderer.vala"
	cairo_close_path (ctx);
}


#line 293 "renderer.vala"
void grava_renderer_line (cairo_t* ctx, double x, double y, double w, double h) {
#line 293 "renderer.vala"
	g_return_if_fail (ctx != NULL);
#line 294 "renderer.vala"
	cairo_move_to (ctx, x, y);
#line 295 "renderer.vala"
	cairo_rel_line_to (ctx, w, h);
#line 296 "renderer.vala"
	cairo_stroke (ctx);
}


#line 23 "renderer.vala"
GravaRenderer* grava_renderer_construct (GType object_type) {
	GravaRenderer* self;
	self = ((GravaRenderer*) (g_type_create_instance (object_type)));
	return self;
}


#line 23 "renderer.vala"
GravaRenderer* grava_renderer_new (void) {
#line 23 "renderer.vala"
	return grava_renderer_construct (GRAVA_TYPE_RENDERER);
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
		object = value->data[0].v_pointer;
		if (object->parent_instance.g_class == NULL) {
			return g_strconcat ("invalid unclassed object pointer for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		} else if (!g_value_type_compatible (G_OBJECT_TYPE (object), G_VALUE_TYPE (value))) {
			return g_strconcat ("invalid object type `", G_OBJECT_TYPE (object), "' for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		}
	} else {
		value->data[0].v_pointer = NULL;
	}
	return NULL;
}


static gchar* grava_value_renderer_lcopy_value (const GValue* value, guint n_collect_values, GTypeCValue* collect_values, guint collect_flags) {
	GravaRenderer** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
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
		g_type_free_instance (((GTypeInstance *) (self)));
	}
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if (array != NULL && destroy_func != NULL) {
		int i;
		if (array_length >= 0)
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) (array))[i] != NULL)
			destroy_func (((gpointer*) (array))[i]);
		}
		else
		for (i = 0; ((gpointer*) (array))[i] != NULL; i = i + 1) {
			destroy_func (((gpointer*) (array))[i]);
		}
	}
	g_free (array);
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return (str1 != str2);
	}
	return strcmp (str1, str2);
}




