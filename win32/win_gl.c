/*
 * win_gl - windows opengl 
 */

#pragma warning ( disable : 4244 4668 4820 4255 4706 )

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// block deprecated libc warnings
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <windows.h>		
#include <io.h>
#include <gl\gl.h>			
#include <gl\glu.h>			

#include "../common.h"
#include "../virtmem.h"
#include "../pacman.h"

#include "win_global.h"

#include "../resource.h"    // IDI_ICON1

#define WGL_LOGFILE_NAME "pacman.log"
#define WGL_WINDOW_NAME "cheepo pacman"
#define WGL_WINDOW_CLASS_NAME "OpenGL"


wgldriver_t gld = wgldriver_default;



/*
====================
  WGL_OpenLogfile
   

====================
*/
void WGL_OpenLogfile (void)
{
    FILE *fp;

    // logfile
    gld.logfilename = V_malloc(16, PU_STATIC, 0);
    C_strncpy ( gld.logfilename, WGL_LOGFILE_NAME, 16 ); 
    if ((fp = fopen(gld.logfilename, "a")) == NULL) {
        sys_error_warn ("couldn't open logfile for writing");
        return;
    }
    gld.logfile_p = fp;
}

void WGL_WriteLog (char *msg)
{
    if (gld.logfile_p)
        fprintf (gld.logfile_p, "%s\n", msg);
}



/*
==================== 
  WGL_KillGLWindow

==================== 
*/

void WGL_KillWindow(void)		
{
    
	if (gld.fullscreen)			
	{
		ChangeDisplaySettings(NULL,0);					
		ShowCursor(TRUE);
	}

	if (gld.hGLRC)									
	{
		if (!wglMakeCurrent(NULL,NULL))		
		{
            WGL_WriteLog("Release Of DC And RC Failed.");
		}

		if (!wglDeleteContext( gld.hGLRC ))
		{
			WGL_WriteLog("Release Rendering Context Failed.");
		}
		gld.hGLRC=NULL;
	}

	if (gld.hDC && !ReleaseDC( gld.hWnd, gld.hDC ))
	{
		WGL_WriteLog("Release Device Context Failed.");
		gld.hDC=NULL;
	}

	if (gld.hWnd && !DestroyWindow( gld.hWnd ))
	{
		WGL_WriteLog("Could Not Release hWnd.");
		gld.hWnd=NULL;
	}

	if (!UnregisterClass( "OpenGL", gld.hInstance ))
	{
		WGL_WriteLog("Could Not Unregister Class.");
		gld.hInstance=NULL;	
	}

	if (gld.logfile_p)
		fclose (gld.logfile_p);
}


/*
====================
  WGL_CreatePFD
   - create pixelformatdescriptor
   - seen in quake3 gpl src
   - looks like they copied it from the same place that I first saw it
====================
*/
static void WGL_CreatePFD ( PIXELFORMATDESCRIPTOR *pPFD, int colorbits, int depthbits, int stencilbits )
{
    PIXELFORMATDESCRIPTOR src = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		COLORBITS,								// 24-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		24,								// 24-bit z-buffer	
		8,								// 8-bit stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
    };

	src.cColorBits      = (BYTE) colorbits;
	src.cDepthBits      = (BYTE) depthbits;
	src.cStencilBits    = (BYTE) stencilbits;

	*pPFD = src;
}


static gbool WGL_InitContext ( int colorbits)
{
    int depthbits, stencilbits;
    static PIXELFORMATDESCRIPTOR pfd;

    if ( gld.hDC == NULL )
    {
        if ( ( gld.hDC = GetDC ( gld.hWnd ) ) == NULL )
        {
            return gfalse;
        }
    }

    if ( colorbits == 0 )
    {
        colorbits = COLORBITS;
    }

    if ( colorbits > 16 )
        depthbits = 24;
    else
        depthbits = 16;

    if ( depthbits < 24 )
        stencilbits = 0;
    else
        // FIXME: not sure what's good bitamount
        stencilbits = 8;

    if ( !gld.pixelFormatSet )
    {
		WGL_CreatePFD ( &pfd, colorbits, depthbits, stencilbits );
        gld.pixelFormatSet = gtrue;
        gld.pfd = &pfd;
    } 

    return gtrue;
}

