/* $Id: axes.c,v 1.4 2007/02/26 09:34:54 dave Exp $ */

#include <gdfonts.h>
#include <string.h>
#include <math.h>
#include "s710.h"



static int get_pixel ( gdImagePtr im, int shade, int n );


/* draw the x axis */

void
draw_x_axis ( gdImagePtr    im, 
	      int           t_or_d, /* S710_X_AXIS_DISTANCE/S710_X_AXIS_TIME */
	      units_data_t *units,
	      float         max_x,  /* in meters (DISTANCE) or seconds */
	      int           xi,     /* x coordinate of start of x axis */
	      int           yi,     /* y coordinate of start of y axis */
	      int           xf,     /* x coord of end of x axis */
	      int           yf,     /* y coord of end of y axis */
	      int           pixel ) /* color of tics and axis labels */
{
  float   ts;
  int     tic;
  char   *x_units;
  int     tdx;
  int     tdy;
  char    buf[32];
  int     i;
  int     dx;
  float   ppx;

  if ( t_or_d == S710_X_AXIS_DISTANCE ) {

    ts  = max_x / 1000.0;  /* km or mi */

    if      ( ts <   2 ) tic =  200;    /* meters or ft */
    else if ( ts <   5 ) tic =  500;    /* meters or ft */
    else if ( ts <  10 ) tic =  1;      /* km or mi */
    else if ( ts <  20 ) tic =  2;
    else if ( ts <  30 ) tic =  3;
    else if ( ts <  40 ) tic =  4;
    else if ( ts <  50 ) tic =  5;
    else if ( ts <  80 ) tic =  8;
    else if ( ts < 120 ) tic = 10;
    else if ( ts < 150 ) tic = 12;
    else if ( ts < 180 ) tic = 15;
    else if ( ts < 240 ) tic = 20;
    else                 tic = 30;

    if ( tic > 100 ) {
      x_units  = (units != NULL ) ? units->altitude : "m"; /* m or ft */
      ts      *= 1000.0;
    } else {
      x_units = (units != NULL ) ? units->distance : "km";  /* km or mi */
    }

  } else {                    /* VS_TIME */

    ts  = max_x / 60.0;       /* minutes */

    if      ( ts <  10 ) tic =  2; 
    else if ( ts <  30 ) tic =  5;
    else if ( ts <  60 ) tic = 10;
    else if ( ts < 120 ) tic = 20;
    else if ( ts < 180 ) tic = 30;
    else if ( ts < 270 ) tic = 45;
    else                 tic =  1;  /* hour */
    
    if ( tic == 1 ) {
      x_units  = "hr";
      ts      /= 60;
    } else {
      x_units = "min";
    }
  }

  /* first tic also gets the label */
  
  ppx = (float)(xf-xi)/ts;
  tdx = gdFontSmall->w/2;
  tdy = gdFontSmall->h/2;
  gdImageLine(im,xi,yi,xi,yi+4,pixel);
  snprintf(buf,sizeof(buf),"0 %s",x_units);
  gdImageString(im,gdFontSmall,xi-tdx,yi+tdy,buf,pixel);

  for ( i = tic; i < ts; i += tic ) {
    dx = ppx * i;
    gdImageLine(im,xi+dx,yi,xi+dx,yi+4,pixel);
    snprintf(buf,sizeof(buf),"%d",i);
    gdImageString(im,gdFontSmall,xi+dx-tdx*strlen(buf),yi+tdy,buf,pixel);
  }
}


/* draw the y axis */

