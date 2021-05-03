// sglib selftest. This file invokes ALL documented
// functions and macros from sglib library and checks
// the internal consistency of results.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#include "sglib.h"

#define DEBUG 0
#define REPEAT 10
#define ORDER 1000

int val[ORDER];

int counts[ORDER];
int check_counts[ORDER];

struct MyIntList {
  unsigned 			n;
  struct MyIntList 	*next;

  // double linked list supplement
  struct MyIntList 	*previous;
  
  // rbtree supplement
  char color;
  struct MyIntList *left_ptr;
  struct MyIntList *right_ptr;
};

typedef struct MyIntList DoubleLinkedList;
typedef struct MyIntList SortedList;
typedef struct MyIntList ReverseSortedList;
typedef struct MyIntList SimpleList;
typedef struct MyIntList Tree;

#define ITERATOR_EQ_CHECK_VALUE 5

#define HASH_TAB_DIM 100

#define ARRAY_CORRESPONDENCE_FUN(val) (((val)<<2) ^ (val)>>1)

#define MY_REVERSE_LIST_CMP(e1, e2) ((e2)->n - (e1)->n)
#define MY_AB_EXCHANGER(type, a, i, j) {\
  SGLIB_ARRAY_ELEMENTS_EXCHANGER(int, a, i, j);\
  SGLIB_ARRAY_ELEMENTS_EXCHANGER(int, b, i, j);\
}

int myListCmp(struct MyIntList *e1, struct MyIntList *e2) {
  return(e1->n - e2->n);
}

unsigned slistHashFunction(struct MyIntList *e) {
  return((unsigned) e->n);
}

typedef int rint;


SGLIB_DEFINE_LIST_PROTOTYPES(SimpleList, myListCmp, next);
SGLIB_DEFINE_LIST_FUNCTIONS(SimpleList, myListCmp, next);
SGLIB_DEFINE_DL_LIST_PROTOTYPES(DoubleLinkedList, myListCmp, previous, next);
SGLIB_DEFINE_DL_LIST_FUNCTIONS(DoubleLinkedList, myListCmp, previous, next);
SGLIB_DEFINE_SORTED_LIST_PROTOTYPES(SortedList, myListCmp, next);
SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(SortedList, myListCmp, next);
SGLIB_DEFINE_SORTED_LIST_PROTOTYPES(ReverseSortedList, MY_REVERSE_LIST_CMP, next);
SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(ReverseSortedList, MY_REVERSE_LIST_CMP, next);
SGLIB_DEFINE_RBTREE_PROTOTYPES(Tree, left_ptr, right_ptr, color, myListCmp);
SGLIB_DEFINE_RBTREE_FUNCTIONS(Tree, left_ptr, right_ptr, color, myListCmp);
SGLIB_DEFINE_HASHED_CONTAINER_PROTOTYPES(SimpleList, HASH_TAB_DIM, slistHashFunction);
SGLIB_DEFINE_HASHED_CONTAINER_FUNCTIONS(SimpleList, HASH_TAB_DIM, slistHashFunction);


void generate_values() {
	int i;
	for(i=0; i<ORDER; i++) {
		val[i] = random() % ORDER;
	}
}

void compute_orders_for_array(int *a, int *counts, int max) {
	int i;
	memset(counts, 0, ORDER*sizeof(int));
	for(i=0; i<max; i++) {
		counts[a[i]]++;
	}
}

void compute_orders_for_list(struct MyIntList *list, int *counts) {
	struct MyIntList *l;
	memset(counts, 0, ORDER*sizeof(int));
	for(l=list; l!=NULL; l=l->next) {
		counts[l->n] ++;
	}
}

void compute_orders_for_hashed_list(struct MyIntList *htab[], int *counts) {
  struct MyIntList *l;
  int i;
  memset(counts, 0, ORDER*sizeof(int));
  for(i=0; i<HASH_TAB_DIM; i++) {
	for(l=htab[i]; l!=NULL; l=l->next) {
	  counts[l->n] ++;
	}
  }
}

void compare_counts(int *counts, int *check_counts) {
	int i;
	for(i=0; i<ORDER; i++) {
		if (counts[i] != check_counts[i]) {
			fprintf(stderr,"[ERROR] DIFFERENT NUMBERS IN TEST STRUCTURE !!!!!!!!!!!!!!!\n");
			fprintf(stderr,"[ERROR] %d ", i);
			if (counts[i] > check_counts[i]) {
				fprintf(stderr,"IS MISSING\n");
			} else {
				fprintf(stderr,"APPEARED\n");
			}
			assert(0);
		}
	}
}

