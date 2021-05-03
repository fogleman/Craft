// This program uses queue to echo its parameters
// Then it prints parameters in descending order
// using priority queue
// For example:
//   a.out 1 3 5 2
// writes:
//   1 3 5 2
//   5 3 2 1


#include <stdio.h>
#include <stdlib.h>
#include "sglib.h"

#define MAX_PARAMS 100

typedef struct iq {int a[MAX_PARAMS]; int i,j;} iq;
SGLIB_DEFINE_QUEUE_FUNCTIONS(iq, int, a, i, j, MAX_PARAMS);

int main(int argc, char **argv) {
  int i, ai,aj, n;
  int a[MAX_PARAMS];

  // echo parameters using a queue
  SGLIB_QUEUE_INIT(int, a, ai, aj);
  for (i=1; i<argc; i++) {
	sscanf(argv[i],"%d", &n);
	SGLIB_QUEUE_ADD(int, a, n, ai, aj, MAX_PARAMS);
  }
  while(! SGLIB_QUEUE_IS_EMPTY(int, a, ai, aj)) {
	printf("%d ", SGLIB_QUEUE_FIRST_ELEMENT(int, a, ai, aj));
	SGLIB_QUEUE_DELETE(int, a, ai, aj, MAX_PARAMS);
  }
  printf("\n");

  // print parameters in descending order
  SGLIB_HEAP_INIT(int, a, ai);
  for (i=1; i<argc; i++) {
	sscanf(argv[i],"%d", &n);
	SGLIB_HEAP_ADD(int, a, n, ai, MAX_PARAMS, SGLIB_NUMERIC_COMPARATOR);
  }
  while(! SGLIB_HEAP_IS_EMPTY(int, a, ai)) {
	printf("%d ", SGLIB_HEAP_FIRST_ELEMENT(int, a, ai));
	SGLIB_HEAP_DELETE(int, a, ai, MAX_PARAMS, SGLIB_NUMERIC_COMPARATOR);
  }
  printf("\n");


  return(0);
}
