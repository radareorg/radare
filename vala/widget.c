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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <float.h>
#include <math.h>
#include <gdk/gdk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cairo.h>


#define GRAVA_TYPE_WIDGET (grava_widget_get_type ())
#define GRAVA_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_WIDGET, GravaWidget))
#define GRAVA_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_WIDGET, GravaWidgetClass))
#define GRAVA_IS_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_WIDGET))
#define GRAVA_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_WIDGET))
#define GRAVA_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_WIDGET, GravaWidgetClass))

typedef struct _GravaWidget GravaWidget;
typedef struct _GravaWidgetClass GravaWidgetClass;
typedef struct _GravaWidgetPrivate GravaWidgetPrivate;

#define GRAVA_TYPE_GRAPH (grava_graph_get_type ())
#define GRAVA_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_GRAPH, GravaGraph))
#define GRAVA_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_GRAPH, GravaGraphClass))
#define GRAVA_IS_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_GRAPH))
#define GRAVA_IS_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_GRAPH))
#define GRAVA_GRAPH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_GRAPH, GravaGraphClass))

typedef struct _GravaGraph GravaGraph;
typedef struct _GravaGraphClass GravaGraphClass;

#define GRAVA_WIDGET_TYPE_WHEEL_ACTION (grava_widget_wheel_action_get_type ())

#define GRAVA_TYPE_NODE (grava_node_get_type ())
#define GRAVA_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_NODE, GravaNode))
#define GRAVA_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_NODE, GravaNodeClass))
#define GRAVA_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_NODE))
#define GRAVA_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_NODE))
#define GRAVA_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_NODE, GravaNodeClass))

typedef struct _GravaNode GravaNode;
typedef struct _GravaNodeClass GravaNodeClass;
typedef struct _GravaGraphPrivate GravaGraphPrivate;

#define GRAVA_TYPE_LAYOUT (grava_layout_get_type ())
#define GRAVA_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_LAYOUT, GravaLayout))
#define GRAVA_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_LAYOUT, GravaLayoutClass))
#define GRAVA_IS_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_LAYOUT))
#define GRAVA_IS_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_LAYOUT))
#define GRAVA_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_LAYOUT, GravaLayoutClass))

typedef struct _GravaLayout GravaLayout;
typedef struct _GravaLayoutClass GravaLayoutClass;

#define GRAVA_TYPE_EDGE (grava_edge_get_type ())
#define GRAVA_EDGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_EDGE, GravaEdge))
#define GRAVA_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_EDGE, GravaEdgeClass))
#define GRAVA_IS_EDGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_EDGE))
#define GRAVA_IS_EDGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_EDGE))
#define GRAVA_EDGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_EDGE, GravaEdgeClass))

typedef struct _GravaEdge GravaEdge;
typedef struct _GravaEdgeClass GravaEdgeClass;
typedef struct _GravaNodePrivate GravaNodePrivate;

struct _GravaWidget {
	GObject parent_instance;
	GravaWidgetPrivate * priv;
	GtkDrawingArea* da;
	GravaGraph* graph;
	gint separator;
};

struct _GravaWidgetClass {
	GObjectClass parent_class;
};

typedef enum  {
	GRAVA_WIDGET_WHEEL_ACTION_PAN = 0,
	GRAVA_WIDGET_WHEEL_ACTION_ZOOM = 1,
	GRAVA_WIDGET_WHEEL_ACTION_ROTATE = 2
} GravaWidgetWheelAction;

struct _GravaWidgetPrivate {
	GravaWidgetWheelAction wheel_action;
	GtkScrolledWindow* sw;
	GtkMenu* menu;
	double opanx;
	double opany;
	double offx;
	double offy;
	GravaNode* on;
};

struct _GravaGraph {
	GObject parent_instance;
	GravaGraphPrivate * priv;
	GravaLayout* layout;
	GSList* selhist;
	GSList* nodes;
	GSList* edges;
	GHashTable* data;
	double zoom;
	double panx;
	double pany;
	double angle;
};

struct _GravaGraphClass {
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



GType grava_widget_get_type (void);
GType grava_graph_get_type (void);
static GType grava_widget_wheel_action_get_type (void);
GType grava_node_get_type (void);
#define GRAVA_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRAVA_TYPE_WIDGET, GravaWidgetPrivate))
enum  {
	GRAVA_WIDGET_DUMMY_PROPERTY
};
#define GRAVA_WIDGET_SIZE 30
#define GRAVA_WIDGET_ZOOM_FACTOR 0.1
#define GRAVA_WIDGET_S (double) 96
GtkWidget* grava_widget_get_widget (GravaWidget* self);
static gboolean grava_widget_expose (GravaWidget* self, GtkDrawingArea* w, const GdkEventExpose* ev);
static gboolean _grava_widget_expose_gtk_widget_expose_event (GtkDrawingArea* _sender, const GdkEventExpose* event, gpointer self);
static gboolean grava_widget_motion (GravaWidget* self, GtkDrawingArea* da, const GdkEventMotion* em);
static gboolean _grava_widget_motion_gtk_widget_motion_notify_event (GtkDrawingArea* _sender, const GdkEventMotion* event, gpointer self);
static gboolean grava_widget_button_release (GravaWidget* self, GtkDrawingArea* da, const GdkEventButton* em);
static gboolean _grava_widget_button_release_gtk_widget_button_release_event (GtkDrawingArea* _sender, const GdkEventButton* event, gpointer self);
static gboolean grava_widget_button_press (GravaWidget* self, GtkDrawingArea* da, const GdkEventButton* eb);
static gboolean _grava_widget_button_press_gtk_widget_button_press_event (GtkDrawingArea* _sender, const GdkEventButton* event, gpointer self);
static gboolean grava_widget_scroll_press (GravaWidget* self, GtkDrawingArea* da, const GdkEventScroll* es);
static gboolean _grava_widget_scroll_press_gtk_widget_scroll_event (GtkDrawingArea* _sender, const GdkEventScroll* event, gpointer self);
static gboolean grava_widget_key_press (GravaWidget* self, GtkWidget* w, const GdkEventKey* ek);
static gboolean _grava_widget_key_press_gtk_widget_key_press_event (GtkScrolledWindow* _sender, const GdkEventKey* event, gpointer self);
static gboolean grava_widget_key_release (GravaWidget* self, GtkWidget* w, const GdkEventKey* ek);
static gboolean _grava_widget_key_release_gtk_widget_key_release_event (GtkScrolledWindow* _sender, const GdkEventKey* event, gpointer self);
static void _lambda9_ (GravaWidget* obj, const char* addr, GravaWidget* self);
static void __lambda9__grava_widget_load_graph_at (GravaWidget* _sender, const char* addr, gpointer self);
void grava_widget_create_widgets (GravaWidget* self);
GType grava_layout_get_type (void);
GType grava_edge_get_type (void);
extern GravaNode* grava_graph_selected;
char* grava_node_get (GravaNode* self, const char* key);
void grava_graph_select_next (GravaGraph* self);
void grava_graph_undo_select (GravaGraph* self);
void grava_graph_select_true (GravaGraph* self);
void grava_graph_select_false (GravaGraph* self);
static void _lambda4_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda4__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda5_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda5__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda6_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda6__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda7_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda7__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda8_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda8__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
void grava_widget_do_popup_generic (GravaWidget* self);
static void _lambda0_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda0__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda1_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda1__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda2_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda2__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
static void _lambda3_ (GtkImageMenuItem* imi, GravaWidget* self);
static void __lambda3__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self);
void grava_widget_do_popup_menu (GravaWidget* self);
GravaNode* grava_graph_click (GravaGraph* self, double x, double y);
void grava_node_fit (GravaNode* self);
static double grava_widget_abs (GravaWidget* self, double x);
void grava_widget_draw (GravaWidget* self);
void grava_graph_draw (GravaGraph* self, cairo_t* ctx);
void grava_renderer_square (cairo_t* ctx, double w, double h);
GravaWidget* grava_widget_new (void);
GravaWidget* grava_widget_construct (GType object_type);
GravaWidget* grava_widget_new (void);
GravaGraph* grava_graph_new (void);
GravaGraph* grava_graph_construct (GType object_type);
void grava_graph_update (GravaGraph* self);
static GObject * grava_widget_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer grava_widget_parent_class = NULL;
static void grava_widget_finalize (GObject* obj);


