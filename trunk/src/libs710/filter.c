/* $Id: filter.c,v 1.2 2002/10/08 03:26:58 dave Exp $ */

#include "s710.h"
#include "config.h"

#ifndef S710_MAX_VALID_HR
#define S710_MAX_VALID_HR 206
#endif /* S710_MAX_VALID_HR */

#ifndef S710_MAX_VALID_CAD
#define S710_MAX_VALID_CAD 170
#endif /* S710_MAX_VALID_CAD */


/* 
   This function filters out bad HR data from a workout and recomputes
   the average and max HR from the filtered data.  
*/

void
filter_workout ( workout_t *w )
{
  int              v;
  int              lv;
  int              i;
  int              j;
  float            f_interp;
  int              remax = 0;
  float            avg;

  /* clean up the sample data */

  if ( w->hr_data != NULL ) {
    v  = 1;
    lv = 0;
    for ( i = 0; i < w->samples; i++ ) {
      if ( !v && w->hr_data[i] <= S710_MAX_VALID_HR && w->hr_data[i] != 0 ) {
	if ( lv >= 0 ) {
	  for ( j = lv; j < i; j++ ) {
	    f_interp = (float)w->hr_data[lv] + 
	      (w->hr_data[i]-w->hr_data[lv])*(j-lv)/(i-lv);
	    w->hr_data[j] = (int) f_interp;
	  }
	}
	v = 1;
	remax = 1;
      } else if ( v && 
		  (w->hr_data[i] > S710_MAX_VALID_HR || w->hr_data[i] == 0) ) {
	v  = 0;
	lv = i - 1;
      }
    }
    
    /* recompute max and avg HR if we have to */
    
    if ( remax != 0 ) {
      w->max_hr = 0;
      avg = 0;
      j = 0;
      for ( i = 0; i < w->samples; i++ ) {
	if ( w->hr_data[i] > w->max_hr ) w->max_hr = w->hr_data[i];
	if ( w->hr_data[i] > 0 ) {
	  avg = (float)(avg * j + w->hr_data[i])/(j+1);
	  j++;
	}
      }
      w->avg_hr = (int)avg;
    }
  }

  if ( w->cad_data != NULL ) {
    v  = 1;
    lv = 0;
    for ( i = 0; i < w->samples; i++ ) {
      if ( !v && w->cad_data[i] <= S710_MAX_VALID_CAD ) {
	if ( lv >= 0 ) {
	  for ( j = lv; j < i; j++ ) {
	    f_interp = (float)w->cad_data[lv] +
	      (w->cad_data[i]-w->cad_data[lv])*(j-lv)/(i-lv);
	    w->cad_data[j] = (int) f_interp;
	  }
	}
	v = 1;
	remax = 1;
      } else if ( v &&
		  ( w->cad_data[i] > S710_MAX_VALID_CAD ) ) {
	v  = 0;
	lv = i - 1;
      }
    }
    
    /* recompute max and avg cadence if we have to */
    
    if ( remax != 0 ) {
      w->max_cad = 0;
      avg = 0;
      j = 0;
      for ( i = 0; i < w->samples; i++ ) {
	if ( w->cad_data[i] > w->max_cad ) w->max_cad = w->cad_data[i];
	if ( w->cad_data[i] > 0 ) {
	  avg = (float)(avg * j + w->cad_data[i])/(j+1);
	  j++;
	}
      }
      w->avg_cad = (int)avg;
    }
  }

  w->filtered = 1;
}