void
draw_y_axis( gdImagePtr  im,
	     int         side,
	     char       *units,
	     float       ymin,
	     float       ymax,
	     int         r,
	     int         g,
	     int         b,
	     int         tic_opts,
	     int         xi,
	     int         yi,
	     int         xf,
	     int         yf,
	     int         pixel )
{
  int     tic;
  float   ts;
  int     st;
  int     i;
  int     dy;
  float   ppy;
  char    buf[32];
  int     tdx;
  int     tdy;
  int     s;
  int     n = 0;
  int     xxi;
  int     xxf;
  int     p;
  int     yyf;
  int     yyi;

  /* decide on the tic size */

  ts = ymax - ymin;
  
  if      ( ts <    6 ) tic =    1;
  else if ( ts <   12 ) tic =    2;
  else if ( ts <   18 ) tic =    3;
  else if ( ts <   24 ) tic =    4;
  else if ( ts <   30 ) tic =    5;
  else if ( ts <   40 ) tic =    6;
  else if ( ts <   50 ) tic =    8;
  else if ( ts <   80 ) tic =   10;
  else if ( ts <  100 ) tic =   15;
  else if ( ts <  150 ) tic =   20;
  else if ( ts <  200 ) tic =   25;
  else if ( ts <  240 ) tic =   30;
  else if ( ts <  400 ) tic =   50;
  else if ( ts <  600 ) tic =   75;
  else if ( ts <  800 ) tic =  100;
  else if ( ts < 1000 ) tic =  150;
  else if ( ts < 1500 ) tic =  200;
  else if ( ts < 2000 ) tic =  250;
  else if ( ts < 2400 ) tic =  300;
  else if ( ts < 3000 ) tic =  400;
  else                  tic =  500;

  st  = floor(ymin-((int)floor(ymin))%tic);

  ppy = (float)(yi-yf)/ts;
  tdx = gdFontSmall->w;
  tdy = gdFontSmall->h/2;
  s   = (side == S710_Y_AXIS_LEFT) ? tdx : 0;

  if ( side == S710_Y_AXIS_LEFT ) {
    xxi = xi;
    xxf = (tic_opts & S710_TIC_LINES) ? xf : xi;
  } else {
    xxi = xf;
    xxf = (tic_opts & S710_TIC_LINES) ? xi : xf;
  }

  /* labels */

  while ( st < ymin ) st += tic;

  dy = yi - ppy * (st-ymin);
  snprintf(buf,sizeof(buf),"%d %s",st,units);
  gdImageString(im,gdFontSmall,xxi+side*(6+s*strlen(buf)),dy-tdy,buf,pixel);

  for ( i = st+tic; i < ymax; i += tic ) {
    dy = yi - ppy * (i-ymin);
    snprintf(buf,sizeof(buf),"%d",i);
    gdImageString(im,gdFontSmall,xxi+side*(6+s*strlen(buf)),dy-tdy,buf,pixel);
    n++;
  }

  n++;

  /* shading */

  if ( tic_opts & S710_TIC_SHADE ) {
    for ( i = st; i < ymax+tic; i += tic ) {
      p   = get_pixel(im,tic_opts,n--);
      yyi = yi - ppy * (i-ymin);
      yyf = yyi + ppy * tic;
      CLAMP(yyf,yf,yi);
      CLAMP(yyi,yf,yyf);
      gdImageFilledRectangle(im,xi,yyi,xf,yyf,p);
    }
  }

  /* tics */

  for ( i = st; i < ymax; i += tic ) {
    dy = yi - ppy * (i-ymin);
    gdImageLine(im,xxi+side*4,dy,xxf,dy,pixel);
  }


}


static int
get_pixel ( gdImagePtr im, int shade, int n )
{
  int r;
  int g;
  int b;
  int p;

  r = 0xee - 10*n;
  g = r;
  b = r;

  if ( (shade & S710_TIC_SHADE_RED) == S710_TIC_SHADE_RED ) {
    r += 5*n;
  } else if ( (shade & S710_TIC_SHADE_GREEN) == S710_TIC_SHADE_GREEN ) {
    g += 5*n;
  } else if ( (shade & S710_TIC_SHADE_BLUE) == S710_TIC_SHADE_BLUE ) {
    b += 5*n;
  }
  
  CLAMP(r,0,0xee);
  CLAMP(g,0,0xee);
  CLAMP(b,0,0xee);
  
  p = gdImageColorExact(im,r,g,b);
  if ( p == -1 ) {
    p = gdImageColorAllocate(im,r,g,b);
  }
  
  return p;
}
