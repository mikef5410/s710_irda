/* $Id: byte_map.c,v 1.2 2002/06/10 06:42:05 dave Exp $ */

#include "s710.h"

unsigned char gByteMap[256];


void
compute_byte_map ( void )
{
  int i, j, m;

  for ( i = 0; i < 0x100; i++ ) {
    m = !(i & 1);
    for (j=7;j>0;j--) m |= ((i+0x100-(1<<(j-1)))&0xff) & (1<<j);
    gByteMap[m] = i;
  }
}

