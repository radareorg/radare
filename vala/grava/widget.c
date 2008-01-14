/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007, 2008  pancake <youterm.com>
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

#include "widget.h"
#include <float.h>
#include <math.h>
#include <gdk/gdk.h>
#include <stdio.h>
#include <cairo.h>
#include "node.h"

struct _GravaWidgetPrivate {
	GtkScrolledWindow* sw;
	GtkMenu* menu;
	double opanx;
	double opany;
};
#define GRAVA_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVA_TYPE_WIDGET, GravaWidgetPrivate))
enum  {
	GRAVA_WIDGET_DUMMY_PROPERTY
};
static const gint GRAVA_WIDGET_SIZE = 30;
static const double GRAVA_WIDGET_ZOOM_FACTOR = 0.1;
static void __lambda0 (GravaWidget* obj, const char* addr, GravaWidget* self);
static gboolean grava_widget_scroll_press (GtkDrawingArea* da, GdkEvent* event, GravaWidget* self);
static gboolean grava_widget_key_press (GtkScrolledWindow* sw, GdkEvent* ev, GravaWidget* self);
static void __lambda1 (GtkMenuItem* imi, GravaWidget* self);
static void __lambda2 (GtkMenuItem* imi, GravaWidget* self);
static void __lambda3 (GtkMenuItem* imi, GravaWidget* self);
static void __lambda4 (GtkMenuItem* imi, GravaWidget* self);
static void __lambda5 (GtkMenuItem* imi, GravaWidget* self);
static gboolean grava_widget_button_press (GtkDrawingArea* da, GdkEvent* event, GravaWidget* self);
static gboolean grava_widget_motion (GtkDrawingArea* da, GdkEvent* ev, GravaWidget* self);
static gboolean grava_widget_expose (GtkDrawingArea* da, GdkEvent* ev, GravaWidget* self);
static GObject * grava_widget_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_widget_parent_class = NULL;
static void grava_widget_dispose (GObject * obj);


/*public signal void focus_at(string addr);*/
#line 37 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
GtkWidget* grava_widget_get_widget (GravaWidget* self) {
	GtkScrolledWindow* _tmp0;
#line 37 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (GRAVA_IS_WIDGET (self), NULL);
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp0 = NULL;
#line 39 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	return GTK_WIDGET ((_tmp0 = self->priv->sw, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
}


static void __lambda0 (GravaWidget* obj, const char* addr, GravaWidget* self) {
	char* _tmp0;
	g_return_if_fail (obj == NULL || GRAVA_IS_WIDGET (obj));
	_tmp0 = NULL;
#line 128 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	fprintf (stdout, (_tmp0 = g_strconcat ("HOWHOWHOW ", addr, NULL)));
	(_tmp0 = (g_free (_tmp0), NULL));
}


#line 93 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
void grava_widget_create_widgets (GravaWidget* self) {
	GtkDrawingArea* _tmp0;
	GtkScrolledWindow* _tmp3;
	GtkAdjustment* _tmp2;
	GtkAdjustment* _tmp1;
	GtkAdjustment* _tmp5;
	GtkAdjustment* _tmp4;
	GtkViewport* _tmp6;
	GtkViewport* vp;
#line 93 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_if_fail (GRAVA_IS_WIDGET (self));
	_tmp0 = NULL;
#line 95 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	self->da = (_tmp0 = g_object_ref_sink (gtk_drawing_area_new ()), (self->da == NULL ? NULL : (self->da = (g_object_unref (self->da), NULL))), _tmp0);
	/* add event listeners */
#line 98 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_widget_add_events (GTK_WIDGET (self->da), GDK_BUTTON1_MOTION_MASK | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	/*da.set_events(  Gdk.EventMask.BUTTON1_MOTION_MASK );
	 Gdk.EventMask.POINTER_MOTION_MASK );*/
#line 105 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self->da, "expose-event", ((GCallback) grava_widget_expose), self, 0);
#line 106 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self->da, "motion-notify-event", ((GCallback) grava_widget_motion), self, 0);
#line 107 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self->da, "button-release-event", ((GCallback) grava_widget_motion), self, 0);
#line 108 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self->da, "button-press-event", ((GCallback) grava_widget_button_press), self, 0);
	/*da.key_press_event += key_press;
	da.key_release_event += key_press;*/
