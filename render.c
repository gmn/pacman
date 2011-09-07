/**********************************************************************
 * render.c - main gameboard rendering functions
 * - all the actual drawing to the screen happens here
 *********************************************************************/

#pragma warning ( disable : 4244 4668 4820 4255 )

#ifdef _WIN32
# include <windows.h>
#endif
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h> // sin()

#include "pacman.h"
#include "i_system.h"
#include "common.h"
#include "gameboard.h"
#include "render.h"

#include "player.h"
#include "virtmem.h"
#include "sector.h"
#include "bytesprite.h"
#include "image.h"
#include "thinker.h"

#include "game.h"
#include "event.h"

#include "splash.h" //

extern gameboard_t gameboard;


GLuint gameboard_dl;

scene_t sn = scene_default;

extern player_t pl;
extern pacengine_t pe;
extern thinker_t thinkercap;

static int *gb_vector = NULL;
static int *gb_wall_handles = NULL;
static int gb_cell_count = 0;

int currentfont = 2;


GLdouble near_plane = 100000.0;
GLdouble far_plane  = 112400.0;
//  use for rotation, clears near clip plane
//GLdouble camera_z   = -100500.0;
GLdouble camera_z   = -100001.0;


GLdouble axisrot = 0.0;


//#define GL_FRUSTUM() glFrustum( -SCREENWIDTH/2, SCREENWIDTH/2, \
//                                -SCREENHEIGHT/2, SCREENHEIGHT/2, \
//                                near_plane, far_plane )

#define GL_FRUSTUM() glOrtho(0.0, SCREENWIDTH, 0.0, SCREENHEIGHT, -1.0, 1.0)




#define MAX_FLOATING_NUMBER 6
floating_number_t floating_numbers[MAX_FLOATING_NUMBER];
int current_floating_number = 0;

void InitFloatingNumberList (void)
{
    int i = 0;
    for (i = 0; i < MAX_FLOATING_NUMBER; i++) {
        floating_numbers[i].on = 0;
        floating_numbers[i].x = 0;
        floating_numbers[i].y = 0;
        floating_numbers[i].start_time = 0;
        floating_numbers[i].now = 0;
        floating_numbers[i].msperframe = 10;
        floating_numbers[i].lastframe_ms = 0;
        floating_numbers[i].pixmvperframe = 1.0;
    }
}

void PostFloatingNumber (const char *text, int x, int y, int len)
{
    C_strncpy(floating_numbers[current_floating_number].text, text, FLOATING_NUMBER_TEXT_SIZE);
    floating_numbers[current_floating_number].x = x;
    floating_numbers[current_floating_number].x0 = x;
    floating_numbers[current_floating_number].y = y;
    floating_numbers[current_floating_number].start_time = I_CurrentMillisecond();
    floating_numbers[current_floating_number].on = 1;
    floating_numbers[current_floating_number].length = len;
    floating_numbers[current_floating_number].thisFrame = pe.logicframe;

    floating_numbers[current_floating_number].now = floating_numbers[current_floating_number].lastframe_ms = I_CurrentMillisecond();

    current_floating_number = (current_floating_number+1)%MAX_FLOATING_NUMBER;
}

// 0 is off, anything else running
int gamemsgend = 0;
char gamemsgbuf[128];
int gamemsgend2 = 0;
char gamemsgbuf2[128];

void RunFloatingNumbers (void)
{
    int i = 0;
    int counts, c2=0;
    floating_number_t *p;
    uint white = 0xFFFFFFFF;
    uint msec;

    float Mag =  8.33; 
    float freq = 10.0;

        
    msec = I_CurrentMillisecond();

    for (i = 0; i < MAX_FLOATING_NUMBER; i++) {
        if (floating_numbers[i].on)
        {
            p = &floating_numbers[i];
            p->now = msec;

            if (msec - p->start_time >= p->length)
            {
                p->on = 0;
            }
            else
            {
                counts = (p->now - p->lastframe_ms) / p->msperframe;
                if (counts > 0) {
                    p->lastframe_ms = p->now;
                    c2 = counts;
                }

                while (counts--) {
                    p->y += p->pixmvperframe;
                }

                if (c2) {
                    // FIXME: use sin table instead of stdlib/crt
                    p->x = p->x0 + Mag * sin( (1/freq) * p->y );
                }

                F_Printf(p->x, p->y, 3, white, "%s", p->text);
            }
        }
    }

    // print game messages
    if ( gamemsgend ) {
        if (I_CurrentMillisecond() > gamemsgend) 
            gamemsgend = 0;

        F_Printf(100,100,5,0xFFFFFFFF,"%s", gamemsgbuf);
    }
    if ( gamemsgend2 ) {
        if (I_CurrentMillisecond() > gamemsgend2) 
            gamemsgend2 = 0;

        F_Printf(100,50,5,0xFFFFFFFF,"%s", gamemsgbuf2);
    }

}

// uint to float
#define UTF(x) (float)(((float)(x))/255.0)

void GL_ScaledDownGameboardDraw (void)
{
    int i, j;

    glPushAttrib(GL_CURRENT_BIT);

    for (i = 0; i < gb_cell_count; i++) 
    {
        j = i * 4;

        glEnable (GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);
        glAlphaFunc (GL_GREATER, 0.0);

	   	glEnable(GL_TEXTURE_2D);
	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glBindTexture(GL_TEXTURE_2D, gb_wall_handles[i]);
        glBegin(GL_QUADS);

	    glTexCoord2f(0.0, 0.0); 
        glVertex3i (gb_vector[j+0], gb_vector[j+1], 0);
        glTexCoord2f(0.0, 1.0); 
        glVertex3i (gb_vector[j+0], gb_vector[j+3], 0);
        glTexCoord2f(1.0, 1.0); 
        glVertex3i (gb_vector[j+2], gb_vector[j+3], 0);
	    glTexCoord2f(1.0, 0.0); 
        glVertex3i (gb_vector[j+2], gb_vector[j+1], 0);

        glEnd();
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_DEPTH_TEST);
    }

    glPopAttrib();
}