static void g_cclosure_user_marshal_INT__POINTER_STRING (GClosure * closure, GValue * return_value, guint n_param_values, const GValue * param_values, gpointer invocation_hint, gpointer marshal_data);


static GType grava_widget_wheel_action_get_type (void) {
	static GType grava_widget_wheel_action_type_id = 0;
	if (G_UNLIKELY (grava_widget_wheel_action_type_id == 0)) {
		static const GEnumValue values[] = {{GRAVA_WIDGET_WHEEL_ACTION_PAN, "GRAVA_WIDGET_WHEEL_ACTION_PAN", "pan"}, {GRAVA_WIDGET_WHEEL_ACTION_ZOOM, "GRAVA_WIDGET_WHEEL_ACTION_ZOOM", "zoom"}, {GRAVA_WIDGET_WHEEL_ACTION_ROTATE, "GRAVA_WIDGET_WHEEL_ACTION_ROTATE", "rotate"}, {0, NULL, NULL}};
		grava_widget_wheel_action_type_id = g_enum_register_static ("GravaWidgetWheelAction", values);
	}
	return grava_widget_wheel_action_type_id;
}


#line 46 "widget.vala"
GtkWidget* grava_widget_get_widget (GravaWidget* self) {
#line 246 "widget.c"
	GtkWidget* _tmp0_;
#line 46 "widget.vala"
	g_return_val_if_fail (self != NULL, NULL);
#line 48 "widget.vala"
	_tmp0_ = NULL;
#line 48 "widget.vala"
	return (_tmp0_ = (GtkWidget*) self->priv->sw, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_));
#line 254 "widget.c"
}


#line 517 "widget.vala"
static gboolean _grava_widget_expose_gtk_widget_expose_event (GtkDrawingArea* _sender, const GdkEventExpose* event, gpointer self) {
#line 260 "widget.c"
	return grava_widget_expose (self, _sender, event);
}


#line 477 "widget.vala"
static gboolean _grava_widget_motion_gtk_widget_motion_notify_event (GtkDrawingArea* _sender, const GdkEventMotion* event, gpointer self) {
#line 267 "widget.c"
	return grava_widget_motion (self, _sender, event);
}


#line 470 "widget.vala"
static gboolean _grava_widget_button_release_gtk_widget_button_release_event (GtkDrawingArea* _sender, const GdkEventButton* event, gpointer self) {
#line 274 "widget.c"
	return grava_widget_button_release (self, _sender, event);
}


#line 425 "widget.vala"
static gboolean _grava_widget_button_press_gtk_widget_button_press_event (GtkDrawingArea* _sender, const GdkEventButton* event, gpointer self) {
#line 281 "widget.c"
	return grava_widget_button_press (self, _sender, event);
}


#line 96 "widget.vala"
static gboolean _grava_widget_scroll_press_gtk_widget_scroll_event (GtkDrawingArea* _sender, const GdkEventScroll* event, gpointer self) {
#line 288 "widget.c"
	return grava_widget_scroll_press (self, _sender, event);
}


#line 151 "widget.vala"
static gboolean _grava_widget_key_press_gtk_widget_key_press_event (GtkScrolledWindow* _sender, const GdkEventKey* event, gpointer self) {
#line 295 "widget.c"
	return grava_widget_key_press (self, _sender, event);
}


#line 133 "widget.vala"
static gboolean _grava_widget_key_release_gtk_widget_key_release_event (GtkScrolledWindow* _sender, const GdkEventKey* event, gpointer self) {
#line 302 "widget.c"
	return grava_widget_key_release (self, _sender, event);
}


static void _lambda9_ (GravaWidget* obj, const char* addr, GravaWidget* self) {
	g_return_if_fail (obj != NULL);
	g_return_if_fail (addr != NULL);
}


static void __lambda9__grava_widget_load_graph_at (GravaWidget* _sender, const char* addr, gpointer self) {
	_lambda9_ (_sender, addr, self);
}