static int WGL_InitGLSubsystem ( void )
{
	uint pixelFormat;			

	if (!(gld.hDC = GetDC( gld.hWnd )))	
	{
		WGL_KillWindow();	
		sys_error_warn("Can't Create A GL Device Context.");
		return -4;
	}

	if (!(pixelFormat=ChoosePixelFormat( gld.hDC, gld.pfd )))
	{
		WGL_KillWindow();
		sys_error_warn("Can't Find A Suitable PixelFormat.");
		return -5;
	}

	if(!SetPixelFormat( gld.hDC, pixelFormat, gld.pfd ))
	{
		WGL_KillWindow();
		sys_error_warn("Can't Set The PixelFormat.");
		return -6;
	}

	if (!(gld.hGLRC = wglCreateContext( gld.hDC )))
	{
		WGL_KillWindow();		
		sys_error_warn("Can't Create A GL Rendering Context.");
		return -7;
	}

	if(!wglMakeCurrent( gld.hDC, gld.hGLRC ))
	{
		WGL_KillWindow();		
		sys_error_warn("Can't Activate The GL Rendering Context.");
		return -8;	
	}

    return 0;
}

//LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WGL_CreateWindow (char* title, int width, int height, 
                        int colorbits, gbool fullscreenflag)
{
	WNDCLASS	wc;						
	DWORD		dwExStyle;				
	DWORD		dwStyle;			
	RECT		WindowRect;		

	HINSTANCE	hInstance;
	DEVMODE dmScreenSettings;


	WindowRect.left   = (long)0;
	WindowRect.right  = (long)width;
	WindowRect.top    = (long)0;
	WindowRect.bottom = (long)height;
    
	gld.fullscreen      = fullscreenflag;

//	if (! gld.hInstance)
		gld.hInstance	= GetModuleHandle(NULL);

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= gld.WndProc;

	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= gld.hInstance;

    // FIXME : load your own icon, arrow
	//wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hIcon			= LoadIcon(gld.hInstance, MAKEINTRESOURCE(IDI_ICON1) );
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);

    // 
	wc.hbrBackground	= NULL;	
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= WGL_WINDOW_CLASS_NAME;

    if ( !gld.classRegistered ) 
    {
	    if (!RegisterClass(&wc))
	    {
		    sys_error_warn("Failed To Register The Window Class.");
		    return -1;
	    } 
        else
            gld.classRegistered = gtrue;
    }

	
	if (gld.fullscreen)
	{
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= width;
		dmScreenSettings.dmPelsHeight	= height;
		dmScreenSettings.dmBitsPerPel	= colorbits;
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","Cheepo Pacman",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				gld.fullscreen = gfalse;
			}
			else
			{
			    sys_error_warn("Program Will Now Close.");
				return -2;
			}
		}
	}

    // setup for fullscreen or window
	if (gld.fullscreen)			
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;		
		ShowCursor(FALSE);	
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);


    if ( !gld.hWnd )
    {
	    // Create The actual Window
        gld.hWnd=CreateWindowEx(	
                    dwExStyle,					
                    WGL_WINDOW_CLASS_NAME,
                    WGL_WINDOW_NAME,				
                    dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                    // TODO: double check on how these coordinates affect
                    //  the window placement
                    // TODO: erase this comment
                    0, 0,		
                    WindowRect.right  - WindowRect.left,	
                    WindowRect.bottom - WindowRect.top,
                    NULL,							
                    NULL,						
                    gld.hInstance,	
                    NULL);

	    if (! gld.hWnd )
	    {
		    WGL_KillWindow();	
		    sys_error_warn("Window Creation Error.");
		    return -3;
	    }
    }


    // setup PFD
    WGL_InitContext ( colorbits );

    WGL_InitGLSubsystem ();


	ShowWindow( gld.hWnd, SW_SHOW );
	SetForegroundWindow( gld.hWnd );
	SetFocus( gld.hWnd ) ;			

	return 0;	
}

