// This program reads parameters, sorts them
// and write them in both directions.
// The program is using level 1 interface.
// For example:
//   a.out 6 7 3 4 1 5
// writes
//   1 3 4 5 6 7
//   7 6 5 4 3 1


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "sglib.h"

typedef struct dllist {
    int i;
    struct dllist *ptr_to_next;
    struct dllist *ptr_to_previous;
} dllist;

#define DLLIST_COMPARATOR(e1, e2) (e1->i - e2->i)

SGLIB_DEFINE_DL_LIST_PROTOTYPES(dllist, DLLIST_COMPARATOR, ptr_to_previous, ptr_to_next);
SGLIB_DEFINE_DL_LIST_FUNCTIONS(dllist, DLLIST_COMPARATOR, ptr_to_previous, ptr_to_next);

int main(int argc, char **argv) {
  int                           i,a;
  dllist                        *l, *the_list;
  struct sglib_dllist_iterator  it;

  the_list = NULL;
  for (i=1; i<argc; i++) {
    sscanf(argv[i],"%d", &a);
    l = malloc(sizeof(dllist));
    l->i = a;
    sglib_dllist_add(&the_list, l);
  }
  // sort the list
  sglib_dllist_sort(&the_list);
  // print the list
  for(l=sglib_dllist_get_first(the_list); l!=NULL; l=l->ptr_to_next) printf("%d ", l->i);
  printf("\n");
  // print the list in reversed direction
  for(l=sglib_dllist_get_last(the_list); l!=NULL; l=l->ptr_to_previous) printf("%d ", l->i);
  printf("\n");
  // free the list
  for(l=sglib_dllist_it_init(&it,the_list); l!=NULL; l=sglib_dllist_it_next(&it)) {
    free(l);
  }
  return(0);
}