#line 57 "widget.vala"
void grava_widget_create_widgets (GravaWidget* self) {
#line 320 "widget.c"
	GtkDrawingArea* _tmp0_;
	GtkScrolledWindow* _tmp3_;
	GtkAdjustment* _tmp2_;
	GtkAdjustment* _tmp1_;
	GtkAdjustment* _tmp5_;
	GtkAdjustment* _tmp4_;
	GtkViewport* _tmp6_;
	GtkViewport* vp;
#line 57 "widget.vala"
	g_return_if_fail (self != NULL);
#line 331 "widget.c"
	_tmp0_ = NULL;
#line 59 "widget.vala"
	self->da = (_tmp0_ = g_object_ref_sink ((GtkDrawingArea*) gtk_drawing_area_new ()), (self->da == NULL) ? NULL : (self->da = (g_object_unref (self->da), NULL)), _tmp0_);
#line 335 "widget.c"
	/* add event listeners */
#line 62 "widget.vala"
	gtk_widget_add_events ((GtkWidget*) self->da, (gint) (((GDK_BUTTON1_MOTION_MASK | GDK_SCROLL_MASK) | GDK_BUTTON_PRESS_MASK) | GDK_BUTTON_RELEASE_MASK));
#line 339 "widget.c"
	/*da.set_events(  Gdk.EventMask.BUTTON1_MOTION_MASK );
	 Gdk.EventMask.POINTER_MOTION_MASK );*/
#line 68 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->da, "expose-event", (GCallback) _grava_widget_expose_gtk_widget_expose_event, self, 0);
#line 69 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->da, "motion-notify-event", (GCallback) _grava_widget_motion_gtk_widget_motion_notify_event, self, 0);
#line 70 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->da, "button-release-event", (GCallback) _grava_widget_button_release_gtk_widget_button_release_event, self, 0);
#line 71 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->da, "button-press-event", (GCallback) _grava_widget_button_press_gtk_widget_button_press_event, self, 0);
#line 72 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->da, "scroll-event", (GCallback) _grava_widget_scroll_press_gtk_widget_scroll_event, self, 0);
#line 352 "widget.c"
	_tmp3_ = NULL;
	_tmp2_ = NULL;
	_tmp1_ = NULL;
#line 74 "widget.vala"
	self->priv->sw = (_tmp3_ = g_object_ref_sink ((GtkScrolledWindow*) gtk_scrolled_window_new (_tmp1_ = g_object_ref_sink ((GtkAdjustment*) gtk_adjustment_new ((double) 0, (double) 10, (double) 1000, (double) 2, (double) 100, (double) 1000)), _tmp2_ = g_object_ref_sink ((GtkAdjustment*) gtk_adjustment_new ((double) 0, (double) 10, (double) 1000, (double) 2, (double) 100, (double) 1000)))), (self->priv->sw == NULL) ? NULL : (self->priv->sw = (g_object_unref (self->priv->sw), NULL)), _tmp3_);
#line 358 "widget.c"
	(_tmp2_ == NULL) ? NULL : (_tmp2_ = (g_object_unref (_tmp2_), NULL));
	(_tmp1_ == NULL) ? NULL : (_tmp1_ = (g_object_unref (_tmp1_), NULL));
#line 77 "widget.vala"
	gtk_scrolled_window_set_policy (self->priv->sw, GTK_POLICY_NEVER, GTK_POLICY_NEVER);
#line 363 "widget.c"
	_tmp5_ = NULL;
	_tmp4_ = NULL;
#line 79 "widget.vala"
	_tmp6_ = NULL;
#line 368 "widget.c"
	vp = (_tmp6_ = g_object_ref_sink ((GtkViewport*) gtk_viewport_new (_tmp4_ = g_object_ref_sink ((GtkAdjustment*) gtk_adjustment_new ((double) 0, (double) 10, (double) 1000, (double) 2, (double) 100, (double) 1000)), _tmp5_ = g_object_ref_sink ((GtkAdjustment*) gtk_adjustment_new ((double) 0, (double) 10, (double) 1000, (double) 2, (double) 100, (double) 1000)))), (_tmp5_ == NULL) ? NULL : (_tmp5_ = (g_object_unref (_tmp5_), NULL)), (_tmp4_ == NULL) ? NULL : (_tmp4_ = (g_object_unref (_tmp4_), NULL)), _tmp6_);
#line 82 "widget.vala"
	gtk_container_add ((GtkContainer*) vp, (GtkWidget*) self->da);
#line 84 "widget.vala"
	gtk_widget_add_events ((GtkWidget*) self->priv->sw, (gint) (GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK));
#line 85 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->sw, "key-press-event", (GCallback) _grava_widget_key_press_gtk_widget_key_press_event, self, 0);
#line 86 "widget.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->sw, "key-release-event", (GCallback) _grava_widget_key_release_gtk_widget_key_release_event, self, 0);
#line 88 "widget.vala"
	gtk_scrolled_window_add_with_viewport (self->priv->sw, (GtkWidget*) vp);
#line 90 "widget.vala"
	g_signal_connect (self, "load-graph-at", (GCallback) __lambda9__grava_widget_load_graph_at, self);
#line 382 "widget.c"
	(vp == NULL) ? NULL : (vp = (g_object_unref (vp), NULL));
}


/*stdout.printf("HOWHOWHOW "+addr);
 capture mouse motion */
#line 96 "widget.vala"
static gboolean grava_widget_scroll_press (GravaWidget* self, GtkDrawingArea* da, const GdkEventScroll* es) {
#line 96 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 96 "widget.vala"
	g_return_val_if_fail (da != NULL, FALSE);
#line 98 "widget.vala"
	gtk_widget_grab_focus ((GtkWidget*) self->priv->sw);
#line 100 "widget.vala"
	switch ((*es).direction) {
#line 399 "widget.c"
		case GDK_SCROLL_UP:
		{
#line 102 "widget.vala"
			switch (self->priv->wheel_action) {
#line 404 "widget.c"
				case GRAVA_WIDGET_WHEEL_ACTION_PAN:
				{
#line 104 "widget.vala"
					self->graph->pany = self->graph->pany + ((double) 64);
#line 105 "widget.vala"
					break;
#line 411 "widget.c"
				}
				case GRAVA_WIDGET_WHEEL_ACTION_ZOOM:
				{
#line 107 "widget.vala"
					self->graph->zoom = self->graph->zoom + GRAVA_WIDGET_ZOOM_FACTOR;
#line 108 "widget.vala"
					break;
#line 419 "widget.c"
				}
				case GRAVA_WIDGET_WHEEL_ACTION_ROTATE:
				{
#line 110 "widget.vala"
					self->graph->angle = self->graph->angle - 0.04;
#line 111 "widget.vala"
					break;
#line 427 "widget.c"
				}
			}
#line 113 "widget.vala"
			break;
#line 432 "widget.c"
		}
		case GDK_SCROLL_DOWN:
		{
#line 115 "widget.vala"
			switch (self->priv->wheel_action) {
#line 438 "widget.c"
				case GRAVA_WIDGET_WHEEL_ACTION_PAN:
				{
#line 117 "widget.vala"
					self->graph->pany = self->graph->pany - ((double) 64);
#line 118 "widget.vala"
					break;
#line 445 "widget.c"
				}
				case GRAVA_WIDGET_WHEEL_ACTION_ZOOM:
				{
#line 120 "widget.vala"
					self->graph->zoom = self->graph->zoom - GRAVA_WIDGET_ZOOM_FACTOR;
#line 121 "widget.vala"
					break;
#line 453 "widget.c"
				}
				case GRAVA_WIDGET_WHEEL_ACTION_ROTATE:
				{
#line 123 "widget.vala"
					self->graph->angle = self->graph->angle + 0.04;
#line 124 "widget.vala"
					break;
#line 461 "widget.c"
				}
			}
#line 126 "widget.vala"
			break;
#line 466 "widget.c"
		}
	}
#line 129 "widget.vala"
	gtk_widget_queue_draw_area ((GtkWidget*) da, 0, 0, 5000, 2000);
#line 130 "widget.vala"
	return FALSE;
#line 473 "widget.c"
}


#line 133 "widget.vala"
static gboolean grava_widget_key_release (GravaWidget* self, GtkWidget* w, const GdkEventKey* ek) {
#line 133 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 133 "widget.vala"
	g_return_val_if_fail (w != NULL, FALSE);
#line 135 "widget.vala"
	gtk_widget_grab_focus ((GtkWidget*) self->priv->sw);
#line 485 "widget.c"
	/*stdout.printf("Key released %d (%c)\n", (int)ek.keyval, (int)ek.keyval);*/
#line 139 "widget.vala"
	switch ((*ek).keyval) {
#line 489 "widget.c"
		case 65507:
		{
#line 141 "widget.vala"
			self->priv->wheel_action = GRAVA_WIDGET_WHEEL_ACTION_PAN;
#line 142 "widget.vala"
			break;
#line 496 "widget.c"
		}
		case 65505:
		{
#line 144 "widget.vala"
			self->priv->wheel_action = GRAVA_WIDGET_WHEEL_ACTION_PAN;
#line 145 "widget.vala"
			break;
#line 504 "widget.c"
		}
	}
#line 148 "widget.vala"
	return TRUE;
#line 509 "widget.c"
}


