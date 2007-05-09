/* $Id: add_attr.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "s710.h"

void
add_string_attribute ( attribute_map_t *m, 
		       char            *name, 
		       void            *ptr, 
		       int              length )
{
  m->pairs = add_attribute(&m->pairs,S710_ATTRIBUTE_TYPE_STRING,name,ptr);
  if ( (m->pairs->values = calloc(1,sizeof(attribute_value_t))) != NULL ) {
    m->pairs->values[0].int_value = length;
    m->pairs->vcount = 1;
  }
}


void
add_integer_attribute ( attribute_map_t *m, 
			char            *name, 
			void            *ptr, 
			int              min, 
			int              max,
			int              offset )
{
  m->pairs = add_attribute(&m->pairs,S710_ATTRIBUTE_TYPE_INTEGER,name,ptr);
  if ( (m->pairs->values = calloc(3,sizeof(attribute_value_t))) != NULL ) {
    m->pairs->values[0].int_value = min;
    m->pairs->values[1].int_value = max;
    m->pairs->values[2].int_value = offset;
    m->pairs->vcount = 3;
  }
}


void
add_boolean_attribute ( attribute_map_t *m, 
			char            *name,
			void            *ptr )
{
  m->pairs = add_attribute(&m->pairs,S710_ATTRIBUTE_TYPE_BOOLEAN,name,ptr);
}


void
add_byte_attribute ( attribute_map_t *m, 
		     char            *name,
		     void            *ptr )
{
  m->pairs = add_attribute(&m->pairs,S710_ATTRIBUTE_TYPE_BYTE,name,ptr);
}


void 
add_enum_integer_attribute ( attribute_map_t *m, 
			     char            *name,
			     void            *ptr,
			     ... )
{
  va_list ap;
  int     eval;
  int     ival;

  m->pairs = add_attribute(&m->pairs,
			   S710_ATTRIBUTE_TYPE_ENUM_INTEGER,
			   name,
			   ptr);
  va_start(ap,ptr);
  while ( 1 ) {
    eval = va_arg(ap,int);
    if ( eval < 0 ) break;
    ival = va_arg(ap,int);
    m->pairs->vcount++;
    m->pairs->values = realloc(m->pairs->values,
			       m->pairs->vcount * sizeof(attribute_value_t));
    m->pairs->values[m->pairs->vcount-1].enum_int_value.eval = eval;
    m->pairs->values[m->pairs->vcount-1].enum_int_value.ival = ival;
  }
  va_end(ap);
}


void 
add_enum_string_attribute ( attribute_map_t *m, 
			    char            *name, 
			    void            *ptr,
			    ... )
{
  va_list  ap;
  char    *cval;
  int      ival;

  m->pairs = add_attribute(&m->pairs,
			   S710_ATTRIBUTE_TYPE_ENUM_STRING,
			   name,
			   ptr);
  va_start(ap,ptr);
  
  while ( 1 ) {
    cval = va_arg(ap,char *);
    if ( cval == NULL ) break;
    ival = va_arg(ap,int);
    m->pairs->vcount++;
    m->pairs->values = realloc(m->pairs->values,
			       m->pairs->vcount * sizeof(attribute_value_t));
    m->pairs->values[m->pairs->vcount-1].enum_string_value.sval = strdup(cval);
    m->pairs->values[m->pairs->vcount-1].enum_string_value.ival = ival;
  }
  va_end(ap);
}


attribute_pair_t *
add_attribute ( attribute_pair_t     **p, 
		S710_Attribute_Type    type, 
		char                  *name,
		void                  *ptr)
{
  attribute_pair_t *a, *v = *p;

  if ( (a = calloc(1,sizeof(attribute_pair_t))) != NULL ) {
    a->type   = type;
    a->next   = v;
    v         = a;
    a->name   = strdup(name);
    a->ptr    = ptr;
  }
  
  *p = v;
  return *p;
}
