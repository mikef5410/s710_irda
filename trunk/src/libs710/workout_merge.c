/* $Id: workout_merge.c,v 1.1 2007/02/26 09:34:54 dave Exp $ */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "s710.h"
#include "config.h"


#ifndef MIN
#define MIN(a,b) ((a) < (b)) ? (a) : (b);
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b)) ? (a) : (b);
#endif


static int
workouts_overlap ( workout_t *w1, workout_t *w2 )
{
  time_t ft1;
  time_t ft2;

  /* workouts overlap if the start time of w1 is earlier than the end
     time of w2, or vice versa. */

  ft1 = w1->unixtime + s710_time_to_seconds(&w1->duration);
  ft2 = w2->unixtime + s710_time_to_seconds(&w2->duration);

  return ( (w2->unixtime < w1->unixtime && w1->unixtime < ft2) ||
	   (w1->unixtime < w2->unixtime && w2->unixtime < ft1) );
}


/* returns the number of seconds between the end of the first workout
   and the beginning of the second. */

static time_t
workout_gap ( workout_t *w1, workout_t *w2 )
{
  return w2->unixtime - (w1->unixtime + s710_time_to_seconds(&w1->duration));
}


/* 
   This function takes a pair of workouts and merges them into a single
   workout.  The function works if:

   1) The workouts do not overlap.
   2) The user has specified "true" merging, and the time gap between 
      the end of the first workout and the beginning of the second workout 
      is not too large, in terms of the number of "blank" samples that will 
      have to be written.  The maximum number of allowed "blank" samples is 
      the value of S710_BLANK_SAMPLE_LIMIT, which is defined in s710.h.
   3) The user has specified "concatenation" merging, in which case the
      second workout begins immediately after the first workout ends.  This
      results in an inaccurate representation of when the exercise took 
      place (at least for the second workout, anyway), but it may be useful
      to some people.
*/

