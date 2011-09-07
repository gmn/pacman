
#ifndef __SECTOR_H__
#define __SECTOR_H__


#ifndef __COMMON_H__
# include "common.h"
#endif

#ifndef MAX_X
# include "gameboard.h"
#endif

#ifndef bytesprite_t
# include "bytesprite.h"
#endif




#define EMPTY               0
#define TOP_LEFT_CORNER     1
#define TOP_MIDDLE          2
#define TOP_RIGHT_CORNER    4
#define LEFT_MIDDLE         8
#define MIDDLE              16
#define RIGHT_MIDDLE        32
#define BOT_LEFT_CORNER     64
#define BOT_MIDDLE          128
#define BOT_RIGHT_CORNER    256
#define FULL_SECTOR         0x1FF
#define TOP_ROW (TOP_LEFT_CORNER|TOP_MIDDLE|TOP_RIGHT_CORNER)
#define BOT_ROW (BOT_LEFT_CORNER|BOT_MIDDLE|BOT_RIGHT_CORNER)
#define LEFT_COL (TOP_LEFT_CORNER|LEFT_MIDDLE|BOT_LEFT_CORNER)
#define RIGHT_COL (TOP_RIGHT_CORNER|RIGHT_MIDDLE|BOT_RIGHT_CORNER)
#define MIDDLE_ROW (LEFT_MIDDLE|MIDDLE|RIGHT_MIDDLE)
#define MIDDLE_COL (TOP_MIDDLE|MIDDLE|BOT_MIDDLE)

typedef struct sector_s {
    int total;
    uint set[];
} sector_t;

struct sectorlist_s {
    bytesprite_t *bytesprite;
    sector_t *sector;
};
typedef struct sectorlist_s sectorlist_t[6];

struct sectorset_s {
    uint sector;
    int test;
    int integer;
};
typedef struct sectorset_s sectorset_t[9];

void S_GenSectorSetTest (const sectorset_t , sectorset_t , int , int ); 
uint S_GetSectorNumber (int, int);
bytesprite_t * S_GetBytespriteFromSecnum (uint sn);

#endif

// defined only for sector.c
#ifdef __GET_SECTOR_SETS__

sector_t top_left_corner = {
    5,
    {
( BOT_MIDDLE | MIDDLE | RIGHT_MIDDLE ),
( BOT_MIDDLE | BOT_RIGHT_CORNER | MIDDLE | RIGHT_MIDDLE ),
( FULL_SECTOR & ~BOT_RIGHT_CORNER ),
( FULL_SECTOR & ~( LEFT_COL | BOT_RIGHT_CORNER ) ),
( FULL_SECTOR & ~( TOP_ROW | BOT_RIGHT_CORNER ) ),
    }
};

sector_t top_right_corner = {
    5,
    {
( LEFT_MIDDLE | MIDDLE | BOT_MIDDLE ), 
( LEFT_MIDDLE | MIDDLE | BOT_LEFT_CORNER | BOT_MIDDLE ),
( FULL_SECTOR & ~BOT_LEFT_CORNER ),
( FULL_SECTOR & ~( BOT_LEFT_CORNER | TOP_ROW ) ),
( FULL_SECTOR & ~( BOT_LEFT_CORNER | RIGHT_COL ) ),
    }
};

sector_t bot_left_corner = {
    5,
    {
( MIDDLE | RIGHT_MIDDLE | TOP_MIDDLE ),
( MIDDLE | RIGHT_MIDDLE | TOP_MIDDLE | TOP_RIGHT_CORNER ),
( FULL_SECTOR & ~TOP_RIGHT_CORNER ),
( FULL_SECTOR & ~( TOP_RIGHT_CORNER | BOT_ROW ) ),
( FULL_SECTOR & ~( TOP_RIGHT_CORNER | LEFT_COL ) )
    }
};

sector_t bot_right_corner = {
    5,
    {
( LEFT_MIDDLE | MIDDLE | TOP_MIDDLE ),
( LEFT_MIDDLE | MIDDLE | TOP_MIDDLE | TOP_LEFT_CORNER ),
( FULL_SECTOR & ~TOP_LEFT_CORNER ),
( FULL_SECTOR & ~( TOP_LEFT_CORNER | RIGHT_COL ) ),
( FULL_SECTOR & ~( TOP_LEFT_CORNER | BOT_ROW ) ),
    }
};

