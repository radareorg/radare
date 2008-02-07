unsigned char gdb_hash(char *str);
int gdb_send(char *cmd);
int hex_to_int(char ch);
int gdb_read_bytes(char *buf, int bytes);
int gdb_handle_fd(int fd);
int gdb_open(char *file, int mode, int flags);
int gdb_handle_open(const char *file);
int gdb_seek(int fd, int offset, int whence);
void gdb_system(char *str);
void gdb_init();
void gdb_close();

struct gdbps {
	int fd;         /* related metadata */
	int verbose;
	pid_t pid;
	char *filename;
	off_t offset;
	off_t ldentry;
	off_t entrypoint;
	off_t pc;       /*/ program counter */
	int isbpaddr;
	int opened;
	int steps;
	int is_file;
	char *args;
	char *argv[256];
};
