#ifndef _INCLUDE_CONFIG_H_
#define _INCLUDE_CONFIG_H_

#include "radare.h"
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "rdb.h"
#include "list.h"

#define VISUAL_PROMPT ":> "
#define DEFAULT_BLOCK_SIZE 512

enum mode_t {
	MODE_SHELL   = 0,
	MODE_HEXDUMP = 2,
	MODE_STRINGS = 3
};

struct zoom_t {
	ut64 to; // full file size
	ut64 from; // start (default is 0)
	ut64 piece; // piece size (config.size/block_size)
	int enabled;
};

struct config_t {
	mode_t mode;
	int fd;
	int width;
	int height;
	int lines;
	int lang;
	int color;
	int debug;
	int assume_yes;
	int arch;
	int endian;
	int buf; // scr.buf
	/* modes */
	int visual;
	int scrdelta;
	int graph;
	int scrfit;
	int cursor_mode; // visual cursor toggle
	int insert_mode; // visual insert toggle
	/* ... */
	int noscript;
	char *script[10];
	int skip;    // conflict with noscript?? imho no
	int ene;
	int interrupted;
	int verbose;
	int unksize;
	char *file;
	int realfile;
	unsigned char *block; // data block
	int block_size; // size of the data block // why signed ?
	int cursor; // position of the cursor inside the block XXX THIS IS UNSIGNED :O -1 must be funny
	ut64 cursor_ptr; // where arch_aop makes point with cursor
	int acursor; // position of the cursor inside the block
	int ocursor; // position of the cursor inside the block
	ut64 size;
	ut64 vaddr; // virtual addr
	ut64 paddr; // physical addr
	ut64 seek;
	ut64 last_seek;
	ut64 limit;
	ut64 last_cmp;
	struct zoom_t zoom;
	struct list_head rdbs; // linked list with all opened rdbs
};


#define D if (config.verbose)
#define C if (config.color)
#define V if (config.visual)
extern struct config_t config;
extern int config_get_notfound;

int rdb_init();
void config_init(int first);
void config_fit_block_size();

/* NEW STFU */

#define CN_BOOL  0x000001
#define CN_INT   0x000002
#define CN_OFFT  0x000004
#define CN_STR   0x000008
#define CN_RO    0x000010
#define CN_RW    0x000020

struct config_node_t {
	char *name;
	int hash; /* hash of the name - optimized search */
	int flags;
	char *value;
	ut64 i_value;
	int (*callback)(void *data);
	struct list_head list;
};

struct config_new_t {
	int lock;
	int n_nodes;
	struct list_head nodes;
};

void config_init();
void config_lock(int l);
//int config_bsize_callback(void *data);
//int config_zoombyte_callback(void *data);
void config_eval(char *str);
struct config_node_t *config_set_i(const char *name, const ut64 i);
int config_rm(const char *name);
struct config_node_t *config_set(const char *name, const char *value);
ut64 config_get_i(const char *name);
const char *config_get(const char *name);
void config_list(const char *str);
struct config_node_t *config_node_get(const char *name);
struct config_node_t *config_node_new(const char *name, const char *value);

extern struct config_new_t config_new;

#endif
