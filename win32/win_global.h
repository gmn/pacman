//
// global win32 system exports
//
#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>


#include "../common.h" 


#ifndef __WIN_GLOBAL_H__
#define __WIN_GLOBAL_H__ 1

#define COLORBITS 16

typedef struct {
    char name[32];
    int handle;
} texhandle_t;

typedef struct 
{
    gbool fullscreen;
    int width;
    int height;

    HDC	    	hDC;		// Private GDI Device Context
    HGLRC		hGLRC;		// Permanent Rendering Context
    HWND		hWnd;		// Holds Our Window Handle

    texhandle_t *texhandles;
    FILE        *logfile_p;
    char        *logfilename;

    void *      WndProc;
    PIXELFORMATDESCRIPTOR *pfd;
    gbool       pixelFormatSet;

    HINSTANCE	hInstance;		// Holds The Instance Of The Application

    gbool       classRegistered;

	HINSTANCE	hinstOpenGL;
} wgldriver_t;

#define wgldriver_default { gfalse, SCREENWIDTH, SCREENHEIGHT, \
                            NULL, NULL, NULL, \
                            NULL, NULL, NULL, \
                            NULL, NULL, gfalse, \
                            NULL, gfalse }

#endif




// win_system.c
void win_InitVideo (void);
void win_ShutdownVideo (void);
void win_CheckEventQueue (void);
int  win_GetKey (int);
void win_ToggleFullscreen (int , int);
void win_Refresh (void);
void win_SetFullscreen (gbool);

// win_gl.c
void WGL_OpenLogfile (void);
void WGL_WriteLog (char *);
void WGL_KillWindow (void);
int  WGL_CreateWindow (char* , int , int , int , gbool);