sector_t vertical_wall = {
    17,
    {
( FULL_SECTOR & ~LEFT_COL ),
( FULL_SECTOR & ~RIGHT_COL ),
( MIDDLE | BOT_MIDDLE ),
( MIDDLE | TOP_MIDDLE ),
( MIDDLE_COL ),
( MIDDLE_COL | TOP_LEFT_CORNER ),
( MIDDLE_COL | TOP_RIGHT_CORNER ),
( MIDDLE_COL | BOT_LEFT_CORNER ),
( MIDDLE_COL | BOT_RIGHT_CORNER ),
( MIDDLE_COL | TOP_LEFT_CORNER | BOT_RIGHT_CORNER ),
( MIDDLE_COL | TOP_RIGHT_CORNER | BOT_LEFT_CORNER ),
( MIDDLE_COL | TOP_LEFT_CORNER | BOT_LEFT_CORNER ), 
( MIDDLE_COL | TOP_RIGHT_CORNER | BOT_RIGHT_CORNER ),
( FULL_SECTOR & ~( LEFT_MIDDLE | BOT_LEFT_CORNER ) ),
( FULL_SECTOR & ~( LEFT_MIDDLE | TOP_LEFT_CORNER ) ),
( FULL_SECTOR & ~( RIGHT_MIDDLE | BOT_RIGHT_CORNER ) ),
( FULL_SECTOR & ~( RIGHT_MIDDLE | TOP_RIGHT_CORNER ) ),
    }
};

sector_t horizontal_wall = {
    17,
    {
( FULL_SECTOR & ~TOP_ROW ),
( FULL_SECTOR & ~BOT_ROW ),
( MIDDLE | LEFT_MIDDLE ),
( MIDDLE | RIGHT_MIDDLE ),
( MIDDLE_ROW ),
( MIDDLE_ROW | TOP_LEFT_CORNER ),
( MIDDLE_ROW | TOP_RIGHT_CORNER ),
( MIDDLE_ROW | BOT_LEFT_CORNER ),
( MIDDLE_ROW | BOT_RIGHT_CORNER ),
( MIDDLE_ROW | TOP_LEFT_CORNER | BOT_RIGHT_CORNER ),
( MIDDLE_ROW | TOP_RIGHT_CORNER | BOT_LEFT_CORNER ),
( MIDDLE_ROW | TOP_LEFT_CORNER | TOP_RIGHT_CORNER ),
( MIDDLE_ROW | BOT_LEFT_CORNER | BOT_RIGHT_CORNER ),
( FULL_SECTOR & ~( TOP_MIDDLE | TOP_LEFT_CORNER ) ),
( FULL_SECTOR & ~( TOP_MIDDLE | TOP_RIGHT_CORNER ) ),
( FULL_SECTOR & ~( BOT_MIDDLE | BOT_LEFT_CORNER ) ),
( FULL_SECTOR & ~( BOT_MIDDLE | BOT_RIGHT_CORNER ) ),
    }
};

extern bytesprite_t tl_corner;
extern bytesprite_t tr_corner;
extern bytesprite_t bl_corner;
extern bytesprite_t br_corner;
extern bytesprite_t v_wall;
extern bytesprite_t h_wall;

sectorlist_t sectorlist = {
    { &tl_corner, &top_left_corner },
    { &tr_corner, &top_right_corner },
    { &bl_corner, &bot_left_corner },
    { &br_corner, &bot_right_corner },
    { &v_wall   , &vertical_wall },
    { &h_wall   , &horizontal_wall } 
}; 

sectorset_t glset = 
    {
{ TOP_LEFT_CORNER,      MAX_X + -1,  0 },
{ TOP_MIDDLE,           MAX_X ,     0 },
{ TOP_RIGHT_CORNER,     MAX_X + 1,  0 },
{ LEFT_MIDDLE,          -1       ,  0 },
{ MIDDLE,               0        , 0  },
{ RIGHT_MIDDLE,         1        ,  0 },
{ BOT_LEFT_CORNER,      -MAX_X + -1, 0 },
{ BOT_MIDDLE,           -MAX_X    , 0 },
{ BOT_RIGHT_CORNER,     -MAX_X + 1, 0 }
    };


#endif
