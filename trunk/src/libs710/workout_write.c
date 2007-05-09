/* $Id: workout_write.c,v 1.1 2007/02/26 09:34:54 dave Exp $ */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "s710.h"
#include "config.h"


static void
write_preamble ( workout_t * w, unsigned char * buf )
{
  /* number of bytes in the buffer (including these two) */

  buf[0] = w->bytes & 0xff;
  buf[1] = (w->bytes >> 8) & 0xff;

  /* exercise number and label */

  buf[2] = w->exercise_number;
  if ( w->exercise_number > 0 && w->exercise_number <= 5 ) {
    encode_label(w->exercise_label,&buf[3],7);
  }
}


static void
write_date ( workout_t * w, unsigned char * buf )
{
  int tmp1;

  /* date of workout */

  buf[0] = HEX(w->date.tm_sec);
  buf[1] = HEX(w->date.tm_min);

  tmp1 = w->date.tm_hour;
  if ( w->ampm & S710_AM_PM_MODE_SET ) {
    if ( w->ampm & S710_AM_PM_MODE_PM ) {
      if ( tmp1 > 12 ) tmp1 -= 12;
    } else {
      if ( tmp1 == 0 ) tmp1 = 12;
    }
  }
  
  buf[2] = HEX(tmp1);
  buf[3] = HEX(w->date.tm_mday) & 0x7f;

  if ( w->ampm & S710_AM_PM_MODE_PM  ) buf[2] |= 0x80;
  if ( w->ampm & S710_AM_PM_MODE_SET ) buf[3] |= 0x80;

  buf[4] = HEX(w->date.tm_year - 100);
  buf[5] = (w->date.tm_mon + 1) & 0x0f;
}


static void
write_duration ( workout_t * w, unsigned char * buf )
{
  buf[0] |= (w->duration.tenths << 4);
  buf[1] = HEX(w->duration.seconds);
  buf[2] = HEX(w->duration.minutes);
  buf[3] = HEX(w->duration.hours);
}


static void
write_recording_interval ( workout_t * w, unsigned char * buf )
{
  switch ( w->recording_interval ) {
  case 60: buf[0] = 2; break;
  case 15: buf[0] = 1; break;
  case  5:
  default: buf[0] = 0; break;
  }
}


static void
write_hr_limits ( workout_t * w, unsigned char * buf )
{
  /* HR limits */

  buf[0] = w->hr_limit[0].lower;
  buf[1] = w->hr_limit[0].upper;
  buf[2] = w->hr_limit[1].lower;
  buf[3] = w->hr_limit[1].upper;
  buf[4] = w->hr_limit[2].lower;
  buf[5] = w->hr_limit[2].upper;

  /* identifies this as an S610 workout (???) */

  buf[34] = 0;
  buf[36] = 251;

  /* time below, within, above hr limits 1 */
  
  buf[9]  = HEX(w->hr_zone[0][0].seconds);
  buf[10] = HEX(w->hr_zone[0][0].minutes);
  buf[11] = HEX(w->hr_zone[0][0].hours);
  buf[12] = HEX(w->hr_zone[0][1].seconds);
  buf[13] = HEX(w->hr_zone[0][1].minutes);
  buf[14] = HEX(w->hr_zone[0][1].hours);
  buf[15] = HEX(w->hr_zone[0][2].seconds);
  buf[16] = HEX(w->hr_zone[0][2].minutes);
  buf[17] = HEX(w->hr_zone[0][2].hours);

  /* time below, within, above hr limits 2 */

  buf[18] = HEX(w->hr_zone[1][0].seconds);
  buf[19] = HEX(w->hr_zone[1][0].minutes);
  buf[20] = HEX(w->hr_zone[1][0].hours);
  buf[21] = HEX(w->hr_zone[1][1].seconds);
  buf[22] = HEX(w->hr_zone[1][1].minutes);
  buf[23] = HEX(w->hr_zone[1][1].hours);
  buf[24] = HEX(w->hr_zone[1][2].seconds);
  buf[25] = HEX(w->hr_zone[1][2].minutes);
  buf[26] = HEX(w->hr_zone[1][2].hours);

  /* time below, within, above hr limits 3 */

  buf[27] = HEX(w->hr_zone[2][0].seconds);
  buf[28] = HEX(w->hr_zone[2][0].minutes);
  buf[29] = HEX(w->hr_zone[2][0].hours);
  buf[30] = HEX(w->hr_zone[2][1].seconds);
  buf[31] = HEX(w->hr_zone[2][1].minutes);
  buf[32] = HEX(w->hr_zone[2][1].hours);
  buf[33] = HEX(w->hr_zone[2][2].seconds);
  buf[34] = HEX(w->hr_zone[2][2].minutes);
  buf[35] = HEX(w->hr_zone[2][2].hours);
}


