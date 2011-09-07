//
// pathfind.c

#include "pathfind.h"
#include "gameboard.h"
#include "virtmem.h"
#include "thinker.h"
#include "i_system.h"

// why 160
#define SQUAREPOOLSIZE 320

// tags in header
//#define GOAL_TAG  1
//#define START_TAG 2
//#define BOTH_TAG  3

squarepool_t *squarepool; 

squarelist_t *openlist;
squarelist_t *closedlist;
squarelist_t *pathlist;
squarelist_t *currentset;

extern gameboard_t gameboard;


/*
==================== 
 PF_ConcatenateLists
 - not used so far, I only keep because I like "messenger_rna"
==================== 
*/
squarelist_t * PF_ConcatenateLists (squarelist_t *sl, squarelist_t *set)
{
    int i;
    square_t *messenger_rna[160];
    square_t *sp;

    if (!set->list)
        return NULL;

    i = 0;
    sp = set->list;
    do {
        messenger_rna[i++] = sp;
        sp = sp->next;
    } while ( sp != set->list );

    while (--i >= 0) 
        sl->insert(sl, messenger_rna[i]);

    return sl;
}


/*
====================
 PF_AddSquareToList()
 - used in PF_CreateSurroundingSet.  sets parent  
====================
*/
static square_t * PF_AddSquareToList (squarelist_t *sl, int x, int y, square_t *parent)
{
    square_t *sq = ((squarepool_t *)parent->pool)->getone(parent->pool);
	if (!sq)
		return NULL;
	
    sq->x = x;
    sq->y = y;
    sq->tag = SQUARE_TAG;
    sq->parent = parent;

    return sl->insert (sl, sq);
}


#define INBOUNDS(x,y) ((x) >= 0 && (x) < (MAX_X*MAX_Y) && y[(x)])
/*
==================== 
 PF_CreateSurroundingSet
 - builds set surrounding a square. only doing horizontal,vertical movements 
==================== 
*/
#define SURROUNDING_SET_SIZE 4
// TODO : rewrite in ASM, unroll loops
// unrolled loop version
static gbool PF_CreateSurroundingSet (squarelist_t *closed, squarelist_t *curset, square_t *sp)
{
    int     ind[SURROUNDING_SET_SIZE], 
            xset[SURROUNDING_SET_SIZE], 
            yset[SURROUNDING_SET_SIZE];
	gbool	toobig = gfalse;

    // initialize sets
    xset[0] = sp->x;
    xset[1] = sp->x - 1;
    xset[2] = sp->x + 1;
    xset[3] = sp->x;

    yset[0] = sp->y + 1;
    yset[1] = sp->y; 
    yset[2] = sp->y;
    yset[3] = sp->y - 1;

    ind[0] = MAX_X * yset[0] + xset[0];
    ind[1] = MAX_X * yset[1] + xset[1];
    ind[2] = MAX_X * yset[2] + xset[2];
    ind[3] = MAX_X * yset[3] + xset[3];


    // reset the currentset
    curset->reset(curset);

    // check the surrounding squares, if they are on the map
    //  and open and not a member of the closedlist, set x,y,parent
    //  add add to the current set.
    if (( INBOUNDS( ind[0], gameboard.index )) &&
        ( gameboard.index[ind[0]]->type == ST_OPEN ) &&
			!closed->peek( closed, xset[0], yset[0], sp )) {
            if (!PF_AddSquareToList( curset, xset[0], yset[0], sp ))
				return gfalse;
    }
    if (( INBOUNDS( ind[1], gameboard.index )) &&
        ( gameboard.index[ind[1]]->type == ST_OPEN ) &&
			!closed->peek( closed, xset[1], yset[1], sp )) {
            if (!PF_AddSquareToList( curset, xset[1], yset[1], sp ))
				return gfalse;
    }
    if (( INBOUNDS(ind[2], gameboard.index)) &&
        ( gameboard.index[ind[2]]->type == ST_OPEN ) &&
			!closed->peek( closed, xset[2], yset[2], sp )) {
            if (!PF_AddSquareToList( curset, xset[2], yset[2], sp ))
				return gfalse;
    }
    if (( INBOUNDS( ind[3], gameboard.index )) &&
        ( gameboard.index[ind[3]]->type == ST_OPEN ) &&
			!closed->peek( closed, xset[3], yset[3], sp )) {
            if (!PF_AddSquareToList( curset, xset[3], yset[3], sp ))
				return gfalse;
    }

	return gtrue;
}

