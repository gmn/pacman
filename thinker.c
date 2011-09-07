//
// thinker.c
//
#include <string.h>

#include "pacman.h"
#include "gameboard.h"
#include "virtmem.h"
#include "thinker.h"
#include "event.h"
#include "player.h"
#include "i_system.h"
#include "pathfind.h"
#include "common.h"


extern gameboard_t gameboard;
int total_thinkers = 0;

thinker_t thinkercap;

static struct eyesset_t { 
    point2_t goal;
    gbool pathset;
    gbool active;
    int timeout;
    thinker_t *tp;
    gbool reachedgoal;
    int cindex;
};
struct eyesset_t eyesset[4];
eyes_allocated[4] = {gfalse,gfalse,gfalse,gfalse};


static gbool g_quads[4] = { gfalse, gfalse, gfalse, gfalse };



#define GHOST_SPACEOUT_MAX      30
#define GHOST_SEARCHMODE_MAX    30

#define MAXNOISE 10000


#define EATEN_DELAY 4000
#define CHASE_DELAY 6000
#define DEFAULT_CHASED_AMOUNT 8000

#define GHOST_SPEED_BASE     2.60
#define GHOST_SPEED_YELLOW  (GHOST_SPEED_BASE)
#define GHOST_SPEED_PINK    (GHOST_SPEED_BASE+0.00)
#define GHOST_SPEED_CYAN    (GHOST_SPEED_BASE+0.001)
#define GHOST_SPEED_RED     (GHOST_SPEED_BASE+0.001)
#define GHOST_SPEED_CHASED  (GHOST_SPEED_BASE/2)
#define EYES_SPEED           4.00



static struct {
    char *path;
    image_t *img;
} thinkerimageset[] = { 
    
    { IMGDIR"/red_left.tga" ,       NULL } , /* 0-3 */
    { IMGDIR"/red_right.tga" ,      NULL } ,
    { IMGDIR"/red_up.tga" ,         NULL } ,
    { IMGDIR"/red_down.tga" ,       NULL } ,

    { IMGDIR"/yellow_left.tga" ,    NULL } , /* 4-7 */
    { IMGDIR"/yellow_right.tga" ,   NULL } ,
    { IMGDIR"/yellow_up.tga" ,      NULL } ,
    { IMGDIR"/yellow_down.tga" ,    NULL } ,
     
    { IMGDIR"/cyan_left.tga" ,      NULL } , /* 8-11 */
    { IMGDIR"/cyan_right.tga" ,     NULL } ,
    { IMGDIR"/cyan_up.tga" ,        NULL } ,
    { IMGDIR"/cyan_down.tga" ,      NULL } ,
     
    { IMGDIR"/pink_left.tga" ,      NULL } , /* 12-15 */
    { IMGDIR"/pink_right.tga" ,     NULL } ,
    { IMGDIR"/pink_up.tga" ,        NULL } ,
    { IMGDIR"/pink_down.tga" ,      NULL } ,


    { IMGDIR"/purple_left.tga" ,    NULL } , /* 16-19 */
    { IMGDIR"/purple_right.tga" ,   NULL } ,
    { IMGDIR"/purple_up.tga" ,      NULL } ,
    { IMGDIR"/purple_down.tga" ,    NULL } ,

    { IMGDIR"/white_left.tga" ,     NULL } , /* 20-23 */
    { IMGDIR"/white_right.tga" ,    NULL } ,
    { IMGDIR"/white_up.tga" ,       NULL } ,
    { IMGDIR"/white_down.tga" ,     NULL } ,

    { IMGDIR"/eyes_left.tga",       NULL } , /* 24-27 */
    { IMGDIR"/eyes_right.tga",      NULL } ,
    { IMGDIR"/eyes_up.tga",         NULL } ,
    { IMGDIR"/eyes_down.tga",       NULL } ,
    
};
#define THINKER_IMAGE_SET_SIZE (sizeof(thinkerimageset)/sizeof(*thinkerimageset))



static void T_LoadThinkerImageSet ( void )
{
    int i;

    // so I can read in debugger
    //int ss = THINKER_IMAGE_SET_SIZE;

    for (i = 0; i < THINKER_IMAGE_SET_SIZE; i++) 
    {
        /*
        thinkerimageset[i].img = V_malloc(sizeof(image_t), PU_STATIC, 0);
        IMG_readfileTGA( thinkerimageset[i].path, thinkerimageset[i].img ); 
        IMG_compileGLImage( thinkerimageset[i].img );
        IMG_MakeGLTexture( thinkerimageset[i].img );
        */

        IMG_compileImageFile (thinkerimageset[i].path, &thinkerimageset[i].img);

        // tile display size
		thinkerimageset[i].img->sz[0] = 18; 
        thinkerimageset[i].img->sz[1] = 18; 
    }
}



static int global_chased_timer_start;
static int global_chased_timer_end;
static gbool global_chased_mode = gfalse;
static int global_chased_mode_eaten = 0;

void T_SetCurrentAnimation (thinker_t *tp)
{
    int i = 0; 
    int now, blinkstart;

    if ( global_chased_mode && tp->chased ) 
    {
        now = I_CurrentMillisecond();
        blinkstart = global_chased_timer_end - ((DEFAULT_CHASED_AMOUNT*3)/8);

        if ( now >= blinkstart ) 
        { 
            i = 4 + (((now - blinkstart) / 200) % 2) * 4; 
            if (i > 8) i = 8;
        } 
        else 
        {
            i = 4;
        }
    }

	// set animation according to direction
	switch (tp->dirmoving) {
	case DIR_LEFT:
		tp->currentanim = 0 + i;
		break;
	case DIR_RIGHT:
		tp->currentanim = 1 + i;
		break;
	case DIR_UP:
		tp->currentanim = 2 + i;
		break;
	case DIR_DOWN:
		tp->currentanim = 3 + i;
		break;
	}
}

