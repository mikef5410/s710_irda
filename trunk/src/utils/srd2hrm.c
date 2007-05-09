/* $Id: srd2hrm.c,v 1.1 2004/11/13 10:53:49 dave Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "s710.h"

/* externs */

extern char *optarg;
extern int   optind;


int
main ( int argc, char **argv )
{
  workout_t *      w;
  int              ch;
  S710_Filter      filter = S710_FILTER_OFF;

  while ( (ch = getopt(argc,argv,"f")) != -1 ) {
    switch (ch) {
    case 'f':
      filter = S710_FILTER_ON;
      break;
    }
  }
  argc -= optind;
  argv += optind;

  if ( argc != 1 ) {
    fprintf(stderr,"usage: srd2hrm [-f] <SRD file>\n");
    exit(1);
  }

  w = read_workout(argv[0],filter,S710_HRM_AUTO);
  if ( w != NULL ) {
    print_workout_as_hrm(w,stdout);
    free_workout(w);
  } else {
    fprintf(stderr,"%s: unable to read workout\n",argv[0]);
  }

  return 0;
}
