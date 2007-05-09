/* $Id: histogram.c,v 1.5 2004/11/13 10:53:48 dave Exp $ */

#include <gdfonts.h>
#include <gd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "s710.h"

#define  S710_ZONE_BUFFER  9 * gdFontSmall->w
#define  S710_GDFS_Y       gdFontSmall-> h / 2
#define  S710_TIC_FINE     20
#define  S710_TIC_COARSE   50
#define  S710_B_RATIO      3

static byte_zone_t gDefaultHRZones[] = 
{
  /* min,max,   red,green,blue,pixel,  seconds */

  {   0, 130,  {   0,   0,  0xdd,  0},   0 },  /* zone 1 = blue   */
  { 130, 150,  {   0,  0xdd,   0,  0},   0 },  /* zone 2 = green  */
  { 150, 165,  { 0xdd, 0xdd,   0,  0},   0 },  /* zone 3 = yellow */
  { 165, 180,  { 0xff, 0xa5,   0,  0},   0 },  /* zone 4 = orange */
  { 180, 185,  { 0xdd,   0,   0,  0},   0 },  /* zone 5a = red   */
  { 185, 190,  { 0xee,   0,   0,  0},   0 },  /* zone 5b = red   */
  { 190, 255,  { 0xff,   0,   0,  0},   0 }   /* zone 5c = red   */
};

static int gNDefaultHRZones = (sizeof(gDefaultHRZones))/sizeof(byte_zone_t);

static byte_zone_t gDefaultCadZones[] = 
{
  /* min,max,   red,green,blue,pixel,  seconds */

  {   0,  50,  {   0,   0,  0xdd,  0},   0 },  /* zone 1 = blue   */
  {  50,  70,  {   0,  0xdd,   0,  0},   0 },  /* zone 2 = green  */
  {  70,  90,  { 0xdd, 0xdd,   0,  0},   0 },  /* zone 3 = yellow */
  {  90, 110,  { 0xff, 0xa5,   0,  0},   0 },  /* zone 4 = orange */
  { 110, 255,  { 0xdd,   0,   0,  0},   0 }    /* zone 5 = red    */
};

static int gNDefaultCadZones = (sizeof(gDefaultCadZones))/sizeof(byte_zone_t);


static byte_zone_t gDefaultPwrZones[] = 
{
  /* min,max,   red,green,blue,pixel,  seconds */

  {   0, 100,  {   0,   0,  0xdd,  0},   0 },  /* zone 1 = blue   */
  { 100, 200,  {   0,  0xdd,   0,  0},   0 },  /* zone 2 = green  */
  { 200, 300,  { 0xdd, 0xdd,   0,  0},   0 },  /* zone 3 = yellow */
  { 300, 400,  { 0xff, 0xa5,   0,  0},   0 },  /* zone 4 = orange */
  { 400, 600,  { 0xdd,   0,   0,  0},   0 }    /* zone 5 = red    */
};

static int gNDefaultPwrZones = (sizeof(gDefaultPwrZones))/sizeof(byte_zone_t);


byte_histogram_t *
make_byte_histogram ( workout_t *w, byte_zone_t *z, int nz, int what )
{
  byte_histogram_t  *h;
  int                s;
  int                i;
  int                j;

  switch ( what ) {
  case S710_Y_AXIS_HEART_RATE: 
    if ( !z ) { 
      z  = &gDefaultHRZones[0];
      nz = gNDefaultHRZones;
    }
    break;
  case S710_Y_AXIS_CADENCE:
    if ( !z ) { 
      z  = &gDefaultCadZones[0];
      nz = gNDefaultCadZones;
    }
    if ( !(w->mode & S710_MODE_CADENCE) ) z = NULL;
    break;
  case S710_Y_AXIS_POWER:
    if ( !z ) { 
      z  = &gDefaultPwrZones[0];
      nz = gNDefaultPwrZones;
    }
    if ( !(w->mode & S710_MODE_POWER) ) z = NULL;
    break;
  default:
    z  = NULL;
    break;
  }
    
  if ( !z ) {
    fprintf(stderr,"byte_histogram: y_axis [%d] not available\n",what);
    return NULL;
  }

  if ( (h = calloc(1,sizeof(byte_histogram_t))) != NULL ) {
    if ( (h->zone = calloc(nz,sizeof(byte_zone_t))) != NULL ) {

      s = w->recording_interval;
      memcpy(h->zone,z,nz * sizeof(byte_zone_t));
      h->zones = nz;

      switch ( what ) {
      case S710_Y_AXIS_HEART_RATE:
	for ( i = 0; i < w->samples; i++ ) {
	  if ( w->hr_data[i] ) h->hist[w->hr_data[i]] += s;
	}
	break;
      case S710_Y_AXIS_CADENCE:
	for ( i = 0; i < w->samples; i++ ) {
	  if ( w->cad_data[i] ) h->hist[w->cad_data[i]] += s;
	}
	break;
      case S710_Y_AXIS_POWER:
	for ( i = 0; i < w->samples; i++ ) {
	  if ( w->power_data[i].power ) h->hist[w->power_data[i].power] += s;
	}
	break;
      default:
	break;
      }

      /* dither */

      for ( i = 0; i < nz; i++ ) {
	for ( j = h->zone[i].min; j < h->zone[i].max; j++ ) {
	  h->zone[i].seconds += h->hist[j];
	}
      }

    } else {
      free(h);
      h = NULL;
      fprintf(stderr,"calloc(%d,%ld): %s\n",
	      nz,(long)sizeof(byte_zone_t),strerror(errno));
    }
  }
  
  return h;
}