workout_t *
merge_workouts ( workout_t *wa, workout_t *wb, S710_Merge_Type mtype )
{
  workout_t * w;
  workout_t * w1;
  workout_t * w2;
  time_t      wg;
  int         ok;
  int         recint;
  int         samp1;
  int         samp2;
  int         w1fac;
  int         w2fac;
  int         blw1;
  int         blw2;
  int         i, j, k;
  int
    sum_w1_hr           = 0,
    sum_w1_cad          = 0,
    sum_w1_power        = 0,
    sum_w1_lr_balance   = 0,
    sum_w1_pedal_index  = 0,
    sum_w1_speed        = 0,
    sum_w2_hr           = 0,
    sum_w2_cad          = 0,
    sum_w2_power        = 0,
    sum_w2_lr_balance   = 0,
    sum_w2_pedal_index  = 0,
    sum_w2_speed        = 0;
  int
    samp_w1_hr          = 0,
    samp_w1_cad         = 0,
    samp_w1_power       = 0,
    samp_w1_lr_balance  = 0,
    samp_w1_pedal_index = 0,
    samp_w1_speed       = 0,
    samp_w2_hr          = 0,
    samp_w2_cad         = 0,
    samp_w2_power       = 0,
    samp_w2_lr_balance  = 0,
    samp_w2_pedal_index = 0,
    samp_w2_speed       = 0;
  int            bpl;
  int            bps;
  lap_data_t *   lap1;
  lap_data_t *   lap2;
  int            w2lap1;
  int            w1samp1;
  int            w2samp1;
  unsigned long  accum;

  if ( (w = calloc(1,sizeof(workout_t))) == NULL ) {
    fprintf(stderr,"merge_workouts: calloc(%ld): %s\n",
	    (long)sizeof(workout_t),strerror(errno));
    return NULL;
  }

  if ( wa == NULL || wb == NULL ) {
    fprintf(stderr,"merge_workouts: NULL argument\n");
    return NULL;
  }

  /* do they overlap? */

  if ( workouts_overlap(wa,wb) ) {
    fprintf(stderr,"merge_workouts: workouts overlap, bailing out.\n");
    return NULL;
  }

  /* figure out which workout comes first. */
  
  wg = workout_gap(wa,wb);
  if ( wg >= 0 ) {
    w1 = wa;
    w2 = wb;
  } else {
    w1 = wb;
    w2 = wa;
    wg = workout_gap(wb,wa);
  }

  /* what sample interval are we going to use?  use the finer of the two. */

  recint = MIN(w1->recording_interval,w2->recording_interval);

  /* if the merge type is S710_MERGE_TRUE, ensure that the sample gap isn't
     too large. */

  if ( mtype == S710_MERGE_TRUE && wg/recint > S710_BLANK_SAMPLE_LIMIT ) {
    fprintf(stderr,"merge_workouts: too many blank samples, bailing out.\n");
    free(w);
    return NULL;
  }

  /* OK, we're good to go.  DO IT. */

  /* Begin by copying w1's data over into w2. */

  memcpy(w,w1,sizeof(workout_t));

  /* Make the pointers in w be NULL, we'll handle that stuff later... */

  w->lap_data   = NULL;
  w->hr_data    = NULL;
  w->alt_data   = NULL;
  w->speed_data = NULL;
  w->dist_data  = NULL;
  w->cad_data   = NULL;
  w->power_data = NULL;

  /* 
     The output HRM type corresponds to the more recent or feature-rich
     model of the two. 
  */

  if ( w1->type == S710_HRM_S625X || w2->type == S710_HRM_S625X ) {
    w->type = S710_HRM_S625X;
  } else if ( w1->type == S710_HRM_S710 || w2->type == S710_HRM_S710 ) {
    w->type = S710_HRM_S710;
  } else {
    w->type = S710_HRM_S610;
  }

  /* Set the recording interval. */

  w->recording_interval = recint;

  /* The mode is the bitwise OR of the two modes. */

  w->mode = w1->mode | w2->mode;

  /* Compute the correct duration. */

  /* Set cumulative counters to their w2 values. */

  w->cumulative_exercise = w2->cumulative_exercise;
  w->cumulative_ride     = w2->cumulative_ride;
  w->odometer            = w2->odometer;
  w->total_energy        = w2->total_energy;

  /* Set min and max values correctly. */

  w->min_alt             = MIN(w1->min_alt,w2->min_alt);
  w->min_temp            = MIN(w1->min_temp,w2->min_temp);
  w->max_cad             = MAX(w1->max_cad,w2->max_cad);
  w->max_alt             = MAX(w1->max_alt,w2->max_alt);
  w->max_temp            = MAX(w1->max_temp,w2->max_temp);
  w->max_power           = (w1->max_power.power > w2->max_power.power) 
    ? w1->max_power 
    : w2->max_power;
  w->max_speed           = MAX(w1->max_speed,w2->max_speed);
  w->highest_speed       = MAX(w1->highest_speed,w2->highest_speed);

  /* Certain values are the sums of the two workouts' values. */

  w->exercise_distance   = w1->exercise_distance + w2->exercise_distance;
  w->ascent              = w1->ascent + w2->ascent;
  w->energy              = w1->energy + w2->energy;

  /* 
     Now let's figure out how many samples we're going to need.  This depends
     on the user's choice of merge mode.  If they want a "true" merge, then 
     we're going to insert a bunch of blank samples.  We'll also insert a 
     fake lap for the empty data. 
  */

  samp1                  = w1->samples;
  samp2                  = w2->samples;
  w1fac                  = w1->recording_interval / w->recording_interval;
  w2fac                  = w2->recording_interval / w->recording_interval;

  samp1 *= w1fac;
  samp2 *= w2fac;
  
  w->samples = samp1 + samp2;
  w->laps = w1->laps + w2->laps;
  if ( mtype == S710_MERGE_TRUE ) {
    w->samples += wg/recint;
    w->laps    += 1;
  }

  /* For average temperature and altitude, we compute a weighted mean of
     the data that we have. */

  w->avg_temp = (w1->avg_temp*samp1+w2->avg_temp*samp2)/(samp1+samp2);
  w->avg_alt  = (w1->avg_alt*samp1+w2->avg_alt*samp2)/(samp1+samp2);

  /* Average HR, cadence, power and speed have to be computed for the
     data for which those values were nonzero.  This is consistent 
     with how Polar does it. */

#define XDR(a,b,c)    (a)->b##_data[c]
#define XDP(a,b,c)    (a)->power_data[c].b

#define SAMPLESUM(a,b,c,Z)                                \
  do {                                                    \
    if ( (a)->b##_data != NULL ) {                        \
      for ( i = 0; i < (a)->samples; i++ ) {              \
        if ( XD##Z(a,c,i) > 0 ) {                         \
          sum_##a##_##c += XD##Z(a,c,i);                  \
          samp_##a##_##c++;                               \
        }                                                 \
      }                                                   \
      sum_##a##_##c *= a##fac;                            \
    }                                                     \
  } while ( 0 )

  SAMPLESUM(w1,hr,hr,R);
  SAMPLESUM(w1,cad,cad,R);
  SAMPLESUM(w1,speed,speed,R);
  SAMPLESUM(w1,power,power,P);
  SAMPLESUM(w1,power,lr_balance,P);
  SAMPLESUM(w1,power,pedal_index,P);
  SAMPLESUM(w2,hr,hr,R);
  SAMPLESUM(w2,cad,cad,R);
  SAMPLESUM(w2,speed,speed,R);
  SAMPLESUM(w2,power,power,P);
  SAMPLESUM(w2,power,lr_balance,P);
  SAMPLESUM(w2,power,pedal_index,P);
#undef SAMPLESUM

  /* Sorry about that.  OK, we're ready to compute the new averages. */

#define SAMPLEAVG(a) \
  (samp_w1_##a+samp_w2_##a) ? \
    (sum_w1_##a+sum_w2_##a)/(samp_w1_##a+samp_w2_##a) \
    : 0

  w->avg_hr                = SAMPLEAVG(hr);
  w->avg_cad               = SAMPLEAVG(cad);
  w->avg_speed             = SAMPLEAVG(speed);
  w->avg_power.power       = SAMPLEAVG(power);
  w->avg_power.lr_balance  = SAMPLEAVG(lr_balance);
  w->avg_power.pedal_index = SAMPLEAVG(pedal_index);
  w->median_speed          = w->avg_speed;
#undef SAMPLEAVG

  /* 
     The best lap is the shorter of the best laps of w1 and w2.  We don't
     count the fake lap we may or may not stick in there. 
  */

  blw1 = s710_time_to_tenths(&w1->bestlap_split);
  blw2 = s710_time_to_tenths(&w2->bestlap_split);

  if ( blw1 <= blw2 ) {
    w->bestlap_split = w1->bestlap_split;
  } else {
    w->bestlap_split = w2->bestlap_split;
  }

  /* 
     The duration is either the sum of the durations (if they're doing
     a concatenation merge) or the sum of the durations plus an additional
     wg seconds (if they're doing a true merge). 
  */

  sum_s710_time(&w1->duration,&w2->duration,&w->duration);
  if ( mtype == S710_MERGE_TRUE ) {
    increment_s710_time(&w->duration,wg);
  }

  /* 
     Let's just recompute the time spent above/within/below each of the heart
     rate zones.  Note that I am counting HR values of 0 in all this stuff.  
     I'm not sure if that's what I should be doing, but it's easy enough to 
     change if somebody complains. 
  */

  memset(&w->hr_zone[0][0],0,sizeof(w->hr_zone));
  for ( i = 0; i < 3; i++ ) {
    if ( w1->hr_data != NULL ) {
      for ( j = 0; j < w1->samples; j++ ) {
	if ( w1->hr_data[j] > w->hr_limit[i].upper ) {
	  increment_s710_time(&w->hr_zone[i][2],w1->recording_interval);
	} else if ( w1->hr_data[j] >= w->hr_limit[i].lower ) {
	  increment_s710_time(&w->hr_zone[i][1],w1->recording_interval);
	} else {
	  increment_s710_time(&w->hr_zone[i][0],w1->recording_interval);
	}
      }
    }
    if ( w2->hr_data != NULL ) {
      for ( j = 0; j < w2->samples; j++ ) {
	if ( w2->hr_data[j] > w->hr_limit[i].upper ) {
	  increment_s710_time(&w->hr_zone[i][2],w2->recording_interval);
	} else if ( w2->hr_data[j] >= w->hr_limit[i].lower ) {
	  increment_s710_time(&w->hr_zone[i][1],w2->recording_interval);
	} else {
	  increment_s710_time(&w->hr_zone[i][0],w2->recording_interval);
	}	
      }
    }
    if ( mtype == S710_MERGE_TRUE ) {
      if ( w->hr_limit[i].lower > 0 ) {
	increment_s710_time(&w->hr_zone[i][0],wg);
      } else {
	increment_s710_time(&w->hr_zone[i][1],wg);
      }
    }
  }

  /* 
     How many bytes will the output file be?  First, compute the number
     of bytes per lap and per sample. 
  */

  bpl = bytes_per_lap(w->type,w->mode,w->interval_mode);
  bps = bytes_per_sample(w->mode);

  /* 
     OK.  The total size of the resulting .srd file will be the size of the
     header plus bpl times the number of laps plus bps times the number of
     samples. 
  */

  w->bytes = header_size(w) + w->laps * bpl + w->samples * bps;

  /* Fill in the lap data. */

  w->lap_data = calloc(w->laps,sizeof(lap_data_t));
  memcpy(w->lap_data,w1->lap_data,w1->laps * sizeof(lap_data_t));
  if ( mtype == S710_MERGE_TRUE ) {
    w->lap_data[w1->laps].cumulative = 
      w1->duration;
    w->lap_data[w1->laps].cumul_ascent = 
      w->lap_data[w1->laps-1].cumul_ascent;
    w->lap_data[w1->laps].cumul_distance = 
      w->lap_data[w1->laps-1].cumul_distance;
    increment_s710_time(&w->lap_data[w1->laps].split,wg);
    increment_s710_time(&w->lap_data[w1->laps].cumulative,wg);
    memcpy(&w->lap_data[w1->laps+1],
	   w2->lap_data,w2->laps * sizeof(lap_data_t));
    w2lap1 = w1->laps + 1;
  } else {
    memcpy(&w->lap_data[w1->laps],
	   w2->lap_data,w2->laps * sizeof(lap_data_t));
    w2lap1 = w1->laps;
  }

  /* Adjust the cumulative quantities in the laps from the second workout. */

  lap2 = &w->lap_data[w2lap1-1];
  for ( i = 0; i < w2->laps; i++ ) {
    lap1 = &w->lap_data[w2lap1+i];
    lap1->cumul_distance += lap2->cumul_distance;
    lap1->cumul_ascent   += lap2->cumul_ascent;
    sum_s710_time(&lap1->cumulative,&lap2->cumulative,&lap1->cumulative);
  }
  
  /* Allocate the sample data arrays. */

  ok = allocate_sample_space(w);

  /* Never let a partially allocated workout get through. */

  if ( !ok ) {
    free_workout(w);
    return NULL;
  }

  /* 
     Fill in the sample data.  Note that if one of the workouts was recorded
     at a coarser time resolution, we do a linear interpolation of its data
     points to the finer time resolution.  Also note that if a true merge
     was requested, we interpolate the altitude between the workouts but 
     fill in zeroes for HR, speed, cadence and power output (as we should).
     If a concatenation merge was requested, then we do linear interpolation
     between the last sample of the first workout and the first sample of
     the second workout, if the first workout was recorded at a coarser
     sample interval than the second workout.  The only exception is that we 
     do NOT interpolate data to or from zero. 
  */

  /* The index of the first sample for the second workout is: */

  w1samp1 = 0;
  w2samp1 = w1->samples * w1fac;
  if ( mtype == S710_MERGE_TRUE ) {
    w2samp1 += wg / w->recording_interval;
  }

#define WD(a,b,Z)     XD##Z(w,a,b)
#define KEQJ(a,Z)     WD(a,k,Z) = WD(a,j,Z)
#define KJDY(a,b,c,Z) (int)(((a)-WD(c,j,Z))*(k-j)/b##fac)

#define IJLOOP(a) \
  for ( i = 0, j = a##samp1; i < (a)->samples-1; i++, j += a##fac )
#define KJLOOP(a) \
  for ( k = j+1; k < j+a##fac; k++ )

#define FILLSAMPLES(a,b,Z)                                    \
  do {                                                        \
    IJLOOP(a) {                                               \
      WD(b,j,Z) = XD##Z(a,b,i);                               \
      if ( XD##Z(a,b,i) > 0 && XD##Z(a,b,i+1) > 0 ) {         \
	KJLOOP(a) {                                           \
	  WD(b,k,Z) = WD(b,j,Z) + KJDY(XD##Z(a,b,i+1),a,b,Z); \
	}                                                     \
      } else {                                                \
	KJLOOP(a) KEQJ(b,Z);                                  \
      }                                                       \
    }                                                         \
    XD##Z(w,b,j) = XD##Z(a,b,i);                              \
  } while ( 0 )

#define DOITALL(a,b,Z)                                                   \
  do {                                                                   \
    if ( w->a##_data != NULL ) {                                         \
      if ( w1->a##_data != NULL ) {                                      \
        FILLSAMPLES(w1,b,Z);                                             \
        if ( mtype == S710_MERGE_TRUE || w2->a##_data == NULL ) {        \
          KJLOOP(w1) KEQJ(b,Z);                                          \
        } else {                                                         \
          KJLOOP(w1) WD(b,k,Z) = WD(b,j,Z) + KJDY(XD##Z(w2,b,0),w1,b,Z); \
        }                                                                \
      }                                                                  \
      if ( w2->a##_data != NULL ) {                                      \
        FILLSAMPLES(w2,b,Z);                                             \
        KJLOOP(w2) KEQJ(b,Z);                                            \
      }                                                                  \
    }                                                                    \
  } while ( 0 )

  DOITALL(hr,hr,R);
  DOITALL(alt,alt,R);
  DOITALL(cad,cad,R);
  DOITALL(speed,speed,R);
  DOITALL(power,power,P);
  DOITALL(power,lr_balance,P);
  DOITALL(power,pedal_index,P);

  /* now compute the distance data. */

  if ( w->speed_data != NULL ) {
    accum = 0;
    for ( i = 0; i < w->samples; i++ ) {
      w->dist_data[i] = accum / 57600.0;
      accum += w->speed_data[i] * w->recording_interval;
    }
  }

  return w;
}
