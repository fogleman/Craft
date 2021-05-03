// This program sorts its parameters using 
// insertion into sorted list (level 1 interface). 
// For example:
//   a.out 6 7 3 4 1 5
// writes
//   1 3 4 5 6 7


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "sglib.h"

typedef struct ilist {
    int i;
    struct ilist *next_ptr;
} iListType;

#define ILIST_COMPARATOR(e1, e2) (e1->i - e2->i)

SGLIB_DEFINE_SORTED_LIST_PROTOTYPES(iListType, ILIST_COMPARATOR, next_ptr)
SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(iListType, ILIST_COMPARATOR, next_ptr)

int main(int argc, char **argv) {
  int                               i,a;
  struct ilist                      *l, *the_list;
  struct sglib_iListType_iterator   it;
  the_list = NULL;
  for (i=1; i<argc; i++) {
    sscanf(argv[i],"%d", &a);
    l = malloc(sizeof(struct ilist));
    l->i = a;
    // insert the new element into the list while keeping it sorted
    sglib_iListType_add(&the_list, l);
  }
  // print the list
  for (l=the_list; l!=NULL; l=l->next_ptr) {
    printf("%d ", l->i);
  }
  printf("\n");
  // free all
  for(l=sglib_iListType_it_init(&it,the_list); l!=NULL; l=sglib_iListType_it_next(&it)) {
    free(l);  
  }
  return(0);
}
