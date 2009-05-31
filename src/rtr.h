#ifndef _INCLUDE_RTR_H_
#define _INCLUDE_RTR_H_

#define RTR_PROT_RAP 0
#define RTR_PROT_TCP 1
#define RTR_PROT_UDP 2

#define RTR_RAP_OPEN   0x01
#define RTR_RAP_CMD    0x07
#define RTR_RAP_REPLY  0x80

#define RTR_MAX_HOSTS 255

struct rtr_host_t {
	int proto;
	char host[512];
	int port;
	char file[1024];
	int fd;
};

void rtr_help();
void rtr_list();
void rtr_add(char *input);
void rtr_remove(char *input);
void rtr_session(char *input);
void rtr_cmd(char *input);
void rtr_pushout(const char *input);

#endif

