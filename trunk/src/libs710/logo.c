/* $Id: logo.c,v 1.4 2002/10/10 10:11:09 dave Exp $ */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


/* requests and reads the logo into the data structure */

int
get_logo ( S710_Driver *d, logo_t *logo )
{
  packet_t *p;
  int       ok = 0;

  p = get_response ( S710_GET_LOGO, d );
  if ( p != NULL) {
    memcpy(&logo->column[0],&p->data[0],47);
    free ( p );
    ok = 1;
  }

  return ok;
}


/* sets the logo */

int
set_logo ( logo_t *logo, S710_Driver *d )
{
  packet_t  *p;
  
  p = make_set_packet(S710_SET_LOGO);
  if ( p == NULL ) return 0;

  memcpy(&p->data[0],&logo->column[0],47);
  
  return send_set_packet(p,d);
}


/* prints out the logo */

void
print_logo ( logo_t *l, FILE *fp )
{
  int  i;
  int  j;

  fprintf(fp,"\n");
  for ( j = 7; j >= 0; j-- ) {
    for ( i = 0; i < 47; i++ ) {
      if ( l->column[i] & (1<<j) ) {
	fprintf(fp,"*");
      } else {
	fprintf(fp," ");
      }
    }
    fprintf(fp,"\n");
  }
  fprintf(fp,"\n");
}


void
load_logo_attributes ( attribute_map_t *m )
{
  int  i;
  char buf[BUFSIZ];

  for ( i = 47; i >= 1; i-- ) {
    snprintf(buf,sizeof(buf),"col%02d",i);
    add_byte_attribute(m,buf,&m->data.logo.column[i-1]);
  }
}
