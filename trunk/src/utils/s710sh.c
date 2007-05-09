/* $Id: s710sh.c,v 1.7 2004/09/21 08:16:05 dave Exp $ */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include "s710.h"
#include "config.h"

/* static helper function */

static void explain_command  ( int c, attribute_map_t *m );


int
main ( int argc, char **argv )
{
  int               i;
  int               j;
  char              request[BUFSIZ];
  packet_t         *p = NULL;
  int               go;
  char             *filedir;
  char             *rend;
  char             *rbeg;
  char              path[PATH_MAX];
  int               ok;
  S710_Driver       d;
  attribute_map_t   map[S710_MAP_TYPE_COUNT];
  struct tm         now;
  time_t            t;
  static int        reset;

  ok = driver_init ( argc, argv, &d );

  if ( ok != 1 ) {
    printf("usage: %s [-d driver] [device file]\n",argv[0]);
    printf("\tdriver may be either serial, ir, or usb\n");
    printf("\tdevice file is required for serial and ir drivers.\n");
    exit(1);
  }

  if ( (filedir = getenv("S710_FILEDIR")) != NULL ) {
    filedir = realpath(filedir,path);
  } else {
    filedir = S710_FILEDIR;
  }

  setvbuf(stdout,NULL,_IONBF,0);

  if ( driver_open ( &d, S710_MODE_RDWR ) != -1 ) {

    /* initialize */

    memset(map,0,sizeof(map));

    load_user_attributes(&map[S710_MAP_TYPE_USER]);
    load_watch_attributes(&map[S710_MAP_TYPE_WATCH]);
    load_logo_attributes(&map[S710_MAP_TYPE_LOGO]);
    load_bike_attributes(&map[S710_MAP_TYPE_BIKE]);
    load_exercise_attributes(&map[S710_MAP_TYPE_EXERCISE]);
    load_reminder_attributes(&map[S710_MAP_TYPE_REMINDER]);

    printf("\ns710sh> ");

    /* main loop */

    while ( 1 ) {

      fgets(request,sizeof(request),stdin);
      if ( request == NULL || *request == '\n' ) {
	printf("s710sh> ");
	continue;
      }

      if ( is_like(request,"quit") ||
	   is_like(request,"exit") ) {

	/* quit */

	printf("\nExit\n");

	break;
      } else if ( is_like(request,"help") ||
		  is_like(request,"????") ) {

	/* print help */

	printf("\noptions:\n\n");
	for ( i = 0; i < num_packets(); i++ ) {
	  if ( i == S710_CONTINUE_TRANSFER ) continue;
	  p = packet(i);
	  printf("%s\n",p->name);
	}
	printf("synchronize time\n");
	printf("\n");
      }

      /* reset */

      go = 0;

      /* loop through available packets */

      for ( i = 0; i < num_packets(); i++ ) {
	
	/* we don't let people send 'continue transfer' packets directly */

	if ( i == S710_CONTINUE_TRANSFER ) continue;

	/* is the user trying to send this packet?  if so, break. */
	
	p = packet(i);
	if ( is_like(request,p->name) ) {
	  go = 1;
	  break;
	}
      }

      /* 
	 if we've got a packet, deal with it.  note: this could be done
	 much more elegantly, but i don't have time right now.
      */

      if ( go ) {
	if ( i >= S710_GET_OVERVIEW && i <= S710_CLOSE_CONNECTION ) {
	  handle_retrieval(&d,i,filedir);
	} else if ( i >= S710_SET_USER && i <= S710_SET_REMINDER_7 ) {

	  /* this is a transfer request.  we need to make sure we've 
	     seen the '{' character on the FIRST line.  we then need
	     to read as many lines as it takes until we get one with
	     a '}' character.  then we can process the request. */

	  rbeg = &request[0];
	  rend = strchr(request,'{');
	  if ( rend != NULL ) {

	    /* we've got the opening brace.  read until we get the 
	       closing brace.  sorry, no syntax checking. */

	    while ( !strchr(rend,'}') ) {
	      rend = strchr(rend,'\n');
	      fgets(rend,sizeof(request)-(rend-rbeg+1),stdin);
	    }
	    
	    /* ok, now we're ready to go. */

	    handle_transfer(&d,i,request,map);

	  } else {
	    printf("\nSyntax error: opening brace '{' required.\n\n");	    
	    explain_command(i,map);
	  }
	} else if ( i == S710_HARD_RESET ) { 

	  /* don't let people hard reset on the first try. */

	  if ( reset < 2 ) {
	    printf("\nAre you ");
	    for ( j = 0; j < reset; j++ ) {
	      printf("REALLY ");
	    }
	    printf("sure you want do reset to factory defaults?!\n");
	    printf("If so, type \"%s\" %d more time%s\n\n",p->name,
		   2-reset,(reset==1)?"":"s");
	    reset++;
	  } else {
	    printf("\nOK, you asked for it...\n\n");
	    send_packet(p,&d);
	    reset = 0;
	  }
	}
      } else {

	/* a special command. */

	if ( is_like(request,"synchronize time") ) {
	  t = time(NULL);
	  localtime_r(&t,&now);
	  snprintf(request,
		   sizeof(request),
		   "set watch { "
		   "time1.tm_year = %d, "
		   "time1.tm_mon = %d, "
		   "time1.tm_mday = %d, "
		   "time1.tm_hour = %d, "
		   "time1.tm_min = %d, "
		   "time1.tm_sec = %d, "
		   "time1.tm_wday = %d, "
		   "which_time = 1 }\n",
		   now.tm_year + 1900,
		   now.tm_mon + 1,
		   now.tm_mday,
		   now.tm_hour,
		   now.tm_min,		   
		   now.tm_sec,
		   now.tm_wday + 1);
	  handle_transfer(&d,S710_SET_WATCH,request,map);
	}
      }

      printf("s710sh> ");
    }

    driver_close(&d);
    for ( i = 0; i < S710_MAP_TYPE_COUNT; i++ ) {
      clear_attribute_map(&map[i]);
    }

  } else {
    fprintf(stderr,"unable to open port: %s\n",strerror(errno));
  }

  return 0;
}