void GL_setup_gameboard (void)
{
    int i, x, y;
    int count = 0;
    static int old_count = 0;
    uint secnum;
    bytesprite_t *cwallp;

    for (i=0; i < MAX_X*MAX_Y; i++)
    {
		if (!gameboard.index[i])
			continue;
		if (gameboard.index[i]->type != ST_WALL)
			continue;

	    x = gameboard.index[i]->x;
        y = gameboard.index[i]->y;
        secnum = S_GetSectorNumber(x, y);
        cwallp = S_GetBytespriteFromSecnum(secnum);

		if (cwallp)
	        ++count;
    }

    if (count > old_count) {
        if (gb_vector)
            V_free(gb_vector);
        if (gb_wall_handles)
            V_free(gb_wall_handles);
        gb_vector = V_malloc (sizeof(int)*(count*4), PU_STATIC, 0);
        gb_wall_handles  = V_malloc (sizeof(int)*count, PU_STATIC, 0);
        old_count = count;
    }
    gb_cell_count = count;

    count = 0;

    for (i=0; i < MAX_X*MAX_Y; i++)
    {
		if (!gameboard.index[i])
			continue;
		if (gameboard.index[i]->type != ST_WALL)
			continue;
        if (count >= gb_cell_count)
            break;

        x = gameboard.index[i]->x;
        y = gameboard.index[i]->y;

        secnum = S_GetSectorNumber(x, y);
        cwallp = S_GetBytespriteFromSecnum(secnum);

		if (cwallp) {
			gb_vector[count*4+0] = sn.botleftpix.x + x * SQUAREPIX;
			gb_vector[count*4+1] = sn.botleftpix.y + y * SQUAREPIX;
			gb_vector[count*4+2] = gb_vector[count*4+0] + SQUAREPIX;
			gb_vector[count*4+3] = gb_vector[count*4+1] + SQUAREPIX;

			gb_wall_handles[count] = cwallp->texhandle;

			++count;
		}
    }
}
    
/*
====================
GL_SetupViewport

    - the first opengl API calls are here, 
         
====================
*/
void GL_SetupViewport (int width, int height)
{
    // prevent divide by zero 
	if (height==0)										
		height=1;										

    // just to make sure
    glMatrixMode(GL_MODELVIEW);

    // the only time the Viewport is ever set
	glViewport(0,0,width,height);	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	

    //glOrtho(0.0, (GLfloat)width, 0.0, (GLfloat)height, -1.0, 1.0);

    GL_FRUSTUM();

    // experimental
    //glFrustum(0, width, 0, height, 1.0, 10.0); 


	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();	
	glRasterPos3i(0, 0, 0);
}

// called in win32\win_system.c
void GL_ResizeScene (int width, int height)
{
    GL_SetupViewport(width, height);
}

enum {  tp_nil, 
        tp_horizontal, 
        tp_vertical, 
        tp_bl_corner, 
        tp_tl_corner, 
        tp_tr_corner, 
        tp_br_corner };

// macro args: bottom left and right of current square in pixels,
//             width, length, height (z-axis)
#define GL_HLINE(a,b,w,l,z)  \
            glBegin(GL_QUADS); \
            glVertex3f ((a),        (b)+SQUAREPIX/2.0-((w)/2.0), (z)); \
            glVertex3f ((a)+(l),    (b)+SQUAREPIX/2.0-((w)/2.0), (z)); \
            glVertex3f ((a)+(l),    (b)+SQUAREPIX/2.0+((w)/2.0), (z)); \
            glVertex3f ((a),        (b)+SQUAREPIX/2.0+((w)/2.0), (z)); \
            glEnd();

#define GL_VLINE(a,b,w,l,z)  \
            glBegin(GL_QUADS); \
            glVertex3f ((a)+SQUAREPIX/2.0-((w)/2.0), (b),       (z)); \
            glVertex3f ((a)+SQUAREPIX/2.0-((w)/2.0), (b)+(l),   (z)); \
            glVertex3f ((a)+SQUAREPIX/2.0+((w)/2.0), (b)+(l),   (z)); \
            glVertex3f ((a)+SQUAREPIX/2.0+((w)/2.0), (b),       (z)); \
            glEnd();

/*
==================== 
 GL_DrawThinkerPath()
 - kind of an embellishment, just to show the pathfind in action, but also
    useful for fine-tuning the ghosts use of pathfind later.
==================== 
*/
static void GL_DrawThinkerPath(thinker_t *tp)
{ 
    int end;
    int index;
    thinkerpath_t *p;
    float addx, addy;
    int lx, ly, nx, ny, x, y;
    float pixx, pixy;
    int prevdir, nextdir;
    float a;
    int pathchunk;


    if (!tp)
        return;

    addx = addy = 4.0;
    a = 0.55;

    p       = &tp->path;
    end     = p->length;
    index   = p->currentIndex;

    if (tp->type == TT_EYES)
        glColor4f(0.0, 1.0, 0.0, a);
    else if (tp->chased)
        glColor4f(1.0, 1.0, 0.0, a);
    else
        glColor4f(1.0, 0.0, 1.0, a);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glEnable(GL_BLEND);

    for (; index < end; index++) 
    {
        // previous square 
        if (index-1 > p->currentIndex) {
            lx = p->path[index-1][0];
            ly = p->path[index-1][1];
        } else {
            lx = -1;
        }

        // next square
        if (index+1 < end) {
            nx = p->path[index+1][0];
            ny = p->path[index+1][1];
        } else {
            nx = -1;
        }

        // current square
        x = p->path[index][0];
        y = p->path[index][1];

        pixx = sn.botleftpix.x + x * SQUAREPIX;
        pixy = sn.botleftpix.y + y * SQUAREPIX;


        /* last square */
        prevdir = DIR_NONE;
        if ( lx > -1 ) {  
            if ( x > lx ) {
                prevdir = DIR_LEFT;
            } else if ( x < lx ) {
                prevdir = DIR_RIGHT;
            } else if ( y > ly ) {
                prevdir = DIR_DOWN;
            } else if ( y < ly ) {
                prevdir = DIR_UP;
            }
        }

        /* next square */
        nextdir = DIR_NONE;
        if ( nx > -1 ) {
            if ( nx > x ) {
                nextdir = DIR_RIGHT;
            } else if ( nx < x ) {
                nextdir = DIR_LEFT;
            } else if ( ny > y ) {
                nextdir = DIR_UP;
            } else if ( ny < y ) {  
                nextdir = DIR_DOWN;
            }
        }

        pathchunk = tp_nil;
        switch (prevdir) {
        case DIR_LEFT:
            switch (nextdir) {
            case DIR_UP:        pathchunk = tp_br_corner;       break;
            case DIR_RIGHT:     pathchunk = tp_horizontal;      break;
            case DIR_DOWN:      pathchunk = tp_tr_corner;       break; 
            case DIR_NONE:      pathchunk = tp_horizontal;      break;
            }
            break;
        case DIR_UP:
            switch (nextdir) {
            case DIR_LEFT:      pathchunk = tp_br_corner;       break;
            case DIR_DOWN:      pathchunk = tp_vertical;        break; 
            case DIR_RIGHT:     pathchunk = tp_bl_corner;       break; 
            case DIR_NONE:      pathchunk = tp_vertical;        break;
            }
            break;
        case DIR_RIGHT:
            switch (nextdir) {
            case DIR_UP:        pathchunk = tp_bl_corner;       break;
            case DIR_LEFT:      pathchunk = tp_horizontal;      break;
            case DIR_DOWN:      pathchunk = tp_tl_corner;       break; 
            case DIR_NONE:      pathchunk = tp_horizontal;      break;
            }
            break;
        case DIR_DOWN:
            switch (nextdir) {
            case DIR_RIGHT:     pathchunk = tp_tl_corner;       break;
            case DIR_UP:        pathchunk = tp_vertical;        break;
            case DIR_LEFT:      pathchunk = tp_tr_corner;       break; 
            case DIR_NONE:      pathchunk = tp_vertical;        break;
            }
            break;
        case DIR_NONE:
            switch (nextdir) {
            case DIR_RIGHT:     
            case DIR_LEFT:      pathchunk = tp_horizontal;      break; 
            case DIR_UP:        
            case DIR_DOWN:      pathchunk = tp_vertical;        break;
            }
        }

        // draw pathchunk
        switch (pathchunk) {
        case tp_horizontal:
            GL_HLINE(pixx, pixy, 3.0, SQUAREPIX, 1.0);
            break;
        case tp_vertical:
            GL_VLINE(pixx, pixy, 3.0, SQUAREPIX, 1.0);
            break;
        case tp_bl_corner:
            GL_HLINE(pixx+(SQUAREPIX/2.0), pixy, 3.0, SQUAREPIX/2.0, 1.0);
            GL_VLINE(pixx, pixy+(SQUAREPIX/2.0), 3.0, SQUAREPIX/2.0, 1.0);
            break;
        case tp_br_corner:
            GL_HLINE(pixx, pixy, 3.0, SQUAREPIX/2.0, 0.1);
            GL_VLINE(pixx, pixy+(SQUAREPIX/2.0), 3.0, SQUAREPIX/2.0, 1.0);
            break;
        case tp_tl_corner:
            GL_HLINE(pixx+(SQUAREPIX/2.0), pixy, 3.0, SQUAREPIX/2.0, 1.0);
            GL_VLINE(pixx, pixy, 3.0, SQUAREPIX/2.0, 0.1);
            break;
        case tp_tr_corner:
            GL_HLINE(pixx, pixy, 3.0, SQUAREPIX/2.0, 1.0);
            GL_VLINE(pixx, pixy, 3.0, SQUAREPIX/2.0, 1.0);
            break;
        }

    } // for the whole path

    glDisable(GL_BLEND);
    glColor4f(1.0, 1.0, 1.0, 0.5);
}

