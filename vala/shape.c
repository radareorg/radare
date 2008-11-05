
#include "shape.h"








GType grava_shape_get_type (void) {
	static GType grava_shape_type_id = 0;
	if (G_UNLIKELY (grava_shape_type_id == 0)) {
		static const GEnumValue values[] = {{GRAVA_SHAPE_RECTANGLE, "GRAVA_SHAPE_RECTANGLE", "rectangle"}, {GRAVA_SHAPE_CIRCLE, "GRAVA_SHAPE_CIRCLE", "circle"}, {0, NULL, NULL}};
		grava_shape_type_id = g_enum_register_static ("GravaShape", values);
	}
	return grava_shape_type_id;
}




