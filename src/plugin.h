#ifndef _INCLUDE_PLUGIN_H_
#define _INCLUDE_PLUGIN_H_

#include "main.h"
#include "utils.h"
#include <dlfcn.h>

#define FIND_FD(x) int i=0; if ( x ==-1 ) return -1; for(i=0; plugins[i].name && !plugins[i].handle_fd( x ); i++);
#define FIND_OPEN(y) int i=0; for(i=0; plugins[i].name && !plugins[i].handle_open( y ); i++);
#define IF_HANDLED(x,y) if (( x != -1) && (plugins[i].name != NULL) && plugins[i].y )

typedef struct debug_t {
	/* TODO: add function pointers in dbg/os.h here */
} debug_t;

typedef struct plugin_t {
	void *handle;
	char *name;
	char *desc;
	int (*init)();
	struct debug_t *debug;
	int (*system)(const char *);
	int (*open)(const char *, int, mode_t);
	ssize_t (*read)(int fd, void *buf, size_t count);
	off_t (*lseek)(int fildes, off_t offset, int whence);
	ssize_t (*write)(int fd, const void *buf, size_t count);
	int (*close)(int fd);
	int (*handle_open)(const char *);
	int (*handle_fd)(int);
} plugin_t;

extern plugin_t plugins[10];
extern plugin_t posix_plugin;
extern plugin_t haret_plugin;
extern plugin_t remote_plugin;
extern plugin_t winedbg_plugin;
extern plugin_t gxemul_plugin;
extern plugin_t debug_plugin;

#if __linux__
extern plugin_t sysproxy_plugin;
#endif

#if DEBUGGER
extern plugin_t gdb_plugin; //
#endif

#if HAVE_LIB_EWF
extern plugin_t ewf_plugin; //
#endif

#if __WIN32__
extern plugin_t w32_plugin;
#endif

/* functions */
plugin_t *plugin_registry(const char *file);
void plugin_init();
int  plugin_list();

/* io functions */
int     io_open   (const char *pathname, int flags, mode_t mode);
ssize_t io_read   (int fd, void *buf, size_t count);
off_t   io_lseek  (int fd, off_t offset, int whence);
ssize_t io_write  (int fd, const void *buf, size_t count);
int     io_isdbg  (int fd);
int     io_close  (int fd);
int     io_system (const char *command);

#endif
