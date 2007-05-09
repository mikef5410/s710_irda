/* $Id: crc.c,v 1.1 2002/09/17 09:27:15 dave Exp $ */

/* 
   Thanks to Stefan Kleditzsch for decoding the checksum algorithm!
*/

/*
 * crc16 checksum function with polynom=0x8005
 */

void crc_process
( unsigned short * context,
  unsigned char ch )
{
  unsigned short uch  = (unsigned short) ch;
  int i;

  *context ^= ( uch << 8 );

  for ( i = 0; i < 8; i++ )
    {
      if ( *context & 0x8000 )
        *context = ( *context << 1 ) ^ 0x8005;
      else
        *context <<= 1;
    }
}

void crc_block
( unsigned short * context,
  const unsigned char * blk,
  int len )
{
  while ( len -- > 0 )
    crc_process ( context, * blk ++ );
}
