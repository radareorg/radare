#ifndef _LIB_R_IO_H_
#define _LIB_R_IO_H_

#include "r_types.h"

void *r_lib_open(const char *libname);
void *r_lib_sym(void *handle, const char *name);
int r_lib_close(void *handle);

#endif
