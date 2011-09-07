/**********************************************************************
 * bytesprite.c - bytesprite specific routines
 *********************************************************************/

#pragma warning ( disable : 4244 4668 4820 4255 )

#ifdef _WIN32
# include <windows.h>
#endif
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include "bytesprite.h"

// bytesprite_data sprites will be local to this file and can
//  be externed from here
#include "bytesprite_data.h"
#include "virtmem.h"
#include "pacman.h" // C_WriteLog
#include "gameboard.h"

extern gameboard_t gameboard;

bytesprite_t * BS_getpacdude (void)
{
    return &pacdude;
}

extern int total_gl_tex;

void BS_gl_compile_rgba (bytesprite_t *bs)
{
    int i, row, col, max, max4;
    GLenum err = GL_NO_ERROR;

    max = bs->h * bs->w;
    max4 = max * 4;

    if (!bs->compiled) {
        //if (bs->compiled)
        //    V_free(bs->compiled);
        bs->compiled = V_malloc (max4, PU_STATIC, 0);
    }

    memset (bs->compiled, 0, max4);

    // * data is stored top->bottom & left->right
    // * bs->compiled is stored for opengl: bottom->top & left->right 
     
    for (i = 0; i < max; i++)
    {
        if (bs->data[i])
        {
            row = max4 - ( (i / bs->w + 1) * bs->w * 4 );  
            col = (i % bs->w) * 4;
			if (row+col+3 > max4) {
				sys_error_fatal("found accessing illegal memory in %s line %d", __FILE__, __LINE__);
			}
            bs->compiled[row+col  ] = (byte) (bs->c[0] * 255.0);
            bs->compiled[row+col+1] = (byte) (bs->c[1] * 255.0);
            bs->compiled[row+col+2] = (byte) (bs->c[2] * 255.0);
            bs->compiled[row+col+3] = 255;
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &bs->texhandle);
    glBindTexture(GL_TEXTURE_2D, bs->texhandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, bs->w, bs->h, 
                  0, GL_RGBA, GL_UNSIGNED_BYTE, bs->compiled);

	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		C_WriteLog("OPENGL ERROR: %s texnum: %d\n", gluErrorString(err), total_gl_tex);
	} 
}


void BS_create_displaylist (void *bs)
{
    // bogus code
    // compiler shushing
    int *n = (int *) bs;
    *n = 1;
}

void BS_compile_bitarray (void *bs, bitarray_t *ba)
{
    // bogus code
    // compiler shushing
    int *n = (int *) bs;
    *n = 1;
    ba->data = n;
}


void BS_compile_spritelist (void)
{
    int i = 0;
    static gbool gameboard_compiled = gfalse;
    
    
    i = (gameboard_compiled) ? 1 : 0;

    for (; i < totalsprites; i++)
    {
        // pacman is 0, never set his color from the gameboard
        if (i != 0) {
            spritelist[i].bytesprite->c[0] = gameboard.defaultcolorv[0];
            spritelist[i].bytesprite->c[1] = gameboard.defaultcolorv[1];
            spritelist[i].bytesprite->c[2] = gameboard.defaultcolorv[2];
        }
        BS_gl_compile_rgba (spritelist[i].bytesprite);
    }

    gameboard_compiled = gtrue;
}