void GL_ThinkerPaths ( void )
{
    thinker_t *tp;

    for (tp = thinkercap.next ; tp != &thinkercap ; tp = tp->next)
    {
        if (tp->dead)
            continue;

		// may try to draw thinker when they are removed from play but not yet 
		//  fully deleted
		if (!tp->anims || !tp->think)
			continue;

        // move this to later in the render chain, draw paths on top
        if (tp->path.usingPath)
        {
            GL_DrawThinkerPath(tp);
        } 
    }
}


void GL_PaintGhosts (void)
{
    image_t *img;
    vec2_t pix;
    thinker_t *tp;
	GLenum err = GL_NO_ERROR;


    glEnable (GL_ALPHA_TEST);
    glEnable (GL_DEPTH_TEST);

    // desired alpha func (all alphas that are zero are discarded,
    //  everything else is drawn as well as blended
    glAlphaFunc (GL_GREATER, 0.0);

    // no blending after all
    // glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    // glEnable(GL_BLEND);

    // Now I'm thinking that I need two masks.  One to display just the 
    //  central image, the part of the image that I dont want blended,
    //  then draw those fragments with blending off, then enable blending
    //  and draw just the anti-aliased edges using a different mask to create
    //  the image, a mask that causes all the central portion to get thrown
    //  out as well.

    for (tp = thinkercap.next ; tp != &thinkercap ; tp = tp->next)
    {
        if (tp->dead)
            continue;

		// may try to draw thinker when they are removed from play but not yet 
		//  fully deleted
		if (!tp->anims || !tp->think)
			continue;


        img = tp->anims[tp->currentanim].frames[(tp->anims[tp->currentanim].frame)];


        pix[0] = sn.botleftpix.x + tp->sq[0] * SQUAREPIX + tp->pix[0] - 2;
        pix[1] = sn.botleftpix.y + tp->sq[1] * SQUAREPIX + tp->pix[1] - 2;

       	glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glBindTexture(GL_TEXTURE_2D, img->texhandle);
        glBegin(GL_QUADS);

        glTexCoord2f(0.0, 0.0); 
        glVertex3i (pix[0],               pix[1],           0);
        glTexCoord2f(0.0, 1.0); 
        glVertex3i (pix[0]              , pix[1] + img->sz[1], 0);
        glTexCoord2f(1.0, 1.0); 
        glVertex3i (pix[0] + img->sz[0] , pix[1] + img->sz[1], 0);
        glTexCoord2f(1.0, 0.0); 
        glVertex3i (pix[0] + img->sz[0] , pix[1],           0);

        glEnd();
        glDisable(GL_TEXTURE_2D);

        glFlush();

		err = glGetError();
		if (err != GL_NO_ERROR) 
			C_WriteLog("glError: \"%s\" in render ghost\n", gluErrorString(err));

    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

#define STRETCH (SQUAREPIX+4)
//#define STRETCH (SQUAREPIX+6)
///#define STRETCH 32

void GL_PutAlphaSprite(int x, int y, int texhandle)
{
    glEnable (GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
    glAlphaFunc (GL_GREATER, 0.0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, texhandle);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3i(x, y, 0);
	glTexCoord2f(0.0, 1.0); glVertex3i(x, y + STRETCH, 0);
	glTexCoord2f(1.0, 1.0); glVertex3i(x + STRETCH, y + STRETCH, 0);
	glTexCoord2f(1.0, 0.0); glVertex3i(x + STRETCH, y, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
}

void R_PostFloatingText (const char *text, int len)
{
    int x, y;
    x = (pl.sq[0] + sn.botleftsq.x) * SQUAREPIX + pl.pix[0] + sn.leftpad - 2;
    y = (pl.sq[1] + sn.botleftsq.y) * SQUAREPIX + pl.pix[1] + sn.bottompad - 2;
    PostFloatingNumber (text, x, y + 3 * SQUAREPIX , len);
}


void GL_BlitPacdude (void)
{
    float x, y, x1, y1, x0, y0;
    bytesprite_t *pacdude;
    float alpha;
    float one_minus_alpha;
    image_t *img;
    float facing;

    GLenum err = GL_NO_ERROR;

    glColor4f(1.0, 1.0, 0.0, 1.0);


//    pacdude = BS_getpacdude();

    // dont interpolate when not moving
    if ( pe.interpolate && !G_CheckPause() )
    {
        pe.drawtime = I_CurrentMillisecond();
        alpha = ( pe.drawtime - pe.lastlogictime ) / ( pe.logictime - pe.lastlogictime );
        one_minus_alpha = 1.0 - alpha;

        // draw = (1 - a) * pl.prev + a * pl;

        
        // compute from square and pix
        /*
        x0 = (pl.prev.sq[0] + sn.botleftsq.x) * SQUAREPIX + pl.prev.pix[0] + sn.leftpad - 2;
        y0 = (pl.prev.sq[1] + sn.botleftsq.y) * SQUAREPIX + pl.prev.pix[1] + sn.bottompad - 2;
        */
        
        
        // compute from last pix directly
        x0 = pl.lastdrawpix[0]; 
        y0 = pl.lastdrawpix[1];
        

        x1 = (pl.sq[0] + sn.botleftsq.x) * SQUAREPIX + pl.pix[0] + sn.leftpad - 2;
        y1 = (pl.sq[1] + sn.botleftsq.y) * SQUAREPIX + pl.pix[1] + sn.bottompad - 2;

        // directional specific interpolate
        if (pl.cmd.dirmoving == DIR_UP || 
                pl.cmd.dirmoving == DIR_DOWN) {
            if ( x1 - x0 > 4 )
                x = x1;
            else
                x = one_minus_alpha * x0 + alpha * x1;
            y = y1;
        } else if (pl.cmd.dirmoving == DIR_LEFT || 
                pl.cmd.dirmoving == DIR_RIGHT) {
            if ( y1 - y0 > 4 )
                y = y1;
            else
                y = one_minus_alpha * y0 + alpha * y1;
            x = x1;
        } else {
            x = x1; y = y1;
        }

        // just interpolate for all directions
        //x = one_minus_alpha * x0 + alpha * x1;
        //y = one_minus_alpha * y0 + alpha * y1;

        pl.drawpix[0] = x;
        pl.drawpix[1] = y;

        /*
        C_WriteLog("msec: %d\n", (int)pe.drawtime);
        C_WriteLog("renderframe: %d\n", pe.renderframe);
        C_WriteLog("x0: %f\nx1: %f\nx: %f\nalpha %f\n\n", x0, x1, x, alpha);
        */
        

    } 
    else 
    {
    

        x = (pl.sq[0] + sn.botleftsq.x) * SQUAREPIX + pl.pix[0] + sn.leftpad - 2;
        y = (pl.sq[1] + sn.botleftsq.y) * SQUAREPIX + pl.pix[1] + sn.bottompad - 2;
        pl.drawpix[0] = x;
        pl.drawpix[1] = y;
    }

    switch (pl.dirFacing) {
    case DIR_UP:    facing = -90.0;     break;
    case DIR_DOWN:  facing = 90.0;      break;
    case DIR_RIGHT: facing = 180.0;     break;
    case DIR_LEFT:  default: facing = 0.0;       
    }


    img = P_SetAnimation(); 


    if (pl.dirFacing != DIR_LEFT) {
		//glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
		glTranslated (x+9,y+9, 0);
        glRotatef (facing, 0.0, 0.0, 1.0);
		glTranslated (-x-9, -y-9, 0);
    }

    GL_PutAlphaSprite(x, y, img->texhandle);

	if (pl.dirFacing != DIR_LEFT) {
        glPopMatrix();
	}

    err = glGetError();
    glColor4f(1.0, 1.0, 1.0, 0.5);
}

#if 0
void GL_DrawPlayer (void)
{
    image_t *img;
    vec2_t pix;


    glEnable (GL_ALPHA_TEST);
    glEnable (GL_DEPTH_TEST);
    glAlphaFunc (GL_GREATER, 0.0);


//    img = tp->anims[tp->currentanim].frames[(tp->anims[tp->currentanim].frame)];

//    pix[0] = sn.botleftpix.x + tp->sq[0] * SQUAREPIX + tp->pix[0] - 2;
//    pix[1] = sn.botleftpix.y + tp->sq[1] * SQUAREPIX + tp->pix[1] - 2;


    img = P_SetAnimation(); 


   	glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBindTexture(GL_TEXTURE_2D, img->texhandle);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0); 
    glVertex3i (pix[0],               pix[1],           0);
    glTexCoord2f(0.0, 1.0); 
    glVertex3i (pix[0]              , pix[1] + img->sz[1], 0);
    glTexCoord2f(1.0, 1.0); 
    glVertex3i (pix[0] + img->sz[0] , pix[1] + img->sz[1], 0);
    glTexCoord2f(1.0, 0.0); 
    glVertex3i (pix[0] + img->sz[0] , pix[1],           0);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    glFlush();
    

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    
} // GL_DrawPlayer
#endif /* erase this baggage */

    
    /*
    glEnable (GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
    glAlphaFunc (GL_GREATER, 0.0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, pacdude->texhandle);
    */

    /*
    glEnable(GL_BLEND);
    // dest = (Source-Alpha , Source-Alpha, Source-Alpha)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
    */

    /*
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3i(x, y, 0);
	glTexCoord2f(0.0, 1.0); glVertex3i(x, y + SQUAREPIX+4, 0);
	glTexCoord2f(1.0, 1.0); glVertex3i(x + SQUAREPIX+4, y + SQUAREPIX+4, 0);
	glTexCoord2f(1.0, 0.0); glVertex3i(x + SQUAREPIX+4, y, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    //glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    */

    

void GL_CompilePacdude (void)
{
    int i, max;
    byte *wasted;

    bytesprite_t *pacdude;

    pacdude = BS_getpacdude();

    wasted = V_malloc (pacdude->w * pacdude->h * 4, PU_STATIC, 0);
    memset (wasted, 0, sizeof(wasted));

    max = pacdude->w * pacdude->h; 

    for (i=0; i < max; i++)
    {
        if (pacdude->data[i])
        {
            wasted[4*i]   = (byte) pacdude->c[0] * 255.0;
            wasted[4*i+1] = (byte) pacdude->c[1] * 255.0;
            wasted[4*i+2] = (byte) pacdude->c[2] * 255.0;
            wasted[4*i+3] = 255;
        }
    }

    pacdude->compiled = wasted;


//    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &pacdude->texhandle);
    glBindTexture(GL_TEXTURE_2D, pacdude->texhandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, pacdude->w, 
                    pacdude->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                    pacdude->compiled);
    glDisable(GL_DEPTH_TEST);
}

// ds == dotswell
static struct {
    int now;
    int lastdraw_ms;
    int dx[8], dy[8];
    int frame;
    int maxframe;
    int delay;
    float center[8];
} ds;

gbool ds_init = gfalse;

static void R_DSInit ( void )
{
    ds_init = gtrue;
    ds.lastdraw_ms = I_CurrentMillisecond();
    ds.frame = 0;
    ds.maxframe = 8;
    ds.dx[0] = 0; ds.dy[0] = 0;
    ds.dx[1] = 3; ds.dy[1] = -3;
    ds.dx[2] = 6; ds.dy[2] = -6;
    ds.dx[3] = 3; ds.dy[3] = -3;
    ds.dx[4] = 0; ds.dy[4] = 0;
    ds.dx[5] = -3;  ds.dy[5] = 3;
    ds.dx[6] = -6; ds.dy[6] = 6;
    ds.dx[7] = -3; ds.dy[7] = 3;
    ds.delay = 66;
}

void R_SetDSFrame( void )
{
    int now;
    if ( !ds_init )
        R_DSInit();

    now = I_CurrentMillisecond();
    if (now - ds.lastdraw_ms > ds.delay) {
        ds.frame = (ds.frame+1) % ds.maxframe;
        ds.lastdraw_ms = now;
    }
}

void GL_DrawDots (void)
{
    int i, x, y, addx= 0, addy=0;
    vec2_t pix;
    float f;

    int draw = 0;

    glColor4f(1.0, 0.80, 0.70, 0.5);

    // animated large dots
    //R_SetDSFrame();

    for (i=0; i < MAX_X*MAX_Y; i++) 
    {
        if (!gameboard.index[i])
			continue;
		if (gameboard.index[i]->type != ST_OPEN)
			continue;

        x = gameboard.index[i]->x;
        y = gameboard.index[i]->y;

        switch (gameboard.dots[i]) { 
        case OT_BIG:
            /*
            // original
            pix[0] = sn.botleftpix.x + x * SQUAREPIX + 2;
            pix[1] = sn.botleftpix.y + y * SQUAREPIX + 2;

            addx = addy = 9;

            // 2nd try
            pix[0] = sn.botleftpix.x + x * SQUAREPIX + 2 - ds.dx[ds.frame] + ((float)ds.dx[ds.frame])/2.0;
            pix[1] = sn.botleftpix.y + y * SQUAREPIX + 2 - ds.dy[ds.frame] + ((float)ds.dy[ds.frame])/2.0;
            addx = 9 + ds.dx[ds.frame];
            addy = 9 + ds.dy[ds.frame];
            */

            pix[0] = sn.botleftpix.x + x * SQUAREPIX + 3;
            pix[1] = sn.botleftpix.y + y * SQUAREPIX + 3;
            addx = addy = 7;

            draw = 2;
            break;
        case OT_LITTLE:
            pix[0] = sn.botleftpix.x + x * SQUAREPIX + 5;
            pix[1] = sn.botleftpix.y + y * SQUAREPIX + 5;
            addx = 3;
            addy = 3;
            draw = 1;
            break;
        }

        f = 1.0;

        // big dot
        if (draw == 2) {
            glBegin(GL_QUADS);
            //for (x = -2; x < 3; x++) { 
            for (x = -1; x < 2; x++) { 
                if (!x) { 
                    if (I_CurrentMillisecond()%800 > 400)
                        continue;
                }
                glVertex3i (pix[0]+x*f,      pix[1]-x*f,        0);
                glVertex3i (pix[0]+x*f,      pix[1]+addy+x*f,   0);
                glVertex3i (pix[0]+addx-x*f, pix[1]+addy+x*f,   0);
                glVertex3i (pix[0]+addx-x*f, pix[1]-x*f,        0);
            }
            glEnd();
        }
        else if (draw) {
            glBegin(GL_QUADS);
            glVertex3i (pix[0],      pix[1],        0);
            glVertex3i (pix[0],      pix[1]+addy,   0);
            glVertex3i (pix[0]+addx, pix[1]+addy,   0);
            glVertex3i (pix[0]+addx, pix[1],        0);
            glEnd();
        }
        draw = 0;
    }
}

void GL_DrawTheBloodyGameboard (void)
{
    int i, x, y;
	
//	GLenum err = GL_NO_ERROR;
    uint secnum = EMPTY;
    bytesprite_t *cwall;
    vec2_t pix;

    for (i=0; i < MAX_X*MAX_Y; i++)
    {
		if (!gameboard.index[i])
			continue;
		if (gameboard.index[i]->type != ST_WALL)
			continue;

        x = gameboard.index[i]->x;
        y = gameboard.index[i]->y;
        pix[0] = sn.botleftpix.x + x * SQUAREPIX;
        pix[1] = sn.botleftpix.y + y * SQUAREPIX;

        secnum = S_GetSectorNumber(x, y);
        cwall = S_GetBytespriteFromSecnum(secnum);

        if (cwall)
        {
//            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//            glEnable(GL_DEPTH_TEST);
           	glEnable(GL_TEXTURE_2D);
   	        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	        glBindTexture(GL_TEXTURE_2D, cwall->texhandle);
            glBegin(GL_QUADS);

	        glTexCoord2f(0.0, 0.0); // bl
            glVertex3i (pix[0],           pix[1],           0);
            glTexCoord2f(0.0, 1.0); // tl
            glVertex3i (pix[0]          , pix[1]+SQUAREPIX, 0);
            glTexCoord2f(1.0, 1.0); // tr
            glVertex3i (pix[0]+SQUAREPIX, pix[1]+SQUAREPIX, 0);
	        glTexCoord2f(1.0, 0.0); // br
            glVertex3i (pix[0]+SQUAREPIX, pix[1],           0);

            glEnd();
            glDisable(GL_TEXTURE_2D);

			glFlush();
        }
    }
}


void GL_CompileGameboard_curved (void)
{
    int i, x, y;
	
	GLenum err = GL_NO_ERROR;
    uint secnum = EMPTY;
    bytesprite_t *cwall;
    vec2_t pix;

/*
    // compute botleft pix 
    sn.botleftpix.x = sn.leftpad   + sn.botleftsq.x * SQUAREPIX;
    sn.botleftpix.y = sn.bottompad + sn.botleftsq.y * SQUAREPIX;

    BS_compile_spritelist();
*/

    glPushAttrib(GL_CURRENT_BIT);

    // compile dl
    gameboard_dl = glGenLists(1);
    glNewList(gameboard_dl, GL_COMPILE);

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
//    glEnable (GL_DEPTH_TEST);
//    glEnable (GL_ALPHA_TEST);
//    glAlphaFunc (GL_GREATER, 0.0);

   	glEnable(GL_TEXTURE_2D);
   	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


    for (i=0; i < MAX_X*MAX_Y; i++)
    {
		if (!gameboard.index[i])
			continue;
		if (gameboard.index[i]->type != ST_WALL)
			continue;

        x = gameboard.index[i]->x;
        y = gameboard.index[i]->y;
        pix[0] = sn.botleftpix.x + x * SQUAREPIX;
        pix[1] = sn.botleftpix.y + y * SQUAREPIX;

        secnum = S_GetSectorNumber(x, y);
        cwall = S_GetBytespriteFromSecnum(secnum);

        if (cwall)
        {
   	        glEnable(GL_TEXTURE_2D);
   	        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	        glBindTexture(GL_TEXTURE_2D, cwall->texhandle);
            glBegin(GL_QUADS);

	        glTexCoord2f(0.0, 0.0); // bl
            glVertex3i (pix[0],           pix[1],           0);
            glTexCoord2f(0.0, 1.0); // tl
            glVertex3i (pix[0]          , pix[1]+SQUAREPIX, 0);
            glTexCoord2f(1.0, 1.0); // tr
            glVertex3i (pix[0]+SQUAREPIX, pix[1]+SQUAREPIX, 0);
	        glTexCoord2f(1.0, 0.0); // br
            glVertex3i (pix[0]+SQUAREPIX, pix[1],           0);

            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
    }
    glDisable(GL_TEXTURE_2D);
//	glDisable(GL_ALPHA_TEST);
//	glDisable(GL_DEPTH_TEST);
    glEndList();

    glPopAttrib();

	err = glGetError();
}

void GL_CompileGameboard (void)
{
    int i;
    int x, y;
    vec2_t pix;

    // compute botleft pix 
    sn.botleftpix.x = sn.leftpad   + sn.botleftsq.x * SQUAREPIX;
    sn.botleftpix.y = sn.bottompad + sn.botleftsq.y * SQUAREPIX;

    // compile dl
    gameboard_dl = glGenLists(1);
    glNewList(gameboard_dl, GL_COMPILE);

 	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1.0, 1.0, 1.0, 0.5);
    glBegin(GL_QUADS);

    for (i = 0; i < MAX_X*MAX_Y; i++)
    {
		if (!gameboard.index[i])
			continue;
        x = gameboard.index[i]->x;
        y = gameboard.index[i]->y;
        pix[0] = sn.botleftpix.x + x * SQUAREPIX;
        pix[1] = sn.botleftpix.y + y * SQUAREPIX;

        if (gameboard.index[i]->type == ST_WALL)
            glColor4f(1.0, 1.0, 1.0, 0.5);
        else if (gameboard.index[i]->type == ST_OPEN)
            glColor3f(0.0, 0.0, 0.0);
        else
            glColor3f(0.0, 0.0, 0.0);
    
        // bl
        glVertex3i (pix[0],           pix[1],           0);
        // br
        glVertex3i (pix[0]+SQUAREPIX, pix[1],           0);
        // tr
        glVertex3i (pix[0]+SQUAREPIX, pix[1]+SQUAREPIX, 0);
        // tl
        glVertex3i (pix[0]          , pix[1]+SQUAREPIX, 0);
    }

    glEnd();
    glEndList();
}

static gbool extra_life_posted = gfalse;
static gbool lose_life_posted = gfalse;
static int extra_life_timer = 0;
static int lose_life_timer = 0;

void R_PostLoseGuy (void)
{
    // turn off extra life blinking
    extra_life_posted = gfalse;
    lose_life_timer = I_CurrentMillisecond();
    lose_life_posted = gtrue;
}

void R_PostExtraLife (void)
{
    extra_life_timer = I_CurrentMillisecond();
    extra_life_posted = gtrue;
}

void GL_PaintExtraGuys (void)
{
    int x, y, i, tot;
    image_t *img;

    // turn off once expired
    if (extra_life_posted) {
        if (I_CurrentMillisecond() - extra_life_timer > 3000) {
            extra_life_posted = gfalse;
        }
    }

    if (lose_life_posted) {
        if (I_CurrentMillisecond() - lose_life_timer > 1000) {
            lose_life_posted = gfalse;
        }
    }

    x = 110;
    y = 0;

    img = P_GetGenericSprite();

    tot = pl.lives;
    if (extra_life_posted) 
        --tot;
    if (lose_life_posted)  
        ++tot;

    for (i = 1; i < tot; i++)
    {
        GL_PutAlphaSprite (x + i*20, y, img->texhandle);
    }

    if (extra_life_posted) {
        if ((I_CurrentMillisecond() % 300) > 149) 
            GL_PutAlphaSprite (x + i*20, y, img->texhandle);
    }
}


void GL_PaintScore (void)
{
    int hscore;
    uint white = 0xFFFFFFFF;
    //glColor4f(1.0, 1.0, 1.0, 0.5);

    hscore = C_GetHighScore();
    if (pl.score > hscore) 
        hscore = pl.score;

    F_Printf(200, 470, currentfont, white, "%d", pl.score);
    F_Printf(300, 470, currentfont, white, "High Score");   
    F_Printf(435, 470, currentfont, white, "%d", hscore);
}


/*
==================
INITIALIZATIONS
==================
*/
int GL_Init (void)										
{
    /// DEPRECATED
    //GL_CompileGameboard();    
    //GL_CompilePacdude();
    /// a good idea, but also deprecated
	//GL_CompileGameboard_curved();
	GLint err = GL_NO_ERROR;

	glShadeModel(GL_FLAT);							
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		

	glClearDepth(1.0f);						
	glEnable(GL_DEPTH_TEST);			
	glDepthFunc(GL_LEQUAL);			

	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    //glFrustum(left, right, bottom, top, near, far);
    //glFrustum(0, 640, 0, 480, 1.0, 10.0); 
    //glFrustum(-320.0, 320.0, -240.0, 240.0, 0.00001, 1000.0); 
    //glFrustum(-640, 640, -480, 480, 0.01, 10.0); 
    //glFrustum(0, 1600, 0, 1200, 1.0, 10.0); 

	//glViewport(0, 0, 640, 480);	
    //gluPerspective(fovy, w/h ratio, near, far)
    //gluPerspective(60, 640/480, -6.0, 20.0);
    // eye, focus-point, side-up
    //gluLookAt(0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	//glOrtho(0.0, SCREENWIDTH, 0.0, SCREENHEIGHT, -1.0, 1.0);
    
    GL_FRUSTUM();


    // always leave things the way you found them
	glMatrixMode(GL_MODELVIEW);
	//glRasterPos3i(0, 0, 0);

	//glLoadIdentity();

    // ?? .. elsewhere
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	err = glGetError();
	if (err != GL_NO_ERROR)
		C_WriteLog("gl error: %s \n", gluErrorString(err));

	return TRUE;		
}

gbool enable_rotation = gfalse;
float rotation = 0.0;

typedef struct {
    int start;
    gbool on;
    gbool finished;
    int now;
    int dur;
    float rot;
    int type; // 0 none, 1 cw-out, 2 ccw-in
    float velocity;
    vec3_t scale;
    float scalefac;
    int delay;
    int phase;
    uint thisFrame;
    int lastframe_ms;
    int msperframe;
} rot_t;
rot_t rot;


// helper function for R_SpinBoard
static void R_SwitchSpinModeLoadMap (void)
{
    rot.on = gtrue;
    rot.finished = gfalse;
    rot.type = 2;
    rot.scale[0] = 0.0;
    rot.scale[1] = 0.0;

    // load the new gameboard right in the middle
    G_LoadNextLevel ();
}

//#define RSCALEFAC 0.0092;
#define RSCALEFAC 0.00092;

// global
void R_SpinBoardIn (int delay, int duration)
{
    rot.start = I_CurrentMillisecond();
    rot.thisFrame = pe.logicframe;
    rot.on = gtrue;
    rot.finished = gfalse;
    rot.now = rot.start;
    rot.dur = duration + delay;
    rot.rot = 0.0;
    rot.type = 2;
    //rot.velocity = 6.0;
    rot.velocity = 0.6;
    rot.scale[0] = 0.0;
    rot.scale[1] = 0.0;
    rot.scale[2] = 0.0;
    rot.scalefac = RSCALEFAC;
    rot.delay = delay;
    //rot.msperframe = 10;
    rot.msperframe = 1;
    rot.lastframe_ms = rot.start + delay;
}

// global
void R_SpinBoardOut (int delay, int duration) 
{
    rot.start = I_CurrentMillisecond();
    rot.thisFrame = pe.logicframe;
    rot.on = gtrue;
    rot.finished = gfalse;
    rot.now = rot.start;
    rot.dur = duration + delay;
    rot.rot = 0.0;
    rot.type = 1;
    //rot.velocity = 6.0;
    rot.velocity = 0.6;
    rot.scale[0] = 1.0;
    rot.scale[1] = 1.0;
    rot.scale[2] = 0.0;
    rot.scalefac = RSCALEFAC;
    rot.delay = delay;
    rot.phase = duration / 2;
    //rot.msperframe = 10;
    rot.msperframe = 1;
    rot.lastframe_ms = rot.start + delay;
}

// local
static gbool R_SpinBoard (void)
{
    int counts;

    if (!rot.on) {
        return gfalse;
    }

    rot.now = I_CurrentMillisecond();
    if (rot.now - rot.start > rot.dur) {
        rot.finished = gtrue;
    }

    if (rot.finished) {
        rot.on = gfalse;
        return gfalse;
    }

    if (rot.now - rot.start < rot.delay) 
        return gtrue;

    // change spintype
    if (rot.type == 1) {
        if (rot.now - rot.start > rot.delay + rot.phase) {
            R_SwitchSpinModeLoadMap();
        }
    }

    // actual rotation
    glTranslated (320, 240, 0);
    glScalef (rot.scale[0], rot.scale[1], rot.scale[2]);
    glRotatef (rot.rot, 0.0, 0.0, 1.0);
    glTranslated (-320, -240, 0);

#if 0
    // only one rotation advancer per logicframe
    if ( pe.logicframe <= rot.thisFrame )
        return gfalse;
    counts = pe.logicframe - rot.thisFrame;
    rot.thisFrame = pe.logicframe;
#endif

    if ( rot.now <= rot.lastframe_ms )
        return gfalse;
    counts = (rot.now - rot.lastframe_ms) / rot.msperframe;
    if (counts <= 0)
        return gfalse;
    rot.lastframe_ms = rot.now;

    while (counts--) {
        if (rot.type == 1) {
            rot.rot += rot.velocity;
            rot.scale[0] -= rot.scalefac;
            rot.scale[1] -= rot.scalefac;
            if (rot.scale[0] <= 0)
                rot.scale[0] = 0.01;
            if (rot.scale[1] <= 0)
                rot.scale[1] = 0.01;
        } else if (rot.type == 2 ) {
            rot.rot -= rot.velocity;
            rot.scale[0] += rot.scalefac;
            rot.scale[1] += rot.scalefac;
            if (rot.scale[0] > 1.0)
                rot.scale[0] = 1.0;
            if (rot.scale[1] > 1.0)
                rot.scale[1] = 1.0;
        }
    }

    return gtrue;
}


void R_DrawBackground( void )
{
    image_t *img;
    GLenum err = GL_NO_ERROR;

    if (!(img = S_GetSplashImage()))
        return;

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, img->texhandle);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3i(0,   0,      -1.0);
	glTexCoord2f(0.0, 1.0); glVertex3i(0,   479,    -1.0);
	glTexCoord2f(1.0, 1.0); glVertex3i(639, 479,    -1.0);
	glTexCoord2f(1.0, 0.0); glVertex3i(639, 0,      -1.0);
    glEnd();

	err = glGetError();
	if (err != GL_NO_ERROR)
		C_WriteLog("GL Error: %s \n", gluErrorString(err));
}


/*
====================
Draw GL Scene - called in main loop
====================
*/
int GL_DrawScene(void)
{
    int before = 0, total = 0;


    glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLoadIdentity();
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1.0, 1.0, 1.0, 0.5);

//=====================
//	glMatrixMode(GL_MODELVIEW);	
//	glLoadIdentity();
 //   glTranslatef ( -320, -240, camera_z );

	// push the scene back to be inside the near clipping plane
    //glTranslatef ( 0.0, 0.0, -1.4 );

    //glTranslated (-320, -240, 0);

    /*
     * to spin with constant velocity must keep track of time 
     *
     * now = I_CurrentMillisecond();
            counts = (now - lastframe_ms) / msperframe;
            while(counts--) { axisrot+= 0.075; }
            lastframe_ms = now;
     */

//	glRotatef (axisrot ,		1.0, 1.0, 0.0);
//    axisrot += 0.075;
	
	//glTranslated (320, 240, 0);

//    gluLookAt( -0.5, 0.0, 0.0, 
//                0.0, 0.0, 0.0,
//                0.0, 1.0, 0.0 );   
//=====================


    // background
    R_DrawBackground();

    
#if 0
    
    useless

    // manual rotation, w/ 'r' key
    if (enable_rotation) {
        glTranslated (320, 240, 0);
        glRotatef (rotation, 0.0, 0.0, 1.0);
        rotation += 0.1;
        glTranslated (-320, -240, 0);
    } else {
        if (rotation > 0) {
            rotation--;
            if (rotation < 0)
                rotation = 0;
            glTranslated (320, 240, 0);
            glRotatef (rotation, 0.0, 0.0, 1.0);
            glTranslated (-320, -240, 0);
            rotation--;
            if (rotation < 0)
                rotation = 0;
        } else
            rotation = 0;
    }
#endif

    R_SpinBoard();
    
    // broken: cant get display lists working?
    //glCallList(gameboard_dl);    
	//GL_DrawTheBloodyGameboard();

    GL_ScaledDownGameboardDraw ();

    GL_DrawDots ();

    // draw the player
    GL_BlitPacdude();

    if (pe.drawPaths)
        GL_ThinkerPaths ();

    GL_PaintGhosts();

    GL_PaintExtraGuys ();

//before = I_CurrentMillisecond();
    GL_PaintScore ();
//total = I_CurrentMillisecond() - before;
//C_WriteLog("GL_PaintScore: %d\n", total);

    RunFloatingNumbers ();

    R_DrawPurgMsg ();

	glFlush();

    glPopAttrib();

	return TRUE;	
}




void R_InitGraphics (void)
{
    // starts the (windows) opengl system
    I_InitVideo ();

    //  setup the viewport once
    GL_SetupViewport ( SCREENWIDTH, SCREENHEIGHT );
    
    //  other gl initializations 
    GL_Init ();

    // compute bottom left pixel 
    sn.botleftpix.x = sn.leftpad   + sn.botleftsq.x * SQUAREPIX;
    sn.botleftpix.y = sn.bottompad + sn.botleftsq.y * SQUAREPIX;

    BS_compile_spritelist();
    T_InitThinkerGraphics ();
    P_SetupPlayerAnimations();
    GL_setup_gameboard ();
    S_LoadSplashData ();

	//
	// display list 3rd try
	gameboard_dl = glGenLists(1);
    glNewList(gameboard_dl, GL_COMPILE);
	GL_ScaledDownGameboardDraw ();
	glEndList();


    InitFloatingNumberList();
}


extern highscore_t hs;

#define TOTAL_MENU_COLORS 9
void R_DrawHighScores (void)
{
    gbool found = gfalse;
    int y; 
    int i;
    int lineheight;
    int xstart, ystart;
    scorelist_t *slist;
    uint cdef = 0x0000FFFF;
    uint white = 0xFFFFFFFF;
    uint black = 0x00000000;
    uint green = 0x00FF00FF;
    uint c;
    int f;
    static int current_color = 0;
    static int last_moment = -600;
    int new_color;
    uint ccycle[TOTAL_MENU_COLORS] = {  
        0x0000FFFF,
        0xFFFFFFFF,
        0xFF77FFFF,
        0x0077FFFF,
        0xFF7777FF,
        0xFF7700FF,
        0xFFFF00FF,
        0x997700FF,
        0x336699FF };
    static int corder[TOTAL_MENU_COLORS];
    int orderi;
    static gbool initialized = gfalse;

    // INIT
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLoadIdentity();
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1.0, 1.0, 1.0, 0.5);
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
    glTranslatef ( -320, -240, camera_z );

    ystart = 440;
    y = ystart;
    xstart = 200;
    lineheight = 24;
    c = cdef;
    f = 2;

    if (!initialized) {
        orderi = 0;
        while (orderi < TOTAL_MENU_COLORS) 
		{
GET_ANOTHER:
            new_color = getrand(TOTAL_MENU_COLORS);
            for (i = 0; i < orderi; i++) {
                if (corder[i] == new_color)
					goto GET_ANOTHER;
            }
            corder[orderi] = new_color;
            ++orderi;
        }
        initialized = gtrue;

        C_WriteLog("Menu ColorSet: ");
        for (i = 0; i < 8; i++)
            C_WriteLog("%d ", corder[i]);
        C_WriteLog("\n");
    }

    if (I_CurrentMillisecond() - last_moment >= 600) 
    {
        current_color = (++current_color) % TOTAL_MENU_COLORS;
        last_moment = I_CurrentMillisecond();
    }
    // comment this out to default to blue only
    cdef = ccycle[corder[current_color]];



    slist = C_GetScoreListP();

	glLoadIdentity();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    F_Printf(xstart, y, f, cdef, "High Scores");
    y -= lineheight;

    for (i = 1; i <= 12; i++)
    {
        if (i-1 == hs.rank) {
        //if (slist[i-1].score == pl.score && slist[i-1].score != 0 && !found) {
            found = gtrue;
            c = white;
        } else
            c = cdef;

        F_Printf(xstart, y, f, c, "%2d", i); 
        F_Printf(xstart + 50, y, f, c, "%3s", slist[i-1].initials);
        F_Printf(xstart + 150, y, f, c, "%d", slist[i-1].score);
        y -= lineheight;
    }
    c = cdef;
    c = green;
    y -= lineheight;
    F_Printf(xstart, y, f, c, "RET to start"); 
    y -= lineheight;
    F_Printf(xstart, y, f, c, "Q to quit");
    y -= lineheight;
    F_Printf(xstart, y, f, c, "ESC for menu");

    glFlush ();
    glPopAttrib();
}


