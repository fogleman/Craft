// This program sorts its parameters using
// binary search to implement insert sort.
// For example:
//   a.out 6 7 3 4 1 5
// writes
//   1 3 4 5 6 7


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sglib.h"

#define MAX_ELEMS 1000

int main(int argc, char **argv) {
  int i, size, found, index, tmp;
  int a[MAX_ELEMS];
  size = argc-1;
  for (i=0; i<size; i++) {
    sscanf(argv[i+1],"%d", &a[i]);
  }
  for(i=1; i<size; i++) {
    tmp = a[i];
    SGLIB_ARRAY_BINARY_SEARCH(int, a, 0, i, tmp, SGLIB_NUMERIC_COMPARATOR, found, index);
    memmove(a+index+1, a+index, (i-index)*sizeof(int));
    a[index]=tmp;
  }
  for (i=0; i<size; i++) {
    printf("%d ", a[i]);
  }
  printf("\n");
  return(0);
}
