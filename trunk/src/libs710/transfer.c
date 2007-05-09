/* $Id: transfer.c,v 1.2 2002/12/28 07:51:49 dave Exp $ */

#include <stdio.h>
#include "s710.h"

void
handle_retrieval ( S710_Driver *d, int r, char *filedir )
{
  overview_t     ov;
  user_t         user;
  watch_t        watch;
  logo_t         logo;
  bike_t         bike;
  exercise_t     ex;
  reminder_t     rm;
  files_t        files;
  
  switch ( r ) {
  case S710_GET_OVERVIEW:
    if ( get_overview(d,&ov) )        print_overview(&ov,stdout); break;
  case S710_GET_USER:
    if ( get_user(d,&user) )          print_user(&user,stdout);   break;
  case S710_GET_WATCH:
    if ( get_watch(d,&watch) )        print_watch(&watch,stdout); break;
  case S710_GET_LOGO:
    if ( get_logo(d,&logo) )          print_logo(&logo,stdout);   break;
  case S710_GET_BIKE:
    if ( get_bike(d,&bike) )          print_bike(&bike,stdout);   break;
  case S710_GET_EXERCISE_1:
  case S710_GET_EXERCISE_2:
  case S710_GET_EXERCISE_3:
  case S710_GET_EXERCISE_4:
  case S710_GET_EXERCISE_5:
    if ( get_exercise(d,r,&ex) )      print_exercise(&ex,stdout); break;
  case S710_GET_REMINDER_1:
  case S710_GET_REMINDER_2:
  case S710_GET_REMINDER_3:
  case S710_GET_REMINDER_4:
  case S710_GET_REMINDER_5:
  case S710_GET_REMINDER_6:
  case S710_GET_REMINDER_7:
    if ( get_reminder(d,r,&rm) )      print_reminder(&rm,stdout); break;
  case S710_GET_FILES:
    if ( get_files(d,&files,stdout) ) {
      save_files(&files,filedir,stdout);
      print_files(&files,stdout); 
    }                                                             break;
  case S710_CLOSE_CONNECTION:
    close_connection(d);                                          break;
  default: 	                                                  break;
  }
}


#define TRANSFER1(a,b)                                             \
    m = &maps[S710_MAP_TYPE_##b];                                  \
    a = &m->data.a;                                                \
    parse_attribute_pairs(request,m);                              \
    if ( m->oosync ) {                                             \
      if ( get_##a(d,a) ) {                                        \
	merge_attribute_map(m);                                    \
	if ( set_##a(a,d) ) {                                      \
	  sync_attribute_map(m);                                   \
	  get_##a(d,a);                                            \
	}                                                          \
	print_##a(a,stdout);                                       \
      } else {                                                     \
	printf("\nFailed\n\n");                                    \
      }                                                            \
    } else {                                                       \
      printf("\nNo legal " #a " attributes specified\n\n");        \
    }

#define TRANSFER2(a,b)                                             \
    m = &maps[S710_MAP_TYPE_##b];                                  \
    a = &m->data.a;                                                \
    parse_attribute_pairs(request,m);                              \
    if ( m->oosync ) {                                             \
      diff = S710_SET_##b##_1 - S710_GET_##b##_1;                  \
      if ( get_##a(d,r-diff,a) ) {                                 \
	merge_attribute_map(m);                                    \
	if ( set_##a(a,r,d) ) {                                    \
	  sync_attribute_map(m);                                   \
	  get_##a(d,r-diff,a);                                     \
	}                                                          \
	print_##a(a,stdout);                                       \
      } else {                                                     \
	printf("\nFailed\n\n");                                    \
      }                                                            \
    } else {                                                       \
      printf("\nNo legal " #a " attributes specified\n\n");        \
    }

void
handle_transfer ( S710_Driver *d, int r, char *request, attribute_map_t *maps )
{
  user_t                 *user;
  watch_t                *watch;
  logo_t                 *logo;
  bike_t                 *bike;
  exercise_t             *exercise;
  reminder_t             *reminder;
  attribute_map_t        *m = NULL;
  int                     diff;

  printf("\nRequest: %s\n",request);

  switch ( r ) {
  case S710_SET_USER:        TRANSFER1(user,USER);         break;
  case S710_SET_WATCH:       TRANSFER1(watch,WATCH);       break;
  case S710_SET_LOGO:        TRANSFER1(logo,LOGO);         break; 
  case S710_SET_BIKE:        TRANSFER1(bike,BIKE);         break;
  case S710_SET_EXERCISE_1:
  case S710_SET_EXERCISE_2:
  case S710_SET_EXERCISE_3:
  case S710_SET_EXERCISE_4:
  case S710_SET_EXERCISE_5:  TRANSFER2(exercise,EXERCISE); break;
  case S710_SET_REMINDER_1:
  case S710_SET_REMINDER_2:
  case S710_SET_REMINDER_3:
  case S710_SET_REMINDER_4:
  case S710_SET_REMINDER_5:
  case S710_SET_REMINDER_6:
  case S710_SET_REMINDER_7:  TRANSFER2(reminder,REMINDER); break;
  default:                                                 break;
  }
}

#undef TRANSFER1
#undef TRANSFER2
