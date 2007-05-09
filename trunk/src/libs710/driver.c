/* $Id: driver.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <unistd.h>
#include <string.h>
#include "s710.h"


/* externs */

extern char *optarg;
extern int   optind;


int
driver_init ( int argc, char **argv, S710_Driver *d )
{
  int  init = 0;
  int  needpath = 1;
  int  ch;

  d->type = S710_DRIVER_SERIAL;
  
  while ( (ch = getopt(argc,argv,"d:")) != -1 ) {
    switch (ch) {
    case 'd':
      if ( !strcmp(optarg,"serial") ) {
	d->type = S710_DRIVER_SERIAL;
      } else if ( !strcmp(optarg,"ir") ) {
	d->type = S710_DRIVER_IR;
      } else if ( !strcmp(optarg,"usb") ) {
	d->type = S710_DRIVER_USB;
	needpath = 0;
      }
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if ( needpath != 0 && argv[0] != NULL ) {
    strncpy(d->path,argv[0],sizeof(d->path)-1);
    init = 1;
  } else if ( needpath == 0 && argc == 0 ) {
    init = 1;
  }

  return init;
}


int
driver_open ( S710_Driver *d, S710_Mode mode )
{
  int ret = -1;

  d->mode = mode;

  switch ( d->type ) {
  case S710_DRIVER_SERIAL:
    compute_byte_map();
  case S710_DRIVER_IR:
    ret = init_serial_port(d,mode);
    break;
  case S710_DRIVER_USB:
    ret = init_usb_port(d,mode);
    break;
  default:
    break;
  }

  return ret;
}


int
driver_close ( S710_Driver *d )
{
  int ret = 0;

  switch ( d->type ) {
  case S710_DRIVER_SERIAL:    
  case S710_DRIVER_IR:
    ret = close((int)d->data);
    break;
  case S710_DRIVER_USB:
    ret = shutdown_usb_port(d);
    break;
  default:
    break;
  }

  return ret;
}
