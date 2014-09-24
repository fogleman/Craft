#include <stdio.h>
#include <stdlib.h>

#include "../sglib.h"

/* 
  an alternative level-0 interface to queues, 
  it will probably not be accepted to Sglib core, because
  its level-1 interface is doing the same level of abstraction.
*/

#define SGLIB_NQUEUE_TYPEDEF(type, dim, queuetype) \
  typedef struct queuetype { \
    type a[dim]; \
    int i,j; \
    int dim; \
  } queuetype;

#define SGLIB_NQUEUE_INIT(type, q) SGLIB_QUEUE_INIT(type, (q).a, (q).i, (q).j)
#define SGLIB_NQUEUE_IS_EMPTY(type, q) SGLIB_QUEUE_IS_EMPTY(type, (q).a, (q).i, (q).j)
#define SGLIB_NQUEUE_IS_FULL(type, q) SGLIB_QUEUE_IS_FULL(type, (q)., (q).i, (q).j, (q).dim)
#define SGLIB_NQUEUE_FIRST_ELEMENT(type, q) SGLIB_QUEUE_FIRST_ELEMENT(type, (q).a, (q).i, (q).j)
#define SGLIB_NQUEUE_ADD_NEXT(type, q) SGLIB_QUEUE_ADD_NEXT(type, (q).a, (q).i, (q).j, (q).dim)
#define SGLIB_NQUEUE_ADD(type, q, elem) SGLIB_QUEUE_ADD(type, (q).a, elem, (q).i, (q).j, (q).dim) 
#define SGLIB_NQUEUE_DELETE_FIRST(type, q) SGLIB_QUEUE_DELETE_FIRST(type, (q).a, (q).i, (q).j, (q).dim)
#define SGLIB_NQUEUE_DELETE(type, q) SGLIB_NQUEUE_DELETE_FIRST(type, q)


/* *********************************************************** */

SGLIB_NQUEUE_TYPEDEF(int, 100, myqueue);


void main() {
	myqueue t;
	


}