#line 151 "widget.vala"
static gboolean grava_widget_key_press (GravaWidget* self, GtkWidget* w, const GdkEventKey* ek) {
#line 515 "widget.c"
	gboolean handled;
#line 151 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 151 "widget.vala"
	g_return_val_if_fail (w != NULL, FALSE);
#line 521 "widget.c"
	handled = TRUE;
	/*DrawingArea da = (DrawingArea)w;*/
#line 155 "widget.vala"
	gtk_widget_grab_focus ((GtkWidget*) self->priv->sw);
#line 526 "widget.c"
	/* */
#line 158 "widget.vala"
	fprintf (stdout, "Key pressed %d (%c)\n", (gint) (*ek).keyval, (gint) (*ek).keyval);
#line 161 "widget.vala"
	switch ((*ek).keyval) {
#line 532 "widget.c"
		case 'b':
		case 65471:
		{
			gint _tmp1_;
			char* _tmp0_;
#line 61 "node.vala"
			_tmp0_ = NULL;
#line 164 "widget.vala"
			g_signal_emit_by_name (self, "set-breakpoint", NULL, _tmp0_ = grava_node_get (grava_graph_selected, "label"), &_tmp1_);
#line 164 "widget.vala"
			_tmp1_;
#line 544 "widget.c"
			_tmp0_ = (g_free (_tmp0_), NULL);
#line 165 "widget.vala"
			break;
#line 548 "widget.c"
		}
		case 'B':
		{
			gint _tmp4_;
			char* _tmp3_;
			char* _tmp2_;
#line 811 "glib-2.0.vapi"
			_tmp3_ = NULL;
#line 61 "node.vala"
			_tmp2_ = NULL;
#line 168 "widget.vala"
			g_signal_emit_by_name (self, "set-breakpoint", NULL, _tmp3_ = g_strdup_printf ("-%s", _tmp2_ = grava_node_get (grava_graph_selected, "label")), &_tmp4_);
#line 168 "widget.vala"
			_tmp4_;
#line 563 "widget.c"
			_tmp3_ = (g_free (_tmp3_), NULL);
			_tmp2_ = (g_free (_tmp2_), NULL);
#line 169 "widget.vala"
			break;
#line 568 "widget.c"
		}
		case 'S':
		{
#line 171 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", "!stepo");
#line 172 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", ".!regs*");
#line 173 "widget.vala"
			g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 174 "widget.vala"
			break;
#line 580 "widget.c"
		}
		case 's':
		case 65476:
		{
#line 177 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", "!step");
#line 178 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", ".!regs*");
#line 180 "widget.vala"
			g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 181 "widget.vala"
			break;
#line 593 "widget.c"
		}
		case 65478:
		{
#line 183 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", "!cont");
#line 184 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", ".!regs*");
#line 185 "widget.vala"
			g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 186 "widget.vala"
			break;
#line 605 "widget.c"
		}
		case 65507:
		{
#line 188 "widget.vala"
			self->priv->wheel_action = GRAVA_WIDGET_WHEEL_ACTION_ZOOM;
#line 189 "widget.vala"
			break;
#line 613 "widget.c"
		}
		case 65505:
		{
#line 191 "widget.vala"
			self->priv->wheel_action = GRAVA_WIDGET_WHEEL_ACTION_ROTATE;
#line 192 "widget.vala"
			break;
#line 621 "widget.c"
		}
		case 65056:
		{
#line 207 "widget.vala"
			break;
#line 627 "widget.c"
		}
		case 65289:
		{
#line 209 "widget.vala"
			grava_graph_select_next (self->graph);
#line 210 "widget.vala"
			if (grava_graph_selected == NULL) {
#line 211 "widget.vala"
				grava_graph_select_next (self->graph);
#line 637 "widget.c"
			}
#line 212 "widget.vala"
			if (grava_graph_selected != NULL) {
#line 641 "widget.c"
				/*sw.get_size(ref w, ref, h);
				 XXXX get window size*/
#line 215 "widget.vala"
				self->graph->panx = (-grava_graph_selected->x) + 350;
#line 216 "widget.vala"
				self->graph->pany = (-grava_graph_selected->y) + 350;
#line 648 "widget.c"
			}
#line 218 "widget.vala"
			break;
#line 652 "widget.c"
		}
		case '1':
		{
#line 220 "widget.vala"
			self->graph->zoom = (double) 1;
#line 221 "widget.vala"
			break;
#line 660 "widget.c"
		}
		case 65361:
		case 'h':
		{
#line 224 "widget.vala"
			self->graph->panx = self->graph->panx + (GRAVA_WIDGET_S * self->graph->zoom);
#line 225 "widget.vala"
			break;
#line 669 "widget.c"
		}
		case 65364:
		case 'j':
		{
#line 228 "widget.vala"
			self->graph->pany = self->graph->pany - (GRAVA_WIDGET_S * self->graph->zoom);
#line 229 "widget.vala"
			break;
#line 678 "widget.c"
		}
		case 65362:
		case 'k':
		{
#line 232 "widget.vala"
			self->graph->pany = self->graph->pany + (GRAVA_WIDGET_S * self->graph->zoom);
#line 233 "widget.vala"
			break;
#line 687 "widget.c"
		}
		case 65363:
		case 'l':
		{
#line 236 "widget.vala"
			self->graph->panx = self->graph->panx - (GRAVA_WIDGET_S * self->graph->zoom);
#line 237 "widget.vala"
			break;
#line 696 "widget.c"
		}
		case 'H':
		{
#line 239 "widget.vala"
			self->graph->panx = self->graph->panx + (GRAVA_WIDGET_S * self->graph->zoom);
#line 240 "widget.vala"
			if (grava_graph_selected != NULL) {
#line 241 "widget.vala"
				grava_graph_selected->x = grava_graph_selected->x - (GRAVA_WIDGET_S * self->graph->zoom);
#line 706 "widget.c"
			}
#line 242 "widget.vala"
			break;
#line 710 "widget.c"
		}
		case 'J':
		{
#line 244 "widget.vala"
			self->graph->pany = self->graph->pany - (GRAVA_WIDGET_S * self->graph->zoom);
#line 245 "widget.vala"
			if (grava_graph_selected != NULL) {
#line 246 "widget.vala"
				grava_graph_selected->y = grava_graph_selected->y + (GRAVA_WIDGET_S * self->graph->zoom);
#line 720 "widget.c"
			}
#line 247 "widget.vala"
			break;
#line 724 "widget.c"
		}
		case 'K':
		{
#line 249 "widget.vala"
			self->graph->pany = self->graph->pany + (GRAVA_WIDGET_S * self->graph->zoom);
#line 250 "widget.vala"
			if (grava_graph_selected != NULL) {
#line 251 "widget.vala"
				grava_graph_selected->y = grava_graph_selected->y - (GRAVA_WIDGET_S * self->graph->zoom);
#line 734 "widget.c"
			}
#line 252 "widget.vala"
			break;
#line 738 "widget.c"
		}
		case 'L':
		{
#line 254 "widget.vala"
			if (self->priv->wheel_action == GRAVA_WIDGET_WHEEL_ACTION_ZOOM) {
#line 744 "widget.c"
			} else {
#line 256 "widget.vala"
				self->graph->panx = self->graph->panx - (GRAVA_WIDGET_S * self->graph->zoom);
#line 257 "widget.vala"
				if (grava_graph_selected != NULL) {
#line 258 "widget.vala"
					grava_graph_selected->x = grava_graph_selected->x + (GRAVA_WIDGET_S * self->graph->zoom);
#line 752 "widget.c"
				}
			}
#line 260 "widget.vala"
			break;
#line 757 "widget.c"
		}
		case '.':
		{
#line 262 "widget.vala"
			if (grava_graph_selected != NULL) {
#line 763 "widget.c"
				char* _tmp5_;
#line 61 "node.vala"
				_tmp5_ = NULL;
#line 263 "widget.vala"
				g_signal_emit_by_name (self, "load-graph-at", _tmp5_ = grava_node_get (grava_graph_selected, "label"));
#line 769 "widget.c"
				_tmp5_ = (g_free (_tmp5_), NULL);
			} else {
#line 264 "widget.vala"
				grava_graph_select_next (self->graph);
#line 774 "widget.c"
			}
#line 265 "widget.vala"
			break;
#line 778 "widget.c"
		}
		case ':':
		{
#line 267 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", "s eip");
#line 268 "widget.vala"
			g_signal_emit_by_name (self, "load-graph-at", "eip");
#line 269 "widget.vala"
			grava_graph_selected = grava_graph_selected = NULL;
#line 270 "widget.vala"
			break;
#line 790 "widget.c"
		}
		case 'u':
		{
#line 272 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", "s-");
#line 273 "widget.vala"
			g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 274 "widget.vala"
			grava_graph_undo_select (self->graph);
#line 275 "widget.vala"
			break;
#line 802 "widget.c"
		}
		case 'U':
		{
#line 277 "widget.vala"
			g_signal_emit_by_name (self, "run-cmd", "s+");
#line 278 "widget.vala"
			g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 279 "widget.vala"
			grava_graph_undo_select (self->graph);
#line 280 "widget.vala"
			break;
#line 814 "widget.c"
		}
		case 't':
		{
#line 282 "widget.vala"
			grava_graph_select_true (self->graph);
#line 283 "widget.vala"
			break;
#line 822 "widget.c"
		}
		case 'f':
		{
#line 285 "widget.vala"
			grava_graph_select_false (self->graph);
#line 286 "widget.vala"
			break;
#line 830 "widget.c"
		}
		case '+':
		{
#line 288 "widget.vala"
			self->graph->zoom = self->graph->zoom + GRAVA_WIDGET_ZOOM_FACTOR;
#line 289 "widget.vala"
			break;
#line 838 "widget.c"
		}
		case '-':
		{
#line 291 "widget.vala"
			self->graph->zoom = self->graph->zoom - GRAVA_WIDGET_ZOOM_FACTOR;
#line 292 "widget.vala"
			break;
#line 846 "widget.c"
		}
		case '*':
		{
#line 294 "widget.vala"
			self->graph->angle = self->graph->angle + 0.05;
#line 295 "widget.vala"
			break;
#line 854 "widget.c"
		}
		case '/':
		{
#line 297 "widget.vala"
			self->graph->angle = self->graph->angle - 0.05;
#line 298 "widget.vala"
			break;
#line 862 "widget.c"
		}
		default:
		{
#line 300 "widget.vala"
			handled = FALSE;
#line 301 "widget.vala"
			break;
#line 870 "widget.c"
		}
	}
	/*expose(da, ev);*/
#line 305 "widget.vala"
	gtk_widget_queue_draw_area ((GtkWidget*) self->da, 0, 0, 5000, 2000);
#line 307 "widget.vala"
	return TRUE;
#line 878 "widget.c"
}


