// This program sorts its parameters using 
// list sort (level 0 interface).
// For example:
//   a.out 6 7 3 4 1 5
// writes
//   1 3 4 5 6 7


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "sglib.h"

struct ilist {
    int i;
    struct ilist *next_ptr;
};

#define ILIST_COMPARATOR(e1, e2) (e1->i - e2->i)

int main(int argc, char **argv) {
  int i,a;
  struct ilist *l, *the_list, *ll;
  the_list = NULL;
  for (i=1; i<argc; i++) {
    sscanf(argv[i],"%d", &a);
    l = malloc(sizeof(struct ilist));
    l->i = a;
    SGLIB_LIST_ADD(struct ilist, the_list, l, next_ptr);
  }
  // it is useless, but anyway, get parameters in the right order
  SGLIB_LIST_REVERSE(struct ilist, the_list, next_ptr);
  // now sort them
  SGLIB_LIST_SORT(struct ilist, the_list, ILIST_COMPARATOR, next_ptr);
  // print the list
  SGLIB_LIST_MAP_ON_ELEMENTS(struct ilist, the_list, ll, next_ptr, {
    printf("%d ", ll->i);
  });
  printf("\n");
  // free all
  SGLIB_LIST_MAP_ON_ELEMENTS(struct ilist, the_list, ll, next_ptr, {
    free(ll);
  });
  return(0);
}
