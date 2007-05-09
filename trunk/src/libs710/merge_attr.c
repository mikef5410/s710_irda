/* $Id: merge_attr.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "s710.h"

/* 
   this function takes attribute values which are "out of sync" in the given
   attribute map and tries to merge them with the relevant data structure in 
   the attribute map.
*/

void
merge_attribute_map ( attribute_map_t *map )
{
  attribute_pair_t *p;
  S710_On_Off      *boolptr;
  int              *intptr;
  S710_Label       *labelptr;
  unsigned char    *byteptr;

  if ( map->oosync ) {
    for ( p = map->pairs; p != NULL; p = p->next ) {
      if ( p->oosync ) {

	switch ( p->type ) {
	case S710_ATTRIBUTE_TYPE_INTEGER:
	  intptr = (int *)p->ptr;
	  if ( intptr != NULL ) {
	    *intptr = p->value.int_value;
	  }
	  break;
	case S710_ATTRIBUTE_TYPE_STRING:
	  labelptr = (S710_Label *)p->ptr;
	  if ( labelptr != NULL ) {
	    strncpy(*labelptr,p->value.string_value,sizeof(S710_Label)-1);
	  }
	  break;
	case S710_ATTRIBUTE_TYPE_BOOLEAN:
	  boolptr = (S710_On_Off *)p->ptr;
	  if ( boolptr != NULL ) {
	    *boolptr = p->value.bool_value;
	  }
	  break;
	case S710_ATTRIBUTE_TYPE_BYTE:
	  byteptr = (unsigned char *)p->ptr;
	  if ( byteptr != NULL ) {
	    *byteptr = p->value.byte_value;
	  }
	  break;
	case S710_ATTRIBUTE_TYPE_ENUM_INTEGER:
	  intptr = (int *)p->ptr;
	  if ( intptr != NULL ) {
	    *intptr = p->value.enum_int_value.ival;
	  }
	  break;
	case S710_ATTRIBUTE_TYPE_ENUM_STRING:
	  intptr = (int *)p->ptr;
	  if ( intptr != NULL ) {
	    *intptr = p->value.enum_string_value.ival;
	  }
	  break;
	default:
	  break;
	}
      }
    }
  }
}


/* this function resets the synchronization flags of an attribute map */

void
sync_attribute_map ( attribute_map_t *map )
{
  attribute_pair_t *p;

  if ( map->oosync ) {
    for ( p = map->pairs; p != NULL; p = p->next ) 
      p->oosync = 0;
    map->oosync = 0;
  }
}


/* this function returns 1 if a value was successfully merged. */

int
merge_attribute_value ( char *value, attribute_pair_t *p )
{
  int merged = 0;
  
  switch ( p->type ) {
  case S710_ATTRIBUTE_TYPE_INTEGER:
    merged = merge_integer_attribute_value(value,p);
    break;
  case S710_ATTRIBUTE_TYPE_STRING:
    merged = merge_string_attribute_value(value,p);
    break;
  case S710_ATTRIBUTE_TYPE_BOOLEAN:
    merged = merge_boolean_attribute_value(value,p);
    break;    
  case S710_ATTRIBUTE_TYPE_BYTE:
    merged = merge_byte_attribute_value(value,p);
    break;
  case S710_ATTRIBUTE_TYPE_ENUM_INTEGER:
    merged = merge_enum_integer_attribute_value(value,p);
    break;
  case S710_ATTRIBUTE_TYPE_ENUM_STRING:
    merged = merge_enum_string_attribute_value(value,p);
    break;
  default:
    break;
  }
  
  if ( merged ) {
    p->oosync = 1;
    printf("OK: %s is ",p->name);
    print_current_attribute_value(p,stdout);
    printf("\n");
  } else {
    printf("NO: %s cannot be %s\n",p->name,value);
  }

  return merged;
}


int
merge_integer_attribute_value ( char *value, attribute_pair_t *p )
{
  int merged = 0;
  unsigned long ival;

  ival = strtoul(value,NULL,0);
  if ( ival >= p->values[0].int_value && 
       ival <= p->values[1].int_value ) {
    p->value.int_value = ival + p->values[2].int_value;
    merged = 1;
  }

  return merged;
}


int
merge_string_attribute_value ( char *value, attribute_pair_t *p )
{
  int len = sizeof(S710_Label) - 1;
  char buf[sizeof(S710_Label)+1];
  int merged = 0;

  if ( value != NULL ) {
    if ( p->values[0].int_value < len ) len = p->values[0].int_value;
    strncpy(p->value.string_value,value,len);
    encode_label(p->value.string_value,buf,len);
    extract_label(buf,&p->value.string_value,len);
    merged = 1;
  }

  return merged;
}


int
merge_boolean_attribute_value ( char *value, attribute_pair_t *p )
{
  int merged = 0;
  
  if ( !strcmp(value,"1")    || 
       !strcmp(value,"True") || !strcmp(value,"true") || 
       !strcmp(value,"Yes")  || !strcmp(value,"yes" ) ||
       !strcmp(value,"On")   || !strcmp(value,"on" ) ) {
    p->value.bool_value = S710_ON;
    merged = 1;
  } else if ( !strcmp(value,"0")     || 
	      !strcmp(value,"False") || !strcmp(value,"false") || 
	      !strcmp(value,"No")    || !strcmp(value,"no" )   ||
	      !strcmp(value,"Off")   || !strcmp(value,"off" ) ) {
    p->value.bool_value = S710_OFF;
    merged = 1;
  }

  return merged;
}


int
merge_byte_attribute_value ( char *value, attribute_pair_t *p )
{
  unsigned long ival;

  ival = strtoul(value,NULL,0);
  p->value.byte_value = (unsigned char)ival;

  return 1;
}


int
merge_enum_integer_attribute_value ( char *value, attribute_pair_t *p )
{
  unsigned long eval;
  int i;
  int merged = 0;
  
  eval = strtoul(value,NULL,0);
  for ( i = 0; i < p->vcount; i++ ) {
    if ( eval == p->values[i].enum_int_value.eval ) {
      p->value.enum_int_value.eval = eval;
      p->value.enum_int_value.ival = p->values[i].enum_int_value.ival;
      merged = 1;
      break;
    }
  }

  return merged;
}


int
merge_enum_string_attribute_value ( char *value, attribute_pair_t *p )
{
  int i;
  int merged = 0;

  for ( i = 0; i < p->vcount; i++ ) {
    if ( is_like(value,p->values[i].enum_string_value.sval) ) {
      p->value.enum_string_value.sval = p->values[i].enum_string_value.sval;
      p->value.enum_string_value.ival = p->values[i].enum_string_value.ival;
      merged = 1;
      break;
    }
  }

  return merged;
}