static void _lambda4_ (GtkImageMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi != NULL);
	/* foo */
#line 319 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", "s-");
#line 320 "widget.vala"
	g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 889 "widget.c"
}


static void __lambda4__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda4_ (_sender, self);
}


static void _lambda5_ (GtkImageMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi != NULL);
	/* foo */
#line 327 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", "s+");
#line 328 "widget.vala"
	g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 905 "widget.c"
}


static void __lambda5__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda5_ (_sender, self);
}


static void _lambda6_ (GtkImageMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi != NULL);
	/* foo */
#line 335 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", "s eip");
#line 336 "widget.vala"
	g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 921 "widget.c"
}


static void __lambda6__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda6_ (_sender, self);
}


static void _lambda7_ (GtkImageMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi != NULL);
	/* foo */
#line 345 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", "!step");
#line 346 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", ".!regs*");
#line 347 "widget.vala"
	g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 939 "widget.c"
}


static void __lambda7__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda7_ (_sender, self);
}


static void _lambda8_ (GtkImageMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi != NULL);
	/* foo */
#line 354 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", "!continue");
#line 355 "widget.vala"
	g_signal_emit_by_name (self, "run-cmd", ".!regs*");
#line 356 "widget.vala"
	g_signal_emit_by_name (self, "load-graph-at", "$$");
#line 957 "widget.c"
}


static void __lambda8__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda8_ (_sender, self);
}


#line 310 "widget.vala"
void grava_widget_do_popup_generic (GravaWidget* self) {
#line 968 "widget.c"
	GtkImageMenuItem* imi;
	GtkMenu* _tmp0_;
	GtkImageMenuItem* _tmp1_;
	GtkImageMenuItem* _tmp2_;
	GtkImageMenuItem* _tmp3_;
	GtkSeparatorMenuItem* _tmp4_;
	GtkImageMenuItem* _tmp5_;
	GtkImageMenuItem* _tmp6_;
#line 310 "widget.vala"
	g_return_if_fail (self != NULL);
#line 979 "widget.c"
	imi = NULL;
	_tmp0_ = NULL;
#line 313 "widget.vala"
	self->priv->menu = (_tmp0_ = g_object_ref_sink ((GtkMenu*) gtk_menu_new ()), (self->priv->menu == NULL) ? NULL : (self->priv->menu = (g_object_unref (self->priv->menu), NULL)), _tmp0_);
#line 984 "widget.c"
	/* XXX: most of this should be done in a tab panel or so */
	_tmp1_ = NULL;
#line 316 "widget.vala"
	imi = (_tmp1_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_from_stock ("undo seek", NULL)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp1_);
#line 317 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda4__gtk_menu_item_activate, self);
#line 322 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 993 "widget.c"
	_tmp2_ = NULL;
#line 324 "widget.vala"
	imi = (_tmp2_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_from_stock ("redo seek", NULL)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp2_);
#line 325 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda5__gtk_menu_item_activate, self);
#line 330 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1001 "widget.c"
	_tmp3_ = NULL;
#line 332 "widget.vala"
	imi = (_tmp3_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_from_stock ("Seek to eip", NULL)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp3_);
#line 333 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda6__gtk_menu_item_activate, self);
#line 338 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1009 "widget.c"
	_tmp4_ = NULL;
#line 340 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) (_tmp4_ = g_object_ref_sink ((GtkSeparatorMenuItem*) gtk_separator_menu_item_new ()))));
#line 1013 "widget.c"
	(_tmp4_ == NULL) ? NULL : (_tmp4_ = (g_object_unref (_tmp4_), NULL));
	_tmp5_ = NULL;
#line 342 "widget.vala"
	imi = (_tmp5_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_from_stock ("Step", NULL)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp5_);
#line 343 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda7__gtk_menu_item_activate, self);
#line 349 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1022 "widget.c"
	_tmp6_ = NULL;
#line 351 "widget.vala"
	imi = (_tmp6_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_from_stock ("Continue", NULL)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp6_);
#line 352 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda8__gtk_menu_item_activate, self);
#line 358 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 359 "widget.vala"
	gtk_widget_show_all ((GtkWidget*) self->priv->menu);
#line 360 "widget.vala"
	gtk_menu_popup (self->priv->menu, NULL, NULL, NULL, NULL, (guint) 0, (guint32) 0);
#line 1034 "widget.c"
	(imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL));
}


