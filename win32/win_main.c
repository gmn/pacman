
/*
 * win_main.c - entry point
 */

#pragma warning ( disable : 4244 4668 4820 4255 )

#include <windows.h>

#include "../pacman.h"
#include "win_global.h"

extern wgldriver_t gld;


int WINAPI WinMain (HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance,
                    PSTR szCmdLine,
                    int iCmdShow )
{
    int rv;

    gld.hInstance = hInstance;
	
    rv = Pacman_Main ( szCmdLine );

    return rv;
}


