#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "s710.h"


/* 
   This file contains functions that are used in common by multiple files.
*/


int
header_size ( workout_t * w )
{
  int size = 0;

  switch ( w->type ) {
  case S710_HRM_S610:  size = S710_HEADER_SIZE_S610;  break;
  case S710_HRM_S625X: size = S710_HEADER_SIZE_S625X; break;
  case S710_HRM_S710:  size = S710_HEADER_SIZE_S710;  break;
  default:                                            break;
  }

  return size;
}


int
bytes_per_lap ( S710_HRM_Type type, unsigned char bt, unsigned char bi )
{
  int lap_size = 6;

  /* Compute the number of bytes per lap. */

  if ( S710_HAS_ALTITUDE(bt) )  lap_size += 5;
  if ( S710_HAS_SPEED(bt) ) {
    if ( S710_HAS_CADENCE(bt) ) lap_size += 1;
    if ( S710_HAS_POWER(bt) )   lap_size += 4;
    lap_size += 4;
  }
  
  /* 
     This is Matti Tahvonen's fix for handling laps with interval mode.
     Applies to the S625X and the S710/S720i.
  */

  if ( type != S710_HRM_S610 && bi != 0 ) {
    lap_size += 5;
  }

  return lap_size;
}


int
bytes_per_sample ( unsigned char bt )
{
  int recsiz = 1;

  if ( S710_HAS_ALTITUDE(bt) )   recsiz += 2;
  if ( S710_HAS_SPEED(bt) ) {
    if ( S710_HAS_ALTITUDE(bt) ) recsiz -= 1;
    recsiz += 2;
    if ( S710_HAS_POWER(bt) )    recsiz += 4;
    if ( S710_HAS_CADENCE(bt) )  recsiz += 1;
  }

  return recsiz;
}


int
allocate_sample_space ( workout_t * w )
{
  int ok = 1;

#define  MAKEBUF(a,b)				                \
  if ( (w->a = calloc(w->samples,sizeof(b))) == NULL ) {	\
    fprintf(stderr,"%s: calloc(%d,%ld): %s\n",			\
	    #a,w->samples,(long)sizeof(b),strerror(errno));	\
    ok = 0;							\
  }

  MAKEBUF(hr_data,S710_Heart_Rate);
  if ( S710_HAS_ALTITUDE(w->mode) ) MAKEBUF(alt_data, S710_Altitude);
  if ( S710_HAS_SPEED(w->mode) ) {
    MAKEBUF(speed_data, S710_Speed);
    MAKEBUF(dist_data, S710_Distance);
    if ( S710_HAS_POWER(w->mode) )   MAKEBUF(power_data, S710_Power);
    if ( S710_HAS_CADENCE(w->mode) ) MAKEBUF(cad_data, S710_Cadence);
  }

#undef MAKEBUF

  return ok;
}


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
