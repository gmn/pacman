
#ifndef __THINKER_H__
#define __THINKER_H__

#ifndef MAXANIMFRAMES
# define MAXANIMFRAMES 32
#endif

#if !defined(__GBOOL__) || !defined(__UINT__)
# include "common.h"
#endif

// should match tags in pathfind.h
typedef enum {
    TP_NONE,
    TP_START,
    TP_GOAL,
    TP_BOTH,
    TP_SQUARE,
    TP_PATH_SQUARES,
    TP_PATH_TIME,
    TP_PATH_SQUARES_AND_TIME,
    TP_PATH_NONE
} thinkerpathtype_t;

typedef enum {
    TT_NONE,
    TT_REDGHOST,
    TT_YELLOWGHOST,
    TT_CYANGHOST,
    TT_PINKGHOST,
    TT_CHERIES,
    TT_PEACHES,
    TT_EYES,
    TOTAL_THINKER_TYPES
} thinkertype_t;

typedef enum {
    AT_LEFT,
    AT_RIGHT,
    AT_UP,
    AT_DOWN,
    AT_LEFTCHASED,
    AT_RIGHTCHASED,
    AT_UPCHASED,
    AT_DOWNCHASED,
	AT_LEFTBLINK,
	AT_RIGHTBLINK,
	AT_UPBLINK,
	AT_DOWNBLINK,
    TOTAL_GHOST_ANIMATIONS
} ghostanimations_t;

#include "image.h"

// one for each type of animation 
//  ie, for ghosts chasing & being chased & up, down, left & right versions
typedef struct animation_s {
	char *name;
	// enemy mode connected to, if any
	int mode;
	int maxframes;
	int frame;
	int lastframe;
	int nextframe;
	int tpframe;	// ticks per frame
    int mspframe;   // milli-seconds per frame (use one or the other)
    int tic;
    gbool needdraw;

	image_t *frames[MAXANIMFRAMES];
} animation_t;

#define __animation_default { "", -1, 1, 0, 0, 0, 6, 0, gfalse }

// should match square pool size in pathfind.h
#ifndef SQUAREPOOLSIZE
# define SQUAREPOOLSIZE 320
#endif


typedef struct thinkerpath_s {
    // path length held
    int length;
    // time generated 
    int msecGenerated;
    // max size this struct holds
    int maxSize;
    // toggles whether to recompute by time or by number
    //  of squares traveled.
    int pathType;
    // 
    int path[SQUAREPOOLSIZE][2];
    // how long until re-compute
    int msecTilRegen;
    int nSqrTilRegen;
    // current index into path[] that thinker is using
    int currentIndex;
    // first square (usually 1)
    int startIndex;
    // if set, force a regen
    gbool forceRegen;
    // id tag
    int tag[SQUAREPOOLSIZE];
    // square index into path.  0, 1, 2, 3
    int index[SQUAREPOOLSIZE];
    // turn off to not gen a path at all
    gbool usingPath;
    //
	void *owner;
    // tracking coordinates
    point2_t sq;
    vec2_t pix;
    
} thinkerpath_t;

typedef struct thinker_s {
    // square thinker is on
    point2_t sq;
    // pix offset in the square 
    vec2_t pix;
    // move amount
    vec2_t ma;

    thinkertype_t type;

    int hitpoints;

    void (*think) (void *);

    uint dirmoving;

    int spaceout;
    int searchmode;
    uint maxanim;
    uint currentanim;

    animation_t *anims;
    struct thinker_s *next;
    struct thinker_s *prev;

    int id;
    gbool eaten;
    int eaten_timer;
    gbool chased;
    gbool dead;
    gbool startChasedMode;

    vec2_t spawn;
    vec2_t respawn;
    vec2_t respawnpix;
    int chasedQuad;
    void *owner;

    thinkerpath_t path;

} thinker_t;

#define __thinker_default {{-1,-1}, {-1,-1},{0,0}, TT_NONE, 1, NULL, \
                            DIR_NONE, 600, 0, 1, 0, NULL, NULL, NULL, \
                            -1, gfalse, 0, gfalse, gfalse, gfalse, \
                            {-1,-1}, {-1,-1}, {0,0}, -1, NULL }

void T_EatGhost (thinker_t *tp);
void T_InitThinkerPath (thinker_t *);
gbool T_SpawnEyes (int x, int y, int gx, int gy);


#endif /* __THINKER_H__ */