#line 111 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self->da, "scroll-event", ((GCallback) grava_widget_scroll_press), self, 0);
	_tmp3 = NULL;
#line 115 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp2 = NULL;
#line 114 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp1 = NULL;
#line 113 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	self->priv->sw = (_tmp3 = g_object_ref_sink (gtk_scrolled_window_new ((_tmp1 = g_object_ref_sink (gtk_adjustment_new (0, 10, 1000, 2, 100, 1000))), (_tmp2 = g_object_ref_sink (gtk_adjustment_new (0, 10, 1000, 2, 100, 1000))))), (self->priv->sw == NULL ? NULL : (self->priv->sw = (g_object_unref (self->priv->sw), NULL))), _tmp3);
	(_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref (_tmp2), NULL)));
	(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
#line 116 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_scrolled_window_set_policy (self->priv->sw, GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
#line 120 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp5 = NULL;
#line 119 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp4 = NULL;
#line 118 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp6 = NULL;
#line 118 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	vp = (_tmp6 = g_object_ref_sink (gtk_viewport_new ((_tmp4 = g_object_ref_sink (gtk_adjustment_new (0, 10, 1000, 2, 100, 1000))), (_tmp5 = g_object_ref_sink (gtk_adjustment_new (0, 10, 1000, 2, 100, 1000))))), (_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref (_tmp5), NULL))), (_tmp4 == NULL ? NULL : (_tmp4 = (g_object_unref (_tmp4), NULL))), _tmp6);
#line 121 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_container_add (GTK_CONTAINER (vp), GTK_WIDGET (self->da));
#line 122 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_widget_add_events (GTK_WIDGET (self->priv->sw), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
#line 123 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self->priv->sw, "key-press-event", ((GCallback) grava_widget_key_press), self, 0);
#line 125 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_scrolled_window_add_with_viewport (self->priv->sw, GTK_WIDGET (vp));
#line 127 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (self, "load-graph-at", ((GCallback) __lambda0), self, 0);
	(vp == NULL ? NULL : (vp = (g_object_unref (vp), NULL)));
}


