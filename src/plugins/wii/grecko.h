#ifndef _INCLUDE_GRECKO_H_
#define _INCLUDE_GRECKO_H_

#include "usbdev.h"

enum {
	BPX = 3,
	BPR = 5,
	BPW = 6
};

int grecko_bp(unsigned long addr, int type);
void grecko_getregs();
int grecko_read(unsigned long addr, unsigned char *buf, int size);
int grecko_write(unsigned long addr, unsigned char *buf, int bsize);
void grecko_continue();
void grecko_stop();
void grecko_load();
int grecko_open();
void grecko_send_command(unsigned char cmd);

#endif