static void
write_bestlap_split ( workout_t * w, unsigned char * buf )
{
  /* Best lap */

  buf[0] = (w->bestlap_split.tenths & 0x0f) << 4;
  buf[1] = HEX(w->bestlap_split.seconds);
  buf[2] = HEX(w->bestlap_split.minutes);
  buf[3] = HEX(w->bestlap_split.hours);
}


static void
write_energy ( workout_t * w, unsigned char * buf )
{
  int tmp1;
  int tmp2;
  int tmp3;

  /* energy, total energy (if OwnCal enabled) */

  tmp1 = (w->energy - w->energy % 1000)/1000;
  tmp2 = (w->energy % 1000 - w->energy % 10)/10;
  tmp3 = w->energy % 10;

  buf[0] = HEX(tmp3) << 4;
  buf[1] = HEX(tmp2);
  buf[2] = HEX(tmp1);

  tmp1 = (w->total_energy - w->total_energy % 10000)/10000;
  tmp2 = (w->total_energy % 10000 - w->total_energy % 100)/100;
  tmp3 = w->total_energy % 100;

  buf[3] = HEX(tmp3);
  buf[4] = HEX(tmp2);
  buf[5] = HEX(tmp1);
}


static void
write_cumulative_exercise ( workout_t * w, unsigned char * buf )
{
  int tmp1;
  int tmp2;

  tmp1 = (w->cumulative_exercise.hours-w->cumulative_exercise.hours % 100)/100;
  tmp2 = w->cumulative_exercise.hours % 100;

  buf[0] = HEX(tmp2);
  buf[1] = HEX(tmp1);
  buf[2] = HEX(w->cumulative_exercise.minutes);
}


