/* $Id: srdmerge.c,v 1.1 2004/11/23 18:55:22 dave Exp $ */

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
  workout_t *      w1;
  workout_t *      w2;
  int              ch;
  S710_Filter      filter = S710_FILTER_OFF;
  S710_Merge_Type  mtype = S710_MERGE_TRUE;

  while ( (ch = getopt(argc,argv,"cf")) != -1 ) {
    switch (ch) {
    case 'c': mtype  = S710_MERGE_CONCAT; break;
    case 'f': filter = S710_FILTER_ON;    break;
    default:                              break;
    }
  }
  argc -= optind;
  argv += optind;

  if ( argc != 3 ) {
    fprintf(stderr,
	    "usage: srdmerge [-c] [-f] "
	    "<SRD file 1> <SRD file 2> <output SRD file name>\n");
    exit(1);
  }

  w1 = read_workout(argv[0],filter,S710_HRM_AUTO);
  w2 = read_workout(argv[1],filter,S710_HRM_AUTO);
  if ( w1 != NULL && w2 != NULL ) {
    w = merge_workouts(w1,w2,mtype);
    if ( w != NULL ) {
      if ( w->bytes <= 0xffff ) {
	write_workout(w,argv[2]);
      } else {
	fprintf(stderr,"srdmerge: combined file would be too large.\n");
      }
      free_workout(w);
    } else {
      fprintf(stderr,"srdmerge: unable to merge workouts.\n");
    }
    free_workout(w1);
    free_workout(w2);
  } else {
    fprintf(stderr,"srdmerge: unable to read workouts.\n");
  }

  return 0;
}