#if 0 
    //- 1st draft, working version
static gbool PF_CreateSurroundingSet (squarelist_t *closed, squarelist_t *curset, square_t *sp)
{
    int     i;
    int     ind[SURROUNDING_SET_SIZE], 
            xset[SURROUNDING_SET_SIZE], 
            yset[SURROUNDING_SET_SIZE];
	gbool	toobig = gfalse;

    // initialize sets
    xset[0] = sp->x;
    xset[1] = sp->x - 1;
    xset[2] = sp->x + 1;
    xset[3] = sp->x;

    yset[0] = sp->y + 1;
    yset[1] = sp->y; 
    yset[2] = sp->y;
    yset[3] = sp->y - 1;

    for (i = 0; i < SURROUNDING_SET_SIZE; i++)
        ind[i] = MAX_X * yset[i] + xset[i];

    // reset the currentset
    curset->reset(curset);

    // check the surrounding squares, if they are on the map
    //  and open and not a member of the closedlist, set x,y,parent
    //  add add to the current set.
    for (i = 0; i < SURROUNDING_SET_SIZE; i++) 
    {
        if ((INBOUNDS(ind[i], gameboard.index)) &&
            (gameboard.index[ind[i]]->type == ST_OPEN) &&
			!closed->peek(closed, xset[i], yset[i], sp)) 
		{
            //!openset->peek( openset, xset[i], yset[i], sp )) {
            if (!PF_AddSquareToList ( curset, xset[i], yset[i], sp ))
				return gfalse;
        }
    }
	return gtrue;
}
#endif


#if 0
// - diagonal version (8 surrounding squares)
void PF_CreateCurrentSet_w_diagonal (square_t *sp)
{
    int ind[8], i;
    int xset[8], yset[8];
    int x = sp->x;
    int y = sp->y;

    ind[0] = MAX_X*(y+1)+x-1;
    ind[1] = MAX_X*(y+1)+x  ;
    ind[2] = MAX_X*(y+1)+x+1;
    ind[3] = MAX_X*y+x-1;
    ind[4] = MAX_X*y+x+1;
    ind[5] = MAX_X*(y-1)+x-1;
    ind[6] = MAX_X*(y-1)+x  ;
    ind[7] = MAX_X*(y-1)+x+1;
    xset[0] = x-1;
    xset[1] = x;
    xset[2] = x+1;
    xset[3] = x-1;
    xset[4] = x+1;
    xset[5] = x-1;
    xset[6] = x;
    xset[7] = x+1;
    yset[0] = yset[1] = yset[2] = y+1;
    yset[3] = yset[4] = y;
    yset[5] = yset[6] = yset[7] = y-1;

    currentset->reset(currentset);

    for (i = 0; i < 8; i++) 
    {
        if ((INBOUNDS(ind[i], gameboard.index)) &&
            (gameboard.index[ind[i]]->type == ST_OPEN)) 
        {
            // not in closed set
            if (!closedset->peek(closedset, xset[i], yset[i], sp))
                PF_AddSquareToList ( currentset, xset[i], yset[i], sp );
        }
    }
}
#endif


#define ABS(x) (((x)< 0) ? -(x) : (x))

