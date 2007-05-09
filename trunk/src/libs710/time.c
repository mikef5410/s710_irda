/* $Id: time.c,v 1.5 2007/02/26 09:34:54 dave Exp $ */

#include "s710.h"

/* 
   typedef struct S710_Time {
     int    hours;
     int    minutes;
     int    seconds;
     int    tenths;
   } S710_Time;
*/

/* 
   the "format" argument can be one of:

   "hmst" => 12:34:56.7
   "hms"  => 12:34:56
   "hm"   => 12:34
   "ms"   => 34:56
   "mst"  => 34:56.7
   "st"   => 56.7
*/

/* Get a S710 time and returns that time span as 
 * hours and fractions of hours
 */

float
get_hours_from_s710_time ( S710_Time *t )
{

  long seconds;

  seconds  = t->seconds;
  seconds += t->minutes * 60;
  seconds += t->hours * 3600;

  return (float)seconds / 3600.0;
}


/* Tenths of a second. */

time_t
s710_time_to_tenths ( S710_Time *t )
{
  time_t tm;

  tm = ((t->hours * 60 + t->minutes) * 60 + t->seconds) * 10 + t->tenths + 5;

  return tm;
}


/* Rounds to the nearest second. */

time_t
s710_time_to_seconds ( S710_Time *t )
{
  return s710_time_to_tenths(t)/10;
}


/* This probably doesn't work if the S710_Time argument is negative. */

void
increment_s710_time ( S710_Time *t, unsigned int seconds )
{
  int   hours;
  int   minutes;
  int   secs;

  hours   = seconds/3600;
  minutes = (seconds/60)%60;
  secs    = seconds % 60;

  if ( secs + t->seconds >= 60 ) {
    minutes++;
    secs -= 60;
  }
  if ( minutes + t->minutes >= 60 ) {
    hours++;
    minutes -= 60;
  }

  t->seconds += secs;
  t->minutes += minutes;
  t->hours += hours;
}


void
diff_s710_time ( S710_Time *t1, S710_Time *t2, S710_Time *diff )
{
  int t_t1;
  int t_t2;
  int t_diff;
  int negative = 0;

  /* first compute t1 and t2 in tenths of a second */

  t_t1 = ((t1->hours * 60 + t1->minutes) * 60 + t1->seconds) * 10 + t1->tenths;
  t_t2 = ((t2->hours * 60 + t2->minutes) * 60 + t2->seconds) * 10 + t2->tenths;

  t_diff = t_t2 - t_t1;
  if ( t_diff < 0 ) {
    negative = 1;
    t_diff = -t_diff;
  }

  diff->tenths = t_diff % 10;
  t_diff = (t_diff - diff->tenths) / 10;
  
  /* now t_diff is in seconds */

  diff->seconds = t_diff % 60;
  t_diff = (t_diff - diff->seconds) / 60;

  /* now t_diff is in minutes */

  diff->minutes = t_diff % 60;
  t_diff = (t_diff - diff->minutes) / 60;
  
  /* the rest is the hours */

  diff->hours = t_diff;

  /* if we got a negative time, switch the sign of everything. */

  if ( negative ) {
    diff->hours   = -diff->hours;
    diff->minutes = -diff->minutes;
    diff->seconds = -diff->seconds;
    diff->tenths  = -diff->tenths;
  }
}


void
sum_s710_time ( S710_Time *t1, S710_Time *t2, S710_Time *sum )
{
  int t_t1;
  int t_t2;
  int t_sum;
  int negative = 0;

  /* first compute t1 and t2 in tenths of a second */

  t_t1 = ((t1->hours * 60 + t1->minutes) * 60 + t1->seconds) * 10 + t1->tenths;
  t_t2 = ((t2->hours * 60 + t2->minutes) * 60 + t2->seconds) * 10 + t2->tenths;
  
  t_sum = t_t2 + t_t1;
  if ( t_sum < 0 ) {
    negative = 1;
    t_sum = -t_sum;
  }

  sum->tenths = t_sum % 10;
  t_sum = (t_sum - sum->tenths) / 10;
  
  /* now t_sum is in seconds */

  sum->seconds = t_sum % 60;
  t_sum = (t_sum - sum->seconds) / 60;

  /* now t_sum is in minutes */

  sum->minutes = t_sum % 60;
  t_sum = (t_sum - sum->minutes) / 60;
  
  /* the rest is the hours */

  sum->hours = t_sum;

  /* if we got a negative time, switch the sign of everything. */

  if ( negative ) {
    sum->hours   = -sum->hours;
    sum->minutes = -sum->minutes;
    sum->seconds = -sum->seconds;
    sum->tenths  = -sum->tenths;
  }
}


void
print_s710_time ( S710_Time *t, char *format, FILE *fp )
{
  char *p;
  int   r = 0;

  for ( p = format; *p != 0; p++ ) {
    switch ( *p ) {
    case 'h': case 'H':
      if ( r ) fprintf(fp,":"); 
      fprintf(fp,"%d",t->hours); 
      r = 1;
      break;
    case 'm': case 'M':
      if ( r ) fprintf(fp,":"); 
      fprintf(fp,"%02d",t->minutes); 
      r = 1;
      break;
    case 's': case 'S':
      if ( r ) fprintf(fp,":"); 
      fprintf(fp,"%02d",t->seconds); 
      r = 1;
      break;
    case 't': case 'T':
      if ( r ) fprintf(fp,"."); 
      fprintf(fp,"%d",t->tenths); 
      r = 1;
      break;
    default:
      break;
    }
  }
}
 
