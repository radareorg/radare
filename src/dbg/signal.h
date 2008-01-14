#ifndef SIGNAL_H
#define SIGNAL_H

/* signal or exception entry */
struct sig {
  int sig;
  char *name;
  char *string;
};

#endif