static void T_StoreEntityDataFromGameboard (thinker_t *tp, entity_t *ep)
{
    tp->sq[0]       = ep->x;
    tp->sq[1]       = ep->y;
    tp->pix[0]      = ep->pix[0];
    tp->pix[1]      = ep->pix[1];

    tp->id          = ep->id;
    tp->spawn[0]    = ep->spawn[0];
    tp->spawn[1]    = ep->spawn[1];

    tp->respawn[0]  = ep->spawn[0];
    tp->respawn[1]  = ep->spawn[1];

    tp->respawnpix[0] = ep->pix[0];
    tp->respawnpix[1] = ep->pix[1];
}

// resets coords for an already initialized set of thinkers
void T_ResetThinkerCoordinates (void)
{
    thinker_t *tp;

    // respawn each ghost
    tp = thinkercap.next;
    while (tp != &thinkercap) 
    {
        switch (tp->type) {
        case TT_REDGHOST:
            tp->ma[0] = tp->ma[1] = GHOST_SPEED_RED;
            goto ghostsetup_general;
        case TT_PINKGHOST:
            tp->ma[0] = tp->ma[1] = GHOST_SPEED_PINK;
            goto ghostsetup_general;
        case TT_YELLOWGHOST:
            tp->ma[0] = tp->ma[1] = GHOST_SPEED_YELLOW;
            goto ghostsetup_general;
        case TT_CYANGHOST:
            tp->ma[0] = tp->ma[1] = GHOST_SPEED_CYAN;
ghostsetup_general:
            tp->sq[0] = tp->spawn[0];
            tp->sq[1] = tp->spawn[1];
            tp->pix[0] = tp->respawnpix[0];
            tp->pix[1] = tp->respawnpix[1];
            tp->dead = gfalse;
            tp->chased = gfalse;
            tp->eaten  = gfalse;
            
            T_SetCurrentAnimation(tp);
            break;
        default: 
            break;
        }

        tp = tp->next;
    }
}


void T_AddThinker (thinker_t *thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
    ++total_thinkers;
}

animation_t *redanim_save = NULL;
animation_t *yellowanim_save = NULL;
animation_t *pinkanim_save = NULL;
animation_t *cyananim_save = NULL;

typedef struct {
    animation_t *red, *yellow, *pink, *cyan;
} animsave_t;

animsave_t animsave = { NULL, NULL, NULL, NULL };

/*
==================== 
 T_RemoveThinker
     - it will be freed when its turn comes up
==================== 
*/
void T_RemoveThinker (thinker_t* thinker)
{
    if (thinker != &thinkercap) {
        thinker->think = NULL;
        --total_thinkers;
        if (thinker->anims) {
            switch (thinker->type) {
            case TT_REDGHOST:
                animsave.red = thinker->anims;
                break;
            case TT_CYANGHOST:
                animsave.cyan = thinker->anims;
                break;
            case TT_YELLOWGHOST:
                animsave.yellow = thinker->anims;
                break;
            case TT_PINKGHOST:
                animsave.pink = thinker->anims;
                break;
            }
            thinker->anims = NULL;
        }
    }
}

extern gameboard_t gameboard;
#define INBOUNDS(x,y) ((x) >= 0 && (x) < (MAX_X*MAX_Y) && y[(x)])

void T_FineMove (direction_t dir, thinker_t *tp)
{
    switch (dir) {
    case DIR_UP:    
        tp->pix[1] += tp->ma[1];
		tp->pix[0] = 0;
        break;
    case DIR_DOWN:   
        tp->pix[1] -= tp->ma[1];
		tp->pix[0] = 0;
        break;
    case DIR_LEFT:   
        tp->pix[0] -= tp->ma[0];
		tp->pix[1] = 0;
        break;
    case DIR_RIGHT:
        tp->pix[0] += tp->ma[0];
		tp->pix[1] = 0;
        break;
    }   

#ifndef SQUARE
# define SQUARE 13
#endif

    if (tp->pix[0] > SQUARE) {
        tp->sq[0]++;
        tp->pix[0] = 0;
    } else if (tp->pix[0] < -SQUARE) {
        tp->sq[0]--;
        tp->pix[0] = 0;
    }
    if (tp->pix[1] > SQUARE) {
        tp->sq[1]++;
        tp->pix[1] = 0;
    } else if (tp->pix[1] < -SQUARE) {
        tp->sq[1]--;
        tp->pix[1] = 0;
    }
}

int T_MoveAllowed (direction_t dir, thinker_t *tp) 
{
    int x = tp->sq[0];
    int y = tp->sq[1];
    switch (dir) {
    case DIR_UP:
        if (INBOUNDS(MAX_X*(y+1)+x, gameboard.index))
            if (gameboard.index[MAX_X*(y+1)+x]->type == ST_OPEN)
                return 1;
        return 0;
    case DIR_DOWN:
        if (INBOUNDS(MAX_X*(y-1)+x, gameboard.index))
            if (gameboard.index[MAX_X*(y-1)+x]->type == ST_OPEN)
                return 1;
        return 0;
    case DIR_LEFT:
        if (INBOUNDS(MAX_X*y+x-1, gameboard.index))
            if (gameboard.index[MAX_X*y+x-1]->type == ST_OPEN)
                return 1;
        return 0;
    case DIR_RIGHT:
        if (INBOUNDS(MAX_X*y+x+1, gameboard.index))
            if (gameboard.index[MAX_X*y+x+1]->type == ST_OPEN)
                return 1;
        return 0;
    }
    return 0;
}




