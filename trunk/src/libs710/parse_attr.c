/* $Id: parse_attr.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <string.h>
#include <ctype.h>
#include "s710.h"

#define PARSE_BEGIN 0
#define PARSE_NAME  1
#define PARSE_EQUAL 2
#define PARSE_VALUE 3


void
parse_attribute_pairs ( char *s, attribute_map_t *map )
{
  char              *start;
  char              *end;
  char              *c;
  char               namebuf[BUFSIZ];
  char               valbuf[BUFSIZ];
  int                state;
  int                npos;
  int                vpos;
  attribute_pair_t  *p;

  start = strchr(s,'{');
  end   = strchr(s,'}');

  if ( !start || !end ) return;

  /* cut out the leading and trailing whitespace from the pair list */

  start++;
  while ( isspace(*start) ) start++;

  /* if we've got nothing left, then there's nothing to parse. */

  if ( !*start || end == start ) return;

  /* ok, we've got some stuff to parse. */

  /* loop through the characters in the string, starting with the first
     character of the name of the first attribute. */

  state = PARSE_BEGIN;
  npos  = 0;
  vpos  = 0;

  for ( c = start; c != end; c++ ) {
    switch ( state ) {
    case PARSE_BEGIN:
      npos = 0;
      vpos = 0;
      if ( !isspace(*c) ) {
	state = PARSE_NAME;
	namebuf[npos++] = *c;
      }
      break;
    case PARSE_NAME:
      if ( isspace(*c) || *c == '=' ) {
	namebuf[npos++] = 0;
	state = PARSE_EQUAL;
      } else {
	namebuf[npos++] = *c;
      }
      break;
    case PARSE_EQUAL:
      if ( !isspace(*c) && *c != '=' ) {
	state = PARSE_VALUE;
	if ( *c != '"' ) valbuf[vpos++] = *c;
      }
      break;
    case PARSE_VALUE:
      if ( !*c || *c == ',' || c == end-1 ) {	
	valbuf[vpos++] = 0;
	for ( p = map->pairs; p != NULL; p = p->next ) {
	  if ( is_like(namebuf,p->name) ) {
	    map->oosync = merge_attribute_value(valbuf,p);
	    break;
	  }
 	}	
	state = PARSE_BEGIN;
      } else {
	if ( *c != '"' ) valbuf[vpos++] = *c;
      }
      break;
    default:
      break;
    }
  }
}


#undef PARSE_BEGIN
#undef PARSE_NAME
#undef PARSE_EQUAL
#undef PARSE_VALUE
