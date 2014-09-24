// This program sorts its parameters using 
// array sort level 0 interface. 
// For example:
//   a.out 6 7 3 4 1 5
// writes
//   1 3 4 5 6 7


#include <stdio.h>
#include <stdlib.h>
#include "sglib.h"

#define MAX_ELEMS 1000

int main(int argc, char **argv) {
  int i,size;
  int a[MAX_ELEMS];
  size = argc-1;
  for (i=0; i<size; i++) {
    sscanf(argv[i+1],"%d", &a[i]);
  }
  SGLIB_ARRAY_SINGLE_QUICK_SORT(int, a, size, SGLIB_NUMERIC_COMPARATOR);
  for (i=0; i<size; i++) {
    printf("%d ", a[i]);
  }
  printf("\n");
  return(0);
}
