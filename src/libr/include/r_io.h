#ifndef _LIB_R_IO_H_
#define _LIB_R_IO_H_

#include "r_types.h"

#define R_IO_READ 0
#define R_IO_WRITE 1
#define R_IO_RDWR 2

#define R_IO_NFDS 32

#define R_IO_SEEK_SET 0
#define R_IO_SEEK_CUR 1
#define R_IO_SEEK_END 2

struct r_io_handle_t {
        void *handle;
        char *name;
        char *desc;
        void *widget;
        int (*init)();
        struct debug_t *debug;
        int (*system)(const char *);
        int (*open)(const char *, int rw, int mode);
        int (*read)(int fd, u8 *buf, int count);
        u64 (*lseek)(int fildes, u64 offset, int whence);
        u32 (*write)(int fd, const u8 *buf, u32 count);
        int (*close)(int fd);
        int (*handle_open)(const char *);
        int (*handle_fd)(int);
	int fds[R_IO_NFDS];
};

/* io/handle.c */
extern u64 r_io_seek;
int r_io_handle_init();
int r_io_handle_open(int fd, struct r_io_handle_t *plugin);
int r_io_handle_close(int fd, struct r_io_handle_t *plugin);
int r_io_handle_generate();
int r_io_handle_add(struct r_io_handle_t *plugin);
// TODO: _del ??
struct r_io_handle_t *r_io_handle_resolve(const char *filename);
struct r_io_handle_t *r_io_handle_resolve_fd(int fd);

/* io/io.c */
int r_io_init();
int r_io_set_write_mask(int fd, const u8 *buf, int len);
int r_io_open(const char *file, int flags, int mode);
int r_io_read(int fd, u8 *buf, int len);
int r_io_write(int fd, const u8 *buf, int len);
u64 r_io_lseek(int fd, u64 offset, int whence);
int r_io_system(int fd, const char *cmd);
int r_io_close(int fd);
u64 r_io_size(int fd);

/* io/map.c */
void r_io_map_init();
int r_io_map_rm(int fd);
int r_io_map_list();
int r_io_map(const char *file, u64 offset);
int r_io_map_read_at(u64 off, u8 *buf, u64 len);
int r_io_map_read_rest(u64 off, u8 *buf, u64 len);
int r_io_map_write_at(u64 off, const u8 *buf, u64 len);

/* io/section.c */
u64 r_io_section_align(u64 addr, u64 vaddr, u64 paddr);

#endif
