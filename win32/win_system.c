
// these 3 in quake win32 code
#pragma warning (disable : 4113 4133 4047 )
#pragma warning ( disable : 4244 4668 4820 4255 )

#include <windows.h>
#include <io.h>
#include "../event.h"
#include "../common.h"
#include "../pacman.h"
#include "win_global.h"


extern wgldriver_t gld;


int ( WINAPI * gwglSwapIntervalEXT)( int interval ) = 0;
PROC  ( WINAPI * gwglGetProcAddress)(LPCSTR) = 0;
#	define GPA( a ) GetProcAddress( gld.hinstOpenGL, a )


// 
// prototypes
// ---------------should never resize---------------
//void GL_ResizeScene (GLsizei width, GLsizei height);



void win_SetFullscreen(gbool fullscreenflag) 
{
    gld.fullscreen = fullscreenflag;
}

void win_ShutdownVideo (void)
{
	WGL_KillWindow();	
}

#define MAX_MSG_TWEEK 3

// FIXME: is this right?
void win_CheckEventQueue (void)
{
    int i;
    MSG msg;

    i = 0;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && i++ < MAX_MSG_TWEEK)	
	{
	    if (msg.message==WM_QUIT)				
		{
            //WGL_KillWindow();
            sys_shutdown_safe ();
		}
		else									
		{
            // process Window Messages
			TranslateMessage(&msg);				
			DispatchMessage(&msg);				
		}
	}
}

// FIXME: needs work
int win_GetKey (int vk)
{
    // 'A'-'Z' or '0'-'9'
    if (vk > 47 && vk < 91)
        if (vk < 58 || vk > 64)
            return vk;

    // F1 - F12
    if (vk < 0x7C && vk > 0x6F)
        return KEY_F1 + (vk - 0x70);

    switch (vk) {
    case VK_BACK:
        return KEY_BACKSPACE;
    case VK_TAB:
        return KEY_TAB;
    case VK_RETURN:
        return KEY_RETURN;
    case VK_ESCAPE:
        return KEY_ESCAPE;
    case VK_SPACE:
        return KEY_SPACEBAR;
    case VK_LEFT:
        return KEY_LEFTARROW;
    case VK_UP:
        return KEY_UPARROW;
    case VK_RIGHT:
        return KEY_RIGHTARROW;
    case VK_DOWN:
        return KEY_DOWNARROW;
    case VK_DELETE:
        return KEY_DELETE;
    case VK_LSHIFT:
        return KEY_LSHIFT;
    case VK_RSHIFT:
        return KEY_RSHIFT;
    case VK_LCONTROL:
        return KEY_LCONTROL;
    case VK_RCONTROL:
        return KEY_RCONTROL;
    }

    return KEY_NULLKEY;
}
/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39) (48-57)
 * 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A) (65-90)
 */
/*
#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F

#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
*/

void win_ToggleFullscreen (int w, int h)
{
}

// dont need, glFlush called in render
//  ?? is this true ??
void win_Refresh (void)
{
    if (gwglSwapIntervalEXT)
        gwglSwapIntervalEXT( 0 );
	SwapBuffers (gld.hDC);
}


/*
====================
WndProc 

- Callback for window
====================
*/
LRESULT CALLBACK WndProc(	HWND	hWnd,
							UINT	uMsg,
							WPARAM	wParam,
							LPARAM	lParam)
{
    event_t this_ev;

	switch (uMsg)						
	{
        case WM_CREATE:
        {
        /*    if ( !gld.hWnd )
                gld.hWnd = hWnd;  */
            return 0;
        }
		case WM_ACTIVATE:			
		{
            // ?
			return 0;		
		}

		case WM_SYSCOMMAND:	
		{
			switch (wParam)	
			{
				case SC_SCREENSAVE:	
				case SC_MONITORPOWER:	
				return 0;		
			}
			break;		
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
            sys_shutdown_safe ();
			return 0;		
		}

		case WM_KEYDOWN:
		{
            this_ev.type  = ev_keydown;
            this_ev.data1 = win_GetKey (wParam);
            E_PostEvent(&this_ev);
			return 0;			
		}

		case WM_KEYUP:		
		{
            this_ev.type  = ev_keyup;
            this_ev.data1 = win_GetKey (wParam);
            E_PostEvent(&this_ev);
			return 0;			
		}

		case WM_SIZE:		
		{
			//GL_ResizeScene(LOWORD(lParam),HIWORD(lParam));  
			return 0;		
		}
	}

	// pass unhandled messages To DefWindowProc
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

void win_InitVideo (void)
{
    int c = 0;

	if ( ( gld.hinstOpenGL = LoadLibrary( "opengl32" ) ) != 0 )
	    gwglGetProcAddress = GPA( "wglGetProcAddress" );


    //gld.WndProc = (WNDPROC) WndProc;
	gld.WndProc = WndProc;

//    WGL_OpenLogfile ();

    if ((c=WGL_CreateWindow( "cheepo Pacman", 640, 480, COLORBITS, gld.fullscreen )) < 0)
        sys_error_fatal( "unable to create GL Window, code %d", c );

    if (gwglGetProcAddress)
        gwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) gwglGetProcAddress( "wglSwapIntervalEXT" );

}