/*
====================
 T_IncGhostChasedDuration
 - global, used to compensate for game pauses
====================
*/
void T_IncGhostChasedDuration (int inc)
{
    global_chased_timer_end += inc;
}


/*
==================== 
 T_ResetQuads
==================== 
*/ 
void T_ResetQuads ( void )
{
    g_quads[0] = g_quads[1] = g_quads[2] = g_quads[3] = gfalse;
}


/*
==================== 
 T_StartChaseMode

  - called when a big dot is eaten
==================== 
*/
void T_StartChaseMode (void)
{
    thinker_t *tp ;
    global_chased_timer_start = I_CurrentMillisecond();
    global_chased_timer_end = global_chased_timer_start + DEFAULT_CHASED_AMOUNT;

    T_ResetQuads();

    for (tp = thinkercap.next; tp != &thinkercap; tp = tp->next )
    {
        if (tp->dead)
            continue;

        // remove all eyes from thinker list
        if (tp->type == TT_EYES) {
            tp->think = NULL;
            continue;
        }

        tp->chased = gtrue;
        tp->ma[0] = tp->ma[1] = GHOST_SPEED_CHASED;
		tp->startChasedMode = gtrue;
    }

    global_chased_mode = gtrue;
}

static void T_RegularSpeed (thinker_t *tp)
{
    switch (tp->type) {
    case TT_REDGHOST:
        tp->ma[0] = tp->ma[1] = GHOST_SPEED_RED;
        return;
    case TT_CYANGHOST:
        tp->ma[0] = tp->ma[1] = GHOST_SPEED_CYAN;
        return;
    case TT_YELLOWGHOST:
        tp->ma[0] = tp->ma[1] = GHOST_SPEED_YELLOW;
        return;
    case TT_PINKGHOST:
        tp->ma[0] = tp->ma[1] = GHOST_SPEED_PINK;
        return;
    }
}


/*
====================
 T_PickQuad()
 - picks an open g_quad at random and stores coords of 1st
   open square it finds to (xy[0], xy[1])
 - if (*which == -1), a quad is found, if not, *which is just used
====================
*/
void T_PickQuad (int *which, int *xy) 
{
    // cartesian orientation: ( ur=0, ul=1, ll=2, lr=3 )
    int i = 0, j;
    gbool notfound = gtrue;


    if (*which == -1) 
    {
        // pick first open random quadrant
        while ( i < 4 ) {
            *which=getrand(4);
            if ( *which < 4 && !g_quads[*which] )
                break;
            ++i;
        }
        if ( *which < 4 && g_quads[*which] )
            return;
        g_quads[*which] = gtrue;
    }
    

    // scan for 1st open square in that quadrant
    switch (*which) {
    case 0: // ur
        for ( j = MAX_Y-1; j >= (MAX_Y/2) && notfound; j-- ) {
            for ( i = MAX_X-1; i >= (MAX_X/2) ; i-- ) {
			    if ( INBOUNDS(MAX_X*j+i, gameboard.index) ) {
				    if ( ST_OPEN == gameboard.index[MAX_X*j+i]->type ) {
					    xy[0] = i;	
					    xy[1] = j;
					    notfound = gfalse;
					    break;
				    }
                }
            }
        }
        break;
    case 1: // ul
        for ( j = MAX_Y-1; j >= (MAX_Y/2) && notfound; j-- ) {
            for ( i = 0; i < (MAX_X/2) ; i++ ) {
			    if ( INBOUNDS(MAX_X*j+i, gameboard.index) ) {
				    if ( ST_OPEN == gameboard.index[MAX_X*j+i]->type ) {
					    xy[0] = i;	
					    xy[1] = j;
					    notfound = gfalse;
					    break;
				    }
                }
            }
        }
        break;
    case 2: // ll
        for ( j = 0; j < (MAX_Y/2) && notfound; j++ ) {
            for ( i = 0; i < (MAX_X/2) ; i++ ) {
			    if ( INBOUNDS(MAX_X*j+i, gameboard.index) ) {
				    if ( ST_OPEN == gameboard.index[MAX_X*j+i]->type ) {
					    xy[0] = i;	
					    xy[1] = j;
					    notfound = gfalse;
					    break;
				    }
                }
            }
        }
        break;
    case 3: // lr
        for ( j = 0; j < (MAX_Y/2) && notfound; j++ ) {
            for ( i = MAX_X-1; i >= (MAX_X/2) ; i-- ) {
			    if ( INBOUNDS(MAX_X*j+i, gameboard.index) ) {
				    if ( ST_OPEN == gameboard.index[MAX_X*j+i]->type ) {
					    xy[0] = i;	
					    xy[1] = j;
					    notfound = gfalse;
					    break;
				    }
                }
            }
        }
        break;
    }



}