/* capture mouse motion */
#line 133 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
static gboolean grava_widget_scroll_press (GtkDrawingArea* da, GdkEvent* event, GravaWidget* self) {
	GdkEventScroll* es;
	GdkScrollDirection _tmp0;
#line 133 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (GRAVA_IS_WIDGET (self), FALSE);
#line 133 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (da == NULL || GTK_IS_DRAWING_AREA (da), FALSE);
#line 135 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	es = &event->scroll;
	_tmp0 = es->direction;
	if (_tmp0 == GDK_SCROLL_UP)
	do {
#line 139 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->zoom = self->graph->zoom + (GRAVA_WIDGET_ZOOM_FACTOR);
#line 141 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == GDK_SCROLL_DOWN)
	do {
#line 143 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->zoom = self->graph->zoom - (GRAVA_WIDGET_ZOOM_FACTOR);
#line 145 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0);
	/*graph.angle+=0.02;*/
#line 148 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_widget_queue_draw_area (GTK_WIDGET (da), 0, 0, 5000, 2000);
}


#line 151 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
static gboolean grava_widget_key_press (GtkScrolledWindow* sw, GdkEvent* ev, GravaWidget* self) {
	gboolean handled;
	GdkEventKey* ek;
	guint _tmp0;
#line 151 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (GRAVA_IS_WIDGET (self), FALSE);
#line 151 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (sw == NULL || GTK_IS_SCROLLED_WINDOW (sw), FALSE);
#line 153 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	handled = TRUE;
#line 154 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	ek = &ev->key;
	/* 
	stdout.printf("Key pressed %d (%c)\n", ek.keyval,ek.keyval);*/
	_tmp0 = ek->keyval;
	if (_tmp0 == 46)
	do {
#line 160 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected != NULL) {
			/*sw.get_size(ref w, ref, h);*/
#line 162 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			self->graph->panx = -grava_graph_selected->x + 350;
#line 163 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			self->graph->pany = -grava_graph_selected->y + 350;
		}
#line 165 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 65056)
	do {
#line 168 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 65289)
	do {
#line 170 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		grava_graph_select_next (self->graph);
#line 171 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected == NULL) {
#line 172 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			grava_graph_select_next (self->graph);
		}
#line 173 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected != NULL) {
			/*sw.get_size(ref w, ref, h);
			 XXXX get window size*/
#line 176 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			self->graph->panx = -grava_graph_selected->x + 350;
#line 177 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			self->graph->pany = -grava_graph_selected->y + 350;
		}
#line 179 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 65361 || _tmp0 == 'h')
	do {
#line 182 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->panx = self->graph->panx + (GRAVA_WIDGET_S * self->graph->zoom);
#line 183 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 65364 || _tmp0 == 'j')
	do {
#line 186 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->pany = self->graph->pany - (GRAVA_WIDGET_S * self->graph->zoom);
#line 187 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 65362 || _tmp0 == 'k')
	do {
#line 190 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->pany = self->graph->pany + (GRAVA_WIDGET_S * self->graph->zoom);
#line 191 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 65363 || _tmp0 == 'l')
	do {
#line 194 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->panx = self->graph->panx - (GRAVA_WIDGET_S * self->graph->zoom);
#line 195 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 'u')
	do {
#line 197 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->pany = self->graph->pany + (GRAVA_WIDGET_S * 2);
#line 198 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == ' ')
	do {
#line 200 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->pany = self->graph->pany - (GRAVA_WIDGET_S * 2);
#line 201 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 'H')
	do {
#line 203 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->panx = self->graph->panx + (GRAVA_WIDGET_S * self->graph->zoom);
#line 204 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected != NULL) {
#line 205 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			grava_graph_selected->x = grava_graph_selected->x - (GRAVA_WIDGET_S * self->graph->zoom);
		}
#line 206 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 'J')
	do {
#line 208 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->pany = self->graph->pany - (GRAVA_WIDGET_S * self->graph->zoom);
#line 209 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected != NULL) {
#line 210 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			grava_graph_selected->y = grava_graph_selected->y + (GRAVA_WIDGET_S * self->graph->zoom);
		}
#line 211 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 'K')
	do {
#line 213 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->pany = self->graph->pany + (GRAVA_WIDGET_S * self->graph->zoom);
#line 214 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected != NULL) {
#line 215 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			grava_graph_selected->y = grava_graph_selected->y - (GRAVA_WIDGET_S * self->graph->zoom);
		}
#line 216 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == 'L')
	do {
#line 218 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->panx = self->graph->panx - (GRAVA_WIDGET_S * self->graph->zoom);
#line 219 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (grava_graph_selected != NULL) {
#line 220 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			grava_graph_selected->x = grava_graph_selected->x + (GRAVA_WIDGET_S * self->graph->zoom);
		}
#line 221 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == '+')
	do {
#line 223 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->zoom = self->graph->zoom + (GRAVA_WIDGET_ZOOM_FACTOR);
#line 224 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else if (_tmp0 == '-')
	do {
#line 226 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->zoom = self->graph->zoom - (GRAVA_WIDGET_ZOOM_FACTOR);
#line 227 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0); else
	do {
#line 229 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		handled = FALSE;
#line 230 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		break;
	} while (0);
	/*expose(da, ev);*/
#line 234 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_widget_queue_draw_area (GTK_WIDGET (self->da), 0, 0, 5000, 2000);
#line 236 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	return TRUE;
}


static void __lambda1 (GtkMenuItem* imi, GravaWidget* self) {
	char* _tmp0;
	g_return_if_fail (imi == NULL || GTK_IS_MENU_ITEM (imi));
#line 247 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	fprintf (stdout, "Focus!\n");
#line 248 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	_tmp0 = NULL;
#line 248 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	core_load_graph_at_label ((_tmp0 = grava_node_get (grava_graph_selected, "label")));
	(_tmp0 = (g_free (_tmp0), NULL));
}


static void __lambda2 (GtkMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi == NULL || GTK_IS_MENU_ITEM (imi));
#line 257 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	fprintf (stdout, "Focus!\n");
}


static void __lambda3 (GtkMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi == NULL || GTK_IS_MENU_ITEM (imi));
#line 263 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	fprintf (stdout, "Focus!\n");
}


static void __lambda4 (GtkMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi == NULL || GTK_IS_MENU_ITEM (imi));
#line 269 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	fprintf (stdout, "Focus!\n");
}


static void __lambda5 (GtkMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi == NULL || GTK_IS_MENU_ITEM (imi));
	/*stdout.printf("FUCKME: \n"+imi.submenu_placement());*/
#line 280 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_emit_by_name (G_OBJECT (self), "load-graph-at", gtk_label_get_text ((GTK_LABEL (GTK_BIN (imi)->child))));
}


#line 239 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
void grava_widget_do_popup_menu (GravaWidget* self) {
	GtkImageMenuItem* imi;
	GtkMenu* _tmp0;
	GtkImageMenuItem* _tmp1;
	GtkImageMenuItem* _tmp2;
	GtkImageMenuItem* _tmp3;
	GtkImageMenuItem* _tmp4;
#line 239 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_if_fail (GRAVA_IS_WIDGET (self));
#line 241 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	imi = NULL;
	_tmp0 = NULL;
#line 242 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	self->priv->menu = (_tmp0 = g_object_ref_sink (gtk_menu_new ()), (self->priv->menu == NULL ? NULL : (self->priv->menu = (g_object_unref (self->priv->menu), NULL))), _tmp0);
	/*imi = new ImageMenuItem.with_label("Focus");*/
	_tmp1 = NULL;
#line 245 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	imi = (_tmp1 = g_object_ref_sink (gtk_image_menu_item_new_from_stock ("gtk-zoom-in", NULL)), (imi == NULL ? NULL : (imi = (g_object_unref (imi), NULL))), _tmp1);
#line 246 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (imi, "activate", ((GCallback) __lambda1), self, 0);
	/*MenuItem mi = menu.get_active();
	load_graph_at(((Label)imi.child).get_text()); //"0x400");
	stdout.printf(" cocococo "+ menu.);*/
#line 253 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->menu), GTK_WIDGET (imi));
	_tmp2 = NULL;
#line 255 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	imi = (_tmp2 = g_object_ref_sink (gtk_image_menu_item_new_with_label ("Breakpoint here")), (imi == NULL ? NULL : (imi = (g_object_unref (imi), NULL))), _tmp2);
#line 256 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (imi, "activate", ((GCallback) __lambda2), self, 0);
#line 259 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->menu), GTK_WIDGET (imi));
	_tmp3 = NULL;
#line 261 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	imi = (_tmp3 = g_object_ref_sink (gtk_image_menu_item_new_with_label ("Remove true branch")), (imi == NULL ? NULL : (imi = (g_object_unref (imi), NULL))), _tmp3);
#line 262 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (imi, "activate", ((GCallback) __lambda3), self, 0);
#line 265 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->menu), GTK_WIDGET (imi));
	_tmp4 = NULL;
#line 267 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	imi = (_tmp4 = g_object_ref_sink (gtk_image_menu_item_new_with_label ("Remove false branch")), (imi == NULL ? NULL : (imi = (g_object_unref (imi), NULL))), _tmp4);
#line 268 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_signal_connect_object (imi, "activate", ((GCallback) __lambda4), self, 0);
#line 271 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->menu), GTK_WIDGET (imi));
#line 273 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	if (grava_graph_selected != NULL) {
		GtkSeparatorMenuItem* _tmp5;
#line 274 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		_tmp5 = NULL;
#line 274 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->menu), GTK_WIDGET ((_tmp5 = g_object_ref_sink (gtk_separator_menu_item_new ()))));
		(_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref (_tmp5), NULL)));
		{
			GSList* str_collection;
			GSList* str_it;
#line 276 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			str_collection = grava_graph_selected->calls;
			for (str_it = str_collection; str_it != NULL; str_it = str_it->next) {
				char* str;
#line 276 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
				str = str_it->data;
				{
					GtkImageMenuItem* _tmp6;
					_tmp6 = NULL;
#line 277 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
					imi = (_tmp6 = g_object_ref_sink (gtk_image_menu_item_new_with_label (str)), (imi == NULL ? NULL : (imi = (g_object_unref (imi), NULL))), _tmp6);
#line 278 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
					g_signal_connect_object (imi, "activate", ((GCallback) __lambda5), self, 0);
					/*"0x400");*/
#line 282 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
					gtk_menu_shell_append (GTK_MENU_SHELL (self->priv->menu), GTK_WIDGET (imi));
				}
			}
		}
	}
#line 286 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_widget_show_all (GTK_WIDGET (self->priv->menu));
	/*menu.popup(null, null, null, null, eb.button, 0);*/
#line 288 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	gtk_menu_popup (self->priv->menu, NULL, NULL, ((GtkMenuPositionFunc) NULL), NULL, 0, 0);
	(imi == NULL ? NULL : (imi = (g_object_unref (imi), NULL)));
}


#line 291 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
static gboolean grava_widget_button_press (GtkDrawingArea* da, GdkEvent* event, GravaWidget* self) {
	GdkEventButton* eb;
	GdkEventMotion* em;
	GravaNode* n;
#line 291 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (GRAVA_IS_WIDGET (self), FALSE);
#line 291 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (da == NULL || GTK_IS_DRAWING_AREA (da), FALSE);
#line 293 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	eb = &event->button;
#line 294 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	em = &event->motion;
#line 295 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	n = grava_graph_click (self->graph, em->x - self->graph->panx, em->y - self->graph->pany);
#line 297 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	grava_graph_selected = n;
#line 298 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	if (n != NULL) {
		/*graph.draw(Gdk.cairo_create(da.window));*/
#line 300 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		gtk_widget_queue_draw_area (GTK_WIDGET (da), 0, 0, 1024, 768);
#line 302 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if (eb->button == 3) {
#line 303 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			grava_widget_do_popup_menu (self);
		}
	}
#line 305 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	return TRUE;
}


#line 312 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
static gboolean grava_widget_motion (GtkDrawingArea* da, GdkEvent* ev, GravaWidget* self) {
	GdkEventMotion* em;
	GravaNode* n;
#line 312 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (GRAVA_IS_WIDGET (self), FALSE);
#line 312 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (da == NULL || GTK_IS_DRAWING_AREA (da), FALSE);
#line 314 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	em = &ev->motion;
#line 316 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	if (ev->type == GDK_BUTTON_RELEASE) {
#line 317 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->priv->opanx = self->priv->opany = 0;
#line 318 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		return TRUE;
	}
#line 321 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	n = grava_graph_click (self->graph, em->x - self->graph->panx, em->y - self->graph->pany);
#line 322 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	if (n != NULL) {
		cairo_t* ctx;
		/* zoom view */
#line 324 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		em->x = em->x - (self->graph->panx);
#line 325 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		em->y = em->y - (self->graph->pany);
#line 326 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		em->x = em->x / (self->graph->zoom);
#line 327 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		em->y = em->y / (self->graph->zoom);
#line 328 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		n->x = n->x + ((em->x - n->x) - (n->w / 2));
#line 329 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		n->y = em->y - (n->h / 2);
#line 330 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		ctx = gdk_cairo_create (GDK_DRAWABLE (GTK_WIDGET (da)->window));
		/*Renderer.draw_node ( ctx , n);
		/graph.draw(ctx);
		da.queue_draw_area(0,0,1024,768);*/
#line 334 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		gtk_widget_queue_draw_area (GTK_WIDGET (da), 0, 0, 5000, 3000);
#line 335 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		grava_graph_selected = n;
		(ctx == NULL ? NULL : (ctx = (cairo_destroy (ctx), NULL)));
	} else {
		/* pan view */
#line 338 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		if ((self->priv->opanx != 0) && (self->priv->opany != 0)) {
			double x;
			double y;
#line 339 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			x = em->x - self->priv->opanx;
#line 340 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			y = em->y - self->priv->opany;
#line 341 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			self->graph->panx = self->graph->panx + (x * 0.9);
#line 342 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			self->graph->pany = self->graph->pany + (y * 0.9);
			/*graph.draw(Gdk.cairo_create(da.window));*/
#line 344 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
			gtk_widget_queue_draw_area (GTK_WIDGET (da), 0, 0, 5000, 3000);
		}