/*
====================
 PF_BuildPath
 - called by thinker to build a path to destination (gx,gy)
====================
*/
gbool PF_BuildPath ( int x, int y, int gx, int gy, int length, thinkerpath_t *thinkerpath )
{
	/* allocate the squarepool within the thinkerpath, and then connect the squarepool
	to the thinkerpath, and then, use the separate pool for each thinkerpath passed in.
	Now I am haveing a problem that I didn't forsee.  There are 160 squares allocated and 
	stored in the one squarepool , but there are n entities that each store their own 
	path.  

	or wait, couldn't I just reset the lists after this function exits, so that the 
	160 squares are all there waiting in the pool initialized.
	
	*/
    square_t *csp;
    square_t *holdnext;
    square_t *test;
    square_t *selected;
	square_t *setarray[160];
    gbool currentSetEmpty = gfalse;
    gbool goalSquareFound = gfalse;
	gbool openListEmpty = gfalse;
    int i, size;

    // reset these lists
    openlist->reset(openlist);
    closedlist->reset(closedlist);

    // mark start square as selected and add to the openlist
    selected = squarepool->getone(squarepool);
    selected->x = x;
    selected->y = y;
    selected->tag = START_TAG;
    selected->G = 0;
    selected->F = selected->H = 10 * (ABS(x - gx) + ABS(y - gy));
    openlist->insert (openlist, selected);

    for (;;)
    {
        // popLowest returns lowest totalpath (F)
        selected = openlist->popLowest (openlist);

		if (!selected) {
			openListEmpty = gtrue;
			goto loop_exit;
		}

        // insert selected into closedlist
        closedlist->insert(closedlist, selected);

        if (selected->x == gx && selected->y == gy) {
            selected->tag |= GOAL_TAG;
            goalSquareFound = gtrue;
            goto loop_exit;
        }

        // create a surrounding set in 'currentset' of squares around the 
		//  selected square.  need to make sure none of the surrounding squares 
		//  are already in the 'closedlist'.  
        if (!PF_CreateSurroundingSet (closedlist, currentset, selected))
			goto return_fail;
        
        if (currentset->list == NULL || !currentset->size) {
			continue;
        }

        // compute path weights F, G, H for currentset
        currentset->computeWeights(currentset, gx, gy);
        


        //
        // For Each Square in Currentset, add to openlist, or re-compute path
        // 



		//1 - get array of pointers to what is in set
		size = currentset->getSetArray(currentset, setarray);

		//2 - just loop through those pointers (the currentset changes so we can't
		//     trust that to check as a loop condition every time.
        for ( i = 0 ; i < size; i++ )
        {
			if (!setarray[i])
				break;

			csp = setarray[i];

            // this square of currentset is already in the openlist
            if ((test = openlist->peekp(openlist, csp)) == csp)
            {
                // if path of this square found in current square is
                //  better (in G) than one stored in openlist, change
                //  the parent of the one in open list and re-compute
                //  path weights (F & G)
                if (csp->G < test->G)
                {
                    test->parent = selected;
                    test->G = test->parent->G + 10;
                    test->F = test->G + test->H;
                }

                // TODO: (if keeping openlist sorted by F, might
                //        need to re-sort here)

            }
            // not in openlist, go ahead and add it
            else
            {
				currentset->pop(currentset, csp);
                openlist->insert(openlist, csp);
            }
        } 
		
        
    } // inf loop

loop_exit:

    if (!openListEmpty && !goalSquareFound)
        goto return_fail;

    // find GOAL_TAG square
    csp = closedlist->list;
    do {
        if (csp->tag & GOAL_TAG)
            break;
        csp = csp->next;

    } while (csp != closedlist->list);
    
    // didn't locate goal 
    // TODO: should put best guess path here
    if (!(csp->tag & GOAL_TAG))
        goto return_fail;
    
    // gen list for return
    // note: order will be in reverse, with (list->prev->tag | START_TAG)
    pathlist->reset(pathlist);
    do {

        // square is both start and goal
        if ( csp->tag & START_TAG && csp->tag & GOAL_TAG ) {
            thinkerpath->length = 1;
            thinkerpath->msecGenerated = I_CurrentMillisecond();
            thinkerpath->maxSize = SQUAREPOOLSIZE;
            thinkerpath->path[0][0] = csp->x;
            thinkerpath->path[0][1] = csp->y;
            thinkerpath->currentIndex = 0;
			thinkerpath->tag[0] = (TP_START|TP_GOAL);
			thinkerpath->index[0] = 0;
            return gtrue;
        }

        pathlist->insert(pathlist, csp);
        csp = csp->parent;

		if ( csp->tag & START_TAG ) {
			pathlist->insert(pathlist, csp);
		}

    } while (!(csp->tag & START_TAG) && csp->parent);

    // re-sort pathlist in reverse into thinkerpath
    i = 0;
    csp = pathlist->list;

    if (!(csp->tag & START_TAG ))
        goto return_fail;

    do {

        thinkerpath->path[i][0] = csp->x;
        thinkerpath->path[i][1] = csp->y;
        thinkerpath->index[i] = i;
        thinkerpath->tag[i] = csp->tag;

		if (csp->tag & GOAL_TAG) 
            break;

        ++i;
        csp = csp->next;

    } while (csp != pathlist->list);

	thinkerpath->index[++i] = 0;
	thinkerpath->path[i][0] = -1;
	thinkerpath->path[i][1] = -1;
    thinkerpath->length = i;
    thinkerpath->maxSize = SQUAREPOOLSIZE;
    thinkerpath->msecGenerated = I_CurrentMillisecond();

	squarepool->reset(squarepool);
	openlist->quickReset(openlist);
	closedlist->quickReset(closedlist);
	currentset->quickReset(currentset);
	pathlist->quickReset(pathlist);
    return gtrue;

return_fail:
	thinkerpath->length = 0;
	squarepool->reset(squarepool);
	openlist->quickReset(openlist);
	closedlist->quickReset(closedlist);
	currentset->quickReset(currentset);
	pathlist->quickReset(pathlist);
	return gfalse;
}




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///
///    squarelist, squarepool - member function definitions
///     private, static
///
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/*
==================== 
 peek & peekp
 - determine membership in set, returns NULL if not a member,
   otherwise pointer. 
 - doesn't alter set
==================== 
*/
// search by pointer
static square_t * SquareList_peekp (squarelist_t *sl, square_t *sp)
{
    square_t *i = sl->list;

	if (!i || !sl->size)
		return NULL;

    do {
        if (i == sp)
            return i;
        i = i->next;
    } while ( i != sl->list );

    return NULL;
}