void compare_unique_counts(int *counts, int *check_counts) {
	int i;
	for(i=0; i<ORDER; i++) {
		if (counts[i] == 0) assert(check_counts[i]==0);
		else assert(check_counts[i]==1);
	}
}

void check_array_values(int *a, int max) {
	compute_orders_for_array(val, counts, ORDER);
	compute_orders_for_array(a, check_counts, max);
	compare_counts(counts, check_counts);
}

void check_int_list_values(struct MyIntList *list) {
	compute_orders_for_array(val, counts, ORDER);
	compute_orders_for_list(list, check_counts);
	compare_counts(counts, check_counts);
}

void check_hashed_list_values(struct MyIntList *htab[]) {
	compute_orders_for_array(val, counts, ORDER);
	compute_orders_for_hashed_list(htab, check_counts);
	compare_counts(counts, check_counts);
}

void check_int_unique_list_values(struct MyIntList *list) {
	compute_orders_for_array(val, counts, ORDER);
	compute_orders_for_list(list, check_counts);
	compare_unique_counts(counts, check_counts);
}

void check_int_unique_hashed_list_values(struct MyIntList *htab[]) {
	compute_orders_for_array(val, counts, ORDER);
	compute_orders_for_hashed_list(htab, check_counts);
	compare_unique_counts(counts, check_counts);
}

void check_list_equality(struct MyIntList *list1, struct MyIntList *list2) {
  struct MyIntList *l1, *l2;
  l1=list1; l2=list2;
  while(l1 != NULL) {
	assert(l2!=NULL && l1->n == l2->n);
	l1 = l1->next;
	l2 = l2->next;
  }
  assert(l2 == NULL);
}

void check_multiple_array_correspondence(int *a, int *b) {
	int i;
	for (i=0; i<ORDER; i++) {
		if (b[i] != ARRAY_CORRESPONDENCE_FUN(a[i])) {
			fprintf(stderr,"[ERROR] ARRAY CORRESPONDENCE PROBLEM AT INDEX %d !!!!!!!!!!!!!!!\n", i);
			assert(0);
		}
	}
}

void check_that_int_array_is_sorted(int *a, int max) {
  int i;
  for(i=0; i<max-1; i++) {
	if (a[i]>a[i+1]) {
	  fprintf(stderr,"[ERROR] ARRAY UNSORTED AT INDEX %d !!!!!!!!!!!!!!!\n", i);
	  assert(0);
	}
  }
  return;
}

void check_that_int_list_is_sorted(struct MyIntList *l) {
  if (l==NULL) return;
  for(; l->next!=NULL; l=l->next) {
	if (l->n > l->next->n) {
	  fprintf(stderr,"[ERROR] LIST UNSORTED !!!!!!!!!!!!!!!\n");
	  assert(0);
	}
  }
  return;
}


void check_that_int_list_is_reverse_sorted(struct MyIntList *l) {
  if (l==NULL) return;
  for(; l->next!=NULL; l=l->next) {
	if (l->n < l->next->n) {
	  fprintf(stderr,"[ERROR] LIST UNSORTED !!!!!!!!!!!!!!!\n");
	  assert(0);
	}
  }
  return;
}

void check_double_linked_list_element(DoubleLinkedList *l) {
  if (l==NULL) return;
  if (l->previous != NULL) assert(l->previous->next == l);
  if (l->next != NULL) assert(l->next->previous == l);
}

