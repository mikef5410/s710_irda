/* $Id: user.c,v 1.7 2004/09/21 08:16:05 dave Exp $ */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


/* requests and reads the user's data over fd into the data structure */

int
get_user ( S710_Driver *d, user_t *user )
{
  packet_t *p;
  int       ok = 0;

  p = get_response ( S710_GET_USER, d );
  if ( p != NULL) {
    user->user_id = BCD(p->data[13]);
    extract_label ( &p->data[14], &user->name, 7 );
    user->gender = (p->data[10] & 0x01) ? S710_GENDER_FEMALE:S710_GENDER_MALE;
    user->activity_level = UNIB(p->data[7]);
    user->weight = p->data[3];
    user->height = p->data[4];
    user->units  = (p->data[1] & 0x02) ? S710_UNITS_ENGLISH:S710_UNITS_METRIC;
    user->vo2max = p->data[8];
    user->max_hr = p->data[9];
    user->unknown[0] = p->data[11];
    user->unknown[1] = p->data[12];
    user->recording_interval    = UNIB(p->data[2]);
    user->heart_touch           = LNIB(p->data[2]);
    set_user_birthday(user,BCD(p->data[5]),LNIB(p->data[7]),BCD(p->data[6]));
    user->altimeter             = (p->data[0] & 0x08) ? 1 : 0;
    user->fitness_test          = (p->data[0] & 0x04) ? 1 : 0;
    user->predict_hr_max        = (p->data[0] & 0x02) ? 1 : 0;
    user->energy_expenditure    = (p->data[0] & 0x01) ? 1 : 0;
    user->options_lock          = (p->data[1] & 0x08) ? 1 : 0;
    user->help                  = (p->data[1] & 0x04) ? 1 : 0;
    user->activity_button_sound = (p->data[1] & 0x01) ? 1 : 0;
    free ( p );
    ok = 1;
  }

  return ok;
}


int
set_user ( user_t *u, S710_Driver *d )
{
  packet_t  *p;

  p = make_set_packet(S710_SET_USER);
  if ( p == NULL ) return 0;

  p->data[0]  = (u->altimeter << 3) | (u->fitness_test << 2) |
    (u->predict_hr_max << 1) | (u->energy_expenditure);
  p->data[1]  = (u->options_lock << 3) | (u->help << 2) |
    (u->activity_button_sound);
  if ( u->units == S710_UNITS_ENGLISH ) p->data[1] |= 0x02;
  p->data[2]  = (u->recording_interval << 4) | u->heart_touch;
  p->data[3]  = u->weight;
  p->data[4]  = u->height;
  p->data[5]  = HEX(u->birth_date.tm_mday);
  p->data[6]  = HEX(u->birth_date.tm_year);
  p->data[7]  = BINU(u->activity_level) | BINL(u->birth_date.tm_mon + 1);
  p->data[8]  = u->vo2max;
  p->data[9]  = u->max_hr;
  p->data[10] = (u->gender == S710_GENDER_MALE) ? 0x50 : 0x51;
  p->data[11] = u->unknown[0];
  p->data[12] = u->unknown[1];
  p->data[13] = HEX(u->user_id);
  encode_label(u->name,&p->data[14],7);

  return send_set_packet(p,d);
}


/* a couple of helper functions for the nontrivial settings */

void
set_user_name ( user_t *u, char *name )
{
  strncpy((char *)u->name,name,sizeof(u->name));
}


void
set_user_birthday ( user_t *u, int day, int month, int year )
{
  time_t    bd;

  memset(&u->birth_date,0,sizeof(u->birth_date));
  u->birth_date.tm_year = year % 200;  /* years since 1900 */
  u->birth_date.tm_mon  = month - 1;   /* count from 0     */
  u->birth_date.tm_mday = day;
  bd = mktime(&u->birth_date);
  memcpy(&u->birth_date,localtime(&bd),sizeof(u->birth_date));
}


/* prints out the user data */

