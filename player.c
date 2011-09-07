/*
 * player.c
 */
#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h> // memcpy
#include <math.h> // floor()

#include "virtmem.h"
#include "player.h"
#include "ticcmd.h"
#include "pacman.h"
#include "event.h"
#include "gameboard.h"
#include "i_system.h"
#include "thinker.h"

player_t pl;
extern gameboard_t gameboard;
extern pacengine_t pe;
gameboard_t *gb = &gameboard;


#define OVERSHOOT 4
#define PLAYER_SPEED 2.60



static struct {
    const char *path;
    image_t *img;
} pacimages[] = {

    { IMGDIR"/pac1.tga" , NULL }, 
    { IMGDIR"/pac2.tga" , NULL }, 
    { IMGDIR"/pac3.tga" , NULL }, 
    { IMGDIR"/pac4.tga" , NULL }, 

    /*
    { IMGDIR"/clearbg/pac1.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac2.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac3.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac4.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac5.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac6.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac7.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac8.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac9.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac10.bmp" , NULL }, 
    { IMGDIR"/clearbg/pac11.bmp" , NULL }, 
    */
};

#define TOTAL_PAC_IMAGES (sizeof(pacimages)/sizeof(pacimages[0]))


image_t * P_GetGenericSprite ( void )
{
    return pacimages[3].img;
}

static void P_LoadImageData (void) {
    int i;
    for (i = 0; i < TOTAL_PAC_IMAGES; i++) {
        /*
		pacimages[i].img = V_malloc (sizeof(image_t), PU_STATIC, 0);
        IMG_readfileTGA( pacimages[i].path, pacimages[i].img );
        IMG_compileGLImage( pacimages[i].img );
        IMG_MakeGLTexture ( pacimages[i].img );
        */

        IMG_compileImageFile ( pacimages[i].path, &pacimages[i].img );
    }
}

static void P_LoadImages ( void ) {
    int i;

    P_LoadImageData();

    for (i = 0; i < TOTAL_PAC_IMAGES; i++) 
        pl.anims[0].frames[i] = pacimages[i].img;

    for (i-=2; i >= 0; i--) 
        pl.anims[0].frames[2*TOTAL_PAC_IMAGES-i-2] = pacimages[i].img;
}

void P_SetupPlayerAnimations ( void ) 
{
    int i;
    animation_t adef = __animation_default;

    pl.anims = V_malloc ( sizeof(*pl.anims) , PU_STATIC, 0 );
    pl.anims[0] = adef;    

    for (i = 0; i < 6; i++) 
    {
        pl.anims[0].frames[i] = V_malloc (sizeof(image_t), PU_STATIC, 0); 
        memset(pl.anims[0].frames[i], 0, sizeof (image_t));

        // render size, not actual dimension
        pl.anims[0].frames[i]->sz[0] = 18; 
        pl.anims[0].frames[i]->sz[1] = 18; 
    }

    pl.anims[0].maxframes = 2*TOTAL_PAC_IMAGES-2;
    pl.anims[0].frame = 6; // open mouth
    pl.anims[0].tpframe = 1;
    pl.anims[0].tic = 0;
    pl.anims[0].needdraw = gtrue;
    pl.anims[0].mspframe = 5;

    pl.defaultAnim = 0;
    pl.totalAnims = 1;

    // 3 - load images from file
    // 4 - build GL tiles in images
    // 5 - load them into game in universal pointers somewheres
    // 6 - attach the pointers to the frames[0]
    // 7 - write a function that computes and manipulates frame
    // 8 - finish GL_DrawPlayer
    // 9 - put call to frame func in P_PlayerThink, GL_Draw just draws
    //     whichever frame we are set to.

    P_LoadImages();
    
}

void P_PlayerDefaults(void)
{
    pl.name = V_malloc (32, PU_STATIC, 0);
    C_strncpy (pl.name, "Motherball", 32);
    pl.lives = 3;
    pl.sq[0] = 0;
    pl.sq[1] = 0;
    pl.pix[0] = 0;
    pl.pix[1] = 0;
    pl.score = 0;
    //pl.ma[0] = pl.ma[1] = 1.0;
    pl.ma[0] = pl.ma[1] = PLAYER_SPEED;
    pl.state = PS_LEVEL;
	pl.levnum = 1;
    pl.nextguy = 20;
    pl.times_cheated = 0;
    pl.dirFacing = DIR_LEFT;

    P_ResetPlayerCoordinates();
}