void check_double_linked_list_consistency(DoubleLinkedList *l) {
  DoubleLinkedList *ll;
  for(ll=l; ll!=NULL; ll=ll->previous) {
	check_double_linked_list_element(ll);
  }
  for(ll=l; ll!=NULL; ll=ll->next) {
	check_double_linked_list_element(ll);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


void array_quick_sort_test() {
	int a[ORDER];
	int b[ORDER];
	int i,pass;
	//srandom(seed);
	generate_values();
	for(i=0; i<ORDER; i++) {
		a[i] = val[i];
		b[i] = ARRAY_CORRESPONDENCE_FUN(val[i]);
	}
#if DEBUG
	for(i=0; i<ORDER; i++) fprintf(stderr,"%d ", a[i]);
	fprintf(stderr,"\n");
#endif
	SGLIB_ARRAY_QUICK_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
#if DEBUG
	for(i=0; i<ORDER; i++) fprintf(stderr,"%d ", a[i]);
	fprintf(stderr,"\n\n");
#endif
	check_that_int_array_is_sorted(a, ORDER);
	check_array_values(a, ORDER);
	check_multiple_array_correspondence(a, b);

	//&sglib_rint_array_quick_sort(a, ORDER);
}

void array_heap_sort_test() {
	int a[ORDER];
	int b[ORDER];
	int i,pass;
	//srandom(seed);
	generate_values();
	for(i=0; i<ORDER; i++) {
		a[i] = val[i];
		b[i] = ARRAY_CORRESPONDENCE_FUN(val[i]);
	}
#if DEBUG
	for(i=0; i<ORDER; i++) fprintf(stderr,"%d ", a[i]);
	fprintf(stderr,"\n");
#endif
	SGLIB_ARRAY_HEAP_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
#if DEBUG
	for(i=0; i<ORDER; i++) fprintf(stderr,"%d ", a[i]);
	fprintf(stderr,"\n\n");
#endif
	check_that_int_array_is_sorted(a, ORDER);
	check_array_values(a, ORDER);
	check_multiple_array_correspondence(a, b);

	//sglib_heap_sort_rint_array(a, ORDER);
}

void list_sort_test() {
	int 		a[ORDER];
	int 		i,pass;
	SortedList	*list, *elem, *l, *lnext;
	//srandom(seed);
	generate_values();
	list = NULL;
	for(i=0; i<ORDER; i++) {
		elem = malloc(sizeof(SortedList));
		elem->n = val[i];
		elem->next = list;
		list = elem;
	}
#if DEBUG
	for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n);
	fprintf(stderr,"\n");
#endif
	sglib_SimpleList_sort(&list);
#if DEBUG
	for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n);
	fprintf(stderr,"\n\n");
#endif
	check_that_int_list_is_sorted(list);
	check_int_list_values(list);

	for(l=list; l!=NULL; l=lnext) {
		lnext = l->next;
		free(l);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  LIST TEST   ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void list_test() {
	int i, len, cc, cc5, r;
	int a[ORDER];
	int b[ORDER];
	struct MyIntList *list, *list2, *l, *ll, *t;
	struct MyIntList *e, *e2, te, *memb, *memb2, *_current_element_;
	struct sglib_SimpleList_iterator	it;

	//srandom(seed);
	generate_values();

	list = NULL; list2 = NULL;
	for(i=0; i<ORDER; i++) {
		assert(sglib_SimpleList_len(list) == i);
		
		a[i] = val[i];
		b[i] = i;
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_SimpleList_add(&list, e);
		e2 = malloc(sizeof(struct MyIntList));
		e2->n = val[i];
		e2->next = NULL;
		sglib_SimpleList_concat(&list2, e2);
	}

	check_int_list_values(list);
	check_int_list_values(list2);

	//&for(l=list2; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	SGLIB_LIST_REVERSE(struct MyIntList, list2, next);
	check_list_equality(list, list2);


	SGLIB_LIST_MAP_ON_ELEMENTS(SimpleList, list2, _current_element_, next, {
	  free(_current_element_);
	});

	//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	// this creates a permutation in b
	SGLIB_ARRAY_QUICK_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_SimpleList_find_member(list, &te) != NULL);
		te.n = -val[i]-1;
		assert(sglib_SimpleList_find_member(list, &te) == NULL);
	}

	SGLIB_LIST_MAP_ON_ELEMENTS(SimpleList, list, _current_element_, next, {
	  assert(sglib_SimpleList_is_member(list, _current_element_));
	});
	assert( ! sglib_SimpleList_is_member(list, &te));

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		SGLIB_LIST_FIND_MEMBER(SimpleList, list, &te, myListCmp, next, memb);
		assert(memb!=NULL);
		assert(list!=NULL);
		sglib_SimpleList_delete(&list, memb);
		free(memb);
		//&fprintf(stderr,"del(%d)  ", a[b[i]]);
		//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	}
	assert(list==NULL);


	list = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		r = sglib_SimpleList_add_if_not_member(&list, e, &memb2);
		if (r==0) free(e);
	}

	//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");

	check_int_unique_list_values(list);

	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_SimpleList_find_member(list, &te)!=NULL);
	}

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		SGLIB_LIST_FIND_MEMBER(SimpleList, list, &te, myListCmp, next, memb);
		sglib_SimpleList_delete_if_member(&list, &te, &memb2);
		assert(memb == memb2);
		if (memb!=NULL) free(memb);
	}
	assert(list==NULL);


	list = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_SimpleList_add(&list, e);
	}
	
	check_int_list_values(list);
	sglib_SimpleList_sort(&list);
	check_that_int_list_is_sorted(list);
	check_int_list_values(list);
	sglib_SimpleList_reverse(&list);
	check_int_list_values(list);
	check_that_int_list_is_reverse_sorted(list);

	list2 = NULL;
	SGLIB_LIST_MAP_ON_ELEMENTS(SimpleList, list, _current_element_, next, {
		e = malloc(sizeof(struct MyIntList));
		e->n = _current_element_->n;
		sglib_SimpleList_add(&list2, e);	  
	});

	// test iterators
	assert(list!=NULL);
	ll = NULL; cc = cc5 = 0;
	SGLIB_LIST_MAP_ON_ELEMENTS(SimpleList, list, _current_element_, next, {
	  if (cc==0) ll = sglib_SimpleList_it_init(&it, list);
	  assert(ll==_current_element_);
	  cc ++;
	  if (ll!=NULL && ll->n == ITERATOR_EQ_CHECK_VALUE) cc5 ++;
	  ll =  sglib_SimpleList_it_next(&it);
	});
	assert(ll==NULL);
	te.n = ITERATOR_EQ_CHECK_VALUE;
	cc = 0;
	for(ll=sglib_SimpleList_it_init_on_equal(&it, list, myListCmp, &te);
		ll!=NULL;
		ll=sglib_SimpleList_it_next(&it)
		) {
	  cc++;
	  assert(ll->n == ITERATOR_EQ_CHECK_VALUE);
	}
	assert(cc == cc5);

	// free all
	SGLIB_LIST_MAP_ON_ELEMENTS(SimpleList, list, _current_element_, next, {
		free(_current_element_);
	});
	for(ll=sglib_SimpleList_it_init(&it, list2);
		ll!=NULL;
		ll=sglib_SimpleList_it_next(&it)
		) {
		free(ll);
	}

}


