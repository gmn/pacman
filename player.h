
#ifndef __PLAYER_H__
#define __PLAYER_H__


#include "ticcmd.h" 

#if !defined(__POINT__)
# include "common.h" // vec2_t
#endif

typedef enum {
    PS_NONE,
    PS_LEVEL,
    PS_BIGDOT,
    PS_SUPERCHARGED,
    PS_TELEPORT,
    TOTAL_PLAYERSTATES
} playerstate_t;


#ifndef __THINKER_H__
# include "thinker.h"
#endif

typedef struct {
    char *name;
    int lives;

    // coordinate of the current square;
    point2_t sq; 

    // pix offset from the current square
    vec2_t pix;

    // pix move amount
    vec2_t ma;

    int score;
    int highscore;

    ticcmd_t cmd;

    playerstate_t state;

    gbool needsdrawn;

    int levnum;
    int nextguy;
    int times_cheated;

    // previous draw coords
    struct {
        point2_t sq;
        vec2_t pix;
    } prev;

    gbool discoveredCheat;

    vec2_t drawpix;
    vec2_t lastdrawpix;

    animation_t *anims;
    int defaultAnim;
    int totalAnims;
    int currentAnim;

	// for drawing the sprite
	int dirFacing;
	gbool moving;

} player_t;

#define __player_default { NULL, 3, {0, 0}, {0, 0}, {0, 0}, \
                            0, 0, __ticcmd_default_d , 0, gfalse, \
                            1, 20, 0, { {0, 0}, {0, 0} }, gfalse \
                            {0, 0}, {0, 0}, NULL, 0, 0, 0, 0, gfalse }


image_t * P_SetAnimation( void );
image_t * P_GetGenericSprite ( void );


#endif  /* !__PLAYER_H__ */



