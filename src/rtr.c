#include "main.h"
#include "rtr.h"

static int rtr_n = 0;
static struct rtr_host_t rtr_host[RTR_MAX_HOSTS];

void rtr_help()
{
	cons_printf(
	" =                  ; list all open connections\n"
	" =<[fd] cmd         ; send output of local command to remote fd.\n"
	" =[fd] cmd          ; exec cmd at remote 'fd' (last open is default one)\n"
	" =+ [proto://]host  ; add host (default=rap://, tcp://, udp://)\n"
	" =-[fd]             ; remove all hosts or host 'fd'\n"
	" ==[fd]             ; open remote session with host 'fd', 'q' to quit.\n");
}

void rtr_pushout(const char *input)
{
	int fd = atoi(input);
	char *str = NULL, *cmd = NULL;
	if (fd != 0) {
		for (rtr_n = 0; rtr_host[rtr_n].fd != fd && rtr_n < RTR_MAX_HOSTS; rtr_n++);
		if (!(cmd = strchr(input, ' '))) {
			eprintf("Error\n");
			return;
		}
	} else
		cmd = input;
	str = radare_cmd_str(cmd);
	if (str == NULL) {
		eprintf("Error: radare_cmd_str returned NULL\n");
		return;
	}
	
	switch(rtr_host[rtr_n].proto) {
	case RTR_PROT_RAP:
		eprintf("Error: Cannot use '=<' to a rap connection.\n");
		break;
	case RTR_PROT_TCP:
	case RTR_PROT_UDP:
	default:
		socket_write(rtr_host[rtr_n].fd, str, strlen(str));
		break;
	}
	free(str);
}

void rtr_list()
{
	int i;
	for (i = 0; i < RTR_MAX_HOSTS; i++)
		if (rtr_host[i].fd) {
			cons_printf("%i - ", rtr_host[i].fd);
			if (rtr_host[i].proto == RTR_PROT_TCP)
				cons_printf("tcp://");
			else if (rtr_host[i].proto == RTR_PROT_UDP)
				cons_printf("udp://");
			else cons_printf("rap://");
			cons_printf("%s:%i/%s\n", rtr_host[i].host,
				rtr_host[i].port, rtr_host[i].file);
	}
}
		
void rtr_add(char *input)
{
	char *host = NULL, *file = NULL, *ptr = NULL, buf[1024];
	int proto, port, fd, i;

	/* Parse uri */
	if ((ptr = strstr(input, "tcp://"))) {
		proto = RTR_PROT_TCP;
		host = ptr+6;
	} else if ((ptr = strstr(input, "udp://"))) {
		proto = RTR_PROT_UDP;
		host = ptr+6;
	} else if ((ptr = strstr(input, "rap://"))) {
		proto = RTR_PROT_RAP;
		host = ptr+6;
	} else {
		proto = RTR_PROT_RAP;
		host = input;
	}
	while(host[0]&&iswhitechar(host[0]))
		host = host + 1;

	if (!(ptr = strchr(host, ':'))) {
		eprintf("Error: Port is not specified\n");
		return;
	}
	ptr[0] = '\0';
	ptr = ptr+1;

	if (!(file = strchr(ptr, '/'))) {
		eprintf("Error: File is not specified\n");
		return;
	}
	file[0] = '\0';
	file = file+1;

	port = get_math(ptr);

	switch (proto) {
	case RTR_PROT_RAP:
		fd = socket_connect(host, port);
		if (fd == -1) {
			eprintf("Error: Cannot connect to '%s' (%d)\n", host, port);
			return;
		}
		eprintf("Connected to: %s at port %d\n", host, port);
		/* send */
		buf[0] = RTR_RAP_OPEN;
		buf[1] = 0;
		buf[2] = (uchar)(strlen(file)+1);
		memcpy(buf+3, file, buf[2]);
		write(fd, buf, 3+buf[2]);
		/* read */
		eprintf("waiting... "); fflush(stdout);
		read(fd, buf, 5);
		endian_memcpy((uchar *)&i, (uchar*)buf+1, 4);
		if (buf[0] != (char)(RTR_RAP_OPEN|RTR_RAP_REPLY) || i<= 0) {
			eprintf("Error: Wrong reply\n");
			return;
		}
		eprintf("ok\n");
		break;
	case RTR_PROT_TCP:
		fd = socket_connect(host, port);
		if (fd == -1) {
			eprintf("Error: Cannot connect to '%s' (%d)\n", host, port);
			return;
		}
		eprintf("Connected to: %s at port %d\n", host, port);
		break;
	case RTR_PROT_UDP:
		eprintf("Error: Not supported\n");
		return;
		break;
	}

	for (i = 0; i < RTR_MAX_HOSTS; i++)
		if (!rtr_host[i].fd) {
			rtr_host[i].proto = proto;
			memcpy(rtr_host[i].host, host, 512);
			rtr_host[i].port = port;
			memcpy(rtr_host[i].file, file, 1024);
			rtr_host[i].fd = fd;
			rtr_n = i;
			break;
		}

	rtr_list();
}