void R_GetInitials (void)
{
    int x, y, xstart, ystart, lineheight, c, font, i;
    uint white = 0xFFFFFFFF;

    // init
    ystart = 330;
    xstart = 200;

    lineheight = 24;
    c = 0x0000FFFF;
    font = 4;

    x = xstart;
    y = ystart;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (hs.finished)
    {
        y = 264;
        for (i = 0; i <= 2; i++) {
            F_Printf(x, y, 2, white, "%c", hs.initials[i]);
            x += 25;
        }
        x = xstart;

        y = 330;
        F_Printf(x, y, 2, c, "Esc or Back to change");
        y = 200; 
        F_Printf(x, y, 2, c, "time remaining");
        F_Printf(x+200, y, 2, white, "%d", ((hs.dur - (I_CurrentMillisecond() - hs.start)) / 1000) );
        return;
    }

    F_Printf(x, y, 2, c, "Enter your Initials");

    y = 200; 
    F_Printf(x, y, 2, c, "time remaining");
    F_Printf(x+200, y, 2, white, "%d", ((hs.dur - (I_CurrentMillisecond() - hs.start)) / 1000) );

    y = 176;
    F_Printf(x, y, 2, c, "Press Space for more time");

    y = 264;
    for (i = 0; i <= 2; i++) {
        if (i == hs.index) {
            if ((I_CurrentMillisecond() % 300) > 100)
                F_Printf(x, y, 2, white, "%c", hs.initials[hs.index]);
        } else {
            F_Printf(x, y, 2, c, "%c", hs.initials[i]);
        }
        x += 25;
    }
}

