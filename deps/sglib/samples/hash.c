// This program uses hash table containing lists
// to remove multiple occurences of its parameters
// For example:
//   a.out 1 3 1 5 2 3 1 7 11 33 11
// writes:
//   11 1 2 33 3 5 7

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "sglib.h"

#define HASH_TAB_SIZE  10

typedef struct ilist {
  int i;
  struct ilist *next;
} ilist;

ilist *htab[HASH_TAB_SIZE];

#define ILIST_COMPARATOR(e1, e2)    (e1->i - e2->i)

unsigned int ilist_hash_function(ilist *e) {
  return(e->i);
}

SGLIB_DEFINE_LIST_PROTOTYPES(ilist, ILIST_COMPARATOR, next)
SGLIB_DEFINE_LIST_FUNCTIONS(ilist, ILIST_COMPARATOR, next)
SGLIB_DEFINE_HASHED_CONTAINER_PROTOTYPES(ilist, HASH_TAB_SIZE, ilist_hash_function)
SGLIB_DEFINE_HASHED_CONTAINER_FUNCTIONS(ilist, HASH_TAB_SIZE, ilist_hash_function)

int main(int argc, char **argv) {
  int                                   i, ai,aj, n;
  struct ilist                          ii, *nn, *ll;
  struct sglib_hashed_ilist_iterator    it;

  sglib_hashed_ilist_init(htab);

  for (i=1; i<argc; i++) {
    sscanf(argv[i],"%d", &n);
    ii.i = n;
    if (sglib_hashed_ilist_find_member(htab, &ii) == NULL) {
      nn = malloc(sizeof(struct ilist));
      nn->i = n;
      sglib_hashed_ilist_add(htab, nn);
    }
  }

  for(ll=sglib_hashed_ilist_it_init(&it,htab); ll!=NULL; ll=sglib_hashed_ilist_it_next(&it)) {
      printf("%d ", ll->i);
  }
  printf("\n");

  for(ll=sglib_hashed_ilist_it_init(&it,htab); ll!=NULL; ll=sglib_hashed_ilist_it_next(&it)) {
      free(ll);
  }
  return(0);
}
