// This program sorts its parameters using
// array sort level 1 interface. 
// For example:
//   a.out 6 7 3 4 1 5
// writes
//   1 3 4 5 6 7


#include <stdio.h>
#include <stdlib.h>
#include "sglib.h"

#define MAX_ELEMS 1000

SGLIB_DEFINE_ARRAY_SORTING_FUNCTIONS(int, SGLIB_NUMERIC_COMPARATOR)

int main(int argc, char **argv) {
  int i,size;
  int a[MAX_ELEMS];
  size = argc-1;
  for (i=0; i<size; i++) {
    sscanf(argv[i+1],"%d", &a[i]);
  }
  sglib_int_array_heap_sort(a, size);
  for (i=0; i<size; i++) {
    printf("%d ", a[i]);
  }
  printf("\n");
  return(0);
}
