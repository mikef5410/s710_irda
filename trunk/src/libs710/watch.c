/* $Id: watch.c,v 1.6 2002/12/28 07:51:49 dave Exp $ */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


/* requests and reads the watch data into the data structure */

int
get_watch ( S710_Driver *d, watch_t *watch )
{
  packet_t *p;
  int       ok = 0;

  p = get_response ( S710_GET_WATCH, d );
  if ( p != NULL) {
    memset(&watch->time1,0,sizeof(watch->time1));
    watch->time1.tm_sec  = BCD(p->data[0]);
    watch->time1.tm_min  = BCD(p->data[1]);
    watch->time1.tm_hour = BCD(p->data[2]);
    watch->time1.tm_mday = BCD(p->data[3]);
    watch->time1.tm_mon  = LNIB(p->data[5]) - 1;
    watch->time1.tm_year = BCD(p->data[4]) + 100;
    watch->time1.tm_wday = (UNIB(p->data[5])+1)%7;
    watch->time2.hours   = BCD(p->data[9]);
    watch->time2.minutes = BCD(p->data[8]);
    watch->alarm.hours   = BCD(p->data[7]);
    watch->alarm.minutes = BCD(p->data[6]);
    watch->alarm_on      = (p->data[10] & 0x02) ? S710_ON:S710_OFF;
    watch->time1_hours   = (p->data[10] & 0x08) ? S710_HOURS_12:S710_HOURS_24;
    watch->time2_hours   = (p->data[10] & 0x40) ? S710_HOURS_12:S710_HOURS_24;
    watch->time_zone     = ((p->data[10] & 0x10) >> 4) + 1;
    free ( p );
    ok = 1;
  }

  return ok;
}


/* set watch data */

int
set_watch ( watch_t *watch, S710_Driver *d )
{
  packet_t  *p;
  
  p = make_set_packet(S710_SET_WATCH);
  if ( p == NULL ) return 0;

  p->data[0]  = HEX(watch->time1.tm_sec);
  p->data[1]  = HEX(watch->time1.tm_min);
  p->data[2]  = HEX(watch->time1.tm_hour);
  p->data[3]  = HEX(watch->time1.tm_mday);
  p->data[4]  = HEX(watch->time1.tm_year - 100);
  p->data[5]  = BINU((watch->time1.tm_wday+6)%7) | BINL(watch->time1.tm_mon+1);
  p->data[6]  = HEX(watch->alarm.minutes);
  p->data[7]  = HEX(watch->alarm.hours);
  p->data[8]  = HEX(watch->time2.minutes);
  p->data[9]  = HEX(watch->time2.hours);
  if ( watch->alarm_on    == S710_ON       ) p->data[10] |= 0x02;
  if ( watch->time1_hours == S710_HOURS_12 ) p->data[10] |= 0x08;
  if ( watch->time2_hours == S710_HOURS_12 ) p->data[10] |= 0x40;
  if ( watch->time_zone   %  2             ) p->data[10] |= 0x10;
  
  return send_set_packet(p,d);
}


/* prints out the watch data */

void
print_watch ( watch_t *w, FILE *fp )
{
  char buf[BUFSIZ];

  strftime(buf,sizeof(buf),"%A, %d %B %Y %T",&w->time1);
  fprintf(fp,"\nTime 1:      %s\n",buf);
  fprintf(fp,"Time 2:      %d:%02d\n",
	  w->time2.hours,w->time2.minutes);
  fprintf(fp,"Alarm:       %d:%02d (%s)\n",
	  w->alarm.hours,w->alarm.minutes,
	  (w->alarm_on) ? "enabled" : "disabled" );
  fprintf(fp,"Time 1:      %d hr\n",(w->time1_hours)?12:24);
  fprintf(fp,"Time 2:      %d hr\n",(w->time2_hours)?12:24);
  fprintf(fp,"Time zone:   Time %d\n",w->time_zone);
  fprintf(fp,"\n");
}


#define WATCH_ATTR(a)       m,#a,&m->data.watch.a
#define WATCH_BOOL(a)       add_boolean_attribute(WATCH_ATTR(a))
#define WATCH_INT(a,b,c)    add_integer_attribute(WATCH_ATTR(a),b,c,0)
#define WATCH_INT2(a,b,c,d) add_integer_attribute(WATCH_ATTR(a),b,c,d)

void
load_watch_attributes ( attribute_map_t *m )
{
  WATCH_BOOL(alarm_on);
  add_enum_integer_attribute(m,
			     "which_time",
			     &m->data.watch.time_zone,
			     1, 0,
			     2, 1,
			     -1);
  add_enum_integer_attribute(m,
			     "time1_style",
			     &m->data.watch.time1_hours,
			     12,   S710_HOURS_12,
			     24,   S710_HOURS_24,
			     -1);
  add_enum_integer_attribute(m,
			     "time2_style",
			     &m->data.watch.time2_hours,
			     12,   S710_HOURS_12,
			     24,   S710_HOURS_24,
			     -1);

  WATCH_INT(time2.minutes,0,59);
  WATCH_INT(time2.hours,0,23);
  WATCH_INT(alarm.minutes,0,59);
  WATCH_INT(alarm.hours,0,23);
  WATCH_INT(time1.tm_sec,0,59);
  WATCH_INT(time1.tm_min,0,59);
  WATCH_INT(time1.tm_hour,0,23);
  WATCH_INT(time1.tm_mday,1,31);
  WATCH_INT2(time1.tm_wday,1,7,-1);
  WATCH_INT2(time1.tm_mon,1,12,-1);
  WATCH_INT2(time1.tm_year,1900,2155,-1900);
}

#undef WATCH_INT2
#undef WATCH_INT
#undef WATCH_BOOL
#undef WATCH_ATTR
