/* $Id: usb.c,v 1.6 2004/11/13 10:53:48 dave Exp $ */

#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include "s710.h"
#include "config.h"
#ifdef HAVE_LIBUSB
#include <usb.h>

/* typedefs */

typedef struct _S710_USB_Data {
  struct usb_device *device;
  usb_dev_handle    *handle;
  int                endpoint;
  int                interface;
} _S710_USB_Data;

/* static functions */

static int find_first_altsetting ( struct usb_device *dev, 
				   int               *config, 
				   int               *interface, 
				   int               *altsetting );
static int find_endpoint         ( struct usb_device *dev, 
				   int                config, 
				   int                interface, 
				   int                altsetting, 
				   int                dir, 
				   int                type );     
#endif /* HAVE_LIBUSB */


int
init_usb_port ( S710_Driver *d, S710_Mode mode )
{
  int ret = -1;
#ifdef HAVE_LIBUSB
  struct usb_bus                  *bi;
  struct usb_device               *di;
  int                              config = -1;
  int                              interface = -1;
  int                              altsetting = -1;
  _S710_USB_Data                  *data;

  if ( d->type != S710_DRIVER_USB ) {
    fprintf(stderr,"Wrong driver type for init_usb_port\n");
    return -1;
  }

  if ( (data = calloc(1,sizeof(_S710_USB_Data))) != NULL ) {

    fprintf(stderr,"usb_set_debug(99)\n");

    usb_set_debug(99);
    usb_init();
    usb_find_busses();
    usb_find_devices();

    fprintf(stderr,"usb_find_devices is done\n");
    
    for ( bi = usb_busses; bi != NULL; bi = bi->next ) {
      for ( di = bi->devices; di != NULL; di = di->next ) {
	if ( di->descriptor.idVendor  == S710_USB_VENDOR_ID &&
	     di->descriptor.idProduct == S710_USB_PRODUCT_ID ) {
	  data->device = di;
	  fprintf(stderr,
		  "Found Polar USB interface "
		  "[Vendor %04x, ProdID %04x] on %s/%s\n",
		  S710_USB_VENDOR_ID,
		  S710_USB_PRODUCT_ID,
		  bi->dirname,
		  di->filename);
	  break;
	}
      }
      if ( data->device != NULL ) break;
    }
    
    /* if we found the device, find the endpoint and open it. */
    
    if ( data->device != NULL ) {
      find_first_altsetting(data->device,&config,&interface,&altsetting);
      data->endpoint = find_endpoint(data->device,
				     config,
				     interface,
				     altsetting,
				     USB_ENDPOINT_IN,
				     USB_ENDPOINT_TYPE_INTERRUPT);
      if ( (data->handle = usb_open(data->device)) != NULL ) {
	fprintf(stderr,"Opened device: claiming interface now\n");
	if ( usb_claim_interface(data->handle,interface) == 0 ) {
	  fprintf(stderr,"Claimed interface: ready to receive data\n");
	  data->interface = interface;
	  ret = 1;
	}
      }
    } else {
      fprintf(stderr,
	      "The Polar USB interface was not found.  Boo hoo.\n"
	      "\n"
	      "If you turned your computer on with the USB device already\n"
	      "plugged in, try unplugging it and then plugging it back in.\n"
	      "If you're on a Linux system and you can't find anything in\n"
	      "/proc/bus/usb, then you need to mount usbdevfs (as root):\n"
	      "\n"
	      "mount -t usbdevfs usbdevfs /proc/bus/usb\n"
	      "\n"
	      "Unplug the device and plug it back in, and it should show\n"
	      "up under /proc/bus/usb somewhere.  If you're on Mac OS X,\n"
	      "guess what?  You're (almost) totally out of luck.  You can't\n"
	      "use the USB attachment on Mac OS X because Polar's USB device\n"
	      "is broken in a way that Mac OS X won't forgive.  You have to\n"
	      "use Polar's serial interface and connect it to a USB-Serial\n"
	      "adapter such as the Keyspan USA-19QW.  See the following for\n"
	      "details:\n"
	      "\n"
	      "doc/README.usb\n"
	      "doc/README.osx\n"
	      "\n");
    }

    d->data = (void *)data;
  }

#endif /* HAVE_LIBUSB */

  return ret;
}


