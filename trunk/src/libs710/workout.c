/* $Id: workout.c,v 1.17 2007/02/26 09:34:54 dave Exp $ */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


void
free_workout ( workout_t *w )
{
  if ( w != NULL ) {
    if ( w->lap_data )   free(w->lap_data);
    if ( w->alt_data )   free(w->alt_data);
    if ( w->speed_data ) free(w->speed_data);
    if ( w->dist_data )  free(w->dist_data);
    if ( w->cad_data )   free(w->cad_data);
    if ( w->power_data ) free(w->power_data);
    free(w);
  }
}