/* print syntax for "set" commands */

static void
explain_command ( int c, attribute_map_t *m )
{
  static int       general;

  /* this is kind of stupid, but i figure the user doesn't need to see
     this portion of the instructions more than a few times before they
     'get it'. */

  if ( general < 3 ) {
    printf("\"Set\" commands are of the following form:\n\n"
	   "set <property> { attribute-value-list }\n\n"
	   "where attribute-value-list is a comma-separated list of\n"
	   "attribute-value pairs, and an attribute-value pair is of\n"
	   "the form\n\n"
	   "attribute = value\n\n"
	   "Attribute names depend on the property being set.  Values\n"
	   "are either strings enclosed in double quotation marks,\n"
	   "integer values, or boolean values.  Boolean values may be\n"
	   "represented by 1, 0, \"true\", or \"false\".  Integer\n"
	   "values are usually restricted to be within a certain range;\n"
	   "where this is the case, the range is given.  String values\n"
	   "are also sometimes restricted.\n\n"
	   "It is not necessary to provide the entire list of attribute-\n"
	   "value pairs if you wish to set a property.  The attributes\n"
	   "which you do not specify will not be changed.\n\n"
	   "You can shortcut attribute names in the same way that you can\n"
	   "shortcut commands: any unique abbreviation of the attribute\n"
	   "will be expanded to match.\n\n");
    printf("Example command strings:\n\n"
	   "set user { name = \"Dave\", gender = \"male\" }\n"
	   "set user { units = \"metric\", weight = 84, height = 190 }\n"
	   "set user { altimeter = 1, energy_expenditure = 1 }\n\n");
    printf("Example command strings, maximally abbreviated:\n\n"
	   "s u { n = \"Dave\", g = \"male\" }\n"
	   "s u { un = \"metric\", w = 84, h = 190 }\n"
	   "s u { al = 1, e = 1 }\n\n");
    general++;
  }

  /* now for the specific instructions */

  switch ( c ) {
  case S710_SET_USER:
    print_attribute_map(c,&m[S710_MAP_TYPE_USER]);
    break;
  case S710_SET_WATCH:
    print_attribute_map(c,&m[S710_MAP_TYPE_WATCH]);
    break;
  case S710_SET_LOGO:
    print_attribute_map(c,&m[S710_MAP_TYPE_LOGO]);
    break;
  case S710_SET_BIKE:
    print_attribute_map(c,&m[S710_MAP_TYPE_BIKE]);
    break;
  case S710_SET_EXERCISE_1:
  case S710_SET_EXERCISE_2:
  case S710_SET_EXERCISE_3:
  case S710_SET_EXERCISE_4:
  case S710_SET_EXERCISE_5:
    print_attribute_map(c,&m[S710_MAP_TYPE_EXERCISE]);
    break;
  case S710_SET_REMINDER_1:
  case S710_SET_REMINDER_2:
  case S710_SET_REMINDER_3:
  case S710_SET_REMINDER_4:
  case S710_SET_REMINDER_5:
  case S710_SET_REMINDER_6:
  case S710_SET_REMINDER_7:
    print_attribute_map(c,&m[S710_MAP_TYPE_REMINDER]);
    break;
  default:
    break;
  }
}