/*
==================== 
 T_PollEatenChased
==================== 
*/
void T_PollEatenChased (void)
{
    thinker_t *tp ;
    int now = I_CurrentMillisecond();


    // poll for global chased timer
    if ( global_chased_mode ) {
        // timers up, unset the global timer, reset # eaten
        if ( now > global_chased_timer_end ) {
            global_chased_mode_eaten = 0;
            global_chased_mode = gfalse;
            T_ResetQuads ();

            for (tp = thinkercap.next; tp != &thinkercap; tp = tp->next ) {
				if (tp->type == TT_EYES) {
					tp->think = NULL;
				} else {
					tp->chased = gfalse; 
					tp->path.forceRegen = gtrue;
					T_RegularSpeed(tp);
				}
            }
        }
    }



    // poll the eaten, if eaten, check timer
    for (tp = thinkercap.next; tp != &thinkercap; tp = tp->next )
        if ( tp->eaten )
            if ( now - tp->eaten_timer > EATEN_DELAY ) {
                tp->eaten = gfalse;
                T_RegularSpeed(tp);
            }



    //
    // poll ghosts death state, do rebirths if necessary
    //
    for (tp = thinkercap.next; tp != &thinkercap; tp = tp->next )
    {
        if ( tp->dead ) {
            if ( !global_chased_mode && !tp->eaten ) {
                tp->dead = gfalse;
                tp->path.forceRegen = gtrue;
                T_RegularSpeed(tp);
                tp->sq[0] = tp->spawn[0];
                tp->sq[1] = tp->spawn[1];
                tp->chased = gfalse;
            }
        } else { 
            if ( tp->eaten ) {
                tp->dead = gtrue;
            }
        }
    }
}


int T_GetNumEaten (void)
{
    return global_chased_mode_eaten;
}



void T_EatGhost (thinker_t *tp)
{
    if (tp->eaten || !tp->chased || tp->dead)
        return;
    tp->eaten = gtrue;
    tp->dead = gtrue;
    tp->eaten_timer = I_CurrentMillisecond();
    tp->chased = gfalse;
    ++global_chased_mode_eaten;

    T_SpawnEyes(tp->sq[0], tp->sq[1], tp->spawn[0], tp->spawn[1]);
}







extern player_t pl;
extern int bigdot_starttime;

