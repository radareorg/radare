/* Reverse Engineered LibUSB-Gecko library */
/*
 *  radare - Open Free Fiasco Firmware Flasher
 *  Copyright (C) 2007  pancake <pancake@youterm.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "grecko.h"
#include <sys/types.h>
#include <signal.h>

int cc() {
	printf("^C\n");
	grecko_close();
	exit(0);
}

int main(int argc, char **argv)
{
	int c;
	/* open usb device */
	if (grecko_open()<0) {
		printf("Is usbgecko connected and detected? try re-plug in another hole ;)\n");
		return 1;
	}

	/* load game */
	grecko_load();

	printf("press intro to start zapping!\n");
	read(0,&c,1);
	signal(SIGINT, (void *)cc);

	while (1) {
		/* freeze */
		grecko_stop();
		printf(" - stop!\n");
		sleep(2);

		/* unfreeze */
		grecko_continue();
		printf(" + continue!\n");
		sleep(2);
	}
	return 0;
}
