
#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define GRAVA_TYPE_SHAPE (grava_shape_get_type ())

typedef enum  {
	GRAVA_SHAPE_RECTANGLE = 0,
	GRAVA_SHAPE_CIRCLE
} GravaShape;


GType grava_shape_get_type (void);


G_END_DECLS

#endif