void P_FineMove (direction_t dir)
{
    switch (dir) {
    case DIR_UP:    
		pl.pix[0] = 0;
        pl.pix[1] += pl.ma[1];
        break;
    case DIR_DOWN:   
		pl.pix[0] = 0;
        pl.pix[1] -= pl.ma[1];
        break;
    case DIR_LEFT:   
		pl.pix[1] = 0;
        pl.pix[0] -= pl.ma[0];
        break;
    case DIR_RIGHT:
		pl.pix[1] = 0;
        pl.pix[0] += pl.ma[0];
        break;
    }   

#define SQUARE 13

    if (pl.pix[0] > SQUARE) {
        pl.sq[0]++;
        pl.pix[0] = 0;
    } else if (pl.pix[0] < -SQUARE) {
        pl.sq[0]--;
        pl.pix[0] = 0;
    }
    
    if (pl.pix[1] > SQUARE) {
        pl.sq[1]++;
        pl.pix[1] = 0;
    } else if (pl.pix[1] < -SQUARE) {
        pl.sq[1]--;
        pl.pix[1] = 0;
    }
}

#define inbounds(x,y) ((x) >= 0 && (x) < (MAX_X*MAX_Y) && y[(x)])

int P_MoveAllowed (direction_t dir) 
{
    switch (dir) {
    case DIR_UP:
        if (pl.pix[0] > OVERSHOOT || pl.pix[0] < -OVERSHOOT) {
            return 0; 
        } 
// the error msg for the line below says, 'subscript is not of integral type'
// i'm thinking that it almost has to be the y[(x)] part of inbounds()
// the sq[0] & sq[1] vars must not be getting initialized somewhere.  but it
//  is difficult to see since I cant even compile it.
//
        if (inbounds( ((MAX_X*(pl.sq[1]+1))+pl.sq[0]), gb->index))
            if (gb->index[MAX_X*(pl.sq[1]+1)+pl.sq[0]]->type == ST_OPEN)
                return 1;
        return 0;
    case DIR_DOWN:
        if (pl.pix[0] > OVERSHOOT || pl.pix[0] < -OVERSHOOT) {
            return 0; 
        }

        if (inbounds(MAX_X*(pl.sq[1]-1)+pl.sq[0], gb->index))
            if (gb->index[MAX_X*(pl.sq[1]-1)+pl.sq[0]]->type == ST_OPEN)
                return 1;
        return 0;
    case DIR_LEFT:
        if (pl.pix[1] > OVERSHOOT || pl.pix[1] < -OVERSHOOT) {
            return 0; 
        }

        if (inbounds(MAX_X*pl.sq[1]+pl.sq[0]-1, gb->index))
            if (gb->index[MAX_X*pl.sq[1]+pl.sq[0]-1]->type == ST_OPEN)
                return 1;
        return 0;
    case DIR_RIGHT:
        if (pl.pix[1] > OVERSHOOT || pl.pix[1] < -OVERSHOOT) {
            return 0; 
        }
        
        if (inbounds(MAX_X*pl.sq[1]+pl.sq[0]+1, gb->index))
            if (gb->index[MAX_X*pl.sq[1]+pl.sq[0]+1]->type == ST_OPEN)
                return 1;
        return 0;
    }
    return 0;
}



#define tmpMOVE(m,x,y)      switch((m)) { \
                            case DIR_UP: ++(y); break; \
                            case DIR_DOWN: --(y); break; \
                            case DIR_LEFT: --(x); break; \
                            case DIR_RIGHT: ++(x); break; \
                            } 



