
struct syscall_t {
  char *name;
  int num;
  int args;
};

extern struct syscall_t syscalls_netbsd_x86[];
struct syscall_t syscalls_linux_x86[];
struct syscall_t syscalls_freebsd_x86[];
