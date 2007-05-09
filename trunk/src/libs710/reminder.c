/* $Id: reminder.c,v 1.5 2002/12/28 07:51:49 dave Exp $ */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


/* requests and reads reminder data over fd into the data structure */

int
get_reminder ( S710_Driver *d, S710_Packet_Index which, reminder_t *reminder )
{
  packet_t *p;
  int       ok = 0;

  if ( which < S710_GET_REMINDER_1 || 
       which > S710_GET_REMINDER_7 )
    return 0;

  p = get_response ( which, d );
  if ( p != NULL) {
    reminder->which = p->data[0] + 1;
    extract_label ( &p->data[6], &reminder->label, 7 );
    memset(&reminder->date,0,sizeof(reminder->date));
    reminder->date.tm_sec  = 0;
    reminder->date.tm_min  = BCD(p->data[1]);
    reminder->date.tm_hour = BCD(p->data[2]);
    reminder->date.tm_mday = BCD(p->data[3]);
    reminder->date.tm_mon  = LNIB(p->data[5]) - 1;
    reminder->date.tm_year = BCD(p->data[4]) + 100;
    reminder->date.tm_wday = 0;
    reminder->on           = (UNIB(p->data[5]) & 0x01) ? S710_ON : S710_OFF;
    reminder->exercise     = LNIB(p->data[13]);
    reminder->repeat       = UNIB(p->data[13]);
    free ( p );
    ok = 1;
  }

  return ok;
}


/* set reminder */

int
set_reminder ( reminder_t *reminder, S710_Packet_Index which, S710_Driver *d )
{
  packet_t  *p;
  int        lower = S710_SET_REMINDER_1;
  int        upper = S710_SET_REMINDER_7;

  if ( which < lower || which > upper ) {
    fprintf(stderr,"set_reminder: index %d outside of [%d,%d]\n",
	    which,lower,upper);
    return 0;
  }
  
  p = make_set_packet(which);
  if ( p == NULL ) return 0;

  p->data[0] = reminder->which - 1;
  p->data[1] = HEX(reminder->date.tm_min);
  p->data[2] = HEX(reminder->date.tm_hour);
  p->data[3] = HEX(reminder->date.tm_mday);
  p->data[4] = HEX(reminder->date.tm_year - 100);
  p->data[5] = BINU(reminder->on) | BINL(reminder->date.tm_mon + 1);

  encode_label(reminder->label,&p->data[6],7);

  p->data[13] = BINU(reminder->repeat) | BINL(reminder->exercise);

  return send_set_packet(p,d);
}


/* prints out the reminder data */

void
print_reminder ( reminder_t *r, FILE *fp )
{
  char  buf[BUFSIZ];
  char *a;

  fprintf(fp,"\n");
  fprintf(fp,"Reminder %d:  %s\n",r->which,r->label);
  strftime(buf,sizeof(buf),"%d %B %Y %H:%M",&r->date);
  fprintf(fp,"Date:        %s\n",buf);
  fprintf(fp,"On/Off:      %s\n",(r->on)?"On":"Off");
  
  switch ( r->exercise ) {
  case S710_EXE_NONE:       a = "None";      break;
  case S710_EXE_BASIC_USE:  a = "Basic Use"; break;
  case S710_EXE_SET_1:      a = "Exe Set 1"; break;
  case S710_EXE_SET_2:      a = "Exe Set 2"; break;
  case S710_EXE_SET_3:      a = "Exe Set 3"; break;
  case S710_EXE_SET_4:      a = "Exe Set 4"; break;
  case S710_EXE_SET_5:      a = "Exe Set 5"; break;
  default:                  a = "Unknown";   break;
  }
  
  fprintf(fp,"Exercise:    %s\n",a);
  
  switch ( r->repeat ) {
  case S710_REPEAT_OFF:     a = "Off";       break;
  case S710_REPEAT_HOURLY:  a = "Hourly";    break;
  case S710_REPEAT_DAILY:   a = "Daily";     break;
  case S710_REPEAT_WEEKLY:  a = "Weekly";    break;
  case S710_REPEAT_MONTHLY: a = "Monthly";   break;
  case S710_REPEAT_YEARLY:  a = "Yearly";    break;
  default:                  a = "Unknown";   break;
  }
  
  fprintf(fp,"Repeat:      %s\n",a);
  
  fprintf(fp,"\n");
}


#define REMINDER_ATTR(a)       m,#a,&m->data.reminder.a
#define REMINDER_BOOL(a)       add_boolean_attribute(REMINDER_ATTR(a))
#define REMINDER_INT(a,b,c)    add_integer_attribute(REMINDER_ATTR(a),b,c,0)
#define REMINDER_INT2(a,b,c,d) add_integer_attribute(REMINDER_ATTR(a),b,c,d)
#define REMINDER_STRING(a,b)   add_string_attribute(REMINDER_ATTR(a),b)

void
load_reminder_attributes ( attribute_map_t *m )
{
  REMINDER_BOOL(on);
  
  add_enum_string_attribute(REMINDER_ATTR(repeat),
			    "off",     S710_REPEAT_OFF,
			    "hourly",  S710_REPEAT_HOURLY,
			    "daily",   S710_REPEAT_DAILY,
			    "weekly",  S710_REPEAT_WEEKLY,
			    "monthly", S710_REPEAT_MONTHLY,
			    "yearly",  S710_REPEAT_YEARLY,
			    NULL);

  REMINDER_INT(exercise,1,5);
  REMINDER_INT2(date.tm_year,2000,2255,-1900);
  REMINDER_INT2(date.tm_mon,1,12,-1);
  REMINDER_INT(date.tm_mday,1,31);
  REMINDER_INT(date.tm_hour,0,23);
  REMINDER_INT(date.tm_min,0,59);
  REMINDER_STRING(label,7);
}

#undef REMINDER_STRING
#undef REMINDER_INT2
#undef REMINDER_INT
#undef REMINDER_BOOL
#undef REMINDER_ATTR
