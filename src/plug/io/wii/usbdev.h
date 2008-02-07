#ifndef _INCLUDE_USBDEV_H_
#define _INCLUDE_USBDEV_H_

#include <usb.h>

struct piece_t {
	char *name;
	char *type;
	char *vers;
};

struct devices {
  char *name;
  unsigned short vendor_id;
  unsigned short product_id;
  unsigned short flags;
};

#define SUPPORTED_DEVICES 6
extern struct devices supported_devices[SUPPORTED_DEVICES];
extern struct usb_device *device;
extern struct usb_dev_handle *dev;

#endif
