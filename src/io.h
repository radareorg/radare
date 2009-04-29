#ifndef _INCLUDE_IO_H_
#define _INCLUDE_IO_H_

extern int io_write_last;
int io_system(const char *command);
int io_open(const char *pathname, int flags, mode_t mode);
ssize_t io_read(int fd, void *buf, size_t count);
u64 io_lseek(int fd, u64 offset, int whence);
ssize_t io_write(int fd, const void *buf, size_t count);
int io_close(int fd);
int io_isdbg(int fd);

#define IO_MAP_N 10
struct io_maps_t {
	int fd;
	char file[128];
	u64 from;
	u64 to;
	struct list_head list;
};

void io_map_init();
int io_map_rm(const char *file);
int io_map_list();
int io_map(const char *file, u64 offset);
int io_map_read_at(u64 off, u8 *buf, u64 len);
int io_map_read_rest(u64 off, u8 *buf, u64 len);

#endif
