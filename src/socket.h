#ifndef _INCLUDE_SOCKET_H_
#define _INCLUDE_SOCKET_H_

int socket_connect(char *host, int port);
int socket_listen(int port);
int socket_fgets(char *buf, int size);
void socket_printf(int fd, const char *fmt, ...);

#endif
