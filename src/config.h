#ifndef _INCLUDE_CONFIG_H_
#define _INCLUDE_CONFIG_H_

#include "radare.h"
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "rdb/rdb.h"
#include "list.h"

#define VISUAL_PROMPT ":> "
#define DEFAULT_BLOCK_SIZE 512

enum mode_t {
	MODE_SHELL   = 0,
	MODE_HEXDUMP = 2,
	MODE_STRINGS = 3
};

struct zoom_t {
	off_t size; // full file size
	off_t from; // start (default is 0)
	off_t piece; // piece size (config.size/block_size)
	int enabled;
};

struct config_t {
	mode_t mode;
	int fd;
	int width;
	int height;
	int lang;
	int color;
	int debug;
	int assume_yes;
	int endian;
	int buf; // scr.buf
	/* modes */
	int visual;
	int cursor_mode; // visual like toggle
	/* ... */
	int noscript;
	char *script;
	int skip;    // conflict with noscript?? imho no
	int ene;
	int interrupted;
	int verbose;
	int unksize;
	char *file;
	unsigned char *block; // data block
	size_t block_size; // size of the data block
	size_t cursor; // position of the cursor inside the block
	size_t ocursor; // position of the cursor inside the block
	off_t size;
	off_t baddr;
	off_t seek;
	off_t last_seek;
	off_t limit;
	struct zoom_t zoom;
	struct list_head rdbs; // linked list with all opened rdbs
};

#define D if (config.verbose)
#define C if (config.color)
#define V if (config.visual)
extern struct config_t config;

int rdb_init();
void config_init();
void config_fit_block_size();
void radare_set_block_size(char *arg);
void radare_set_block_size_i(size_t i);

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
	off_t i_value;
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
int config_bsize_callback(void *data);
int config_zoombyte_callback(void *data);
void config_eval(char *str);
struct config_node_t *config_set_i(const char *name, const off_t i);
int config_rm(const char *name);
struct config_node_t *config_set(const char *name, const char *value);
off_t config_get_i(const char *name);
const char *config_get(const char *name);
void config_list(char *str);
struct config_node_t *config_node_get(char *name);
struct config_node_t *config_node_new(char *name, char *value);
int strhash(char *str);

extern struct config_new_t config_new;

#endif