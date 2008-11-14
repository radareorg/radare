#ifndef _LIB_R_IO_H_
#define _LIB_R_IO_H_

#include "r_types.h"

/* io/handle.c */
int r_io_handle_open();
int r_io_handle_generate();
int r_io_handle_close();

/* io/io.c */
int r_io_open(const char *file, int flags);
int r_io_read(int fd, u8 *buf, int len);
int r_io_write(int fd, const u8 *buf, int len);
int r_io_system(int fd, const char *cmd);
int r_io_close(int fd);

#endif
