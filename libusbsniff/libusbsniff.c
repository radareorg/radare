/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
/*
 * LIBUSBSNF : Sniffer wrapper library for libusb
 * TODO: also dump the return value of the functions
 * support for replaying streams
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
//#include <usb.h>
#include <stdlib.h>
#include "hexdump.h"
#include <unistd.h>
#include <limits.h>

#if 1
struct usb_device_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int16_t bcdUSB;
	u_int8_t  bDeviceClass;
	u_int8_t  bDeviceSubClass;
	u_int8_t  bDeviceProtocol;
	u_int8_t  bMaxPacketSize0;
	u_int16_t idVendor;
	u_int16_t idProduct;
	u_int16_t bcdDevice;
	u_int8_t  iManufacturer;
	u_int8_t  iProduct;
	u_int8_t  iSerialNumber;
	u_int8_t  bNumConfigurations;
};
struct usb_device {
  struct usb_device *next, *prev;

  char filename[PATH_MAX + 1];

  struct usb_bus *bus;

  struct usb_device_descriptor descriptor;
  struct usb_config_descriptor *config;

  void *dev;		/* Darwin support */

  u_int8_t devnum;

  unsigned char num_children;
  struct usb_device **children;
};
struct usb_dev_handle;
typedef struct usb_dev_handle usb_dev_handle;
#endif

/* symbol descriptors */
usb_dev_handle * (*__usb_open)(struct usb_device *dev);
int (*__usb_close)(usb_dev_handle *dev);
int (*__usb_find_busses)();
int (*__usb_claim_interface)(usb_dev_handle *dev, int interface);
int (*__usb_set_altinterface)(usb_dev_handle *dev, int alternate);
int (*__usb_control_msg)(usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout);
int (*__usb_bulk_write)(usb_dev_handle *dev, int ep, const char *bytes, int size, int timeout);
int (*__usb_bulk_read)(usb_dev_handle *dev, int ep, char *bytes, int size, int timeout);

/*
 * Library initialization
 */
static void _libwrap_init() __attribute__ ((constructor));
static void _libwrap_init()
{   
  __usb_open             = dlsym(RTLD_NEXT, "usb_open");
  __usb_close            = dlsym(RTLD_NEXT, "usb_close");
  __usb_find_busses      = dlsym(RTLD_NEXT, "usb_find_busses");
  __usb_set_altinterface = dlsym(RTLD_NEXT, "usb_set_altinterface");
  __usb_claim_interface  = dlsym(RTLD_NEXT, "usb_claim_interface");
  __usb_control_msg      = dlsym(RTLD_NEXT, "usb_control_msg");
  __usb_bulk_write       = dlsym(RTLD_NEXT, "usb_bulk_write");
  __usb_bulk_read        = dlsym(RTLD_NEXT, "usb_bulk_read");
}

static void _libwrap_fini() __attribute__ ((destructor));
static void _libwrap_fini()
{
	/* do something here */
}

usb_dev_handle *usb_open(struct usb_device *dev)
{
	usb_dev_handle *ret;
	fprintf(stderr, "USBSNF: usb_open(%s, %d);\n", dev->filename, dev->devnum);
	ret = __usb_open(dev);
	fprintf(stderr, "USBSNF:   return       = %x\n", (int)ret);
	return ret;
}

int usb_find_busses()
{
	int ret;
	fprintf(stderr, "USBSNF: usb_find_busses()\n");
	ret = __usb_find_busses();
	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}

int usb_close(struct usb_dev_handle *dev)
{
	int ret;
	fprintf(stderr, "USBSNF: usb_close()\n");
	fprintf(stderr, "USBSNF:   handle       = %x\n", (unsigned int)dev);
	ret = __usb_close(dev);
	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}

int usb_bulk_write(usb_dev_handle *dev, int ep, const char *bytes, int size, int timeout)
{
	int ret;
	fprintf(stderr, "USBSNF: usb_bulk_write()\n");
	fprintf(stderr, "USBSNF:   handle       = %x\n", (unsigned int)dev);
	fprintf(stderr, "USBSNF:   ep           = %d\n", ep);
	fprintf(stderr, "USBSNF:   size         = %d\n", size);
	fprintf(stderr, "USBSNF:   timeout      = %d\n", timeout);
	ret = __usb_bulk_write(dev, ep, bytes, size, timeout);
	dump_bytes((unsigned char*)bytes, ret);
	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}

int usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size, int timeout)
{
	int ret;
	fprintf(stderr, "USBSNF: usb_bulk_read()\n");
	fprintf(stderr, "USBSNF:   handle       = %x\n", (unsigned int)dev);
	fprintf(stderr, "USBSNF:   ep           = %d\n", ep);
	fprintf(stderr, "USBSNF:   size         = %d\n", size);
	fprintf(stderr, "USBSNF:   timeout      = %d\n", timeout);

	ret = __usb_bulk_read(dev, ep, bytes, size, timeout);
	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}

int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
	int value, int index, char *bytes, int size, int timeout)
{
	int ret;
	fprintf(stderr, "USBSNF: usb_control_msg()\n");
	fprintf(stderr, "USBSNF:   handle       = %x\n", (unsigned int)dev);
	fprintf(stderr, "USBSNF:   request_type = %d (%s)\n", requesttype, (requesttype==64)?"read":"write");
	fprintf(stderr, "USBSNF:   request      = %d\n", request);
	fprintf(stderr, "USBSNF:   value        = %d\n", value);
	fprintf(stderr, "USBSNF:   index        = %d\n", index);
	fprintf(stderr, "USBSNF:   size         = %d\n", size);
	fprintf(stderr, "USBSNF:   timeout      = %d\n", timeout);

	if (requesttype == 64) // write
		dump_bytes((unsigned char*)bytes, size);

	ret = __usb_control_msg(dev, requesttype, request, value, index, bytes, size, timeout);

	if (requesttype == 192) // read
		dump_bytes((unsigned char*)bytes, size);

	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}

int usb_claim_interface(usb_dev_handle *dev, int interface)
{
	int ret;
	fprintf(stderr, "USBSNF: usb_claim_interface()\n");
	fprintf(stderr, "USBSNF:   handle       = %x\n", (unsigned int)dev);
	fprintf(stderr, "USBSNF:   interface    = %d\n", interface);

	ret = __usb_claim_interface(dev, interface);
	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}

int usb_set_altinterface(usb_dev_handle *dev, int alternate)
{
	int ret;
	fprintf(stderr, "USBSNF: usb_set_altinterface()\n");
	fprintf(stderr, "USBSNF:   handle       = %x\n", (unsigned int)dev);
	fprintf(stderr, "USBSNF:   alternate    = %d\n", alternate);

	ret = __usb_set_altinterface(dev, alternate);
	fprintf(stderr, "USBSNF:   return       = %d\n", ret);
	return ret;
}