static void _lambda0_ (GtkImageMenuItem* imi, GravaWidget* self) {
	gint _tmp1_;
	char* _tmp0_;
	g_return_if_fail (imi != NULL);
	/*stdout.printf("go in!\n");*/
#line 61 "node.vala"
	_tmp0_ = NULL;
#line 372 "widget.vala"
	g_signal_emit_by_name (self, "focus-at-label", NULL, _tmp0_ = grava_node_get (grava_graph_selected, "label"), &_tmp1_);
#line 372 "widget.vala"
	_tmp1_;
#line 1050 "widget.c"
	_tmp0_ = (g_free (_tmp0_), NULL);
}


static void __lambda0__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda0_ (_sender, self);
}


static void _lambda1_ (GtkImageMenuItem* imi, GravaWidget* self) {
	gint _tmp1_;
	char* _tmp0_;
	g_return_if_fail (imi != NULL);
	/*stdout.printf("add bp!\n");*/
#line 61 "node.vala"
	_tmp0_ = NULL;
#line 382 "widget.vala"
	g_signal_emit_by_name (self, "set-breakpoint", NULL, _tmp0_ = grava_node_get (grava_graph_selected, "label"), &_tmp1_);
#line 382 "widget.vala"
	_tmp1_;
#line 1071 "widget.c"
	_tmp0_ = (g_free (_tmp0_), NULL);
}


static void __lambda1__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda1_ (_sender, self);
}


static void _lambda2_ (GtkImageMenuItem* imi, GravaWidget* self) {
	gint _tmp1_;
	char* _tmp0_;
	g_return_if_fail (imi != NULL);
#line 61 "node.vala"
	_tmp0_ = NULL;
#line 388 "widget.vala"
	g_signal_emit_by_name (self, "unset-breakpoint", NULL, _tmp0_ = grava_node_get (grava_graph_selected, "label"), &_tmp1_);
#line 388 "widget.vala"
	_tmp1_;
#line 1091 "widget.c"
	_tmp0_ = (g_free (_tmp0_), NULL);
}


static void __lambda2__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda2_ (_sender, self);
}


static void _lambda3_ (GtkImageMenuItem* imi, GravaWidget* self) {
	g_return_if_fail (imi != NULL);
	/*stdout.printf("FUCKME: \n"+imi.submenu_placement());*/
#line 414 "widget.vala"
	g_signal_emit_by_name (self, "load-graph-at", gtk_label_get_text (GTK_LABEL (((GtkBin*) imi)->child)));
#line 1106 "widget.c"
}


static void __lambda3__gtk_menu_item_activate (GtkImageMenuItem* _sender, gpointer self) {
	_lambda3_ (_sender, self);
}


#line 363 "widget.vala"
void grava_widget_do_popup_menu (GravaWidget* self) {
#line 1117 "widget.c"
	GtkImageMenuItem* imi;
	GtkMenu* _tmp0_;
	GtkImageMenuItem* _tmp1_;
	GtkImageMenuItem* _tmp2_;
	GtkImageMenuItem* _tmp3_;
#line 363 "widget.vala"
	g_return_if_fail (self != NULL);
#line 1125 "widget.c"
	imi = NULL;
	_tmp0_ = NULL;
#line 366 "widget.vala"
	self->priv->menu = (_tmp0_ = g_object_ref_sink ((GtkMenu*) gtk_menu_new ()), (self->priv->menu == NULL) ? NULL : (self->priv->menu = (g_object_unref (self->priv->menu), NULL)), _tmp0_);
#line 1130 "widget.c"
	/*imi = new ImageMenuItem.with_label("Focus");*/
	_tmp1_ = NULL;
#line 369 "widget.vala"
	imi = (_tmp1_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_from_stock ("gtk-zoom-in", NULL)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp1_);
#line 370 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda0__gtk_menu_item_activate, self);
#line 1137 "widget.c"
	/*MenuItem mi = menu.get_active();
	load_graph_at(((Label)imi.child).get_text()); //"0x400");
	stdout.printf(" cocococo "+ menu.);*/
#line 377 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1143 "widget.c"
	_tmp2_ = NULL;
#line 379 "widget.vala"
	imi = (_tmp2_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_with_label ("Breakpoint here")), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp2_);
#line 380 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda1__gtk_menu_item_activate, self);
#line 384 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1151 "widget.c"
	_tmp3_ = NULL;
#line 386 "widget.vala"
	imi = (_tmp3_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_with_label ("Remove breakpoint")), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp3_);
#line 387 "widget.vala"
	g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda2__gtk_menu_item_activate, self);
#line 390 "widget.vala"
	gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1159 "widget.c"
	/* TODO: add continue until here
	
	imi = new ImageMenuItem.with_label("Remove true branch");
	imi.activate += imi => {
	///stdout.printf("Focus!\n");
	};
	menu.append(imi);
	
	imi = new ImageMenuItem.with_label("Remove false branch");
	imi.activate += imi => {
	//stdout.printf("Focus!\n");
	};
	menu.append(imi);
	*/
#line 407 "widget.vala"
	if (grava_graph_selected != NULL) {
#line 1176 "widget.c"
		GtkSeparatorMenuItem* _tmp4_;
		_tmp4_ = NULL;
#line 408 "widget.vala"
		gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) (_tmp4_ = g_object_ref_sink ((GtkSeparatorMenuItem*) gtk_separator_menu_item_new ()))));
#line 1181 "widget.c"
		(_tmp4_ == NULL) ? NULL : (_tmp4_ = (g_object_unref (_tmp4_), NULL));
		{
			GSList* str_collection;
			GSList* str_it;
#line 410 "widget.vala"
			str_collection = grava_graph_selected->calls;
#line 1188 "widget.c"
			for (str_it = str_collection; str_it != NULL; str_it = str_it->next) {
				const char* _tmp6_;
				char* str;
#line 25 "node.vala"
				_tmp6_ = NULL;
#line 410 "widget.vala"
				str = (_tmp6_ = (const char*) str_it->data, (_tmp6_ == NULL) ? NULL : g_strdup (_tmp6_));
#line 1196 "widget.c"
				{
					GtkImageMenuItem* _tmp5_;
					_tmp5_ = NULL;
#line 411 "widget.vala"
					imi = (_tmp5_ = g_object_ref_sink ((GtkImageMenuItem*) gtk_image_menu_item_new_with_label (str)), (imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL)), _tmp5_);
#line 412 "widget.vala"
					g_signal_connect ((GtkMenuItem*) imi, "activate", (GCallback) __lambda3__gtk_menu_item_activate, self);
#line 1204 "widget.c"
					/*"0x400");*/
#line 416 "widget.vala"
					gtk_menu_shell_append ((GtkMenuShell*) self->priv->menu, (GtkWidget*) ((GtkMenuItem*) imi));
#line 1208 "widget.c"
					str = (g_free (str), NULL);
				}
			}
		}
	}
#line 420 "widget.vala"
	gtk_widget_show_all ((GtkWidget*) self->priv->menu);
#line 1216 "widget.c"
	/*menu.popup(null, null, null, null, eb.button, 0);*/
#line 422 "widget.vala"
	gtk_menu_popup (self->priv->menu, NULL, NULL, NULL, NULL, (guint) 0, (guint32) 0);
#line 1220 "widget.c"
	(imi == NULL) ? NULL : (imi = (g_object_unref (imi), NULL));
}