void rtr_remove(char *input)
{
	int fd, i;

	if (input[0] >= '0' && input[0] <= '9') {
		fd = get_math(input);
		for (i = 0; i < RTR_MAX_HOSTS; i++)
			if (rtr_host[i].fd == fd) {
				socket_close(rtr_host[i].fd);
				rtr_host[i].fd = 0;
				if (rtr_n == i)
					for (rtr_n = 0; !rtr_host[rtr_n].fd && rtr_n < RTR_MAX_HOSTS; rtr_n++);
				break;
		}
	} else {
		for (i = 0; i < RTR_MAX_HOSTS; i++)
			if (rtr_host[i].fd)
				socket_close(rtr_host[i].fd);
		memset(rtr_host, '\0', RTR_MAX_HOSTS * sizeof(struct rtr_host_t));
		rtr_n = 0;
	}
}

void rtr_session(char *input)
{
	char prompt[32], buf[4096];
	int fd;

	if (input[0] >= '0' && input[0] <= '9') {
		fd = get_math(input);
		for (rtr_n = 0; rtr_host[rtr_n].fd != fd && rtr_n < RTR_MAX_HOSTS; rtr_n++);
	}

	while (1) {
		snprintf(prompt, 32, "fd:%d> ", rtr_host[rtr_n].fd);
		dl_prompt = prompt;
		if((cons_fgets(buf, 4095, 0, NULL))) {
			if (*buf == 'q')
				break;
			else if (*buf == 'V') {
				eprintf("Visual mode not supported\n");
				continue;
			}
			rtr_cmd(buf);
			cons_flush();
		}
	}
}

void rtr_cmd(char *input)
{
	char bufw[1024], bufr[16];
	char *cmd = NULL, *cmd_output = NULL;
	int i, fd = atoi(input);
	u64 cmd_len;

	if (fd != 0) {
		for (rtr_n = 0; rtr_host[rtr_n].fd != fd && rtr_n < RTR_MAX_HOSTS; rtr_n++);
		if (!(cmd = strchr(input, ' '))) {
			eprintf("Error\n");
			return;
		}
	} else
		cmd = input;

	if (!rtr_host[rtr_n].fd){
		eprintf("Error: Unknown host\n");
		return;
	}

	if (!rtr_host[rtr_n].proto == RTR_PROT_RAP){
		eprintf("Error: Not a rap:// host\n");
		return;
	}

	/* send */
	bufw[0] = RTR_RAP_CMD;
	i = strlen(cmd) + 1;
	endian_memcpy(bufw+1, (uchar*)&i, 4);
	memcpy(bufw+5, cmd, i);
	write(rtr_host[rtr_n].fd, bufw, 5+i);
	/* read */
	read(rtr_host[rtr_n].fd, bufr, 9);
	if (bufr[0] != (char)(RTR_RAP_CMD|RTR_RAP_REPLY)) {
		eprintf("Error: Wrong reply\n");
		return;
	}
	endian_memcpy((uchar*)&cmd_len, bufr+1, 8);
	if (i == 0)
		return;
	if (i < 0) {
		eprintf("Error: cmd length < 0\n");
		return;
	}
	cmd_output = malloc(cmd_len);
	if (!cmd_output) {
		eprintf("Error: Allocating cmd output\n");
		return;
	}
	read(rtr_host[rtr_n].fd, cmd_output, cmd_len);
	cons_printf("%s\n", cmd_output);
	free(cmd_output);
}