/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  HASHED_LIST TEST   ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void hashed_list_test() {
	int i, len, cc, cc5;
	int a[ORDER];
	int b[ORDER];
	struct MyIntList *htab[HASH_TAB_DIM];
	struct MyIntList te,*e, *ll, *memb, *memb2;
	struct sglib_hashed_SimpleList_iterator it;

	//srandom(seed);
	generate_values();

	sglib_hashed_SimpleList_init(htab);

	for(i=0; i<ORDER; i++) {
		a[i] = val[i];
		b[i] = i;
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_hashed_SimpleList_add(htab, e);
	}

	check_hashed_list_values(htab);

	SGLIB_ARRAY_QUICK_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		memb = sglib_hashed_SimpleList_find_member(htab, &te);
		assert(memb != NULL);
		assert(sglib_hashed_SimpleList_is_member(htab, memb) != 0);
		te.n = -val[i]-1;
		assert(sglib_hashed_SimpleList_find_member(htab, &te) == NULL);
		assert(sglib_hashed_SimpleList_is_member(htab, &te) == 0);
	}

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		memb = sglib_hashed_SimpleList_find_member(htab, &te);
		assert(memb!=NULL);
		sglib_hashed_SimpleList_delete(htab, memb);
		free(memb);
		//&fprintf(stderr,"del(%d)  ", a[b[i]]);
		//&for(l=htab; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	}


	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		if (! sglib_hashed_SimpleList_add_if_not_member(htab, e, &memb2)) free(e);
	}

	//&for(l=hashed_list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");

	check_int_unique_hashed_list_values(htab);

	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_hashed_SimpleList_find_member(htab, &te)!=NULL);
	}

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		memb = sglib_hashed_SimpleList_find_member(htab, &te);
		sglib_hashed_SimpleList_delete_if_member(htab, &te, &memb2);
		assert(memb == memb2);
		if (memb!=NULL) free(memb);
	}

	// iterators
	ll = NULL; cc = cc5 = 0;
	for(i=0; i<HASH_TAB_DIM; i++) {
		SGLIB_LIST_MAP_ON_ELEMENTS(SimpleList, htab[i], _current_element_, next, {
			if (cc==0) ll = sglib_hashed_SimpleList_it_init(&it, htab);
			assert(ll==_current_element_);
			cc ++;
			if (ll!=NULL && ll->n == ITERATOR_EQ_CHECK_VALUE) cc5 ++;
			ll =  sglib_hashed_SimpleList_it_next(&it);
		});
		assert(ll==NULL);
	}
	te.n = ITERATOR_EQ_CHECK_VALUE;
	cc = 0;
	for(ll=sglib_hashed_SimpleList_it_init_on_equal(&it, htab, myListCmp, &te);
		ll!=NULL;
		ll=sglib_hashed_SimpleList_it_next(&it)
		) {
		cc++;
		assert(ll->n == ITERATOR_EQ_CHECK_VALUE);
	}
	assert(cc == cc5);

	// free all
	for(ll=sglib_hashed_SimpleList_it_init(&it, htab);
		ll!=NULL;
		ll=sglib_hashed_SimpleList_it_next(&it)
		) {
		free(ll);
	}

}