void T_GhostThink (thinker_t *tp)
{
    int i, j, tx, ty, g_q[2];
    thinkerpath_t *p;
    gbool timepath = gfalse;
	gbool squarepath = gfalse;
    gbool movemade = gfalse;
    uint dirs[] = { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN,
                    DIR_LEFT_AND_RIGHT, DIR_UP_AND_DOWN };
    uint open[] = { 0, 0, 0, 0, 0, 0 };
    static int noise;
    uint pvec[] = { 0, 0, 0, 0 };

    noise = (++noise) % MAXNOISE;

    if (tp->dead)
        return;
    
    p = &tp->path;

    // check if path needs to be generated
    if (p->usingPath && !tp->chased) 
    {
        switch (p->pathType) {
        case TP_PATH_TIME:
            if (I_CurrentMillisecond() - p->msecGenerated > p->msecTilRegen) 
            {
                timepath = gtrue;
            }
            break;
        case TP_PATH_SQUARES:
            if (p->currentIndex - p->startIndex >= p->nSqrTilRegen)
            {
                squarepath = gtrue;				
            }
            break;
        case TP_PATH_SQUARES_AND_TIME:
            if (p->currentIndex - p->startIndex >= p->nSqrTilRegen)
                squarepath = gtrue;
			else if (I_CurrentMillisecond() - p->msecGenerated > p->msecTilRegen) 
                timepath = gtrue;
            break;
        default:
			break;
        }

        if ( squarepath || timepath || p->forceRegen ) {
            // length has no-effect currently
            PF_BuildPath(tp->sq[0], tp->sq[1], pl.sq[0], pl.sq[1], 16, p);
            p->currentIndex = p->startIndex;
            p->forceRegen = gfalse;

            if (p->length == 0) 
                C_WriteLog("got no path\n");
        }
    }


    // which ways are we allowed to move?
    for (i = 0; i < 4; i++) {
        if (T_MoveAllowed(dirs[i], tp)) {
            open[i] = 1;

            if (i < 2)
                open[4] = 1;
            else
                open[5] = 1;
        }
    }

    // where is the player in relation to us
    if (pl.sq[0] < tp->sq[0])
        pvec[0] = 1;
    else if (pl.sq[0] > tp->sq[0])
        pvec[1] = 1;
    if (pl.sq[1] > tp->sq[1])
        pvec[2] = 1;
    else if (pl.sq[1] < tp->sq[1])
        pvec[3] = 1;

// if chased or spacing out, or not usingPath, change directions at
// NON INTERSECTIONS RARELY !!!
// direction changes should be made at nodes of 3+ open directions

    // plot a new course if we are starting chased mode
    //  startChasedMode called 1st time, forceRegen called successive
    //  times after that, when the goal from the first path has been
    //  reached
    if (tp->startChasedMode || (p->forceRegen && tp->chased)) 
    {
        // 1 - pick 1 out of 4 quads
        //  (heuristic: each ghost takes a quad, makes no diff where player
        //      is, this way, one is the sacraficial lamb.)
        // 2 - find an open square in that quad or next best open square
        // 3 - plot a path to that square
        
        g_q[0] = g_q[1] = -1;
        if (tp->startChasedMode) {
            i = -1;
            T_PickQuad(&i, g_q);
        } else {
            T_PickQuad(&tp->chasedQuad, g_q);
            if (--tp->chasedQuad < 0) 
                tp->chasedQuad = 3;
        }

        if (g_q[0] != -1 && g_q[1] != -1) {
            PF_BuildPath(tp->sq[0], tp->sq[1], g_q[0], g_q[1], 16, p);
            p->currentIndex = p->startIndex;
            p->forceRegen = gfalse;
            tp->startChasedMode = gfalse;
        }
    }


    // make a chased move 
    if (tp->chased) {

    }


    /*
// uncomment to turn off
tp->spaceout = 0;

    // check if ghost is spacing out
    if (tp->spaceout > 0) {
        --tp->spaceout;

        for (i = 0; i < 4 && !movemade; i++) {

            // if the direction we are going in is open, just keep going in it
            if (tp->dirmoving == dirs[i] && open[i]) {
                T_FineMove (dirs[i], tp);
                movemade = gtrue;
            }
        }

        if (!movemade) {
            // pick a new random direction, whateva, forget about it
			for (i=0; i < 4 && !movemade; i++)
			{
                j = (noise+i)%4;
				if (open[j])
				{
					tp->dirmoving = dirs[j];
		            T_FineMove (tp->dirmoving, tp);
					movemade = gtrue;
				}
			}
        }
    }

// debug: uncomment to turn on
tp->searchmode = 1;

    // Check if we are going to space out
    if (!movemade && !tp->spaceout && !tp->searchmode) 
    {
        if (getrand(100) >= 84) 
        { 
            tp->spaceout = 30 + getrand(GHOST_SPACEOUT_MAX);

            // go in the first open direction
            for (i=0; i < 4 ; i++)
            {
                j = (noise+i)%4;
                if (open[j]) 
                {
                    tp->dirmoving = dirs[j];
                    T_FineMove (dirs[j], tp);
                    movemade = gtrue;
                }
            }
        } 
        else if (getrand(100) < 16)
        {
            tp->searchmode = 30 + getrand(GHOST_SEARCHMODE_MAX);
        }
    }

    if (tp->searchmode > 0) --tp->searchmode;
    */



    // follow generated path
    if (!movemade && p->usingPath && p->length > 0 && p->currentIndex < p->length) 
    {
        i = p->path[p->currentIndex][0];
        j = p->path[p->currentIndex][1];
        tx = tp->sq[0];
        ty = tp->sq[1];
        if ( i < tp->sq[0] ) {
			if (open[0]) {
                tp->dirmoving = DIR_LEFT;
				T_FineMove (DIR_LEFT, tp);
				movemade = gtrue;
			}
        } else if ( i > tp->sq[0] ) {
			if (open[1]) {
                tp->dirmoving = DIR_RIGHT;
				T_FineMove (DIR_RIGHT, tp);
				movemade = gtrue;
			}
        } else if ( j < tp->sq[1] ) {
			if (open[3]) {
                tp->dirmoving = DIR_DOWN;
				T_FineMove (DIR_DOWN, tp);
				movemade = gtrue;
			}
        } else if ( j > tp->sq[1] ) {
			if (open[2]) {
                tp->dirmoving = DIR_UP;
				T_FineMove (DIR_UP, tp);
				movemade = gtrue;
			}
        }

        if (movemade) {
            if (tp->sq[0] != tx) {
                if (tp->sq[0] == p->path[p->currentIndex][0])
                    ++p->currentIndex;
            } else if (tp->sq[1] != ty) {
                if (tp->sq[1] == p->path[p->currentIndex][1])
                    ++p->currentIndex;
            }
        }
    }

    // check if end of path reached.  if so, force a new one
    if ( p->currentIndex >= p->length )
        p->forceRegen = gtrue;


    // if the player is roughly
    //  in the direction we are moving in just keep going 
    if (!movemade) {
        switch (tp->dirmoving) {
        case DIR_LEFT:
            if (open[0] && pvec[0]) {
                T_FineMove (tp->dirmoving, tp);
                movemade = gtrue;
            }
            break;
        case DIR_RIGHT:
            if (open[1] && pvec[1]) {
                T_FineMove (tp->dirmoving, tp);
                movemade = gtrue;
            }
            break;
        case DIR_UP:
            if (open[2] && pvec[2]) {
                T_FineMove (tp->dirmoving, tp);
                movemade = gtrue;
            }
            break;
        case DIR_DOWN:
            if (open[3] && pvec[3]) {
                T_FineMove (tp->dirmoving, tp);
                movemade = gtrue;
            }
            break;
        }
    }




    // look for another direction
    if (!movemade) {
        for (i=0; i < 4 && !movemade; i++)
        {
            if (open[i] && pvec[i])
            {

                tp->dirmoving = dirs[i];
                T_FineMove (dirs[i], tp);
                movemade = gtrue;
            }
        }
    }


    // take anything you can get
    if (!movemade) {
        for (i=0; i < 4 && !movemade; i++)
        {
            if (open[i])
            {
                tp->dirmoving = dirs[i];
                T_FineMove (dirs[i], tp);
            }
        }
    }

    T_SetCurrentAnimation (tp);

}



void T_FruitThink (thinker_t *tp)
{
    // silencio compiler
//    tp->needsdrawn ^= tp->eaten; 
//    tp->eaten ^= tp->needsdrawn; 
//    tp->needsdrawn ^= tp->eaten; 
}


