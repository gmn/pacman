
#include "gameboard.h"

#define __GET_SECTOR_SETS__
#include "sector.h"
#undef __GET_SECTOR_SETS__

#include "bytesprite.h"

extern gameboard_t gameboard;

void S_GenSectorSetTest (const sectorset_t ref, sectorset_t gen, int ind, int max) 
{
    int i;

    for (i=0; i < 9; i++)
    {
        gen[i].sector = ref[i].sector;
        gen[i].test = ref[i].test + ind;
        gen[i].integer = (gen[i].test >= 0 && gen[i].test < max) ? 1 : 0;
    }
} 

// returns less than zero on err
uint S_GetSectorNumber (int x, int y)
{
    int i, ind;
    uint secnum;
    sectorset_t set;

    ind = y * MAX_X + x;

    S_GenSectorSetTest (glset, set, ind, (MAX_X*MAX_Y));

    secnum = EMPTY;
    for (i = 0; i < 9; i++) {
        // test index within map bounds
        if (set[i].integer) {
            if (gameboard.index[set[i].test] && gameboard.index[set[i].test]->type == ST_WALL)
                secnum |= set[i].sector;
        }
    }
    return secnum;
}

bytesprite_t * S_GetBytespriteFromSecnum (uint sn)
{
    int i, s;
    for (s = 0; s < 6; s++)
    {
        for (i = 0; i < sectorlist[s].sector->total; i++)
            if (sectorlist[s].sector->set[i] == sn)
                return sectorlist[s].bytesprite;
    }
    return NULL;
}

