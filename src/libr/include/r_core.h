#include "r_types.h"
#include "r_io.h"
#include "r_config.h"

#define R_CORE_BLOCKSIZE 512

struct r_core_t {
	u64 seek;
	u64 size;
	u32 blocksize;
	u8 *block;
//	struct r_io_t io;
	struct r_config_t config;
};