#line 425 "widget.vala"
static gboolean grava_widget_button_press (GravaWidget* self, GtkDrawingArea* da, const GdkEventButton* eb) {
#line 1227 "widget.c"
	GravaNode* _tmp0_;
	GravaNode* n;
	gboolean _tmp2_;
#line 425 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 425 "widget.vala"
	g_return_val_if_fail (da != NULL, FALSE);
#line 1235 "widget.c"
	/*EventButton eb = event.button;
	EventMotion em = event.motion; */
#line 429 "widget.vala"
	_tmp0_ = NULL;
#line 1240 "widget.c"
	n = (_tmp0_ = grava_graph_click (self->graph, (*eb).x - self->graph->panx, (*eb).y - self->graph->pany), (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_));
#line 431 "widget.vala"
	gtk_widget_grab_focus ((GtkWidget*) self->priv->sw);
#line 432 "widget.vala"
	grava_graph_selected = n;
#line 433 "widget.vala"
	if ((*eb).button == 3) {
#line 434 "widget.vala"
		if (n != NULL) {
#line 435 "widget.vala"
			grava_widget_do_popup_menu (self);
#line 1252 "widget.c"
		} else {
#line 436 "widget.vala"
			grava_widget_do_popup_generic (self);
#line 1256 "widget.c"
		}
	}
#line 438 "widget.vala"
	if (n != NULL) {
#line 1261 "widget.c"
		gboolean _tmp1_ = {0};
#line 439 "widget.vala"
		if ((((*eb).y - (16 * self->graph->zoom)) - self->graph->pany) < (n->y * self->graph->zoom)) {
#line 440 "widget.vala"
			_tmp1_ = ((*eb).x - self->graph->panx) > (((n->x + n->w) - (16 * self->graph->zoom)) * self->graph->zoom);
#line 1267 "widget.c"
		} else {
#line 439 "widget.vala"
			_tmp1_ = FALSE;
#line 1271 "widget.c"
		}
#line 439 "widget.vala"
		if (_tmp1_) {
#line 441 "widget.vala"
			n->has_body = !n->has_body;
#line 442 "widget.vala"
			grava_node_fit (n);
#line 1279 "widget.c"
		}
#line 445 "widget.vala"
		self->priv->opanx = (*eb).x;
#line 446 "widget.vala"
		self->priv->opany = (*eb).y;
#line 1285 "widget.c"
		/*graph.draw(Gdk.cairo_create(da.window));*/
#line 449 "widget.vala"
		gtk_widget_queue_draw_area ((GtkWidget*) da, 0, 0, 5000, 3000);
#line 1289 "widget.c"
	}
#line 452 "widget.vala"
	return (_tmp2_ = TRUE, (n == NULL) ? NULL : (n = (g_object_unref (n), NULL)), _tmp2_);
#line 1293 "widget.c"
}


#line 462 "widget.vala"
static double grava_widget_abs (GravaWidget* self, double x) {
#line 462 "widget.vala"
	g_return_val_if_fail (self != NULL, 0.0);
#line 464 "widget.vala"
	if (x > 0) {
#line 465 "widget.vala"
		return x;
#line 1305 "widget.c"
	}
#line 466 "widget.vala"
	return -x;
#line 1309 "widget.c"
}


#line 470 "widget.vala"
static gboolean grava_widget_button_release (GravaWidget* self, GtkDrawingArea* da, const GdkEventButton* em) {
#line 1315 "widget.c"
	GravaNode* _tmp0_;
#line 470 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 470 "widget.vala"
	g_return_val_if_fail (da != NULL, FALSE);
#line 1321 "widget.c"
	_tmp0_ = NULL;
#line 472 "widget.vala"
	self->priv->on = (_tmp0_ = NULL, (self->priv->on == NULL) ? NULL : (self->priv->on = (g_object_unref (self->priv->on), NULL)), _tmp0_);
#line 473 "widget.vala"
	self->priv->opanx = self->priv->opany = (double) 0;
#line 474 "widget.vala"
	return TRUE;
#line 1329 "widget.c"
}


#line 477 "widget.vala"
static gboolean grava_widget_motion (GravaWidget* self, GtkDrawingArea* da, const GdkEventMotion* em) {
#line 1335 "widget.c"
	GravaNode* _tmp0_;
	GravaNode* n;
	gboolean _tmp4_;
#line 477 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 477 "widget.vala"
	g_return_val_if_fail (da != NULL, FALSE);
#line 479 "widget.vala"
	_tmp0_ = NULL;
#line 1345 "widget.c"
	n = (_tmp0_ = grava_graph_selected, (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_));
	/*graph.click(em.x-graph.panx, em.y-graph.pany);*/
#line 480 "widget.vala"
	gtk_widget_grab_focus ((GtkWidget*) self->priv->sw);
#line 481 "widget.vala"
	if (n != NULL) {
#line 1352 "widget.c"
		double emx;
		double emy;
		emx = (*em).x - self->graph->panx;
		/* * graph.zoom;*/
		emy = (*em).y - self->graph->pany;
		/* * graph.zoom;
		 drag node 
		 TODO: properly handle the graph.zoom */
#line 486 "widget.vala"
		if (n != self->priv->on) {
#line 1363 "widget.c"
			GravaNode* _tmp2_;
			GravaNode* _tmp1_;
			/* offx, offy are the delta between click and node x,y 
			emx/=graph.zoom;
			emy/=graph.zoom;*/
#line 490 "widget.vala"
			self->priv->offx = (emx / self->graph->zoom) - n->x;
#line 1371 "widget.c"
			/*graph.zoom;*/
#line 491 "widget.vala"
			self->priv->offy = (emy / self->graph->zoom) - n->y;
#line 1375 "widget.c"
			/*/graph.zoom;*/
			_tmp2_ = NULL;
#line 492 "widget.vala"
			_tmp1_ = NULL;
#line 492 "widget.vala"
			self->priv->on = (_tmp2_ = (_tmp1_ = n, (_tmp1_ == NULL) ? NULL : g_object_ref (_tmp1_)), (self->priv->on == NULL) ? NULL : (self->priv->on = (g_object_unref (self->priv->on), NULL)), _tmp2_);
#line 1382 "widget.c"
		}
#line 495 "widget.vala"
		n->x = (emx - self->priv->offx) / self->graph->zoom;
#line 496 "widget.vala"
		n->y = (emy - self->priv->offy) / self->graph->zoom;
#line 498 "widget.vala"
		gtk_widget_queue_draw_area ((GtkWidget*) da, 0, 0, 5000, 3000);
#line 499 "widget.vala"
		grava_graph_selected = n;
#line 1392 "widget.c"
	} else {
		gboolean _tmp3_ = {0};
#line 502 "widget.vala"
		if (self->priv->opanx != 0) {
#line 502 "widget.vala"
			_tmp3_ = self->priv->opany != 0;
#line 1399 "widget.c"
		} else {
#line 502 "widget.vala"
			_tmp3_ = FALSE;
#line 1403 "widget.c"
		}
		/* pan view */
#line 502 "widget.vala"
		if (_tmp3_) {
#line 1408 "widget.c"
			double x;
			double y;
			x = (*em).x - self->priv->opanx;
			y = (*em).y - self->priv->opany;
#line 505 "widget.vala"
			self->graph->panx = self->graph->panx + x;
#line 1415 "widget.c"
			/**0.8;*/
#line 506 "widget.vala"
			self->graph->pany = self->graph->pany + y;
#line 1419 "widget.c"
			/**0.8;
			graph.draw(Gdk.cairo_create(da.window));*/
#line 508 "widget.vala"
			gtk_widget_queue_draw_area ((GtkWidget*) da, 0, 0, 5000, 3000);
#line 1424 "widget.c"
		}
		/*Graph.selected = null;*/
#line 511 "widget.vala"
		self->priv->opanx = (*em).x;
#line 512 "widget.vala"
		self->priv->opany = (*em).y;
#line 1431 "widget.c"
	}
#line 514 "widget.vala"
	return (_tmp4_ = TRUE, (n == NULL) ? NULL : (n = (g_object_unref (n), NULL)), _tmp4_);
#line 1435 "widget.c"
}