static void
write_ride_info ( workout_t * w, unsigned char * buf )
{
  int tmp1;
  int tmp2;
  int tmp3;

  tmp1 = (w->cumulative_ride.hours-w->cumulative_ride.hours % 100)/100;
  tmp2 = w->cumulative_ride.hours % 100;

  buf[0] = HEX(tmp2);
  buf[1] = HEX(tmp1);
  buf[2] = HEX(w->cumulative_ride.minutes);

  tmp1 = (w->odometer - w->odometer % 10000)/10000;
  tmp2 = (w->odometer % 10000 - w->odometer % 100)/100;
  tmp3 = w->odometer % 100;

  buf[3] = HEX(tmp3);
  buf[4] = HEX(tmp2);
  buf[5] = HEX(tmp1);

  /* exercise distance */

  buf[6] = w->exercise_distance & 0xff;
  buf[7] = (w->exercise_distance >> 8) & 0xff;

  /* avg and max speed */

  buf[8]  = w->avg_speed & 0xff;
  buf[9]  = (w->avg_speed >> 8) & 0x0f;
  buf[9] |= (w->max_speed & 0x0f) << 4;
  buf[10] = (w->max_speed >> 4) & 0xff;

  /* avg, max cadence */
  
  buf[11] = w->avg_cad;
  buf[12] = w->max_cad;

  /* min, avg, max temperature */

  if ( w->units.system == S710_UNITS_ENGLISH ) {

    /* English units */

    buf[19] = w->min_temp;
    buf[20] = w->avg_temp;
    buf[21] = w->max_temp;

  } else {

    /* Metric units */

    buf[19] = abs(w->min_temp) & 0x7f;
    buf[20] = abs(w->avg_temp) & 0x7f;
    buf[21] = abs(w->max_temp) & 0x7f;

    if ( w->min_temp >= 0 ) buf[19] |= 0x80;
    if ( w->avg_temp >= 0 ) buf[20] |= 0x80;
    if ( w->max_temp >= 0 ) buf[21] |= 0x80;
  }

  /* altitude, ascent */

  buf[13]  = abs(w->min_alt) & 0xff;
  buf[14]  = (abs(w->min_alt) >> 8) & 0x7f;

  buf[15]  = abs(w->avg_alt) & 0xff;
  buf[16]  = (abs(w->avg_alt) >> 8) & 0x7f;

  buf[17]  = abs(w->max_alt) & 0xff;
  buf[18]  = (abs(w->max_alt) >> 8) & 0x7f;
  
  if ( w->min_alt >= 0 ) buf[14] |= 0x80;
  if ( w->avg_alt >= 0 ) buf[16] |= 0x80;
  if ( w->max_alt >= 0 ) buf[18] |= 0x80;
  
  buf[22]  = w->ascent & 0xff;
  buf[23]  = (w->ascent >> 8) & 0xff;

  /* avg, max power data */

  buf[24]  = w->avg_power.power & 0xff;
  buf[25]  = (w->avg_power.power >> 8) & 0x0f;
  buf[25] |= (w->max_power.power & 0x0f) << 4;
  buf[26]  = (w->max_power.power >> 4) & 0xff;
  buf[27]  = w->avg_power.pedal_index;
  buf[28]  = w->max_power.pedal_index;
  buf[29]  = w->avg_power.lr_balance;
}


static int
write_workout_header ( workout_t * w, unsigned char * buf )
{
  write_preamble(w,buf);
  write_date(w,buf+10);
  write_duration(w,buf+15);
  
  buf[19] = w->avg_hr;
  buf[20] = w->max_hr;
  buf[21] = HEX(w->laps);
  buf[22] = HEX(w->manual_laps);
  buf[23] = w->interval_mode;
  buf[24] = HEX(w->user_id);
  if ( w->units.system == S710_UNITS_ENGLISH ) {
    buf[25] |= 0x02;
  }

  if ( w->type == S710_HRM_S610 ) {

    write_recording_interval(w,buf+26);
    write_hr_limits(w,buf+28);
    
    /* identifies this as an S610 workout (???) */

    buf[34] = 0;
    buf[36] = 251;

    write_bestlap_split(w,buf+65);
    write_energy(w,buf+69);
    write_cumulative_exercise(w,buf+75);    

  } else {

    buf[26] = w->mode;
    
    write_recording_interval(w,buf+27);
    write_hr_limits(w,buf+29);
    
    /* identifies this as an S710 workout (???) */
    
    buf[35] = 0;
    buf[37] = 251;

    write_bestlap_split(w,buf+66);
    write_energy(w,buf+70);
    write_cumulative_exercise(w,buf+76);
    write_ride_info(w,buf+79);
  }

  return header_size(w);
}


/* This function writes a workout lap to the buffer. */

