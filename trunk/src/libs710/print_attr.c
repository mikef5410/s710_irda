/* $Id: print_attr.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <stdio.h>
#include "s710.h"


void
print_current_attribute_value ( attribute_pair_t *p, FILE *fp )
{
  switch ( p->type ) {
  case S710_ATTRIBUTE_TYPE_INTEGER:
    fprintf(fp,"%d",p->value.int_value);
    break;
  case S710_ATTRIBUTE_TYPE_STRING:
    fprintf(fp,"%s",p->value.string_value);
    break;
  case S710_ATTRIBUTE_TYPE_BOOLEAN:
    fprintf(fp,"%s",(p->value.bool_value) ? "True" : "False");
    break;
  case S710_ATTRIBUTE_TYPE_BYTE:
    fprintf(fp,"%02x",p->value.byte_value);
    break;
  case S710_ATTRIBUTE_TYPE_ENUM_INTEGER:
    fprintf(fp,"%d",p->value.enum_int_value.eval);
    break;
  case S710_ATTRIBUTE_TYPE_ENUM_STRING:
    fprintf(fp,"%s",p->value.enum_string_value.sval);
    break;
  default:
    break;
  }
}


void
print_attribute_map ( int c, attribute_map_t *m )
{
  attribute_pair_t *p;
  int               i;
  packet_t         *k;

  k = packet(c);
  printf("Attributes and attribute types for \"%s\":\n\n",
	 (k != NULL) ? k->name : "???");
  if ( m != NULL ) {
    printf("Attribute             Value\n"
	   "-------------------------------------------------------------\n");
    for ( p = m->pairs; p != NULL; p = p->next ) {
      printf("%-21s ",p->name);
      switch ( p->type ) {
      case S710_ATTRIBUTE_TYPE_INTEGER:
	printf("%d to %d",p->values[0].int_value,p->values[1].int_value);
	break;
      case S710_ATTRIBUTE_TYPE_STRING:
	printf("Up to %d characters",p->values[0].int_value);
	break;
      case S710_ATTRIBUTE_TYPE_BOOLEAN:
	printf("boolean");
	break;
      case S710_ATTRIBUTE_TYPE_BYTE:
	printf("byte (0x00 to 0xff)");
	break;
      case S710_ATTRIBUTE_TYPE_ENUM_INTEGER:
	printf("%d",p->values[0].enum_int_value.eval);
	for ( i = 1; i < p->vcount - 1; i++ ) {
	  printf(", %d",p->values[i].enum_int_value.eval);
	}
	printf(" or %d",p->values[p->vcount - 1].enum_int_value.eval);
	break;
      case S710_ATTRIBUTE_TYPE_ENUM_STRING:
	printf("\"%s\"",p->values[0].enum_string_value.sval);
	for ( i = 1; i < p->vcount - 1; i++ ) {
	  printf(", \"%s\"",p->values[i].enum_string_value.sval);
	}
	printf(" or \"%s\"",p->values[p->vcount - 1].enum_string_value.sval);
	break;
      default:
	printf("UNKNOWN");
	break;
      }
      printf("\n");
    }
    printf("\n");
  }
}