// search by coordinates, returning pointer if located
static square_t * SquareList_peek (squarelist_t *sl, int x, int y, square_t *parent)
{
    square_t *sp = sl->list;

	if (!sp || !sl->size)
		return NULL;

    do
    {
		//if (x == sp->x && y == sp->y && parent == sp->parent) 
        if (x == sp->x && y == sp->y)
            return sp;
        
        sp = sp->next;
    } while (sp != sl->list);

    return NULL;
}


/*
==================== 
 computeWeights
 - compute path weights of a whole list (to destination (gx, gy))
==================== 
*/
static void SquareList_computeWeights (squarelist_t *sl, int gx, int gy)
{
    square_t *sp;
    sp = sl->list;
	if (!sp || !sl->size)
		return;
    do 
    {
        // parent must be right next to it, the cost to 
        //  travel to it is 10 more than the parent's cost
        sp->G = sp->parent->G + 10;
        // manhatten heuristic
        sp->H = 10 * ( ABS(gx - sp->x) + ABS(gy - sp->y) );
        sp->F = sp->G + sp->H;
        sp = sp->next;
    } 
    while ( sp != sl->list );
}


/*
==================== 
 popLowest
 - pops member with the lowest total path
==================== 
*/
static square_t * SquareList_popLowest (squarelist_t *sl) 
{
    int low = 2000000000;
    square_t *sp = sl->list;
    square_t *found = NULL;

	if (!sp || !sl->size)
		return NULL;

    do {
        if (sp->F < low)
        {
            low = sp->F;
            found = sp;
        }
        sp = sp->next;
    } while ( sp != sl->list );

    if (found)
        return sl->pop(sl, found);

    return NULL;
}

/*
==================== 
 popLowestR
 - pops member with the lowest total path
   searches list in reverse of popLowest
==================== 
*/
static square_t * SquareList_popLowestR (squarelist_t *sl) 
{
    int low = 2000000000;
    square_t *sp = sl->list->prev;
    square_t *found = NULL;

	if (!sp || !sl->size)
		return NULL;

    do {
        if (sp->F < low)
        {
            low = sp->F;
            found = sp;
        }
        sp = sp->prev;
    } while ( sp != sl->list->prev );

    if (found)
        return sl->pop(sl, found);

    return NULL;
}

/*
==================== 
 SquareList_pop

 - square ptr should be in the pool of the list passed in
   if found, remove it from the list and return it
==================== 
 */
static square_t * SquareList_pop (squarelist_t *sl, square_t *sq)
{
    square_t *current = sl->list;

    if (!current || !sl->size)
        return NULL;

    // square is first one in list
    if (sq == sl->list)
    {
        // more than one in list
        if (sl->list->next != sl->list) {
            sl->list->prev->next = sl->list->next;
            sl->list->next->prev = sl->list->prev;
            sl->list = sl->list->next;
        // just one
        } else {
            sl->list = NULL;
        }
        --sl->size; 

        //sq->returnToPool(sq);

        return sq;
    }

    current = sl->list->next;
    
    while ( current != sl->list )
    {
        if ( current == sq )
            break;
        current = current->next;
    }

    // made it through the list without finding it
    if (current != sq) 
        return NULL;

    current->prev->next = current->next;
    current->next->prev = current->prev;
    // assume sq is not first on list since we checked for it above

    --sl->size;

    //sq->returnToPool(sq);
    return sq;
}

