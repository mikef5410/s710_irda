/* $Id: comm.c,v 1.10 2004/09/21 08:16:05 dave Exp $ */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <ctype.h>
#include "s710.h"
#include "config.h"

/* This file needs to be rewritten.  */

#define READ_TRIES   10

/* static helper functions */

static void send_byte         ( unsigned char   byte,  S710_Driver *d );
static int  recv_byte         ( unsigned char  *byte,  S710_Driver *d );
static int  recv_short        ( unsigned short *s,     S710_Driver *d );
static unsigned short packet_checksum ( packet_t *packet );
static int  serialize_packet  ( packet_t *p, unsigned char *buf, S710_Driver *d );
void hexDumpBuf(unsigned char *buf, int size);

/* send a packet via the S710 driver */

int 
send_packet ( packet_t *packet, S710_Driver *d )
{
  int           i;
  int           ret = 1;
  unsigned char serialized[BUFSIZ];
  int           bytes;

  /* first, compute the packet checksum */
  
  packet->checksum = packet_checksum(packet);
  
  /* next, serialize the packet into a stream of bytes */

  bytes = serialize_packet ( packet, serialized, d );

  if ( d->type == S710_DRIVER_IRDA ) {
    //printf("TX (%d bytes):\n", bytes);
    //hexDumpBuf(serialized, bytes);
    ret=send(d->sockfd, serialized, bytes, 0);
    return(ret);
  }

  if ( d->type == S710_DRIVER_USB ) {
    
    /* USB packets are sent all at once. */

    ret = send_packet_usb ( serialized, bytes, d );

  } else {
    for ( i = 0; i < bytes; i++ ) {
      send_byte(serialized[i],d);
#ifdef S710_SERIAL_ALT_INTER_CHAR_TIMER_IMP
      usleep(10000); /* 10ms */
#endif
    }
    
    if ( d->type == S710_DRIVER_SERIAL ) {

      /* the data that gets echoed back is not RS-232 data.  it is garbage 
	 data that we have to flush.  there is a pause of at least 0.1 
	 seconds before the real data shows up. */

      usleep(100000);
      tcflush((int)d->data,TCIFLUSH);
    }
  }

  return ret;
}


/* receive a packet from the S710 driver (allocates memory) */

packet_t *
recv_packet ( S710_Driver *d )
{
  int             r;
  int             i;
  unsigned char   c = 0;
  unsigned char   id;
  unsigned short  len;
  size_t          siz;
  packet_t       *p = NULL;
  unsigned short  crc = 0;
  unsigned char   rxbuf[1024];

  if (d->type == S710_DRIVER_IRDA ) {
    //Read from socket and fill the packet structure ...
    memset(rxbuf, 0, 1024);
    siz=recv(d->sockfd, rxbuf, 1024, 0);
    if (siz < 1) return(NULL);
    //printf("RX (%d bytes):\n", (int)siz);
    //hexDumpBuf(rxbuf,siz);
    p=(packet_t *)calloc(1, sizeof(packet_t) + 1024 /*siz*/);
    p->type = S710_RESPONSE;
    p->id = rxbuf[0];
    // rxbuf[1] should always be zero
    p->length = siz-1;
    if (p->length) {
      memcpy(p->data, (void *)rxbuf+1, siz-1);
    }
    return(p);
  }
  
  r = recv_byte ( &c, d );
  crc_process ( &crc, c );

  if ( c == S710_RESPONSE ) {
    r = recv_byte ( &id, d );
    crc_process ( &crc, id );
    r = recv_byte ( &c, d );
    crc_process ( &crc, c );
    r = recv_short ( &len, d );
    crc_process ( &crc, len >> 8 );
    crc_process ( &crc, len & 0xff );
    if ( r ) {
      len -= 5;
      siz = (len <= 1) ? 0 : len - 1;
      p = calloc(1,sizeof(packet_t) + siz);

      if ( !p ) {
	fprintf(stderr,"calloc(1,%ld): %s\n",
		(long)(sizeof(packet_t) + siz), strerror(errno));
      } else {

	p->type   = S710_RESPONSE;
	p->id     = id;
	p->length = len;
	for ( i = 0; i < len; i++ ) {
	  r = recv_byte ( &p->data[i], d );
	  crc_process ( &crc, p->data[i] );
	  if ( !r ) { 
	    free ( p );
	    p = NULL;
	    break;
	  }
	}
	if ( p != NULL ) {
	  recv_short ( &p->checksum, d );

	  if ( crc != p->checksum ) {
	    
	    /* 
	       if the checksum failed, we have to jettison the whole 
	       transmission.  i don't yet know how to request a single
	       packet to be resent by the watch.  all we can do is 
	       cancel the download and have the user attempt it again.
	    */
	    
	    fprintf ( stderr, 
		      "\nCRC failed [id %d, length %d]\n", 
		      p->id, p->length );
	    free ( p );
	    p = NULL;
	  }
	}
      }
    }
  }
  return p;
}