void
print_user ( user_t *u, FILE *fp )
{
  char  buf[BUFSIZ];
  char *a;

  fprintf(fp,"\nUser ID:               %d\n",u->user_id);
  fprintf(fp,"Name:                  %s\n",u->name);
  fprintf(fp,"Gender:                %s\n",(u->gender)?"Female":"Male");
  
  switch ( u->activity_level ) {
  case S710_ACTIVITY_LOW:     a = "Low";     break;
  case S710_ACTIVITY_MEDIUM:  a = "Medium";  break;
  case S710_ACTIVITY_HIGH:    a = "High";    break;
  case S710_ACTIVITY_TOP:     a = "Top";     break;
  default:                    a = "Unknown"; break;
  }
  
  fprintf(fp,"Activity:              %s\n",a);
  fprintf(fp,"Weight:                %d %s\n",
	  u->weight,(u->units==S710_UNITS_ENGLISH)?"lbs":"kg");
  fprintf(fp,"Height:                %d %s\n",
	  u->height,(u->units==S710_UNITS_ENGLISH)?"in":"cm");

  strftime(buf,sizeof(buf),"%A, %d %B %Y",&u->birth_date);

  fprintf(fp,"Born on                %s\n",buf);
  fprintf(fp,"VO2 Max:               %d ml/kg/min\n",u->vo2max);
  fprintf(fp,"Max HR:                %d bpm\n",u->max_hr);
  
  switch ( u->recording_interval ) {
  case S710_RECORD_INT_05:    a = "5";              break;
  case S710_RECORD_INT_15:    a = "15";             break;
  case S710_RECORD_INT_60:    a = "60";             break;
  default:                    a = "??";             break;
  }

  fprintf(fp,"Recording:             %s second intervals\n",a);

  switch ( u->heart_touch ) {
  case S710_HT_SHOW_LIMITS:   a = "Show Limits";    break;
  case S710_HT_STORE_LAP:     a = "Store Lap";      break;
  case S710_HT_SWITCH_DISP:   a = "Switch Display"; break;
  default:                    a = "Unknown";        break;
  }

  fprintf(fp,"Heart Touch:           %s\n",a);
  fprintf(fp,"Altimeter:             %s\n",(u->altimeter)?"On":"Off");
  fprintf(fp,"Fitness Test:          %s\n",(u->fitness_test)?"On":"Off");
  fprintf(fp,"Predict HR Max:        %s\n",(u->predict_hr_max)?"On":"Off");
  fprintf(fp,"Energy Expenditure:    %s\n",(u->energy_expenditure)?"On":"Off");
  fprintf(fp,"Options Lock:          %s\n",(u->options_lock)?"On":"Off");
  fprintf(fp,"Help:                  %s\n",(u->help)?"On":"Off");
  fprintf(fp,"Units:                 %s\n",
	  (u->units==S710_UNITS_ENGLISH)?"lb/ft/mi":"kg/cm/km");
  fprintf(fp,"Activity/Button Sound: %s\n",
	  (u->activity_button_sound)?"On":"Off");
  fprintf(fp,"\n");
}


/* load the user attribute/value pairs (load in reverse order) */

#define USER_ATTR(a)       m,#a,&m->data.user.a
#define USER_BOOL(a)       add_boolean_attribute(USER_ATTR(a))
#define USER_INT(a,b,c)    add_integer_attribute(USER_ATTR(a),b,c,0)
#define USER_INT2(a,b,c,d) add_integer_attribute(USER_ATTR(a),b,c,d)
#define USER_STRING(a,b)   add_string_attribute(USER_ATTR(a),b)

void
load_user_attributes ( attribute_map_t *m )
{
  add_enum_string_attribute(USER_ATTR(heart_touch),
			    "show limits",    S710_HT_SHOW_LIMITS,
			    "store lap",      S710_HT_STORE_LAP,
			    "switch display", S710_HT_SWITCH_DISP,
			    NULL);

  USER_BOOL(activity_button_sound);
  USER_BOOL(help);
  USER_BOOL(options_lock);
  USER_BOOL(energy_expenditure);
  USER_BOOL(predict_hr_max);
  USER_BOOL(fitness_test);
  USER_BOOL(altimeter);
  USER_INT(max_hr,0,255);
  USER_INT(vo2max,0,255);
  USER_INT(height,0,255);
  USER_INT(weight,0,255);
  USER_INT(birth_date.tm_mday,1,31);
  USER_INT2(birth_date.tm_mon,1,12,-1);
  USER_INT2(birth_date.tm_year,1900,2155,-1900);

  add_enum_string_attribute(USER_ATTR(activity_level),
			    "low",    S710_ACTIVITY_LOW,
			    "medium", S710_ACTIVITY_MEDIUM,
			    "high",   S710_ACTIVITY_HIGH,
			    "top",    S710_ACTIVITY_TOP,
			    NULL);
  add_enum_integer_attribute(USER_ATTR(recording_interval),
			     5,       S710_RECORD_INT_05,
			     15,      S710_RECORD_INT_15,
			     60,      S710_RECORD_INT_60,
			     -1);
  add_enum_string_attribute(USER_ATTR(units),
			    "metric",  S710_UNITS_METRIC,
			    "english", S710_UNITS_ENGLISH,
			    NULL);
  add_enum_string_attribute(USER_ATTR(gender),
			    "male",    S710_GENDER_MALE,
			    "female",  S710_GENDER_FEMALE,
			    NULL);
  USER_STRING(name,7);
  USER_INT(user_id,0,99);
}

#undef USER_STRING
#undef USER_INT2
#undef USER_INT
#undef USER_BOOL
#undef USER_ATTR