/*
====================
 reset()
 - pop all members belonging to this squarelist
   and return to pool
====================
*/
static void SquareList_reset (squarelist_t *sl)
{
    square_t *sp = sl->list;

    if (!sp || !sl->size)
        return; 

    do {
        sp->returnToPool(sp);
        sp = sp->next;
    } while ( sp != sl->list );

    sl->size = 0;
}


/*
==================== 
 SquareList_insert

 - returns null on failure, pointer on successful insert
====================
*/
static square_t * SquareList_insert (squarelist_t *sl, square_t *sq) 
{
    square_t *p = sl->list;
    square_t *new;

    // list full
    if (sl->size >= SQUAREPOOLSIZE)
        return NULL;

    // list empty
    if (sl->list == NULL || sl->size <= 0) {
        sq->next = sq->prev = sl->list = sq;
        ++sl->size;
        return sq;
    } 

    // check if square already found in list
    do
    {
        if (sq == p)
            return NULL;
        p = p->next;
    } while (p != sl->list);

	// list guaranteed not empty
    sq->next = sl->list;
    sq->prev = sl->list->prev;
    sl->list->prev->next = sq;
    sl->list->prev = sq;
    sl->list = sq;
    ++sl->size;
    return sq;
}

/*
====================
 SquareList_quickReset
====================
*/
static void SquareList_quickReset(squarelist_t *sl)
{
	sl->list = NULL;
	sl->size = 0;
}

/*
====================
 getone
 - pops one from the pool, sets as used and returns it
====================
*/
static square_t * SquarePool_getone (struct squarepool_s *sp)
{
    square_t *new_p;

	// dont let the last one from the pool get freed
	// futzes up the pointers
    if (sp->numfree < 2 || sp->available == NULL)
        return NULL;

    --sp->numfree;

    new_p = sp->available;

    // unlink from available list
    // only 1 left
    if (new_p == new_p->pnext) {
        sp->available = NULL;
    // more than 1 left
    } else {
        new_p->pnext->pprev = new_p->pprev; 
        new_p->pprev->pnext = new_p->pnext;
        sp->available = new_p->pnext;
    }

    // attach to used list
    if (!sp->used) {
        sp->used = new_p->pprev = new_p->pnext = new_p;
    } else {
        new_p->pnext = sp->used;
        new_p->pprev = sp->used->pprev;
        sp->used->pprev->pnext = new_p;
        sp->used->pprev = new_p;
        sp->used = new_p;
    }

    return new_p;
}

/*
====================
 return_p & return_i
 - return a square back to the available pool
====================
*/
static void SquarePool_return_p (struct squarepool_s *sp, square_t *sq)
{
    square_t *rov;
    gbool notfound = gtrue;

	// none returnable
	if (!sp->used)
		return;

    // only one in use
    if (sq == sp->used && sq == sq->pnext) 
    {
        sq->pnext = sp->available;
        sq->pprev = sp->available->pprev;
        sp->used = NULL;

        sp->available->pprev->pnext = sq;
        sp->available->pprev = sq;
        sp->available = sq;
        ++sp->numfree;
        return;
    }

	
	// make sure we can find square in the used list at all
	for (rov = sp->used->pnext; rov && rov != sq && notfound; rov = rov->pnext) {
		if (rov == sq)
			notfound = gfalse;
	}
    if (notfound)
        return;

    // must be found in used, go ahead and detach
    sq->pnext->pprev = sq->pprev;
    sq->pprev->pnext = sq->pnext;
    // ?
    if (sp->used == sq)
        sp->used = sq->pnext;

    // attach at head of available list
    // available list empty
    if (!sp->available) {
        sp->available = sq->pprev = sq->pnext = sq;
    } else {
        sq->pnext = sp->available;
        sq->pprev = sp->available->pprev;
        sp->available->pprev->pnext = sq;
        sp->available->pprev = sq;
        sp->available = sq;
    }

    ++sp->numfree;
}

static void Square_returnToPool (square_t *sq)
{
    ((squarepool_t *)sq->pool)->return_p (sq->pool, sq);
}