void
free_byte_histogram ( byte_histogram_t *h ) 
{
  if ( h ) {
    if ( h->zone ) free ( h->zone );
    free ( h );
  }
}


/* histogram */

int
plot_byte_histogram ( byte_histogram_t *h,
		      int               r,
		      int               g,
		      int               b,
		      int               min,
		      int               max,
		      int               width,
		      int               height,
		      int               dither,
		      char             *filename )
{
  int         i;
  int         j;
  int         k;
  int         x;
  int         y;
  int         ok = 0;
  int         mh;
  int         ns[65536];
  int         maxs;
  gdImagePtr  im;
  FILE       *fp;
  gdPoint     p[65540];
  int         black;  /* lines, tics, labels, etc */
  int         white;  /* background */
  int         pixel;
  char        buf[4];
  int         x0;
  int         x1;
  int         y0;
  int         y1;
  int         tic;

  /* min and max are the byte value (x axis) min and max.           */
  /* width and height are the width and height of the output image  */
  /* dither is how steps we should take between points              */
  /* filename is the name of the output file.                       */

  /* don't let the image size get too absurd. */

  CLAMP(width,50,1600);
  CLAMP(height,50,1200);

  /* clamp the byte value limits. */

  mh = sizeof(h->hist)/sizeof(h->hist[0]);
  CLAMP(max,0,mh);
  CLAMP(min,0,max);

  /* clamp the dithering */
  
  CLAMP(dither,1,max-min+1);

  /* dithered byte data goes in here. */

  memset(ns,0,sizeof(ns));
  k = 0;
  
  /* do the dithering */

  maxs = 0;
  for ( k = 0, i = min; i < max; i += dither, k++ ) {
    for ( j = i; j < mh && j < i + dither; j++ ) {
      ns[k] += h->hist[j];
    }
    if ( ns[k] > maxs ) maxs = ns[k];
  }
  if ( maxs < 1 ) maxs = 1;

  /* now k is the number of bins, and ns is the bin data.       */
  /* the mean value for dither bin i is i*dither + (dither>>1)  */
  /* maxs is the maximum Y (time) value.                        */

  /* create a GD image */

  if ( filename && (im = gdImageCreate(width,height)) ) {
    white = gdImageColorAllocate(im, 0xff, 0xff, 0xff );        
    black = gdImageColorAllocate(im, 0, 0, 0);      
    pixel = gdImageColorAllocate(im, r, g, b );

    /* do the actual drawing */

    /* fill the HR histogram with white */

    gdImageFilledRectangle(im,0,0,width,height,white);

    x0 = S710_X_MARGIN;
    x1 = width - S710_X_MARGIN;
    y0 = height - S710_Y_MARGIN - 10;
    y1 = y0;

    /* lower left corner is the first point */

    p[0].x = x0;
    p[0].y = y0;

    /* lower right corner is last point */

    p[k+1].x = x1;
    p[k+1].y = y1;

    /* now fill the rest of the coordinates */
    
    for ( i = 0; i < k; i++ ) {
      p[i+1].x = p[0].x + i*(p[k+1].x-p[0].x)/(k-1);
      p[i+1].y = p[0].y - ns[i]*(p[0].y-S710_Y_MARGIN)/(1.1*maxs);
    }

    /* draw the filled polygon */

    gdImageFilledPolygon(im,p,k+2,pixel);

    /* draw a classy black outline around the polygon */

    gdImagePolygon(im,p,k+2,black);

    /* draw the border rectangle */

    gdImageRectangle(im,x0,S710_Y_MARGIN,x1,y1,black);

    /* draw the tics and HR values */

    tic = (max < 300) ? S710_TIC_FINE : S710_TIC_COARSE;
    
    for ( i = 0; i <= 1000/tic; i++ ) {
      if ( tic * i >= min && tic * i <= max ) {

	x = p[0].x + (tic*i-min)*(p[k+1].x-p[0].x)/(max-min);
	y = p[0].y;

	gdImageLine(im,x,y,x,y+4,black);
	
	snprintf(buf,sizeof(buf),"%d",tic * i);
	x -= strlen(buf) * gdFontSmall->w / 2;
	y += S710_GDFS_Y;
	gdImageString(im,gdFontSmall,x,y,buf,black);	
      }
    }

    /* write the image to the file */

    if ( (fp = fopen(filename,"wb")) != NULL ) {
      gdImagePng(im,fp);
      ok = 1;
      fclose(fp);
    } else {
      perror("fopen");
    }

    /* free allocated memory */

    gdImageDestroy(im);
  }

  return ok;
}


