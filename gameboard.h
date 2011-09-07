
#ifndef __GAMEBOARD_H__
#define __GAMEBOARD_H__ 1

#if !defined(__BYTE__) || !defined(__POINT__)
# include "common.h"
#endif

/* NULL definition from MS stdlib.h */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef __IMAGE_H__
# include "image.h"
#endif


//
// *** GLOBAL DEFINITION of WIDTH and HEIGHT of the gameboard ***
//
#define MAX_X 40
#define MAX_Y 31

typedef enum {
    GC_NONE,
    GC_RED,
    GC_YELLOW,
    GC_PINK,
    GC_CYAN,
} ghostcolor_t;

typedef enum {
    WT_NULL,
    WT_BASIC,
    WT_VINES,
    WT_PANEL,
    WT_BASALT,
    WT_SLIDING,
    WT_SECRET,
    WT_ONEWAY,
    TOTAL_WALLTYPES
} walltype_t;

typedef enum {
    OT_NULL,
    OT_OPEN,
    OT_LITTLE,
    OT_BIG,
    OT_AMMO,
    OT_IDOL,
    OT_TELEPORT,
    // blocked: spaces that player cant go, but some entities can
    OT_BLOCKED,  
    // tunnel: player vanish into tunnels 
    //  they're there, but you cant see them
    OT_TUNNEL,
    TOTAL_OPENTYPES
} opentype_t;

// square is NULL, WALL or OPEN.  if OPEN there are 
//  additional opentypes, if wall additional walltypes, etc.
typedef enum {
    ST_NULL =   0,
    ST_WALL =   1,
    ST_OPEN =   2
} squaretype_t;

typedef enum { 
    ST_NONE,
    ST_UP_OK,
    ST_DOWN_OK,
    ST_LEFT_OK,
    ST_RIGHT_OK
} special_t;

typedef struct gamesquare_s {
    int x, y;
    squaretype_t type;
    opentype_t   open;
    walltype_t   wall;
    special_t special;
    vec3_t color;
} gamesquare_t;
#define __gamesquare_default { -1, -1, ST_NULL, OT_NULL, WT_NULL, ST_NONE, \
                                { 0.0, 0.0, 1.0 } }

//===================
typedef enum {
    ME_NONE =        0,
    ME_PLAYERSPAWN = 2048,
    ME_GHOSTSPAWN  = 2,
    ME_FRUITSPAWN  = 4,
    ME_TELEPORT    = 8,
} mapentity_t;

typedef struct {
    float x, y;
    mapentity_t type;
    vec2_t pix;
    image_t *img;
    ghostcolor_t ghostcolor;
    int id;
    vec2_t respawn;
    vec2_t spawn;
} entity_t;
//===================


typedef struct dot_s {
	int x, y;
	opentype_t type;
	struct dot_s *next;
} dot_t;


typedef struct gameboard_s {
    char name[128];
    int numsquares;
    int numentities;
    int numdots;
    int current;
    gamesquare_t *squares;

    // index array is a list of exactly 40x31 pointers 
    //  each pointer indexes into the squares list
    //  this way the index can be used to find the square we are looking
    //  for by gamesquare_t *i = gameboard.index[y*40+x];
    // the coordinate system for this game starts in the lower-left corner
    //  at (0, 0)
    gamesquare_t **index;
    entity_t *entities;
    byte *dots;

	// linked list of dot_t which stores type of dot and location
	//dot_t *metadots;

    int height; 
    int width;
    point_t shift;
    point_t llcorner;
    int maxindex;
    int currentdots;
    vec3_t defaultcolorv;
    point3_t defaultcolori;
} gameboard_t;

#define __gameboard_default { "", 0, 0, 0, 0,  \
                                NULL, NULL, NULL, NULL, \
                                MAX_Y, MAX_X, point_default, \
                                point_default, 0, 0, {0.0, 0.0, 1.0}, \
                                {0,0,255} } 


#endif