static void SquarePool_return_i (struct squarepool_s *sp, int x, int y)
{
    square_t *r = sp->used;
    if (r == NULL)
        return;

    for (r = sp->used->pnext; r != sp->used; r = r->pnext)
    {
        if (r->x == x && r->y == y) {
            sp->return_p(sp, r);
            return;
        }
    }
}

/*
==================== 
 reset
==================== 
*/
static void SquarePool_reset (struct squarepool_s *sp)
{
    square_t *aprev_tmp;

    sp->rover = 0;
    sp->numfree = sp->size;

    // nothing to do
    if (sp->used == NULL)
        return;

    sp->used->pprev->pnext = sp->available;
    sp->available->pprev->pnext = sp->used;

    aprev_tmp = sp->available->pprev;
    sp->available->pprev = sp->used->pprev;
    sp->used->pprev = aprev_tmp;

    sp->available = sp->used;
    sp->used = NULL;
}

static int SquareList_getSize (squarelist_t *s)
{
    return s->size;
}

static int SquareList_getSetArray (squarelist_t *sl, square_t **setarray)
{
	square_t *sp = sl->list;

	if (!sp)
		return 0;

	do {
		*setarray++ = sp;
		sp = sp->next;
	} while (sp != sl->list);

	*setarray = NULL;

	return sl->size;
}

////////////////////////////////////////////////////
// end member functions
////////////////////////////////////////////////////


/*
==================== 
 PF_InitSquareList
==================== 
*/
void PF_InitSquareList (squarelist_t **sl)
{
    static int i = 0;
    *sl = V_malloc(sizeof(squarelist_t), PU_STATIC, 0);
    (*sl)->size = 0;
    (*sl)->list = NULL;
    (*sl)->insert = SquareList_insert;
    (*sl)->pop = SquareList_pop;
    (*sl)->reset = SquareList_reset;
    if ((i = ((i+1) % 2)))
        (*sl)->popLowest = SquareList_popLowest;
    else
        (*sl)->popLowest = SquareList_popLowestR;
    (*sl)->computeWeights = SquareList_computeWeights;
    (*sl)->peekp = SquareList_peekp;
    (*sl)->peek = SquareList_peek;
    (*sl)->getSize = SquareList_getSize;
	(*sl)->getSetArray = SquareList_getSetArray;
	(*sl)->quickReset = SquareList_quickReset;
}


/*
====================
 PF_InitPathFind
 - global init: init the pool, pool members, and init 4 squarelists
====================
*/
void PF_InitPathFind (void)
{
    int i;
    square_t *head, *insert;

    squarepool = V_malloc(sizeof(squarepool_t), PU_STATIC, 0);
    squarepool->pool = V_malloc(sizeof(square_t)*SQUAREPOOLSIZE, PU_STATIC, 0);
    squarepool->rover = 0;
    squarepool->size = SQUAREPOOLSIZE;
    squarepool->numfree = SQUAREPOOLSIZE;
    squarepool->used = NULL;

    head = squarepool->available = &squarepool->pool[0];

    head->pprev = head;
    head->pnext = head;
    head->parent = NULL;
    head->tag = UNUSED_TAG;
    head->pool = (squarepool_t *) squarepool;
    head->returnToPool = Square_returnToPool;
    head->next = head->prev = NULL;
    head->x = -1;
    head->y = -1;
    head->F = head->G = head->H = 0;

    for (i = 1; i < SQUAREPOOLSIZE; i++) 
    {
        insert = &squarepool->pool[i];

        insert->pnext = head;
        insert->pprev = head->pprev;
		head->pprev->pnext = insert;
        head->pprev = insert;
        squarepool->available = insert;

        insert->next = insert->prev = NULL;
        insert->parent = NULL;
        insert->tag = UNUSED_TAG;
        insert->pool = (squarepool_t *) squarepool;
        insert->returnToPool = Square_returnToPool;
        insert->x = -1;
        insert->y = -1;
        insert->F = insert->G = insert->H = 0;

        head = insert;
    }

    squarepool->getone = SquarePool_getone;
    squarepool->return_p = SquarePool_return_p;
    squarepool->return_i = SquarePool_return_i;
    squarepool->reset = SquarePool_reset;

    PF_InitSquareList(&openlist);
    PF_InitSquareList(&closedlist);
    PF_InitSquareList(&pathlist);
    PF_InitSquareList(&currentset);
}


