typedef struct keystroke {
    char sname;
    char *options;
    char *help;
    void (*hook) (char *input);
} keystroke_t;

#define CMD_NAME(hook) cmd_ ## hook
#define CMD_DECL(hook) void cmd_ ## hook (char *input)
#define COMMAND(sn,opt,help,hook) {sn, opt, help, cmd_ ## hook}

void commands_parse (char *cmdline);