/* zone plot */

int
plot_byte_zones ( byte_histogram_t *h,
		  int               width,
		  int               height,
		  char             *filename )
{
  int         i;
  int         p;
  int         x;
  int         y;
  int         my;
  int         hy;
  float       sp;
  float       bs;
  int         ok = 0;
  gdImagePtr  im;
  FILE       *fp;
  int         black;  /* lines, tics, labels, etc */
  int         white;  /* background */
  char        buf[12];
  int         maxs;
  float       pps;
  int         dx;
  
  /* width and height are the width and height of the output image  */
  /* filename is the name of the output file.                           */

  /* don't let the image size get too absurd. */

  CLAMP(width,50,1600);
  CLAMP(height,50,1200);

  /* create a GD image */

  if ( filename && (im = gdImageCreate(width,height)) ) {

    /* background, foreground colors */

    white  = gdImageColorAllocate(im, 0xff, 0xff, 0xff );        
    black  = gdImageColorAllocate(im, 0, 0, 0);      

    /* bar colors */

    for ( i = 0; i < h->zones; i++ ) {
      p = gdImageColorExact(im,
			    h->zone[i].color.red,
			    h->zone[i].color.green,
			    h->zone[i].color.blue);
      if ( p == -1 ) {
	p = gdImageColorAllocate(im,
			    h->zone[i].color.red,
			    h->zone[i].color.green,
			    h->zone[i].color.blue);
      }
      h->zone[i].color.pixel = p;
    }

    /* do the actual drawing */

    /* fill the HR histogram with white */

    gdImageFilledRectangle(im,0,0,width,height,white);

    /* draw the zone labels and bars */

    x  = S710_X_MARGIN + S710_ZONE_BUFFER;
    my = height - S710_Y_MARGIN - 10;
    hy = my - S710_Y_MARGIN;                /* total amount of y space */
    bs = (float)S710_B_RATIO*hy/(h->zones*(S710_B_RATIO+1)+1);
    sp = (float)bs/S710_B_RATIO;

    /* draw the zone labels */

    /* first label */

    snprintf(buf,sizeof(buf),"Above %d",h->zone[h->zones-1].min);
    gdImageString(im,gdFontSmall,S710_X_MARGIN,
		  S710_Y_MARGIN+(sp+bs/2)-S710_GDFS_Y,buf,black);

    /* in-between labels */

    for ( i = h->zones-2; i > 0; i-- ) {
      y = S710_Y_MARGIN+(h->zones-1-i)*(sp+bs)+sp+bs/2;
      snprintf(buf,sizeof(buf),"%d - %d",h->zone[i].min,h->zone[i].max);
      gdImageString(im,gdFontSmall,S710_X_MARGIN,y-S710_GDFS_Y,buf,black);
    }    

    /* last label */

    snprintf(buf,sizeof(buf),"Below %d",h->zone[0].max);
    gdImageString(im,gdFontSmall,S710_X_MARGIN,
		  my-sp-bs/2-S710_GDFS_Y,buf,black);

    /* draw zone bars */

    /* get max number of seconds */

    for ( maxs = 0, i = 0; i < h->zones; i++ ) {
      if ( h->zone[i].seconds > maxs ) maxs = h->zone[i].seconds;
    }

    pps = (float)(width-S710_X_MARGIN-x-6)/(1.1 * maxs);

    /* actually draw the bars */

    for ( i = 0; i < h->zones; i++ ) {
      y  = S710_Y_MARGIN+(h->zones-1-i)*(sp+bs)+sp;
      dx = pps * h->zone[i].seconds;
      gdImageFilledRectangle(im,x+6,y,x+6+dx,y+bs,h->zone[i].color.pixel);
      gdImageRectangle(im,x+6,y,x+dx+6,y+bs,black);
    }

    draw_x_axis(im,S710_X_AXIS_TIME,NULL,1.1*maxs,x+6,my,
		width-S710_X_MARGIN,my,black);
    
    /* draw the border rectangle */

    gdImageRectangle(im,x+6,S710_Y_MARGIN,
		     width-S710_X_MARGIN,height-S710_Y_MARGIN-10,
		     black);

    /* write the image to the file */

    if ( (fp = fopen(filename,"wb")) != NULL ) {
      gdImagePng(im,fp);
      ok = 1;
      fclose(fp);
    } else {
      perror("fopen");
    }

    /* free allocated memory */

    gdImageDestroy(im);
  }

  return ok;
}

		  
