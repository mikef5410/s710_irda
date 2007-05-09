/* $Id: alpha_map.c,v 1.3 2002/09/17 09:27:15 dave Exp $ */

#include <string.h>
#include "s710.h"


/* decode a name or label character */

char
alpha_map ( unsigned char c )
{
  char a = '?';

  switch ( c ) {
  case 0: case 1: case 2: case 3: case 4: 
  case 5: case 6: case 7: case 8: case 9:
    a = '0' + c;
    break;
  case 10:
    a = ' ';
    break;
  case 11: case 12: case 13: case 14: case 15:
  case 16: case 17: case 18: case 19: case 20:
  case 21: case 22: case 23: case 24: case 25:
  case 26: case 27: case 28: case 29: case 30:
  case 31: case 32: case 33: case 34: case 35:
  case 36:
    a = 'A' + c - 11;
    break;
  case 37: case 38: case 39: case 40: case 41:
  case 42: case 43: case 44: case 45: case 46:
  case 47: case 48: case 49: case 50: case 51:
  case 52: case 53: case 54: case 55: case 56:
  case 57: case 58: case 59: case 60: case 61:
  case 62:
    a = 'a' + c - 37;
    break;
  case 63: a = '-'; break;
  case 64: a = '%'; break;
  case 65: a = '/'; break;
  case 66: a = '('; break;
  case 67: a = ')'; break;
  case 68: a = '*'; break;
  case 69: a = '+'; break;
  case 70: a = '.'; break;
  case 71: a = ':'; break;
  case 72: a = '?'; break;
  default:          break;
  }
  
  return a;
}


unsigned char
inverse_alpha_map ( char c )
{
  unsigned char a = 10;  /* default will be the space character. */

  switch ( c ) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    a = c - '0';
    break;
  case ' ':
    a = 10;
    break;
  case 'A': case 'B': case 'C': case 'D': case 'E':
  case 'F': case 'G': case 'H': case 'I': case 'J':
  case 'K': case 'L': case 'M': case 'N': case 'O':
  case 'P': case 'Q': case 'R': case 'S': case 'T':
  case 'U': case 'V': case 'W': case 'X': case 'Y':
  case 'Z':
    a = c - 'A' + 11;
    break;
  case 'a': case 'b': case 'c': case 'd': case 'e':
  case 'f': case 'g': case 'h': case 'i': case 'j':
  case 'k': case 'l': case 'm': case 'n': case 'o':
  case 'p': case 'q': case 'r': case 's': case 't':
  case 'u': case 'v': case 'w': case 'x': case 'y':
  case 'z':
    a = c - 'a' + 37;
    break;
  case '-': a = 63; break;
  case '%': a = 64; break;
  case '/': a = 65; break;
  case '(': a = 66; break;
  case ')': a = 67; break;
  case '*': a = 68; break;
  case '+': a = 69; break;
  case '.': a = 70; break;
  case ':': a = 71; break;
  case '?': a = 72; break;
  default:          break;
  }

  return a;
}


void
extract_label ( unsigned char *buf, S710_Label *label, int bytes )
{
  int   i;
  char *p = (char *)label;

  for ( i = 0; i < bytes && i < sizeof(S710_Label)-1; i++ ) {
    *(p+i) = alpha_map(buf[i]);
  }

  *(p+i) = 0;
}


void
encode_label ( S710_Label label, unsigned char *buf, int bytes )
{
  int   i;
  unsigned char *p = buf;

  /* initialize to all spaces */

  memset(buf,10,bytes);

  /* now encode the actual characters */

  for ( i = 0; i < bytes && i < sizeof(S710_Label)-1; i++ ) {
    *(p+i) = inverse_alpha_map(label[i]);
  }
}
