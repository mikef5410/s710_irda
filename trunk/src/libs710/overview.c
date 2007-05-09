/* $Id: overview.c,v 1.4 2002/10/08 03:26:58 dave Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


int
get_overview ( S710_Driver *d, overview_t *overview )
{
  packet_t *p;
  int       ok = 0;

  p = get_response ( S710_GET_OVERVIEW, d );
  if ( p != NULL) {
    overview->files = BCD(p->data[2]);
    overview->bytes = (p->data[4] << 8) + p->data[5];
    ok = 1;
    free ( p );
  }

  return ok;
}


void
print_overview ( overview_t *o, FILE *fp )
{
  fprintf(fp,"\n%d file%s found\n",o->files,(o->files==1)?"":"s");
  fprintf(fp,"%d bytes stored\n\n",o->bytes);
}
