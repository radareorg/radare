#include "r_types.h"
#include "r_io.h"
#include "r_config.h"

struct r_core_t {
	u64 seek;
	u64 size;
	u64 blocksize;
	struct r_io_t io;
	struct r_config_t config;
};