static int
write_workout_lap ( workout_t * w, int index, unsigned char * buf )
{
  int           pos = 0;
  int           alt;
  int           dist;
  lap_data_t *  l = &w->lap_data[index];

  buf[pos+2]  = l->cumulative.hours;
  buf[pos+1]  = l->cumulative.minutes & 0x3f;
  buf[pos]    = l->cumulative.seconds & 0x3f;
  buf[pos]   |= ((l->cumulative.tenths & 0x03) << 6);
  buf[pos+1] |= ((l->cumulative.tenths & 0x0c) << 4);

  buf[pos+3]  = l->lap_hr;
  buf[pos+4]  = l->avg_hr;
  buf[pos+5]  = l->max_hr;

  pos += 6;

  if ( S710_HAS_ALTITUDE(w->mode) ) {
    alt = l->alt;
    if ( w->units.system == S710_UNITS_ENGLISH ) alt /= 5;
    alt += 0x200; /* 512 */
    buf[pos] = (alt & 0xff);
    buf[pos+1] = ((alt >> 8) & 0xff);
    buf[pos+2] = l->cumul_ascent & 0xff;
    buf[pos+3] = ((l->cumul_ascent >> 8) & 0xff);
    if ( w->units.system == S710_UNITS_ENGLISH ) {  /* English units */
      buf[pos+4] = l->temp - 14;
    } else {
      buf[pos+4] = l->temp + 10;
    }
    
    pos += 5;

    if ( S710_HAS_SPEED(w->mode) ) {
      if ( S710_HAS_CADENCE(w->mode) ) {
	buf[pos++] = l->cad;
      }

      if ( S710_HAS_POWER(w->mode) ) {
	buf[pos]   = l->power.power & 0xff;
	buf[pos+1] = (l->power.power >> 8) & 0xff;
	buf[pos+2] = l->power.pedal_index;
	buf[pos+3] = l->power.lr_balance;
	pos += 4;
      }

      dist = (int)l->cumul_distance;

      buf[pos]   = dist & 0xff;
      buf[pos+1] = (dist >> 8) & 0xff;
      buf[pos+2] = l->speed & 0xff;
      buf[pos+3] = (l->speed >> 4) & 0xf0;

      pos += 4;
    }
  }
  
  return pos;
}


/* This function writes a workout sample to the buffer */

static int
write_workout_sample ( workout_t * w, int index, unsigned char * buf )
{
  int pos = 0;
  int alt;

  buf[pos++] |= w->hr_data[index];
  if ( S710_HAS_ALTITUDE(w->mode) ) {
    alt = w->alt_data[index];
    if ( w->units.system == S710_UNITS_ENGLISH ) alt /= 5;
    alt += 0x200; /* 512 */
    buf[pos++] |= (alt & 0xff);
    buf[pos++] |= ((alt >> 8) & 0x1f);
  }
  if ( S710_HAS_SPEED(w->mode) ) {
    if ( S710_HAS_ALTITUDE(w->mode) ) pos--;
    buf[pos++] |= (((w->speed_data[index] >> 8) & 0x07) << 5);
    buf[pos++] |= (w->speed_data[index] & 0xff);
  }
  if ( S710_HAS_POWER(w->mode) ) {
    buf[pos++] = (w->power_data[index].power & 0xff);
    buf[pos++] = ((w->power_data[index].power >> 8) & 0xff);
    buf[pos++] = w->power_data[index].lr_balance;
    buf[pos++] = w->power_data[index].pedal_index;
  }
  if ( S710_HAS_CADENCE(w->mode) ) {
    buf[pos++] = w->cad_data[index];
  }

  return pos;
}


/* This function writes the workout laps and samples to the buffer. */

static int
write_workout_data ( workout_t * w, unsigned char * buf )
{
  int pos = 0;
  int i;

  for ( i = 0; i < w->laps; i++ ) {
    pos += write_workout_lap(w,i,&buf[pos]);
  }

  for ( i = w->samples-1; i >= 0; i-- ) {
    pos += write_workout_sample(w,i,&buf[pos]);
  }

  return pos;
}


/* This function writes the workout in .srd format to the supplied filename. */

int
write_workout ( workout_t * w, char * filename )
{
  unsigned char * buf;
  int             pos = 0;
  int             fd;

  if ( (buf = calloc(w->bytes,1)) != NULL ) {
    pos  = write_workout_header(w,buf);
    pos += write_workout_data(w,&buf[pos]);
    
    if ( (fd = creat(filename,0664)) != -1 ) {
      pos = write(fd,buf,pos);
      close(fd);
    } else {
      pos = 0;
    }
    
    free(buf);
  }

  return (pos == w->bytes);
}
