/* $Id: driver.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/irda.h>
#include <errno.h>
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
      } else if ( !strcmp(optarg,"irda") ) {
        d->type = S710_DRIVER_IRDA;
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
  case S710_DRIVER_IRDA:
    ret = init_irda(d, mode);
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


#define MAX_DEVICES 10
int discover_devices(int fd)
{
  struct irda_device_list *list;
  unsigned char buf[sizeof(struct irda_device_list) +
                    sizeof(struct irda_device_info) * MAX_DEVICES];
  unsigned int len;
  int daddr=-1;
  int i;

  len = sizeof(struct irda_device_list) + sizeof(struct irda_device_info) * MAX_DEVICES;
  list = (struct irda_device_list *) buf;
        

  fprintf(stderr, "Doing device discovery");
  i=60;
  while (getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &len)) {
    if(errno!=EAGAIN) {
      perror("getsockopt");
      return -1;
    }
    i--;

    if(i==0) {
      fprintf(stderr, "\nDidn't find any devices.\n");
      return -1;
    }

    fprintf(stderr,".");
    fflush(stderr);
    sleep(1);
  }
  if (len > 0) {
    /* 
     * Just pick the first one, but we should really ask the 
     * user 
     */
    daddr = list->dev[0].daddr;

    fprintf(stderr,"\nDiscovered: (list len=%d)\n", list->len);

    for (i=0;i<list->len;i++) {
      fprintf(stderr,"  name:  %s\n", list->dev[i].info);
      fprintf(stderr,"  daddr: %08x\n", list->dev[i].daddr);
      fprintf(stderr,"  saddr: %08x\n", list->dev[i].saddr);
      fprintf(stderr,"\n");
    }
    if(i>1) fprintf(stderr, "Picking the first one.\n");
  }
  return daddr;
}

int init_irda( S710_Driver *d, S710_Mode mode ) {
  struct sockaddr_irda peer;

  d->sockfd = socket(AF_IRDA, SOCK_STREAM, 0);

  d->daddr = discover_devices(d->sockfd);
  if(d->daddr == -1) return 1;
  
  peer.sir_family = AF_IRDA;
  peer.sir_lsap_sel = LSAP_ANY;
  peer.sir_addr = d->daddr;
  strcpy(peer.sir_name, "HRM");

  if(connect(d->sockfd, (struct sockaddr *) &peer, sizeof(struct sockaddr_irda))) {
    perror("connect");
    return(1);
  }
  fprintf(stderr, "Connected!\n");
  return(0);
}