#line 346 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		grava_graph_selected = NULL;
#line 347 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->priv->opanx = em->x;
#line 348 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->priv->opany = em->y;
	}
#line 350 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	return TRUE;
}


#line 353 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
static gboolean grava_widget_expose (GtkDrawingArea* da, GdkEvent* ev, GravaWidget* self) {
	cairo_t* ctx;
	gboolean _tmp0;
#line 353 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (GRAVA_IS_WIDGET (self), FALSE);
#line 353 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	g_return_val_if_fail (da == NULL || GTK_IS_DRAWING_AREA (da), FALSE);
#line 355 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	ctx = gdk_cairo_create (GDK_DRAWABLE (GTK_WIDGET (da)->window));
#line 356 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	if (self->graph->zoom < 0.2) {
#line 357 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph->zoom = 0.2;
	}
#line 359 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	grava_graph_draw (self->graph, ctx);
#line 362 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
	return (_tmp0 = TRUE, (ctx == NULL ? NULL : (ctx = (cairo_destroy (ctx), NULL))), _tmp0);
}


#line 25 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
GravaWidget* grava_widget_new (void) {
	GravaWidget * self;
	self = g_object_newv (GRAVA_TYPE_WIDGET, 0, NULL);
	return self;
}


static GObject * grava_widget_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GravaWidgetClass * klass;
	GObjectClass * parent_class;
	GravaWidget * self;
	klass = GRAVA_WIDGET_CLASS (g_type_class_peek (GRAVA_TYPE_WIDGET));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GRAVA_WIDGET (obj);
	{
		GravaGraph* _tmp0;
		/* initialize graph */
		_tmp0 = NULL;
#line 45 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		self->graph = (_tmp0 = grava_graph_new (), (self->graph == NULL ? NULL : (self->graph = (g_object_unref (self->graph), NULL))), _tmp0);
		/* example usage 
		 nodes
		Node n = new Node();
		n.set("label", "0x8048490  _start:");
		n.set("color", "gray");
		n.set("body",
		"0x08048490  mov eax, 0x3842\n"+
		"0x08048495  xor ebx, ebx\n"+
		"0x08048497  jz 0x08048900");
		graph.add_node(n);
		
		Node n2 = new Node();
		n2.set("label", "0x08048900  _sub_start:");
		n2.set("color","gray");
		n2.set("body",
		"0x08048900  jmp 0x804890");
		graph.add_node(n2);
		
		Node n3 = new Node();
		n3.set("label", "0x0804849b  --");
		n3.set("color","gray");
		n3.set("body",
		"0x0804849b  rdtsc\n"+
		"0x0804849c  nop\n"+
		"            ...\n");
		graph.add_node(n3);
		
		// edges
		Edge e = new Edge().with(n, n2);
		e.set("color", "green");
		graph.add_edge(e);
		
		e = new Edge().with(n, n3);
		e.set("color", "red");
		graph.add_edge(e);
		
		e = new Edge().with(n2, n);
		e.set("color", "blue");
		graph.add_edge(e);
		 */
#line 88 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		grava_graph_update (self->graph);
#line 90 "/home/pancake/prg/pvcroot/radare/vala/grava/home/pancake/prg/pvcroot/radare/vala/grava/widget.vala"
		grava_widget_create_widgets (self);
	}
	return obj;
}