void T_EyesThink (thinker_t *tp)
{
    thinkerpath_t *p;
    struct eyesset_t *e;
    int i,j,tx,ty;
    gbool movemade = gfalse;
    uint dirs[] = { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN,
                    DIR_LEFT_AND_RIGHT, DIR_UP_AND_DOWN };
    uint open[] = { 0, 0, 0, 0, 0, 0 };
    uint cwise[] = { DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_DOWN };
    int next;

    // eyes are in play if this has been called
    
    if (!tp || !tp->owner)
        return;   

    // which ways are we allowed to move?
    for (i = 0; i < 4; i++) {
        if (T_MoveAllowed(dirs[i], tp)) {
            open[i] = 1;

            if (i < 2)
                open[4] = 1;
            else
                open[5] = 1;
        }
    }

	p = &tp->path;
    e = tp->owner;

    //
    // 1. check if the path has been built, if not, build it
    //
    if (!((struct eyesset_t *)tp->owner)->pathset)
    {

        PF_BuildPath(tp->sq[0], tp->sq[1], e->goal[0], e->goal[1], 16, p);
        p->currentIndex = p->startIndex;
        p->forceRegen = gfalse;
        p->usingPath = gtrue;
        if (p->length == 0) 
            C_WriteLog("got no path\n");
        else
            e->pathset = gtrue;
    }

    //
    // 2. check if we are at the end of the path:
    //
    if (!e->reachedgoal && (p->currentIndex >= p->length || (tp->sq[0] == p->path[p->currentIndex][0] && tp->sq[1] == p->path[p->currentIndex][1]))) {
        p->usingPath = gfalse;
        e->reachedgoal = gtrue;
        e->cindex = 0;
    }

    
    //
    // 3. if not, follow the path
    //
    if (p->usingPath && p->length > 0 && p->currentIndex < p->length) 
    {
        i = p->path[p->currentIndex][0];
        j = p->path[p->currentIndex][1];
        tx = tp->sq[0];
        ty = tp->sq[1];
        if ( i < tp->sq[0] ) {
			if (open[0]) {
                tp->dirmoving = DIR_LEFT;
				T_FineMove (DIR_LEFT, tp);
				movemade = gtrue;
			}
        } else if ( i > tp->sq[0] ) {
			if (open[1]) {
                tp->dirmoving = DIR_RIGHT;
				T_FineMove (DIR_RIGHT, tp);
				movemade = gtrue;
			}
        } else if ( j < tp->sq[1] ) {
			if (open[3]) {
                tp->dirmoving = DIR_DOWN;
				T_FineMove (DIR_DOWN, tp);
				movemade = gtrue;
			}
        } else if ( j > tp->sq[1] ) {
			if (open[2]) {
                tp->dirmoving = DIR_UP;
				T_FineMove (DIR_UP, tp);
				movemade = gtrue;
			}
        }

        if (movemade) {
            if (tp->sq[0] != tx) {
                if (tp->sq[0] == p->path[p->currentIndex][0])
                    ++p->currentIndex;
            } else if (tp->sq[1] != ty) {
                if (tp->sq[1] == p->path[p->currentIndex][1])
                    ++p->currentIndex;
            }
        }
    }



    //
    // 4. if so, for now just stop in place
    // ... (series of points to pace between?)
    if (e->reachedgoal) 
    {
        // attempt to travel in a clockwise box/circle
        
        // on lvl 3 just go left-right
        if (pl.levnum%6 == 5 || pl.levnum%6 == 0 ) 
            next = (e->cindex+2)%4;
        else
            next = (e->cindex+1)%4;


        switch (cwise[e->cindex]) {
        case DIR_LEFT:
            if (open[0]) {
                tp->dirmoving = DIR_LEFT;
				T_FineMove (DIR_LEFT, tp);
				movemade = gtrue;
            } else {
                e->cindex = next;
            }
            break;
        case DIR_UP:
            if (open[2]) {
                tp->dirmoving = DIR_UP;
				T_FineMove (DIR_UP, tp);
				movemade = gtrue;
            } else
                e->cindex = next;
            break;
        case DIR_RIGHT:
            if (open[1]) {
                tp->dirmoving = DIR_RIGHT;
				T_FineMove (DIR_RIGHT, tp);
				movemade = gtrue;
            } else
                e->cindex = next;
            break;
        case DIR_DOWN:
            if (open[3]) {
                tp->dirmoving = DIR_DOWN;
				T_FineMove (DIR_DOWN, tp);
				movemade = gtrue;
            } else
                e->cindex = next;
            break;
        }
    }


    T_SetCurrentAnimation (tp);
}



void T_GetAnimSet (animation_t **ap, int num)
{
    int i;
    animation_t adef = __animation_default;

    *ap = V_malloc ( sizeof(animation_t) * num, PU_STATIC, 0 );

    for ( i = 0; i < num; i++ ) 
        (*ap)[i] = adef;
}

thinker_t * T_AllocNewThinker (void)
{
    thinker_t *tp;
    thinker_t tdef = __thinker_default;
    tp = V_malloc ( sizeof(thinker_t), PU_STATIC, 0 );
    *tp = tdef;
    return tp;
}


void T_ResetThinkerDirections (void)
{
    thinker_t *tp;
    for (tp = thinkercap.next; tp != &thinkercap; tp = tp->next )
    {
        switch (tp->type) {
        case TT_REDGHOST:
            tp->dirmoving = DIR_LEFT;
            break;
        case TT_PINKGHOST:
        case TT_YELLOWGHOST:
        case TT_CYANGHOST:
        default:
            tp->dirmoving = DIR_UP;
            break;
        }
    }
}

/* note to self: ghosts and this kind of wily data that are 
 *  obviously sets, but not so obviously sequential should be
 *  a hash lookup that ultimately resolves to an array index.
 *  the speed loss would be negligable, and the code much cleaner.
 *  an all purpose hashtype would just make sense.  w/ C++ you could
 *  redefine the operator[]
 */