/* read a byte from fd */

static int
recv_byte ( unsigned char *byte, S710_Driver *d )
{
  int r = 0;

  switch ( d->type ) {
  case S710_DRIVER_SERIAL:
  case S710_DRIVER_IR:
    r = read_serial_byte(d,byte);
    break;
  case S710_DRIVER_USB:
    r = read_usb_byte(d,byte);
    break;
  default:
    break;
  }

  return r;
}


/* read a short from fd */

static int
recv_short ( unsigned short *s, S710_Driver *d )
{
  int           r = 0;
  unsigned char u = 0;
  unsigned char l = 0;

  r = recv_byte ( &u, d );
  r = recv_byte ( &l, d );

  *s = (unsigned short)(u<<8)|l;

  return r;
}


/* send a mapped byte over fd - MAKE SURE you compute_byte_map() first
   if you are using the serial port IR dongle! */

static void
send_byte ( unsigned char byte, S710_Driver *d )
{
  switch ( d->type ) {
  case S710_DRIVER_SERIAL:
    write((int)d->data,&gByteMap[byte],1);
    break;
  case S710_DRIVER_IR:
    write((int)d->data,&byte,1);
    break;
  default:
    break;
  }
}


static unsigned short
packet_checksum ( packet_t *packet )
{
  unsigned short crc = 0;

  crc_process ( &crc, S710_REQUEST );
  crc_process ( &crc, packet->id );
  crc_process ( &crc, 0 );
  crc_process ( &crc, ( packet->length + 5 ) >> 8 );
  crc_process ( &crc, ( packet->length + 5 ) & 0xff );
  crc_block ( &crc, packet->data, packet->length );
  return crc;
}


static int
serialize_packet ( packet_t *p, unsigned char *buf, S710_Driver *d )
{
  unsigned short l = p->length + 5;

  if (d->type == S710_DRIVER_IRDA) {
    //IRDA Stack does a lot of the work for us.
    l=p->length+1;
    buf[0]=p->id;
    //buf[1]=0;
    //buf[2]=l >> 8;
    //buf[3]=l & 0xff;
    if ( p->length > 0 ) {
      memcpy(&buf[1],p->data,p->length);
    }
    return(l);
  } else {
    buf[0]   = p->type;
    buf[1]   = p->id;
    buf[2]   = 0;
    buf[3]   = l >> 8;
    buf[4]   = l & 0xff;
    if ( p->length > 0 ) {
      memcpy(&buf[5],p->data,p->length);
    }
    buf[l]   = p->checksum >> 8;
    buf[l+1] = p->checksum & 0xff;
    return l+2;
  }
}

void hexDumpBuf(unsigned char *buf, int size)
{
  int row;
  int col;
  int j;
  const int cols = 16;
  char line[255];
  char new[255];
  
  for (row = 0; row <= (size / cols); row++) {
    memset(line, 0, 255);
    line[0] = 0;
    snprintf(line, 255, "%.4x  ", row * cols);
    for (col = 0; col < cols; col++) {
      if (row * cols + col >= size)
	break;
      snprintf(new, 255, "%s %.2x", line, buf[(row * cols) + col]);
      strncpy(line,new,255);
    }
    if (col < cols) {		//line up the ascii decoded out for fractional line
      for (j = 0; j < (cols - col); j++) {
	snprintf(new, 255, "%s   ", line);
        strncpy(line,new,255);
      }
    }
    snprintf(new, 255, "%s   ", line);
    strncpy(line,new,255);
    for (col = 0; col < cols; col++) {
      if (row * cols + col >= size)
	break;
      if (buf[(row * cols) + col] == ' '
	  || isgraph(buf[(row * cols) + col])) {
	snprintf(new, 255, "%s%c", line, buf[(row * cols) + col]);
        strncpy(line,new,255);
      } else {
	snprintf(new, 255, "%s%c", line, '.');
        strncpy(line,new,255);
      }
    }
    printf("%s\r\n", line);
  }
  printf("\r\n");
}