/*
==================== 
 R_PostPurgatoryMsg
==================== 
*/
int purg_msg_start = 0;
gbool purg_msg_on = gfalse;
void R_PostPurgatoryMsg (void)
{
    purg_msg_on = gtrue;
    purg_msg_start = I_CurrentMillisecond();
}

void R_DrawPurgMsg (void)
{
    uint c, red = 0xff0000ff;
    int x;
    int dur;
    static int modea = 0; 
    static int modeb = 0; 

    dur = I_CurrentMillisecond() - purg_msg_start;
    if (dur > 13000) 
        purg_msg_on = gfalse;
    
    if (!purg_msg_on)
        return;

    if (dur < 6000) {
        modeb = 0;
        if (modea == 0) 
            modea = I_CurrentMillisecond();
        if (((I_CurrentMillisecond()-modea) % 1000) < 700) {
            x = 240;
            F_Printf(x, 240, 2, red, "u");
            F_Printf((x+=26), 240, 2, red, "r");
            F_Printf((x+=26), 240, 2, red, "cheat");
            F_Printf((x+=74), 240, 2, red, "too");
            F_Printf((x+=56), 240, 2, red, "much!");
        }
    } else if (dur > 7000) {
        modea = 0;
        if (modeb == 0) 
            modeb = I_CurrentMillisecond();
        if (((I_CurrentMillisecond()-modeb) % 1000) < 700) {
            x = 240;
            F_Printf(x, 240, 2, red, "welcome");
            F_Printf((x+=104), 240, 2, red, "to");
            F_Printf((x+=42), 240, 2, red, "jail!");
        }
    } 
}


 
/*
====================  
 R_RenderGame 
  - takes available information and renders a frame
====================  
*/
extern gamestate_t gamestate;

int intermission_start = 0;

void R_RenderFrame (void)
{

    switch (gamestate) {
    case GS_INTERMISSION:
        if (!intermission_start)
            intermission_start = I_CurrentMillisecond();
        R_DrawHighScores();
        break;
    case GS_HIGHSCORE:
        R_GetInitials();
        break;

    default:
        if (intermission_start)
            intermission_start = 0;
	    GL_DrawScene();
    }
}


