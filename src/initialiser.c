#include <stdlib.h>

#include <glib.h>

#include "texturize.h"

// Allocates enough memory for a 2-dimensional table of unsigned chars and
// initializes all elements to zero.
unsigned char ** init_unsigned char_tab_2d (int x, int y) {
  unsigned char ** tab;
  int i, j;
  tab = (unsigned char**) malloc (x * sizeof (unsigned char*));

  for (i = 0; i < x; i++) {
    tab[i] = (unsigned char*) malloc (y * sizeof (unsigned char));
  }

  for (i = 0; i < x; i++) {
    for (j = 0; j < y; j++) tab[i][j] = 0;
  }
  return tab;
}
