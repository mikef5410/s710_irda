/* $Id: plot.c,v 1.7 2004/11/13 10:53:48 dave Exp $ */

#include <math.h>
#include <gdfonts.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "s710.h"

#define YMINMAX(a) \
    ymin = 1.0e6; \
    for ( i = 0; i < w->samples; i++ ) { \
      if ( w->a[i] / p->divisor > ymax ) ymax = w->a[i] / p->divisor; \
      if ( w->a[i] / p->divisor < ymin ) ymin = w->a[i] / p->divisor; \
    }

#define YMINMAXPOWER(a) \
    ymin = 1.0e6; \
    for ( i = 0; i < w->samples; i++ ) { \
      if ( w->power_data[i].a / p->divisor > ymax ) \
        ymax = w->power_data[i].a / p->divisor; \
      if ( w->power_data[i].a / p->divisor < ymin ) \
        ymin = w->power_data[i].a / p->divisor; \
    }

#define SAMPLES(a) \
      if ( x_axis == S710_X_AXIS_TIME ) { \
	for ( i = 0; i < w->samples; i++ ) \
	  c[i+1].x = x0 + ppx * i * w->recording_interval; \
      } else if ( x_axis == S710_X_AXIS_DISTANCE && \
	          ( w->mode & S710_MODE_SPEED ) ) { \
	for ( i = 0; i < w->samples; i++ ) \
	  c[i+1].x = x0 + ppx * w->dist_data[i] * 1000.0; \
      } \
      for ( i = 0; i < w->samples; i++ ) \
        c[i+1].y = y1 - ppy * (w->a[i] / p->divisor - ymin);

#define SAMPLESPOWER(a) \
      if ( x_axis == S710_X_AXIS_TIME ) { \
	for ( i = 0; i < w->samples; i++ ) \
	  c[i+1].x = x0 + ppx * i * w->recording_interval; \
      } else if ( x_axis == S710_X_AXIS_DISTANCE && \
	          ( w->mode & S710_MODE_SPEED ) ) { \
	for ( i = 0; i < w->samples; i++ ) \
	  c[i+1].x = x0 + ppx * w->dist_data[i] * 1000.0; \
      } \
      for ( i = 0; i < w->samples; i++ ) \
        c[i+1].y = y1 - ppy * (w->power_data[i].a / p->divisor - ymin);


/* 
   y_axis: S710_Y_AXIS_HEART_RATE
           S710_Y_AXIS_ALTITUDE
	   S710_Y_AXIS_SPEED
	   S710_Y_AXIS_CADENCE
	   S710_Y_AXIS_POWER

   x_axis: S710_X_AXIS_TIME
           S710_X_AXIS_DISTANCE

   returns 1 if successful, 0 if unable to do it.  errors go to stderr.
   output goes to filename.
*/

typedef struct plot_info_t {
  float divisor;
  char *units;
} plot_info_t;


static plot_info_t gPlotInfoMetric[] = {
  { 1.0,  "bpm" },
  { 1.0,  "m"   },
  { 16.0, "kph" },
  { 1.0,  "rpm" },
  { 1.0,  "W"   }
};

static plot_info_t gPlotInfoEnglish[] = {
  { 1.0,  "bpm" },
  { 1.0,  "ft"  },
  { 16.0, "mph" },
  { 1.0,  "rpm" },
  { 1.0,  "W"   }
};