#line 517 "widget.vala"
static gboolean grava_widget_expose (GravaWidget* self, GtkDrawingArea* w, const GdkEventExpose* ev) {
#line 517 "widget.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 517 "widget.vala"
	g_return_val_if_fail (w != NULL, FALSE);
#line 1445 "widget.c"
	/*DrawingArea da = (DrawingArea)w;*/
#line 520 "widget.vala"
	grava_widget_draw (self);
#line 521 "widget.vala"
	return TRUE;
#line 1451 "widget.c"
}


#line 524 "widget.vala"
void grava_widget_draw (GravaWidget* self) {
#line 1457 "widget.c"
	cairo_t* ctx;
#line 524 "widget.vala"
	g_return_if_fail (self != NULL);
#line 1461 "widget.c"
	ctx = gdk_cairo_create ((GdkDrawable*) gtk_widget_get_window ((GtkWidget*) self->da));
#line 527 "widget.vala"
	cairo_save (ctx);
#line 528 "widget.vala"
	if (self->graph->zoom < 0.05) {
#line 529 "widget.vala"
		self->graph->zoom = 0.05;
#line 1469 "widget.c"
	}
#line 530 "widget.vala"
	grava_graph_draw (self->graph, ctx);
#line 531 "widget.vala"
	cairo_restore (ctx);
#line 532 "widget.vala"
	if (self->separator != 0) {
#line 533 "widget.vala"
		cairo_set_source_rgba (ctx, 0.6, 0.6, 0.6, 0.2);
#line 534 "widget.vala"
		cairo_move_to (ctx, (double) 0, (double) 0);
#line 535 "widget.vala"
		grava_renderer_square (ctx, (double) self->separator, (double) 2048);
#line 536 "widget.vala"
		cairo_fill (ctx);
#line 1485 "widget.c"
	}
	(ctx == NULL) ? NULL : (ctx = (cairo_destroy (ctx), NULL));
}


#line 25 "widget.vala"
GravaWidget* grava_widget_construct (GType object_type) {
#line 1493 "widget.c"
	GravaWidget * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


#line 25 "widget.vala"
GravaWidget* grava_widget_new (void) {
#line 25 "widget.vala"
	return grava_widget_construct (GRAVA_TYPE_WIDGET);
#line 1504 "widget.c"
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
		GravaGraph* _tmp0_;
		_tmp0_ = NULL;
#line 52 "widget.vala"
		self->graph = (_tmp0_ = grava_graph_new (), (self->graph == NULL) ? NULL : (self->graph = (g_object_unref (self->graph), NULL)), _tmp0_);
#line 53 "widget.vala"
		grava_graph_update (self->graph);
#line 54 "widget.vala"
		grava_widget_create_widgets (self);
#line 1526 "widget.c"
	}
	return obj;
}


static void grava_widget_class_init (GravaWidgetClass * klass) {
	grava_widget_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (GravaWidgetPrivate));
	G_OBJECT_CLASS (klass)->constructor = grava_widget_constructor;
	G_OBJECT_CLASS (klass)->finalize = grava_widget_finalize;
	g_signal_new ("load_graph_at", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	g_signal_new ("breakpoint_at", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	g_signal_new ("run_cmd", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	g_signal_new ("focus_at_label", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_user_marshal_INT__POINTER_STRING, G_TYPE_INT, 2, G_TYPE_POINTER, G_TYPE_STRING);
	g_signal_new ("set_breakpoint", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_user_marshal_INT__POINTER_STRING, G_TYPE_INT, 2, G_TYPE_POINTER, G_TYPE_STRING);
	g_signal_new ("unset_breakpoint", GRAVA_TYPE_WIDGET, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_user_marshal_INT__POINTER_STRING, G_TYPE_INT, 2, G_TYPE_POINTER, G_TYPE_STRING);
}


static void grava_widget_instance_init (GravaWidget * self) {
	self->priv = GRAVA_WIDGET_GET_PRIVATE (self);
	self->priv->wheel_action = GRAVA_WIDGET_WHEEL_ACTION_PAN;
	self->priv->opanx = (double) 0;
	self->priv->opany = (double) 0;
	self->priv->offx = (double) 0;
	self->priv->offy = (double) 0;
	self->priv->on = NULL;
	self->separator = 0;
}


static void grava_widget_finalize (GObject* obj) {
	GravaWidget * self;
	self = GRAVA_WIDGET (obj);
	(self->da == NULL) ? NULL : (self->da = (g_object_unref (self->da), NULL));
	(self->graph == NULL) ? NULL : (self->graph = (g_object_unref (self->graph), NULL));
	(self->priv->sw == NULL) ? NULL : (self->priv->sw = (g_object_unref (self->priv->sw), NULL));
	(self->priv->menu == NULL) ? NULL : (self->priv->menu = (g_object_unref (self->priv->menu), NULL));
	(self->priv->on == NULL) ? NULL : (self->priv->on = (g_object_unref (self->priv->on), NULL));
	G_OBJECT_CLASS (grava_widget_parent_class)->finalize (obj);
}


GType grava_widget_get_type (void) {
	static GType grava_widget_type_id = 0;
	if (grava_widget_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GravaWidgetClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) grava_widget_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GravaWidget), 0, (GInstanceInitFunc) grava_widget_instance_init, NULL };
		grava_widget_type_id = g_type_register_static (G_TYPE_OBJECT, "GravaWidget", &g_define_type_info, 0);
	}
	return grava_widget_type_id;
}



static void g_cclosure_user_marshal_INT__POINTER_STRING (GClosure * closure, GValue * return_value, guint n_param_values, const GValue * param_values, gpointer invocation_hint, gpointer marshal_data) {
	typedef gint (*GMarshalFunc_INT__POINTER_STRING) (gpointer data1, gpointer arg_1, const char* arg_2, gpointer data2);
	register GMarshalFunc_INT__POINTER_STRING callback;
	register GCClosure * cc;
	register gpointer data1, data2;
	gint v_return;
	cc = (GCClosure *) closure;
	g_return_if_fail (return_value != NULL);
	g_return_if_fail (n_param_values == 3);
	if (G_CCLOSURE_SWAP_DATA (closure)) {
		data1 = closure->data;
		data2 = param_values->data[0].v_pointer;
	} else {
		data1 = param_values->data[0].v_pointer;
		data2 = closure->data;
	}
	callback = (GMarshalFunc_INT__POINTER_STRING) (marshal_data ? marshal_data : cc->callback);
	v_return = callback (data1, g_value_get_pointer (param_values + 1), g_value_get_string (param_values + 2), data2);
	g_value_set_int (return_value, v_return);
}



