/* $Id: srdplot.c,v 1.7 2004/11/13 10:53:49 dave Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "s710.h"

/* externs */

extern char *optarg;
extern int   optind;

/* static functions */

static void do_plots ( workout_t  *w, 
		       int         y_axis, 
		       char       *c, 
		       int         r,
		       int         g,
		       int         b,
		       int         tic_opts,
		       int         width, 
		       int         height );
void        do_hist ( workout_t *w, 
		      int        what, 
		      char      *c, 
		      int        r, 
		      int        g, 
		      int        b, 
		      int        min,
		      int        max,
		      int        width, 
		      int        height );

/* main program */

int
main ( int argc, char **argv )
{
  int                i;
  workout_t         *w;
  int                ch;
  S710_Filter        filter = S710_FILTER_OFF;

  while ( (ch = getopt(argc,argv,"f")) != -1 ) {
    switch (ch) {
    case 'f':
      filter = S710_FILTER_ON;
      break;
    }
  }
  argc -= optind;
  argv += optind;

  for ( i = 0; i < argc; i++ ) {
    w = read_workout(argv[i],filter,S710_HRM_AUTO);
    if ( w != NULL ) {
      
      do_plots(w,S710_Y_AXIS_HEART_RATE,"hr",0xff,0,0,
	       S710_TIC_LINES,640,360);
      do_plots(w,S710_Y_AXIS_ALTITUDE,"alt",0,0xff,0,
	       S710_TIC_LINES|S710_TIC_SHADE,640,360);
      do_plots(w,S710_Y_AXIS_SPEED,"spd",0,0,0xff,
	       S710_TIC_LINES,640,360);
      do_plots(w,S710_Y_AXIS_CADENCE,"cad",0,0x66,0x66,
	       S710_TIC_LINES,640,360);
      do_plots(w,S710_Y_AXIS_POWER,"pwr",0,0x66,0x66,
	       S710_TIC_LINES,640,360);
      do_hist(w,S710_Y_AXIS_HEART_RATE,"hr",0xff,0,0,0,200,320,320);
      do_hist(w,S710_Y_AXIS_CADENCE,"cad",0,0,0xff,0,200,320,320);
      do_hist(w,S710_Y_AXIS_POWER,"pwr",0,0x66,0x66,0,500,320,320);
      
      free_workout(w);
    } else {
      printf("%s: invalid file\n",argv[i]);
    }
  }

  return 0;
}


void
do_hist ( workout_t *w, 
	  int        what, 
	  char      *c, 
	  int        r, 
	  int        g, 
	  int        b, 
	  int        min,
	  int        max,
	  int        width, 
	  int        height )
{
  char              buf[BUFSIZ];
  char              tmbuf[128];
  struct timeval    ti;
  struct timeval    tf;
  float             el;
  byte_histogram_t *h;
  int               ok = 0;

  if ( (h = make_byte_histogram(w,NULL,0,what)) != NULL ) {

    strftime(tmbuf,sizeof(tmbuf),"%Y%m%dT%H%M%S",localtime(&w->unixtime));

    sprintf(buf,"%s.%05d.%s.h.png",tmbuf,w->bytes,c);
    gettimeofday(&ti,NULL);
    ok = plot_byte_histogram(h,r,g,b,min,max,width,height,1,buf);
    gettimeofday(&tf,NULL);
    el = tf.tv_sec - ti.tv_sec + (tf.tv_usec-ti.tv_usec)/1000000.0;
    if ( ok ) printf("Wrote %s in %f seconds\n",buf,el);

    sprintf(buf,"%s.%05d.%s.z.png",tmbuf,w->bytes,c);
    gettimeofday(&ti,NULL);
    ok = plot_byte_zones(h,width,height,buf);
    gettimeofday(&tf,NULL);
    el = tf.tv_sec - ti.tv_sec + (tf.tv_usec-ti.tv_usec)/1000000.0;
    if ( ok ) printf("Wrote %s in %f seconds\n",buf,el);
    
    free_byte_histogram(h);
  }
}


void 
do_plots ( workout_t *w, 
	   int        y_axis, 
	   char      *c, 
	   int        r,
	   int        g,
	   int        b,
	   int        tic_opts,
	   int        width, 
	   int        height )
{
  char             buf[BUFSIZ];
  char             tmbuf[128];
  struct timeval   ti;
  struct timeval   tf;
  float            el;
  int              ok = 0;

  strftime(tmbuf,sizeof(tmbuf),"%Y%m%dT%H%M%S",localtime(&w->unixtime));

  snprintf(buf,sizeof(buf),"%s.%05d.%s.t.png",tmbuf,w->bytes,c);
  gettimeofday(&ti,NULL);
  ok = plot_workout_xy(w,y_axis,S710_X_AXIS_TIME,r,g,b,
		       tic_opts,width,height,buf);  
  gettimeofday(&tf,NULL);
  el = tf.tv_sec - ti.tv_sec + (tf.tv_usec-ti.tv_usec)/1000000.0;
  if ( ok ) printf("Wrote %s in %f seconds\n",buf,el);

  snprintf(buf,sizeof(buf),"%s.%05d.%s.d.png",
	   tmbuf,w->bytes,c);
  gettimeofday(&ti,NULL);
  ok = plot_workout_xy(w,y_axis,S710_X_AXIS_DISTANCE,
		       r,g,b,tic_opts,width,height,buf);
  gettimeofday(&tf,NULL);
  el = tf.tv_sec - ti.tv_sec + (tf.tv_usec-ti.tv_usec)/1000000.0;
  if ( ok ) printf("Wrote %s in %f seconds\n",buf,el);
}