animation_t * T_GetPreBuilt (int type)
{
    animation_t *ret;
    switch (type) {
    case GC_RED:
        if (!animsave.red)
            sys_error_fatal("no red anim. line %d", __LINE__);
        ret = animsave.red;

        break;
    case GC_PINK:
        if (!animsave.pink)
            sys_error_fatal("no pink anim. line %d", __LINE__);
        ret = animsave.pink;

        break;
    case GC_YELLOW:
        if (!animsave.yellow)
            sys_error_fatal("no yellow anim. line %d", __LINE__);
        ret = animsave.yellow;
        
        break;
    case GC_CYAN:
        if (!animsave.cyan)
            sys_error_fatal("no cyan anim. line %d", __LINE__);
        ret = animsave.cyan;

        break;
    }
    return ret;
}



void T_RebuildThinkerData (void) 
{
    int i = 0;

    thinker_t *tp;

    for ( i = 0; i < gameboard.numentities; i++ )
    {
        switch (gameboard.entities[i].type) {
        case ME_GHOSTSPAWN:

            tp = T_AllocNewThinker();
            tp->anims = T_GetPreBuilt (gameboard.entities[i].ghostcolor);

            switch (gameboard.entities[i].ghostcolor) {
            case GC_RED:
                tp->type = TT_REDGHOST;
                tp->dirmoving = DIR_LEFT;
                break;
            case GC_YELLOW:
                tp->type = TT_YELLOWGHOST;
                tp->dirmoving = DIR_UP;
                break;
            case GC_PINK:
                tp->type = TT_PINKGHOST;
                tp->dirmoving = DIR_UP;
                break;
            case GC_CYAN:
                tp->type = TT_CYANGHOST;
                tp->dirmoving = DIR_UP;
                break;
            }

            T_RegularSpeed(tp);

            tp->think = T_GhostThink;

            T_StoreEntityDataFromGameboard (tp, &gameboard.entities[i]);

            T_InitThinkerPath(tp);

            T_AddThinker(tp);

            break;
        case ME_FRUITSPAWN:
            break;
        }
    }
}


void T_RebuildThinkers (void)
{
    thinker_t *tp;
    tp = thinkercap.next;
    while (tp != &thinkercap) {
        T_RemoveThinker (tp);
        tp = tp->next;
    }

    // re-allocs a thinker, but re-uses standard thinker graphic
    T_RebuildThinkerData ();
}


extern gbool punishment_mode;

void T_ResetThinkers (void)
{

    // odd numbered board past first two, 
    //  remove thinkers and reload them
    //  otherwise just reset them
    if (punishment_mode || (pl.levnum > 2 && (pl.levnum % 2))) 
    {
        T_RebuildThinkers ();
    } else {
        T_ResetThinkerCoordinates();
        //T_ResetDraws();
        T_ResetThinkerDirections();
    }
}


/*
==================== 
 T_InitThinkerPath
==================== 
*/ 
void T_InitThinkerPath(thinker_t *tp)
{
    thinkerpath_t *p = &tp->path;
    int regenBase = 4000, i;
    static int offset = 0;
	int offsetamt = 200;

    p->length = 0;
    p->msecGenerated = 0;
    p->maxSize = SQUAREPOOLSIZE;
    p->pathType = TP_PATH_SQUARES_AND_TIME;

    memset(p->path, 0, SQUAREPOOLSIZE * 2 * sizeof (p->path[0][0]));

    // stagger path regen durations
    p->msecTilRegen = regenBase + (offset * offsetamt); 
    offset = (offset+1) % 4;

	switch (tp->type) {
	case TT_REDGHOST:
		p->nSqrTilRegen = 20;  
		break;
	case TT_YELLOWGHOST:
		p->nSqrTilRegen = 24;
		break;
	case TT_PINKGHOST:
	    p->nSqrTilRegen = 28;
		break;
	case TT_CYANGHOST:
		p->nSqrTilRegen = 32;
		break;
    case TT_EYES:
        p->nSqrTilRegen = 0;
        break;
	}

    p->currentIndex = 0;
    p->startIndex = 1;
    // force a regen the first time
    p->forceRegen = gtrue;

	for (i = 0; i < SQUAREPOOLSIZE; i++) {
	    // type of square
		p->tag[i] = TP_NONE;
		// number in path.  0 is start, 1 next square and p->length-1 is goal
		p->index[i] = -1;
	}
    p->usingPath = gtrue;
	p->owner = (thinker_t *) tp;
}


/*
==================== 
 T_InitThinkerData
==================== 
*/
void T_InitThinkerData (void)
{
    int i = 0;

    thinker_t *tp;

    for ( i = 0; i < gameboard.numentities; i++ )
    {
        switch (gameboard.entities[i].type) {
        case ME_GHOSTSPAWN:

            tp = T_AllocNewThinker();
            T_GetAnimSet (&tp->anims, TOTAL_GHOST_ANIMATIONS);
            
            switch (gameboard.entities[i].ghostcolor) {
            case GC_RED:
                tp->type = TT_REDGHOST;
                tp->dirmoving = DIR_LEFT;
                break;
            case GC_YELLOW:
                tp->type = TT_YELLOWGHOST;
                tp->dirmoving = DIR_UP;
                break;
            case GC_PINK:
                tp->type = TT_PINKGHOST;
                tp->dirmoving = DIR_UP;
                break;
            case GC_CYAN:
                tp->type = TT_CYANGHOST;
                tp->dirmoving = DIR_UP;
                break;
            }

            T_RegularSpeed(tp);

            tp->think = T_GhostThink;

            T_StoreEntityDataFromGameboard (tp, &gameboard.entities[i]);

            T_InitThinkerPath(tp);

            T_AddThinker(tp);

            break;
        case ME_FRUITSPAWN:
            break;
        }
    }
}


