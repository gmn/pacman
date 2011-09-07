
#ifndef __PATHFIND_H__
#define __PATHFIND_H__ 1


/* 
 * so all squares are basically alloted in PF_InitPathFind and stored 
 * in the pool.  the lists can also add, remove, sort them etc, but
 * they are managed in squarelist by a different set of next/prev pointers 
 * so that they are never actually removed from the pool, just marked as
 * 'in use'.
 */


enum { 
    UNUSED_TAG,
    START_TAG,
    GOAL_TAG,
    BOTH_TAG,
    SQUARE_TAG,
};

typedef enum {
    SI_TOP_LEFT,
    SI_TOP_MIDDLE,
    SI_TOP_RIGHT,
    SI_LEFT,
    SI_RIGHT,
    SI_BOT_LEFT,
    SI_BOT_MIDDLE,
    SI_BOT_RIGHT
} squareindex_t;

#include "common.h" 
#include "thinker.h"


/*
==================== 
square_t
==================== 
*/
typedef struct square_s {
    int x, y;
    int tag;
    struct square_s * parent;
    struct square_s * next;
    struct square_s * prev;
    // pnext and pprev used by pool only 
    struct square_s * pnext;
    struct square_s * pprev;
    int F, G, H;

    // squarepool pointer back into the pool square this square belongs to
    void *pool;
    void (*returnToPool)(struct square_s *);
} square_t;





/*
==================== 
squarepool_t
==================== 
*/
typedef struct squarepool_s {
    // rover indexes into pool
    int rover;
    int size;
    int numfree;

    // available & used are doubly linked lists holding members of pool
    //  in virtual storage bins depending on their state
    square_t *available;
    square_t *used;

    // pool is an array of all the alloc'd square_ts
    square_t *pool;
    
    square_t *(*getone)(struct squarepool_s *);

    // return to pool by pointer
    void (*return_p)(struct squarepool_s *, square_t *);

    // return to pool by coordinates
    void (*return_i)(struct squarepool_s *, int, int);
    void (*reset)(struct squarepool_s *);
} squarepool_t;


/*
==================== 
squarelist_t
==================== 
*/
typedef struct squarelist_s {
    int size;
    square_t *list;

    // insert one into squarelist
    square_t * (*insert)(struct squarelist_s *, square_t *);

    // fetch a particular square, popping it from the list
    square_t * (*pop)(struct squarelist_s *, square_t *);
    
    // reset squarelist
    void (*reset)(struct squarelist_s *);

    square_t * (*popLowest)(struct squarelist_s *);

    // compute the path weights of all the squares in the list
    void (*computeWeights)(struct squarelist_s *, int, int);

    // peek returning NULL or pointer to determine membership in set
    square_t * (*peek)(struct squarelist_s *, int, int, square_t *);
    square_t * (*peekp)(struct squarelist_s *, square_t *);

    int (*getSize)(struct squarelist_s *);

	int (*getSetArray)(struct squarelist_s *, square_t **);

	// only sets list to null and size to zero
	void (*quickReset)(struct squarelist_s *);

} squarelist_t;

//                      x   y   gx      gy    len   retval
gbool PF_BuildPath ( int , int , int , int , int , thinkerpath_t * );
void PF_InitSquareList (squarelist_t **sl);

#endif /* !__PATHFIND_H__ */




