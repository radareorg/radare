/* Reverse Engineered LibUSB-Gecko library */

/*

 vendor:product = 0403:6001

Command table
-------------
 30 getregs
 13 load
 11 screenshot
 10 set breakpoint (rwx)
  7 continue
  6 stop
  4 read
  3 write

*/

#include "grecko.h"
#include <stdio.h>

int timeout = 3000;
int ep = 2; // endpoint


// TODO should return device id to support multiple usb geckos at the time
int grecko_open()
{
        struct usb_device_descriptor udd;
        struct devices it_device;
	int    c = 0;
	static char pbc[]={'/','-','\\', '|'};

	// usb_set_debug(5);
	usb_init();

        /* Tries to get access to the Internet Tablet and retries
         * if any of the neccessary steps fail.
         *
         * Note: While a proper device may be found on the bus it may
         * not be in the right state to be accessed (e.g. the Nokia is
         * not in the boot stage any more).
         */
	while(!dev) {
		usleep(0xc350); // 0.5s
                
                if(!usb_device_found(&udd, &it_device)) {
			printf("\rWaiting for device... %c", pbc[++c%4]);
			fflush(stdout);
			continue;
		}

        	/* open device */
        	if(!(dev = usb_open(device))) {
	        	perror("usb_open");
                        return 1;
	        }

        	if ( usb_claim_interface(dev, device->config->interface->altsetting->bInterfaceNumber) < 0) { // 2 or 0
	        	perror("usb_claim_interface");

			// Something is broken if closing fails.
			if(usb_close(dev)) {
				perror("usb_close");
				return 1;
			}

			dev = NULL;

			sleep(1);
			// Try again later.
			continue;
        	}

		//if (usb_set_altinterface(dev, 1) < 0) {
        	if ( usb_set_altinterface(dev, device->config->interface->altsetting->bAlternateSetting) < 0) { // 2 or 0
			perror("usb_set_altinterface");

			// Something is broken if closing fails.
			if(usb_close(dev)) {
				perror("usb_close");
				return 1;
			}

			dev = NULL;
			// Try again later.
			continue;
		}
	}

        printf("found %s (%04x:%04x)\n", it_device.name,
		it_device.vendor_id, it_device.product_id);

	sleep(1); // take breath

	return 0;
}

int grecko_close()
{
	usb_close(dev);
}

/* load gamecube game from cdrom */
void grecko_send_command(unsigned char cmd)
{
	int ret;
	/*
	int ret2 = usb_control_msg(dev, cmd, 1, 0, 0, (char *)&ret, 4, 2000);

	if (ret2==-1 || ret == -1) {
		printf(stderr, "Oops\n");
		return;
	}
	*/
	usb_bulk_write(dev, ep, &cmd, 1, timeout);
}

void grecko_load()
{
	grecko_send_command(13);
}

/* freeze console */
void grecko_stop()
{
	grecko_send_command(6);
}

/* unfreeze console */
void grecko_continue()
{
	grecko_send_command(7);
}

int grecko_write(unsigned long addr, unsigned char *buf, int bsize)
{
	unsigned char data[8];
	unsigned char *ptr = &addr;
	int i, size = bsize - bsize%4;

	grecko_send_command(3);

	for(i=0; i<size; i+=4) {
		data[0]=ptr[3]; data[1]=ptr[2]; data[2]=ptr[1]; data[3]=ptr[0];
		data[4]=buf[i]; data[5]=buf[i+1]; data[6]=buf[i+2]; data[7]=buf[i+3];
		usb_bulk_write(dev, ep, &data, 8, timeout);
	}

	return size; // may be different than bsize
}

int grecko_read(unsigned long addr, unsigned char *buf, int size)
{
	grecko_send_command(4);
}

void grecko_getregs()
{
	grecko_send_command(30);
	// read 0xa6 bytes here - 6 so..0xa0 = 160 bytes = 40 registers of 32 bits
	/*
		r0 - r31    32
		cr           1
		dsisr        1
		srr0, srr1   2
		xer          1
		ctr          1
		dar          1
		lr           1

		= 40 registers :D
	*/
	
}

int grecko_bp(unsigned long addr, int type)
{
	unsigned char *ptr = &addr;

	addr -= (addr%4) + type;
	grecko_send_command(10);
	addr = htonl(addr);

	return usb_bulk_write(dev, ep, &addr, 4, timeout);
}
