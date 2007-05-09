/* $Id: serial.c,v 1.9 2007/02/24 20:16:39 dave Exp $ */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include "s710.h"

/* This file needs to be rewritten after I figure out how to get the damn
   serial port to work for everybody! */

#define SERIAL_READ_TRIES 10

/* initialize the serial port */

int  
init_serial_port ( S710_Driver *d, S710_Mode mode )
{
  struct termios t;
  int fd;

  fd = open(d->path, O_RDWR | O_NOCTTY | O_NDELAY); 
  if ( fd < 0 ) { 
    fprintf(stderr,"%s: %s\n",d->path,strerror(errno)); 
    return -1; 
  }

  fcntl(fd,F_SETFL,O_RDONLY);
  memset(&t,0,sizeof(t));

  /* 9600 bps, 8 data bits, 1 or 2 stop bits (??), no parity */

  t.c_cflag = B9600 | CS8 | CLOCAL | CREAD;

  /* I don't know why an extra stop bit makes it work for bidirectional
     communication.  Also, it doesn't work for everyone - in fact, it
     may only work for me. */

  if ( mode == S710_MODE_RDWR ) t.c_cflag |= CSTOPB;
  t.c_iflag = IGNPAR;
  t.c_oflag = 0;
  t.c_lflag = 0;

#ifdef S710_SERIAL_ALT_INTER_CHAR_TIMER_IMP
  t.c_cc[VTIME]    = 0;
  t.c_cc[VMIN]     = 0;
#else
  t.c_cc[VTIME]    = 1; /* inter-character timer of 0.1 second used */
  t.c_cc[VMIN]     = 0;  /* blocking read until 1 char / timer expires */
#endif

  /* set up for input */

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&t);

  d->data  = (void *)fd;

  return fd;
}


int
read_serial_byte ( S710_Driver *d, unsigned char *byte )
{
  int r = 0;
  int i = SERIAL_READ_TRIES;
  static struct timeval ti;
  static struct timeval tf;
  static int    n;
  float el;
#ifdef S710_SERIAL_ALT_INTER_CHAR_TIMER_IMP
  struct timeval timeout;
  fd_set readbits;
  int rc;
#endif

  gettimeofday(&tf,NULL);
  el = (n++)? (float)tf.tv_sec-ti.tv_sec+(tf.tv_usec-ti.tv_usec)/1000000.0 : 0;
  memcpy(&ti,&tf,sizeof(struct timeval));

  do {
#ifdef S710_SERIAL_ALT_INTER_CHAR_TIMER_IMP
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000; /* wait for data 10ms at most */

    FD_ZERO(&readbits);
    FD_SET((int)d->data, &readbits);

    rc = select(((int)d->data)+1,&readbits,NULL,NULL,&timeout);

    if( rc == 0 ) {
      r = 0; /* select timeout, no data available */
    } else if( rc < 0 ) {
      r = 0; /* select returned an error */
      fprintf(stderr,"select(): %s\n",strerror(rc));
    } else {
      r = read((int)d->data,byte,1); /* data available, read one byte */
    }
#else
    r = read((int)d->data,byte,1);
#endif
  } while ( !r && i-- );

#if 0
  fprintf(stderr,"%4d: %f",n,el);
  if ( r != 0 ) {
    fprintf(stderr," [%02x]\n",*byte);
  } else {
    fprintf(stderr,"\n");
  }
#endif

  return r;
}