/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  DOUBLE LINKED LIST TEST   /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void double_linked_list_test() {
	int i, len, cc, cc5, r;
	int a[ORDER];
	int b[ORDER];
	struct MyIntList	*list, *list2, *list3, *list4, *l, *ll, *l2, *l3, *l4, *t, *e, *e2, te, *memb, *memb2, *_current_element_;
	struct sglib_DoubleLinkedList_iterator it;


	//srandom(seed);
	generate_values();

	list = NULL; list2 = list3 = list4 = NULL;
	for(i=0; i<ORDER; i++) {
		assert(sglib_DoubleLinkedList_len(list) == i);
		
		a[i] = val[i];
		b[i] = i;
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_DoubleLinkedList_add(&list, e);
		check_double_linked_list_consistency(list);

		e2 = malloc(sizeof(struct MyIntList));
		e2->n = val[i];
		e2->previous = e2->next = NULL;
		sglib_DoubleLinkedList_concat(&list2, e2);
		check_double_linked_list_consistency(list2);

		e2 = malloc(sizeof(struct MyIntList));
		e2->n = val[i];
		sglib_DoubleLinkedList_add_after(&list3, e2);
		check_double_linked_list_consistency(list3);

		e2 = malloc(sizeof(struct MyIntList));
		e2->n = val[i];
		sglib_DoubleLinkedList_add_before(&list4, e2);
		check_double_linked_list_consistency(list4);
	}

	//SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list, previous, next, fprintf(stderr,"%d ", _current_element_->n)); fprintf(stderr,"\n");
	l = sglib_DoubleLinkedList_get_first(list);
	check_int_list_values(l);
	l2 = sglib_DoubleLinkedList_get_first(list2);
	check_int_list_values(l2);
	l3 = sglib_DoubleLinkedList_get_first(list3);
	check_int_list_values(l3);
	l4 = sglib_DoubleLinkedList_get_first(list4);
	check_int_list_values(l4);

	sglib_DoubleLinkedList_sort(&l);
	sglib_DoubleLinkedList_sort(&l2);
	sglib_DoubleLinkedList_sort(&l3);
	sglib_DoubleLinkedList_sort(&l4);

	check_list_equality(l, l2);
	check_list_equality(l, l3);
	check_list_equality(l, l4);


	//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	// this creates a permutation in b
	SGLIB_ARRAY_QUICK_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_DoubleLinkedList_find_member(list, &te) != NULL);
		te.n = -val[i]-1;
		assert(sglib_DoubleLinkedList_find_member(list, &te) == NULL);
	}

	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list, _current_element_, previous, next, {
	  assert(sglib_DoubleLinkedList_is_member(list, _current_element_));
	});
	assert( ! sglib_DoubleLinkedList_is_member(list, &te));

	for(i=0; i<ORDER; i++) {
	    te.n =  val[i];// a[b[i]];
		SGLIB_DL_LIST_FIND_MEMBER(DoubleLinkedList, list, &te, myListCmp, previous, next, memb);
		assert(memb!=NULL);
		assert(list!=NULL);
		//for(l=list; l!=NULL && l->previous!=NULL; l=l->previous);for(; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
		//fprintf(stderr,"del(%d)  ", te.n);
		sglib_DoubleLinkedList_delete(&list, memb);
		free(memb);
		check_double_linked_list_consistency(list);
	}
	//for(l=list; l!=NULL && l->previous!=NULL; l=l->previous);for(; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	assert(list==NULL);

	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list2, _current_element_, previous, next, {
	  free(_current_element_);
	});
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list3, _current_element_, previous, next, {
	  free(_current_element_);
	});
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list4, _current_element_, previous, next, {
	  free(_current_element_);
	});


	list = list2 = list3 = list4 = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		r = sglib_DoubleLinkedList_add_if_not_member(&list, e, &memb2);
		if (r==0) free(e);
		check_double_linked_list_consistency(list);
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		r = sglib_DoubleLinkedList_add_before_if_not_member(&list2, e, &memb2);
		if (r==0) free(e);
		check_double_linked_list_consistency(list2);
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		r = sglib_DoubleLinkedList_add_after_if_not_member(&list3, e, &memb2);
		if (r==0) free(e);
		check_double_linked_list_consistency(list3);
	}

	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_DoubleLinkedList_find_member(list, &te)!=NULL);
	}

	l = sglib_DoubleLinkedList_get_first(list);
	check_int_unique_list_values(l);
	l2 = sglib_DoubleLinkedList_get_first(list2);
	check_int_unique_list_values(l2);
	l3 = sglib_DoubleLinkedList_get_first(list3);
	check_int_unique_list_values(l3);

	sglib_DoubleLinkedList_sort(&l);
	sglib_DoubleLinkedList_sort(&l2);
	sglib_DoubleLinkedList_sort(&l3);

	check_list_equality(l, l2);
	check_list_equality(l, l3);


	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		SGLIB_DL_LIST_FIND_MEMBER(DoubleLinkedList, list, &te, myListCmp, previous, next, memb);
		sglib_DoubleLinkedList_delete_if_member(&list, &te, &memb2);
		assert(memb == memb2);
		if (memb!=NULL) free(memb);
		check_double_linked_list_consistency(list);
	}
	assert(list==NULL);


	list = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_DoubleLinkedList_add(&list, e);
		check_double_linked_list_consistency(list);
	}
	
	for(l=list; l!=NULL && l->previous!=NULL; l=l->previous); 
	check_int_list_values(l);

	sglib_DoubleLinkedList_sort(&list);
	check_double_linked_list_consistency(list);

	for(l=list; l!=NULL && l->previous!=NULL; l=l->previous);
	check_that_int_list_is_sorted(l);
	check_int_list_values(l);

	//for(l=list; l!=NULL && l->previous!=NULL; l=l->previous);for(; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	sglib_DoubleLinkedList_reverse(&list);
	check_double_linked_list_consistency(list);

	for(l=list; l!=NULL && l->previous!=NULL; l=l->previous);
	check_int_list_values(l);
	check_that_int_list_is_reverse_sorted(l);

	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list2, _current_element_, previous, next, {
	  free(_current_element_);
	});
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list3, _current_element_, previous, next, {
	  free(_current_element_);
	});

	list2 = NULL;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list, _current_element_, previous, next, {
		e = malloc(sizeof(struct MyIntList));
		e->n = _current_element_->n;
		sglib_DoubleLinkedList_add(&list2, e);
		check_double_linked_list_consistency(list2);
	});

	// test iterators
	assert(list!=NULL);
	ll = NULL; cc = cc5 = 0;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list, _current_element_, previous, next, {
	  if (cc==0) ll = sglib_DoubleLinkedList_it_init(&it, list);
	  assert(ll==_current_element_);
	  cc ++;
	  if (ll!=NULL && ll->n == ITERATOR_EQ_CHECK_VALUE) cc5 ++;
	  ll =  sglib_DoubleLinkedList_it_next(&it);
	});
	assert(ll==NULL);
	te.n = ITERATOR_EQ_CHECK_VALUE;
	cc = 0;
	for(ll=sglib_DoubleLinkedList_it_init_on_equal(&it, list, myListCmp, &te);
		ll!=NULL;
		ll=sglib_DoubleLinkedList_it_next(&it)
		) {
	  cc++;
	  assert(ll->n == ITERATOR_EQ_CHECK_VALUE);
	}
	assert(cc == cc5);

	// free all
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list, _current_element_, previous, next, {
		free(_current_element_);
	});

	for(ll=sglib_DoubleLinkedList_it_init(&it, list2);
		ll!=NULL;
		ll=sglib_DoubleLinkedList_it_next(&it)
		) {
		free(ll);
	}

}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  SORTED LIST TEST   ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void sorted_list_test() {
	int i, len, cc, cc5, r;
	int a[ORDER];
	int b[ORDER];
	struct MyIntList	*list, *list2, *l, *ll, *e, te, *memb, *memb2;
	SimpleList *el;
	struct sglib_SortedList_iterator it;

	//srandom(seed);
	generate_values();

	list = NULL;
	for(i=0; i<ORDER; i++) {
		assert(sglib_SortedList_len(list) == i);
		
		a[i] = val[i];
		b[i] = i;
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_SortedList_add(&list, e);
		check_that_int_list_is_sorted(list);
	}

	check_int_list_values(list);

	//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	// this creates a permutation in b
	SGLIB_ARRAY_QUICK_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_SortedList_find_member(list, &te)!=NULL);
		te.n = -val[i]-1;
		assert(sglib_SortedList_find_member(list, &te)==NULL);
	}

	SGLIB_SORTED_LIST_MAP_ON_ELEMENTS(SimpleList, list, el, next, {
	  assert(sglib_SortedList_is_member(list, el));
	});
	assert( ! sglib_SortedList_is_member(list, &te));

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		SGLIB_SORTED_LIST_FIND_MEMBER(SortedList, list, &te, myListCmp, next, memb);
		assert(memb!=NULL);
		assert(list!=NULL);
		sglib_SortedList_delete(&list, memb);
		check_that_int_list_is_sorted(list);
		free(memb);
		//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	}
	assert(list==NULL);


	list = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		r = sglib_SortedList_add_if_not_member(&list, e, &memb2);
		if (r==0) free(e);
		check_that_int_list_is_sorted(list);
	}

	check_int_unique_list_values(list);

	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_SortedList_find_member(list, &te)!=NULL);
	}

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		SGLIB_LIST_FIND_MEMBER(SortedList, list, &te, myListCmp, next, memb);
		sglib_SortedList_delete_if_member(&list, &te, &memb2);
		assert(memb == memb2);
		if (memb!=NULL) free(memb);
		check_that_int_list_is_sorted(list);
	}
	assert(list==NULL);


	list = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(struct MyIntList));
		e->n = val[i];
		sglib_SortedList_add(&list, e);
	}
	
	check_int_list_values(list);
	check_that_int_list_is_sorted(list);

	sglib_SortedList_sort(&list);

	list2 = NULL;
	SGLIB_SORTED_LIST_MAP_ON_ELEMENTS(DoubleLinkedList, list, _current_element_, next, {
		e = malloc(sizeof(struct MyIntList));
		e->n = _current_element_->n;
		sglib_SortedList_add(&list2, e);
		check_that_int_list_is_sorted(list2);
	});

	// test iterators
	assert(list!=NULL);
	ll = NULL; cc = cc5 = 0;
	SGLIB_SORTED_LIST_MAP_ON_ELEMENTS(SortedList, list, _current_element_, next, {
	  if (cc==0) ll = sglib_SortedList_it_init(&it, list);
	  assert(ll==_current_element_);
	  cc ++;
	  if (ll!=NULL && ll->n == ITERATOR_EQ_CHECK_VALUE) cc5 ++;
	  ll =  sglib_SortedList_it_next(&it);
	});
	assert(ll==NULL);
	te.n = ITERATOR_EQ_CHECK_VALUE;
	cc = 0;
	for(ll=sglib_SortedList_it_init_on_equal(&it, list, myListCmp, &te);
		ll!=NULL;
		ll=sglib_SortedList_it_next(&it)
		) {
	  cc++;
	  assert(ll->n == ITERATOR_EQ_CHECK_VALUE);
	}
	//&fprintf(stdout,"checking %d == %d\n", cc, cc5); fflush(stdout);
	assert(cc == cc5);

	// free all
	SGLIB_SORTED_LIST_MAP_ON_ELEMENTS(SortedList, list, _current_element_, next, {
		free(_current_element_);
	});

	for(ll=sglib_SortedList_it_init(&it, list2);
		ll!=NULL;
		ll=sglib_SortedList_it_next(&it)
		) {
		free(ll);
	}


}




