/* $Id: free_attr.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <stdlib.h>
#include "s710.h"

void
clear_attribute_map ( attribute_map_t *map )
{
  if ( map->pairs != NULL ) {
    free_attribute_pairs( map->pairs );
    map->pairs = NULL;
  }
}


void
free_attribute_pairs ( attribute_pair_t *pairs )
{
  attribute_pair_t *pi;
  attribute_pair_t *pn;
  int               i;

  if ( pairs != NULL ) {
    for ( pi = pairs; pi != NULL; pi = pn ) {
      pn = pi->next;
      if ( pi->name != NULL ) free ( pi->name );
      if ( pi->values != NULL ) {	
	if ( pi->type == S710_ATTRIBUTE_TYPE_ENUM_STRING ) {
	  for ( i = 0; i < pi->vcount; i++ ) {
	    free ( pi->values[i].enum_string_value.sval );
	  }
	}
	free ( pi->values );
      }
      free ( pi );
    }
  }
}