/* I need to revisit this function at some point. */

int
read_usb_byte ( S710_Driver *d, unsigned char *byte )
{
  int r = 0;
#ifdef HAVE_LIBUSB
  static char buf[BUFSIZ];
  static int  bytes;
  static int  idx;
  int         i = 0;
  _S710_USB_Data *data = (_S710_USB_Data *)d->data;

  if ( d->type == S710_DRIVER_USB ) {
    if ( idx == bytes ) {
      idx = 0;
      do { 
#ifdef S710_USB_BULK_READ
	bytes = usb_bulk_read(data->handle,
			      data->endpoint,
			      buf,sizeof(buf),5000);
#else /* not S710_USB_BULK_READ */
	bytes = usb_interrupt_read(data->handle,
				   data->endpoint,
				   buf,sizeof(buf),5000);
#endif /* S710_USB_BULK_READ */
	if ( bytes == 0 && errno == EOVERFLOW ) {
	  usb_reset(data->handle);
	  shutdown_usb_port(d);
	  init_usb_port(d,d->mode);
	}
	usleep(10000);
      } while ( !bytes && i++ < 100 );
    }
    if ( bytes > 0 ) {
      /*      fprintf(stderr,"[%02x]\n",(unsigned char)buf[idx]); */
      *byte = (unsigned char)buf[idx++];
      r = 1;
    }
  }

#endif /* HAVE_LIBUSB */
  
  return r;
}


/* send a packet to the polar device over the usb interface */

int
send_packet_usb ( unsigned char *serialized, int bytes, S710_Driver *d )
{
  int  ret = 0;
#ifdef HAVE_LIBUSB
  _S710_USB_Data *data = (_S710_USB_Data *)d->data;

  if ( d->type == S710_DRIVER_USB ) {

    /* packets are sent via USB control transfers. */

    ret = usb_control_msg(data->handle,
			  0x43,
			  0x01,
			  0x0801,
			  0x0000,
			  (char *)serialized,
			  bytes,
			  1000);

  } else {
    fprintf(stderr,"Driver is not USB, no USB communication possible.\n");
  }
#endif

  return ret;
}



int
shutdown_usb_port ( S710_Driver *d )
{
  int ret = -1;
#ifdef HAVE_LIBUSB
  _S710_USB_Data *data = (_S710_USB_Data *)d->data;

  if ( d->type == S710_DRIVER_USB ) {
    fprintf(stderr,"Resetting endpoint\n");
    usb_resetep(data->handle,data->endpoint);
    fprintf(stderr,"Releasing interface\n");
    usb_release_interface(data->handle,data->interface);
    fprintf(stderr,"Disconnecting from USB device\n");
    ret = usb_close(data->handle);
  }

  free(data);
  d->data = (void *)NULL;

#endif /* HAVE_LIBUSB */

  return ret;
}



#ifdef HAVE_LIBUSB

/* I copied these helper functions from gphoto2. */

static int
find_first_altsetting ( struct usb_device *dev, 
			int               *config, 
			int               *interface, 
			int               *altsetting )
{
  int i, i1, i2;
  
  if (!dev->config)
    return -1;
  
  for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
    for (i1 = 0; i1 < dev->config[i].bNumInterfaces; i1++)
      for (i2 = 0; i2 < dev->config[i].interface[i1].num_altsetting; i2++)
	if (dev->config[i].interface[i1].altsetting[i2].bNumEndpoints) {

	  *config = i;
	  *interface = i1;
	  *altsetting = i2;
	  
	  return 0;
	}
  
  return -1;
}


static int
find_endpoint ( struct usb_device *dev, 
		int                config, 
		int                interface, 
		int                altsetting, 
		int                dir, 
		int                type )
{
  struct usb_interface_descriptor *intf;
  int i;
  
  if (!dev->config)
    return -1;
  
  intf = &dev->config[config].interface[interface].altsetting[altsetting];
  
  for (i = 0; i < intf->bNumEndpoints; i++) {
    if ((intf->endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir &&
	(intf->endpoint[i].bmAttributes & USB_ENDPOINT_TYPE_MASK) == type)
      return intf->endpoint[i].bEndpointAddress;
  }
  
  return -1;
}

#endif /* HAVE_LIBUSB */