static void grava_widget_class_init (GravaWidgetClass * klass) {
	grava_widget_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (GravaWidgetPrivate));
	G_OBJECT_CLASS (klass)->constructor = grava_widget_constructor;
	G_OBJECT_CLASS (klass)->dispose = grava_widget_dispose;
	g_signal_new ("load_graph_at", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
}


static void grava_widget_init (GravaWidget * self) {
	self->priv = GRAVA_WIDGET_GET_PRIVATE (self);
	self->priv->opanx = 0;
	self->priv->opany = 0;
}


static void grava_widget_dispose (GObject * obj) {
	GravaWidget * self;
	self = GRAVA_WIDGET (obj);
	(self->da == NULL ? NULL : (self->da = (g_object_unref (self->da), NULL)));
	(self->graph == NULL ? NULL : (self->graph = (g_object_unref (self->graph), NULL)));
	(self->priv->sw == NULL ? NULL : (self->priv->sw = (g_object_unref (self->priv->sw), NULL)));
	(self->priv->menu == NULL ? NULL : (self->priv->menu = (g_object_unref (self->priv->menu), NULL)));
	G_OBJECT_CLASS (grava_widget_parent_class)->dispose (obj);
}


GType grava_widget_get_type (void) {
	static GType grava_widget_type_id = 0;
	if (G_UNLIKELY (grava_widget_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaWidgetClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_widget_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaWidget), 0, (GInstanceInitFunc) grava_widget_init };
		grava_widget_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaWidget", &g_define_type_info, 0);
	}
	return grava_widget_type_id;
}




