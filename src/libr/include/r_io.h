#ifndef _LIB_R_IO_H_
#define _LIB_R_IO_H_

#include "r_types.h"

/* io/handle.c */
int r_io_handle_open();
int r_io_handle_generate();
int r_io_handle_close();

/* io/io.c */
int r_io_init();
int r_io_open(const char *file, int flags);
int r_io_read(int fd, u8 *buf, int len);
int r_io_write(int fd, const u8 *buf, int len);
int r_io_system(int fd, const char *cmd);
int r_io_close(int fd);

/* io/map.c */
void r_io_map_init();
int r_io_map_rm(const char *file);
int r_io_map_list();
int r_io_map(const char *file, u64 offset);
int r_io_map_read_at(u64 off, u8 *buf, u64 len);
int r_io_map_read_rest(u64 off, u8 *buf, u64 len);

#define R_IO_READ 0
#define R_IO_WRITE 1
#define R_IO_RDWR 2

#endif
