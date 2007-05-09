/* $Id: exercise.c,v 1.6 2002/12/28 07:51:49 dave Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


/* requests and reads exercise data over fd into the data structure */

int
get_exercise ( S710_Driver *d, S710_Packet_Index which, exercise_t *exercise )
{
  packet_t *p;
  int       ok = 0;
  int       i;

  if ( which < S710_GET_EXERCISE_1 || 
       which > S710_GET_EXERCISE_5 )
    return 0;

  p = get_response ( which, d );
  if ( p != NULL) {
    exercise->which = p->data[0];
    extract_label(&p->data[1],&exercise->label,7);
    for ( i = 0; i < 3; i++ ) {
      exercise->timer[i].hours      = BCD(p->data[9+2*i]);
      exercise->timer[i].minutes    = BCD(p->data[8+2*i]);
      exercise->hr_limit[i].lower   = p->data[14+2*i];
      exercise->hr_limit[i].upper   = p->data[15+2*i];
    }
    exercise->recovery_time.hours   = BCD(p->data[21]);
    exercise->recovery_time.minutes = BCD(p->data[20]);
    exercise->recovery_hr           = p->data[22];
    free ( p );
    ok = 1;
  }

  return ok;
}


/* set exercise */

int
set_exercise ( exercise_t *exercise, S710_Packet_Index which, S710_Driver *d )
{
  packet_t  *p;
  int        lower = S710_SET_EXERCISE_1;
  int        upper = S710_SET_EXERCISE_5;
  int        i;

  if ( which < lower || which > upper ) {
    fprintf(stderr,"set_exercise: index %d outside of [%d,%d]\n",
	    which,lower,upper);
    return 0;
  }

  p = make_set_packet(which);
  if ( p == NULL ) return 0;

  p->data[0] = exercise->which;
  encode_label(exercise->label,&p->data[1],7);

  for ( i = 0; i < 3; i++ ) {
    p->data[9+2*i]  = HEX(exercise->timer[i].hours);
    p->data[8+2*i]  = HEX(exercise->timer[i].minutes);
    p->data[14+2*i] = exercise->hr_limit[i].lower;
    p->data[15+2*i] = exercise->hr_limit[i].upper;
  }

  p->data[20] = HEX(exercise->recovery_time.minutes);
  p->data[21] = HEX(exercise->recovery_time.hours);
  p->data[22] = exercise->recovery_hr;

  return send_set_packet(p,d);
}


/* prints out the exercise data */

void
print_exercise ( exercise_t *e, FILE *fp )
{
  int i;

  fprintf(fp,"\n");
  fprintf(fp,"Exercise %d:     %s\n",e->which,e->label);

  /* timers */

  for ( i = 0; i < 3; i++ ) {
    fprintf(fp,"Timer %d:        %d:%02d\n",
	   i+1,e->timer[i].hours,e->timer[i].minutes);
  }

  /* HR limits */

  for ( i = 0; i < 3; i++ ) {
    printf("HR Limit %d:     %d to %3d\n",
	   i+1,e->hr_limit[i].lower,e->hr_limit[i].upper);
  }

  /* recovery time, HR */

  fprintf(fp,"Recovery time:  %d:%02d\n",
	  e->recovery_time.hours,
	  e->recovery_time.minutes);
  fprintf(fp,"Recovery HR:    %d\n",
	  e->recovery_hr);
  
  fprintf(fp,"\n");
}


#define EXERCISE_ATTR(a)       m,#a,&m->data.exercise.a
#define EXERCISE_INT(a,b,c)    add_integer_attribute(EXERCISE_ATTR(a),b,c,0)
#define EXERCISE_STRING(a,b)   add_string_attribute(EXERCISE_ATTR(a),b)

void
load_exercise_attributes ( attribute_map_t *m )
{
  EXERCISE_INT(recovery_hr,0,255);
  EXERCISE_INT(recovery_time.minutes,0,59);
  EXERCISE_INT(recovery_time.hours,0,99);
  EXERCISE_INT(hr_limit[2].upper,0,255);
  EXERCISE_INT(hr_limit[2].lower,0,255);
  EXERCISE_INT(hr_limit[1].upper,0,255);
  EXERCISE_INT(hr_limit[1].lower,0,255);
  EXERCISE_INT(hr_limit[0].upper,0,255);
  EXERCISE_INT(hr_limit[0].lower,0,255);
  EXERCISE_INT(timer[2].minutes,0,59);
  EXERCISE_INT(timer[2].hours,0,23);
  EXERCISE_INT(timer[1].minutes,0,59);
  EXERCISE_INT(timer[1].hours,0,23);
  EXERCISE_INT(timer[0].minutes,0,59);
  EXERCISE_INT(timer[0].hours,0,23);
  EXERCISE_STRING(label,7);
}

#undef EXERCISE_STRING
#undef EXERCISE_INT
#undef EXERCISE_ATTR
