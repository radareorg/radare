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

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

G_BEGIN_DECLS


#define GRAVA_TYPE_WIDGET (grava_widget_get_type ())
#define GRAVA_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRAVA_TYPE_WIDGET, GravaWidget))
#define GRAVA_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRAVA_TYPE_WIDGET, GravaWidgetClass))
#define GRAVA_IS_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRAVA_TYPE_WIDGET))
#define GRAVA_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRAVA_TYPE_WIDGET))
#define GRAVA_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRAVA_TYPE_WIDGET, GravaWidgetClass))

typedef struct _GravaWidget GravaWidget;
typedef struct _GravaWidgetClass GravaWidgetClass;
typedef struct _GravaWidgetPrivate GravaWidgetPrivate;

struct _GravaWidget {
	GObject parent;
	GravaWidgetPrivate * priv;
	GtkDrawingArea* da;
	GravaGraph* graph;
};
struct _GravaWidgetClass {
	GObjectClass parent;
};

#define GRAVA_WIDGET_S 96
GtkWidget* grava_widget_get_widget (GravaWidget* self);
void grava_widget_create_widgets (GravaWidget* self);
void grava_widget_do_popup_menu (GravaWidget* self);
void core_load_graph_at_label (const char* addr);
GravaWidget* grava_widget_new (void);
GType grava_widget_get_type (void);

G_END_DECLS

#endif