void P_MovePlayer (void)
{
    int i;
    static direction_t momentum = DIR_LEFT;
    direction_t movemade;
    vec2_t pmove, mmove;
    uint dirs[] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_UP_AND_DOWN, DIR_LEFT_AND_RIGHT };
    uint open[] = { 0, 0, 0, 0, 0, 0 };
    ticcmd_t *cmd;


    cmd = &pl.cmd;

    pmove[0] = pmove[1] = 0;
    mmove[0] = mmove[1] = 0;
    movemade = DIR_NONE;

    // which ways are we allowed to move?
    for (i = 0; i < 4; i++) {
        if (P_MoveAllowed(dirs[i])) {
            open[i] = 1;

            if (i < 2)
                open[4] = 1;
            else
                open[5] = 1;
        }
    }
    
    if (cmd->dirmoving) 
    {
        if (!momentum) 
        {
            for (i=1; i < 16; i<<=1) {
                if (P_MoveAllowed ( cmd->dirmoving & i )) {
                    tmpMOVE(cmd->dirmoving&i, pmove[0], pmove[1]);
                }
            }

            // loop through the allowed directions
            for (i = 0; i < 4 && !movemade; i++) {
                if (!open[i])
                    continue;
                switch (dirs[i]) {
                case DIR_UP:
                    if (pmove[1] > 0)
                        movemade = dirs[i];
                    break;
                case DIR_DOWN:
                    if (pmove[1] < 0)
                        movemade = dirs[i];
                    break;
                case DIR_LEFT:
                    if (pmove[0] < 0)
                        movemade = dirs[i];
                    break;
                case DIR_RIGHT:
                    if (pmove[0] > 0)
                        movemade = dirs[i];
                    break;
                }
            }


        } 
        else  // there is momentum
        {
            for ( i = 1; i < 16; i<<=1 ) {
                if ( momentum & i ) {
                    tmpMOVE(momentum & i, mmove[0], mmove[1]);
                }
                if ( cmd->dirmoving & i ) {
                    tmpMOVE(cmd->dirmoving&i, pmove[0], pmove[1]);
                }
            }
            
            for (i = 0; i < 4 && !movemade; i++) {
                if (!open[i])
                    continue;
                switch (dirs[i]) {
                case DIR_UP:
                    if (pmove[1] > 0) {
                        movemade = DIR_UP;
                    } else if (mmove[1] > 0 && pmove[1] == 0 ) { 
                        if (!open[5]) 
                            movemade = DIR_UP;
                        else if (open[2] && pmove[0] < 0) 
                            movemade = DIR_LEFT;
                        else if (open[3] && pmove[0] > 0)
                            movemade = DIR_RIGHT;
                        else /* if (pmove[0] == 0) */
                            movemade = DIR_UP;
                    }
                    break;
                case DIR_DOWN:
                    if (pmove[1] < 0) {
                        movemade = DIR_DOWN;
                    } else if (mmove[1] < 0 && pmove[1] == 0 ) { 
                        if (!open[5]) 
                            movemade = DIR_DOWN;
                        else if (open[2] && pmove[0] < 0) 
                            movemade = DIR_LEFT;
                        else if (open[3] && pmove[0] > 0)
                            movemade = DIR_RIGHT;
                        else /* if (pmove[0] == 0) */
                            movemade = DIR_DOWN;
                    }
                    break;
                case DIR_LEFT:
                    if (pmove[0] < 0) {
                        movemade = DIR_LEFT;
                    } else if (mmove[0] < 0 && pmove[0] == 0 ) { 
                        if (!open[4]) 
                            movemade = DIR_LEFT;
                        else if (open[0] && pmove[1] > 0) 
                            movemade = DIR_UP;
                        else if (open[1] && pmove[1] < 0)
                            movemade = DIR_DOWN;
                        else /* if (pmove[1] == 0) */
                            movemade = DIR_LEFT;
                    }
                    break;
                case DIR_RIGHT:
                    if (pmove[0] > 0) {
                        movemade = DIR_RIGHT;
                    } else if (mmove[0] > 0 && pmove[0] == 0 ) { 
                        if (!open[4]) 
                            movemade = DIR_RIGHT;
                        else if (open[0] && pmove[1] > 0) 
                            movemade = DIR_UP;
                        else if (open[1] && pmove[1] < 0)
                            movemade = DIR_DOWN;
                        else /* if (pmove[1] == 0) */
                            movemade = DIR_RIGHT;
                    } 
                    break;
                }
            } // for each open path


        } // end of both
        

    } 
    else // NO USER INPUT
    {
        if (momentum) {
            for (i=1; i < 16; i<<=1) {
                if (P_MoveAllowed ( momentum & i )) {
                    tmpMOVE(momentum & i, mmove[0], mmove[1]);
                }
            }
             
			// loop through the allowed directions
			for (i = 0; i < 4 && !movemade; i++) {
			    if (!open[i])
			        continue;
			    switch (dirs[i]) {
			    case DIR_UP:
			        if (mmove[1] > 0)
			            movemade = dirs[i];
                    break;
			    case DIR_DOWN:
			        if (mmove[1] < 0)
			            movemade = dirs[i];
                    break;
			    case DIR_LEFT:
			        if (mmove[0] < 0)
			            movemade = dirs[i];
                    break;
			    case DIR_RIGHT:
			        if (mmove[0] > 0)
			            movemade = dirs[i];
                    break;
			    }
			}
		}
    }

    if (movemade) {
        P_FineMove(movemade);
		momentum = movemade;
        pl.dirFacing = movemade;
		pl.moving = gtrue;
	} else {
		pl.moving = gfalse;
	}
    
    cmd->dirmoving = DIR_NONE;
}


