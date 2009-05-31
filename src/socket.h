#ifndef _INCLUDE_SOCKET_H_
#define _INCLUDE_SOCKET_H_

int socket_ready(int fd, int secs, int usecs);
int socket_read(int fd, unsigned char *read, int len);
int socket_write(int fd, unsigned char *buf, int len);
int socket_udp_connect(char *host, int port);
int socket_connect(char *host, int port);
int socket_listen(int port);
int socket_fgets(int fd, char *buf, int size);
void socket_printf(int fd, const char *fmt, ...);

#endif
