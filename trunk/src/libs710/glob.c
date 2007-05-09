/* $Id: glob.c,v 1.1 2002/10/10 10:11:09 dave Exp $ */

#include <ctype.h>
#include "s710.h"

/* 
   This annoying little function attempts to match a given string with
   a "wanted" string by checking to see if the given string is equal to
   the "wanted" string or an abbreviation of it, where "abbreviation"
   is defined like so:

   wanted              ok abbreviation              not ok abbreviation
   ====================================================================
   booyah dot com      boo d c                      booyah dot com yeah
   get exercise 1      g e 1                        g e 12

   this way, the "shell" commands can be abbreviated like so:

   get overview     => g o
   get user         => g u
   get bike         => g b
   close connection => c c
   
   NOTE:  I've made a change to this function to enable "set" functions
   to work.  "set" functions stop matching at the first '{' character,
   so I can deal with things like:

   set user { name = "Dave B", user_id = 0 }
*/

int
is_like ( char *given, char *wanted )
{
  char *p;
  char *q;
  int   ok = 1;

  p = given;
  q = wanted;

  /* early exit if *p == '\n' */

  while ( isspace(*p) ) p++;

  /* loop through both the given and the wanted strings, side by side. */

  while ( *p && *q && *p != '{' ) {

    if ( *p == *q ) {

      /* continue past identical characters */

      p++;
      q++;

    } else {

      /* oh, dear.  we've got a character mismatch. */

      if ( isspace(*p) ) {

	/* if the given string has fallen over to whitespace at a point 
	   where the wanted string is not in whitespace, then the user
	   is abbreviating.  scan the wanted string to the first whitespace,
	   then scan both strings to the first non-whitespace, then see
	   if the characters match up. */

	while ( *q && !isspace(*q) )  q++;
	while (  isspace(*p) )  p++;
	while (  isspace(*q) )  q++;	

	if ( !*p && *q ) {

	  /* bail out if there's nothing left in the given string and we
	     still have non-whitespace chars in the wanted string. */

	  ok = 0;
	  break;
	}
      } else {

	if ( *p != '\n' ) ok = 0;
	break;
      }
    }
  }

  if ( *p ) {
    while ( *p == ' ' || *p == '\t' ) p++;
  }

  /* if the remaining characters in the given string are not newline or '{',
     and there are no characters left in the wanted string, the match fails. */

  if ( *p && *p != '\n' && *p != '{' && !*q ) ok = 0;

  return ok;
}