gbool P_EntityThisSquare (int etype)
{
    int i;
    for (i = 0; i < gb->numentities ; i++) {
        if (pl.sq[0] == gb->entities[i].x && pl.sq[1] == gb->entities[i].y) {
            if (gb->entities[i].type == etype) {
                return gtrue;
            }
        }
    }

    return gfalse;
}




/*
====================
 P_PlayerThink
====================
*/
void P_PlayerThink (void)
{
    static int last_x = 0;
    static int last_y = 0;

    if (pl.state == PS_TELEPORT && !P_EntityThisSquare(ME_TELEPORT))
        pl.state = PS_LEVEL;

    P_MovePlayer();

    /*
    if ((pl.sq[0] != last_x) || (pl.sq[1] != last_y))
    {
        last_x = pl.sq[0];
        last_y = pl.sq[1];
    }
    */
}

#define TELEPORT_PAUSE 400

/*
==================== 
 P_CheckSquareEntities
 - Handles the player's interaction with non-thinker square entities
==================== 
*/
void P_CheckSquareEntities (void)
{
    int i;
    static int teleport_pause = 0;
    gbool keepgoing = gtrue;

    for (i = 0; i < gb->numentities && keepgoing ; i++) {
        if (pl.sq[0] == gb->entities[i].x && pl.sq[1] == gb->entities[i].y)
        {
            switch (gb->entities[i].type) {
            case ME_TELEPORT:
                if (pl.state == PS_TELEPORT)
                    break;
                if (I_CurrentMillisecond() - teleport_pause > TELEPORT_PAUSE) {
                    pl.sq[0] = gb->entities[i].pix[0];
                    pl.sq[1] = gb->entities[i].pix[1];
                    pl.pix[0] = pl.pix[1] = 0;
                    pl.state = PS_TELEPORT;
                    teleport_pause = I_CurrentMillisecond();
                    keepgoing = gfalse;
                }
                break;
            }
        }
    }
}


void P_ResetPlayerCoordinates (void)
{
    int i;
    gbool found = gfalse;

    for (i = 0; i < gameboard.numentities && !found; i++)
    {
        if (gameboard.entities[i].type == ME_PLAYERSPAWN)
        {
            pl.sq[0] = gameboard.entities[i].x;
            pl.sq[1] = gameboard.entities[i].y;
            pl.pix[0]  = gameboard.entities[i].pix[0];
            pl.pix[1]  = gameboard.entities[i].pix[1];
            found = gtrue;
        }
    }

    if (!found)
        sys_error_fatal ("couldn't find player spawn when resetting coordinates!");
}

int P_GetScore (void)
{
    return pl.score;
}

#define LITTLE_DOT_POINTS 10
#define BIG_DOT_POINTS 50
/*
==================== 
 P_GobbleDots
==================== 
*/
void P_GobbleDots (void)
{
    switch (gameboard.dots[pl.sq[1] * MAX_X + pl.sq[0]]) {
    case OT_LITTLE:
        pl.score += LITTLE_DOT_POINTS;
        gameboard.currentdots--;
        gameboard.dots[pl.sq[1] * MAX_X + pl.sq[0]] = OT_NULL;
        break;
    case OT_BIG:
        pl.score += BIG_DOT_POINTS;
        gameboard.currentdots--;
        gameboard.dots[pl.sq[1] * MAX_X + pl.sq[0]] = OT_NULL;
        T_StartChaseMode();
        break;
    }
}

/*
====================
  P_Ticker
====================
*/
void P_Ticker (void)
{
    P_PlayerThink ();
    P_GobbleDots ();
    // non-thinker encounters
    P_CheckSquareEntities ();
}

/*
==================== 
 P_SetAnimation()
==================== 
*/
image_t * P_SetAnimation( void ) 
{
    animation_t *a = &pl.anims[0];

    if ( pe.logicframe - a->lastframe < a->tpframe )
        return a->frames[a->frame];

	if (!G_CheckPause() && pl.moving) {
		a->tic += ( pe.logicframe - a->lastframe ) / a->tpframe;
		a->frame = a->tic % a->maxframes;
	}

	a->lastframe = pe.logicframe;

    return a->frames[a->frame];
}
