//x86_64.c
#define ull unsigned long long

long long arch_syscall(int pid, int sc, ...);
int debug_dr(char *cmd);
int arch_is_jump(unsigned char *buf);
off_t arch_get_entrypoint();
int arch_jmp(off_t ptr);
int arch_dump_registers();
int arch_opcode_size();
int arch_restore_registers();
int arch_inject(unsigned char *data, int size);
off_t arch_pc();
int arch_backtrace();
void dump_eflags(const int eflags);
int arch_ret();
int arch_call(char *arg);
int arch_print_fpregisters(int rad, const char *mask);
int arch_print_syscall();
int arch_print_registers(int rad, const char *mask);
long long get_value(char *str);
int arch_set_register(char *reg, char *value);
int arch_continue();
void *arch_mmap(int fd, int size, off_t addr);
void *arch_alloc_page(int size, int *rsize);
void *arch_dealloc_page(void *addr, int size);
void *arch_get_sighandler(int signum);
int arch_mprotect(char *addr, unsigned int size, int perms);
void *arch_set_sighandler(int signum, off_t handler);
int arch_is_jmp(const unsigned char *cmd, ull *addr);
int arch_is_call(const char *cmd);
int arch_is_soft_stepoverable(const char *cmd);
int arch_is_stepoverable(const unsigned char *cmd);

//x86_64-bp.c
static unsigned long dr_get (int reg);
static int dr_set (int reg, unsigned long val);
static unsigned long dr_get(int reg);
static int dr_set(int reg, unsigned long val);
static inline void dr_set_control (unsigned long control);
static inline unsigned dr_get_control ();
static inline void dr_set_addr (int regnum, unsigned long addr);
static inline void dr_reset_addr (int regnum);
static inline unsigned long dr_get_status (void);
static int arch_bp_hw_state(unsigned long addr, int enable);
void dr_init();
void dr_list();
int arch_set_wp_hw_n(int dr_free, unsigned long addr, int type);
int arch_set_wp_hw(unsigned long addr, int type);
int arch_set_bp_hw(struct bp_t *bp, unsigned long addr);
int arch_rm_bp_hw(struct bp_t *bp);
int get_len_ins(char *buf, int len);
int arch_set_bp_soft(struct bp_t *bp, unsigned long addr);
int arch_rm_bp_soft(struct bp_t *bp);

//debug.c
int inline debug_getregs(pid_t pid, regs_t *reg);
int inline debug_setregs(pid_t pid, regs_t *reg);
