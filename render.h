
#ifndef SQUAREPIX
# define SQUAREPIX    14
#endif

#ifndef __RENDER_H__
#define __RENDER_H__



#if !defined (__POINT__)
# include "common.h"
#endif



typedef struct {
    int leftpad;
    int rightpad;
    int toppad;
    int bottompad;

    // bottom left pix of the bottom-left square of the gameboard
    struct point_s botleftpix;

    // actual bottom-left square start x & y (ie, 6,0)
    struct point_s botleftsq;

} scene_t;

#define scene_default { 12, 12, 2, 2, { 0, 0 }, { 2, 1 } }

typedef struct {
    int height;
    int width;
    int byteperpixel;
    int bitperpixel;
    int pitch;
    int depthbuffer;
    int stencilbuffer;
    int pad;
} screen_t;

#define FLOATING_NUMBER_TEXT_SIZE 32

typedef struct floating_number_s {
    int on; 
    float x, y;
    float x0;

    uint start_time;
    uint thisFrame;
    uint length;
    char text[FLOATING_NUMBER_TEXT_SIZE];

    int lastframe_ms;
    int now;
    int msperframe;
    float pixmvperframe;
} floating_number_t;



#endif

/*
 * MAX_X == 40
 * MAX_Y == 31
 *
 * 476 / 34 == 14  
 * 616 / 44 == 14
 *
 * we move 2 squares over from leftpad:
 * botleftpix.x = leftpad   + botleftsq.x * SQUAREPIX = 12 + 2 * 14 = 40
 * botleftpix.y = bottompad + botleftsq.y * SQUAREPIX = 2 + 1 * 14 = 16
 */