void T_ReadGhostImageData (thinker_t *tp)
{
    int i;
    int offset = -1;

    switch (tp->type) {
    case TT_REDGHOST:    offset = 0; break;
    case TT_YELLOWGHOST: offset = 4; break;
    case TT_CYANGHOST:  offset =  8; break;
    case TT_PINKGHOST:  offset = 12; break;
    }

    if (offset == -1) {
        sys_error_warn("invalid ghost type");
        return;
    }

    for (i = 0; i < 4; i++) {
        // native color
        tp->anims[i].frames[0] = thinkerimageset[offset+i].img;
        // purple
        tp->anims[i+4].frames[0] = thinkerimageset[i+16].img;
        // white
        tp->anims[i+8].frames[0] = thinkerimageset[i+20].img;
    }
}



static void T_ReadEyesImageData()
{
    int i,j;
    thinker_t *t;

    for (i = 0; i < 4; i++)
    {
        t = eyesset[i].tp;
        T_GetAnimSet(&t->anims, 4);
		
        for (j = 0; j < 4; j++) {
			t->anims[j].frame = 0;
            t->anims[j].frames[0] = thinkerimageset[24+j].img;
        }
    }
}


/*
==================== 
 T_InitThinkerGraphics
 - called from renderer.  loads image data from file and attaches
   it to thinkers
==================== 
*/
void T_InitThinkerGraphics (void)
{
    thinker_t *current;


    // reads all images from file data and processes to gl
    T_LoadThinkerImageSet();


    // attaches ghost, fruit graphic
    current = thinkercap.next;
    while (current != &thinkercap)
    {
		switch (current->type) {
        case TT_REDGHOST:
        case TT_PINKGHOST:
        case TT_YELLOWGHOST:
        case TT_CYANGHOST:
            T_ReadGhostImageData (current);
            break;
        case TT_CHERIES:
		case TT_PEACHES:
            break;
        case TT_EYES:
            break;
        }

        current = current->next;
    }

    // attaches eyes images
    T_ReadEyesImageData(); 
}


/*
==================== 
 T_InitEyes
 - eyes are stored in a local set, their thinker is added to and removed 
    from the thinker list as needed.
==================== 
*/
static void T_InitEyes( void )
{
    int i;
    thinker_t *t;
    for (i = 0; i < 4; i++)
    {
        eyesset[i].goal[0] = eyesset[i].goal[1] = -1;
        eyesset[i].pathset = gfalse;
        eyesset[i].active = gfalse;
        eyesset[i].timeout = 100;
        eyesset[i].reachedgoal = gfalse;

        if (!eyes_allocated[i]) {
            eyesset[i].tp = V_malloc(sizeof(*t), PU_STATIC, 0);
            memset(eyesset[i].tp,0,sizeof(*t));
            eyes_allocated[i] = gtrue;
        }

        t = eyesset[i].tp;
        t->sq[0] = t->sq[1] = t->pix[0] = t->pix[1] = -1;
        t->ma[0] = t->ma[1] = EYES_SPEED;
        t->type = TT_EYES;

        t->think = NULL;
        t->dirmoving = DIR_NONE;
        t->spaceout = 0;
        t->searchmode = 0;
		t->chased = gfalse;
        t->maxanim = 4;
        t->currentanim = 0;

        t->owner = (struct eyesset_t *) &eyesset[i];
        //t->anims = NULL;
        //t->next = t->prev = NULL;
        
        T_InitThinkerPath(t);
    }
}

/* 
====================  
  T_InitThinkers
  - resets thinker data for a new board, also called at startup
====================  
*/
void T_InitThinkers (void)
{
    thinkercap.prev = thinkercap.next = &thinkercap;
    T_InitThinkerData();
    T_InitEyes();
}

/*
====================
 T_SpawnEyes
 - global, initial location (x,y), goal (gx,gy)
====================
*/
gbool T_SpawnEyes (int x, int y, int gx, int gy)
{
    int i;
    thinker_t *t;

    t = eyesset[0].tp;

    for (i = 0; i < 4; i++) 
    {
        if (!eyesset[i].active && !eyesset[i].tp->think)
        {    
            eyesset[i].active = gtrue;
            eyesset[i].pathset = gfalse;
            eyesset[i].timeout = 100;
            eyesset[i].goal[0] = gx;
            eyesset[i].goal[1] = gy;
            eyesset[i].reachedgoal = gfalse;

            t = eyesset[i].tp;
            t->think = T_EyesThink;
            t->sq[0] = x;
            t->sq[1] = y;
            t->pix[0] = t->pix[1] = 0;

            T_InitThinkerPath(t);
            T_AddThinker(t);
            return gtrue;
        }
    } 

    return gfalse;
}


void T_RemoveEyes (thinker_t *t)
{
    struct eyesset_t *e = t->owner;
    if (e && e->active)
        e->active = gfalse;
}


//
// T_RunThinkers
//
void T_RunThinkers (void)
{
    thinker_t*	currentthinker;

    currentthinker = thinkercap.next;

    while (currentthinker != &thinkercap)
    {
        if ( currentthinker->think == NULL )
        {
            // time to remove it
            currentthinker->next->prev = currentthinker->prev;
            currentthinker->prev->next = currentthinker->next;

            // remove eyes special
            if (currentthinker->type != TT_EYES)
                V_free (currentthinker);
            else
                T_RemoveEyes( currentthinker );

	    }
	    else
	    {
            currentthinker->think (currentthinker);
	    }
	    currentthinker = currentthinker->next;
    }
}


/*
==================== 
 T_Ticker
==================== 
*/
void T_Ticker (void)
{
    T_RunThinkers();
    T_PollEatenChased();
}