int
plot_workout_xy ( workout_t *w, 
		  int        y_axis, 
		  int        x_axis, 
		  int        r,
		  int        g,
		  int        b,
		  int        tic_opts,
		  int        width,
		  int        height,
		  char      *filename )
{
  gdImagePtr    im;
  FILE         *fp;
  int           black;  /* lines, tics, labels, etc */
  int           white;  /* background */ 
  int           pixel;  /* plot data */
  int           ok = 0;
  plot_info_t  *p = NULL;
  gdPoint      *c;
  int           x0;
  int           x1;
  int           y0;
  int           y1;
  int           xmax = 0;
  float         ymin = 0;
  float         ymax = 0;
  float         ppx  = 0;
  float         ppy  = 0;
  int           i;
  int           got = 0;

  /* make sure we've got the data */

  switch ( y_axis ) {
  case S710_Y_AXIS_HEART_RATE: got = 1; break;
  case S710_Y_AXIS_ALTITUDE:   got = (w->mode & S710_MODE_ALTITUDE); break;
  case S710_Y_AXIS_SPEED:      got = (w->mode & S710_MODE_SPEED);    break;
  case S710_Y_AXIS_CADENCE:    got = (w->mode & S710_MODE_CADENCE);  break;
  case S710_Y_AXIS_POWER:      got = (w->mode & S710_MODE_POWER);    break;
  default:                                                           break;
  }

  if ( got == 0 ) {
    fprintf(stderr,"y_axis [%d] not available\n",y_axis);
    return 0;
  } else {
    p = (w->units.system == S710_UNITS_METRIC) ? 
      &gPlotInfoMetric[y_axis] : 
      &gPlotInfoEnglish[y_axis];
  }

  /* don't let the image size get too absurd. */
  
  CLAMP(width,50,1600);
  CLAMP(height,50,1200);

  /* some basic constants */

  x0 = S710_X_MARGIN + 6 * (gdFontSmall->w + 1);
  x1 = width - S710_X_MARGIN;
  y0 = S710_Y_MARGIN;
  y1 = height - S710_Y_MARGIN - 10;
  
  /* get the min and max values for the x coordinate */
  
  if ( x_axis == S710_X_AXIS_TIME ) {
    xmax = w->samples * w->recording_interval;        /* seconds */
  } else if ( x_axis == S710_X_AXIS_DISTANCE && 
	      ( w->mode & S710_MODE_SPEED ) ) {
    xmax = ceil(w->dist_data[w->samples-1] * 1000.0); /* meters */
  } else {
    fprintf(stderr,"x_axis [%d] not available in this workout!\n",x_axis);
    return 0;
  }
  if ( xmax > 0 ) ppx = (float)(x1-x0)/xmax;

  /* get the min and max values for the y coordinate */

  switch ( y_axis ) {
  case S710_Y_AXIS_HEART_RATE: YMINMAX(hr_data);    break;
  case S710_Y_AXIS_ALTITUDE:   YMINMAX(alt_data);   break;
  case S710_Y_AXIS_SPEED:      YMINMAX(speed_data); break;
  case S710_Y_AXIS_CADENCE:    YMINMAX(cad_data);   break;
  case S710_Y_AXIS_POWER:      YMINMAXPOWER(power); break;
  default:                                          break;
  }
  ymax += 0.025 * (ymax-ymin);
  if ( ymin > 0 ) ymin -= 0.025 * (ymax-ymin);
  if ( ymax - ymin < 0.01 ) ymax = 1.0;
  ppy   = (float)(y1-y0)/(ymax-ymin);

  if ( filename && (im = gdImageCreate(width,height)) ) {

    white = gdImageColorAllocate(im, 0xff, 0xff, 0xff );        
    black = gdImageColorAllocate(im,    0,    0,    0 );      
    pixel = gdImageColorAllocate(im,    r,    g,    b );

    /* fill the plot with white */

    gdImageFilledRectangle(im,0,0,width,height,white);

    /* get the min and max values for the y coordinate */

    if ( (c = calloc(w->samples+2,sizeof(gdPoint))) != NULL ) {

      /* lower left corner is first point */
      
      c[0].x = x0;
      c[0].y = y1;

      /* lower right corner is last point */
      
      c[w->samples+1].x = x1;
      c[w->samples+1].y = y1;
      
      /* now fill the rest of the coordinates */

      switch ( y_axis ) {
      case S710_Y_AXIS_HEART_RATE: SAMPLES(hr_data);    break;
      case S710_Y_AXIS_ALTITUDE:   SAMPLES(alt_data);   break;
      case S710_Y_AXIS_SPEED:      SAMPLES(speed_data); break;
      case S710_Y_AXIS_CADENCE:    SAMPLES(cad_data);   break;
      case S710_Y_AXIS_POWER:      SAMPLESPOWER(power); break;
      default:                                          break;
      }

      draw_x_axis(im,x_axis,&w->units,xmax,x0,y1,x1,y1,black);
      draw_y_axis(im,S710_Y_AXIS_LEFT,p->units,ymin,ymax,
		  r,g,b,tic_opts,x0,y1,x1,y0,black);

      if ( tic_opts & S710_TIC_SHADE ) {
	gdImageFilledPolygon(im,c,w->samples+2,pixel);
	gdImagePolygon(im,c,w->samples+2,black);
      } else {
	for ( i = 0; i < w->samples; i++ ) {
	  gdImageLine(im,c[i].x,c[i].y,c[i+1].x,c[i+1].y,pixel);
	}
      }

      free(c);
      gdImageRectangle(im,x0,y0,x1,y1,black);
      ok = 1;

    } else {
      fprintf(stderr,"calloc(%d,%ld): %s\n",
	      w->samples+2,(long)sizeof(gdPoint),strerror(errno));
    }

    /* write the image to the file */
    
    if ( (fp = fopen(filename,"wb")) != NULL ) {
      gdImagePng(im,fp);
      ok = 1;
      fclose(fp);
    } else {
      fprintf(stderr,"plot_workout_xy: %s: fopen: %s\n",
	      filename,strerror(errno));
    }
    
    /* free allocated memory */
    
    gdImageDestroy(im);
  }


  return ok;
}
 