/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  RED_BLACK TREE TEST   ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


void sglib_Tree_dump_rec(Tree *t) {
  if (t == NULL) return;
  printf("(");fflush(stdout);
  sglib_Tree_dump_rec(t->left_ptr);
  printf("%s%d ", ((t->color==0)?"r":"b"), t->n);fflush(stdout);
  sglib_Tree_dump_rec(t->right_ptr);
  printf(")");fflush(stdout);
}

void sglib_Tree_dump(Tree *t) {
  printf("\n");fflush(stdout);
  sglib_Tree_dump_rec(t);
  printf("\n");fflush(stdout);
}

void rbtree_test() {
	int							i,j,k,r,n;
	int 						a[ORDER];
	int 						b[ORDER];
	Tree						*tree, *e, te, *memb, *memb2, *me, *t, tt;
	SimpleList					*list, *l;
	struct sglib_Tree_iterator	it;
	//srandom(seed);
	generate_values();
	tree = NULL;
	for(i=0; i<ORDER; i++) {
		assert(sglib_Tree_len(tree) == i);
		a[i] = val[i];
		b[i] = i;
		e = malloc(sizeof(Tree));
		e->n = val[i];
		sglib_Tree_add(&tree, e);
		//&SGLIB_BIN_TREE_MAP_ON_ELEMENTS(Tree, tree, me, left_ptr, right_ptr, {fprintf(stderr,"%d ", me->n);});fprintf(stderr,"\n");
		sglib___Tree_consistency_check(tree);
	}

	// create a list of tree elements
	list = NULL;
	SGLIB_BIN_TREE_MAP_ON_ELEMENTS(Tree, tree, me, left_ptr, right_ptr, {
		sglib_SimpleList_add(&list, me);
	});
	check_int_list_values(list);

	SGLIB_BIN_TREE_MAP_ON_ELEMENTS(Tree, tree, me, left_ptr, right_ptr, {
		assert(sglib_Tree_is_member(tree, me));
	});
	te.n = val[0];
	assert(! sglib_Tree_is_member(tree, &te));

	t=sglib_Tree_it_init_inorder(&it, tree); 
	SGLIB_BIN_TREE_MAP_ON_ELEMENTS(Tree, tree, me, left_ptr, right_ptr, {
		assert(t!=NULL);
		assert(t==me);
		t = sglib_Tree_it_next(&it);
		assert(t==sglib_Tree_it_current(&it));
	});

	compute_orders_for_list(list, check_counts);
	for(i=0; i<ORDER; i++) {
		tt.n = i;
		n = 0;
		for(l=sglib_Tree_it_init_on_equal(&it, tree, NULL, &tt); 
			l!=NULL; 
			l=sglib_Tree_it_next(&it)) {
			assert(l->n == i);
			n++;
		}
		assert(n == check_counts[i]);
		n = 0;
		for(l=sglib_Tree_it_init_on_equal(&it, tree, myListCmp, &tt); 
			l!=NULL; 
			l=sglib_Tree_it_next(&it)) {
			assert(l->n == i);
			n++;
		}
		assert(n == check_counts[i]);
	}


	//&for(l=list; l!=NULL; l=l->next) fprintf(stderr,"%d ", l->n); fprintf(stderr,"\n");
	
	SGLIB_ARRAY_QUICK_SORT(int, a, ORDER, SGLIB_NUMERIC_COMPARATOR, MY_AB_EXCHANGER);
	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_Tree_find_member(tree, &te)!=NULL);
		te.n = -val[i]-1;
		assert(sglib_Tree_find_member(tree, &te)==NULL);
	}

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		memb = sglib_Tree_find_member(tree, &te);
		assert(memb!=NULL);
		assert(tree!=NULL);
		sglib_Tree_delete(&tree, memb);
		free(memb);
		sglib___Tree_consistency_check(tree);
	}
	assert(tree==NULL);

	tree = NULL;
	for(i=0; i<ORDER; i++) {
		e = malloc(sizeof(Tree));
		e->n = val[i];
		//sglib_Tree_add(&tree, e);
		r = sglib_Tree_add_if_not_member(&tree, e, &memb2);
		if (r==0) free(e);
		//&SGLIB_BIN_TREE_MAP_ON_ELEMENTS(Tree, tree, me, left_ptr, right_ptr, {fprintf(stderr,"%d ", me->n);});fprintf(stderr,"\n");
		sglib___Tree_consistency_check(tree);
	}

	// create a list of tree elements
	list = NULL;
	SGLIB_BIN_TREE_MAP_ON_ELEMENTS(Tree, tree, me, left_ptr, right_ptr, {
		sglib_SimpleList_add(&list, me);
	});

	check_int_unique_list_values(list);

	// checkin is_member
	for(i=0; i<ORDER; i++) {
		te.n = val[i];
		assert(sglib_Tree_find_member(tree, &te)!=NULL);
	}

	for(i=0; i<ORDER; i++) {
		te.n = a[b[i]];
		memb = sglib_Tree_find_member(tree, &te);
		sglib_Tree_delete_if_member(&tree, &te, &memb2);
		assert(memb == memb2);
		if (memb!=NULL) free(memb);
		sglib___Tree_consistency_check(tree);
	}
	assert(tree==NULL);

}  


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


int main() {
	int pass;
	srandom(0);
	srandom(time(NULL));
	for(pass=0; pass<REPEAT; pass++) {
		array_quick_sort_test();
		array_heap_sort_test();
		list_sort_test();
		list_test();
		hashed_list_test();
		double_linked_list_test();
		sorted_list_test();
		rbtree_test();
	}
	return(0);
}
