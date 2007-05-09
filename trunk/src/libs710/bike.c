/* $Id: bike.c,v 1.5 2002/12/28 07:51:49 dave Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "s710.h"


static void print_bike_data ( bike_t *b, int which, FILE *fp );


/* requests and reads the bike settings over fd into the data structure */

int
get_bike ( S710_Driver *d, bike_t *bike )
{
  packet_t *p;
  int       ok = 0;
  int       i;
  int       j;
  int       k;

  p = get_response ( S710_GET_BIKE, d );
  if ( p != NULL) {
    bike->in_use = p->data[24];
    for ( i = 0; i < 2; i++ ) {
      j = 7 * i;
      k = 5 * i;
      extract_label(&p->data[j+1],&bike->bike[i].label,4);
      bike->bike[i].wheel_size     = BCD(p->data[6+j]) * 100 + 
	BCD(p->data[5+j]);
      bike->bike[i].power_sensor   = (p->data[j] & 0x08) ? S710_ON : S710_OFF;
      bike->bike[i].cadence_sensor = (p->data[j] & 0x04) ? S710_ON : S710_OFF;
      bike->bike[i].chain_weight   = BCD(p->data[16+k]) * 10 + 
	BCD(UNIB(p->data[15+k]));
      bike->bike[i].chain_length   = BCD(p->data[18+k]) * 100 + 
	BCD(p->data[17+k]);
      bike->bike[i].span_length    = BCD(LNIB(p->data[15+k])) * 100 + 
	BCD(p->data[14+k]);
    }
    free ( p );
    ok = 1;
  }
  
  return ok;
}


int
set_bike ( bike_t *bike, S710_Driver *d )
{
  packet_t  *p;
  int        i;
  int        j;
  int        k;

  p = make_set_packet(S710_SET_BIKE);
  if ( p == NULL ) return 0;

  p->data[24] = bike->in_use;
  for ( i = 0; i < 2; i++ ) {
    j = 7 * i;
    k = 5 * i;
    encode_label(bike->bike[i].label,&p->data[j+1],4);
    p->data[5+j] = HEX(DIGITS01(bike->bike[i].wheel_size));
    p->data[6+j] = HEX(DIGITS23(bike->bike[i].wheel_size));
    if ( bike->bike[i].power_sensor   == S710_ON ) p->data[j] |= 0x08;
    if ( bike->bike[i].cadence_sensor == S710_ON ) p->data[j] |= 0x04;
    p->data[14+k] = HEX(DIGITS01(bike->bike[i].span_length));
    p->data[15+k] = BINU(HEX(bike->bike[i].chain_weight)) |
      BINL(HEX(DIGITS23(bike->bike[i].span_length)));
    p->data[16+k] = HEX(bike->bike[i].chain_weight/10);
    p->data[17+k] = HEX(DIGITS01(bike->bike[i].chain_length));
    p->data[18+k] = HEX(DIGITS23(bike->bike[i].chain_length));
  }

  return send_set_packet(p,d);
}


/* prints out the bike data */

void
print_bike ( bike_t *b, FILE *fp )
{
  char  *a;

  switch ( b->in_use ) {
  case S710_BIKE_NONE:   a = "None";    break;
  case S710_BIKE_1:      a = "Bike 1";  break;
  case S710_BIKE_2:      a = "Bike 2";  break;
  default:               a = "Unknown"; break;
  }
  
  fprintf(fp,"\nBike in use:      %s\n",a);
  fprintf(fp,"\n");

  print_bike_data(b,0,fp);
  print_bike_data(b,1,fp);
}


static void
print_bike_data ( bike_t *b, int which, FILE *fp )
{ 
  if ( which < 0 || which > 1 ) return;

  fprintf(fp,"Bike %d:           %s\n",which+1,b->bike[which].label);
  fprintf(fp,"Wheel size:       %d mm\n",b->bike[which].wheel_size);
  
  fprintf(fp,"Power sensor:     %s\n",
	  (b->bike[which].power_sensor) ? "In use" : "Not in use");
  fprintf(fp,"Cadence sensor:   %s\n",
	  (b->bike[which].cadence_sensor) ? "In use" : "Not in use");
  
  fprintf(fp,"Chain weight:     %d g\n",b->bike[which].chain_weight);
  fprintf(fp,"Chain length:     %d mm\n",b->bike[which].chain_length);
  fprintf(fp,"Span length:      %d mm\n",b->bike[which].span_length);
  fprintf(fp,"\n");
}


#define BIKE_ATTR(a)       m,#a,&m->data.bike.a
#define BIKE_BOOL(a)       add_boolean_attribute(BIKE_ATTR(a))
#define BIKE_INT(a,b,c)    add_integer_attribute(BIKE_ATTR(a),b,c,0)
#define BIKE_STRING(a,b)   add_string_attribute(BIKE_ATTR(a),b)

void
load_bike_attributes ( attribute_map_t *m )
{
  BIKE_INT(bike[1].chain_length,0,9999);
  BIKE_INT(bike[1].chain_weight,0,999);
  BIKE_INT(bike[1].span_length,0,9999);
  BIKE_BOOL(bike[1].power_sensor);
  BIKE_BOOL(bike[1].cadence_sensor);
  BIKE_INT(bike[1].wheel_size,0,9999);
  BIKE_STRING(bike[1].label,4);

  BIKE_INT(bike[0].chain_length,0,9999);
  BIKE_INT(bike[0].chain_weight,0,999);
  BIKE_INT(bike[0].span_length,0,9999);
  BIKE_BOOL(bike[0].power_sensor);
  BIKE_BOOL(bike[0].cadence_sensor);
  BIKE_INT(bike[0].wheel_size,0,9999);
  BIKE_STRING(bike[0].label,4);

  add_enum_string_attribute(BIKE_ATTR(in_use),
			    "none", S710_BIKE_NONE,
			    "1",    S710_BIKE_1,
			    "2",    S710_BIKE_2,
			    NULL);
}

#undef BIKE_STRING
#undef BIKE_INT
#undef BIKE_BOOL
#undef BIKE_ATTR
