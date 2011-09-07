//
// i_system.c
//
// API specific abstraction layer
//

#pragma warning ( disable : 4244 4668 4820 4255 4711 )

#if defined (WIN32) || defined (_WIN32)
# include <windows.h>
# include <mmsystem.h>   // for timeGetTime. 
# include <io.h>
#else
# include <unistd.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>	// EXIT_SUCCESS	EXIT_FAILURE	

#if defined (WIN32) || defined (_WIN32)
# include <process.h>  	// exit
# include "win32/win_global.h" 
#endif

#include <time.h>

#include "common.h" 	// byte, FALSE, gbool, LOGIC_FPS
#include "event.h"   // key definitions & calls


#define MEGABYTES_USED 16


byte *I_ZoneBase (int *size)
{
	*size = MEGABYTES_USED * 1024 * 1024;
	return (byte *) malloc (*size);
}

void I_FreeZoneBase (byte *tobefreed)
{
	free (tobefreed);
}




// holds local timer info
static int starttime_ms = 0;
static int currenttime_ms;
static gbool initialized = gfalse;

void I_InitTimers (void)
{
	if (!initialized) {
        if (timeBeginPeriod(1) == TIMERR_NOCANDO)
            C_WriteLog("err init sys timer\n");
		starttime_ms = timeGetTime();
		initialized = gtrue;
	}
}

int I_GetStartMillisecond (void)
{
    if (!initialized)
        return -1;

    return starttime_ms;
}


#if defined (WIN32) || defined (_WIN32)
int I_CurrentMillisecond (void)
{
	currenttime_ms = timeGetTime() - starttime_ms;

	return currenttime_ms;
}

/*
====================
 I_GetTicks
 - there is one tick for each frame of gameplay,
====================
*/
int I_GetTicks (void)
{
    return (timeGetTime()-starttime_ms)*LOGIC_FPS/1000;
}

char *I_GetTimeStamp (void) 
{
    time_t tt;
    tt = time(0);
    return asctime(localtime(&tt));
}



int I_FileSize (const char *filename)
{
    int sz;
	struct _finddata_t c_file;
    intptr_t hFile;

    if ( (hFile = _findfirst (filename, &c_file )) == -1L )
        return -1;
    sz = c_file.size;
    _findclose(hFile);

    return sz;
}

int I_WinMsgBox ( char *msg, char *title )
{
    return MessageBox (NULL, msg, title, MB_OK|MB_ICONEXCLAMATION);
}

int I_WinConfirmBox (char *msg, char *title)
{
    if (MessageBox(NULL, msg, title, MB_YESNO|MB_ICONQUESTION) == IDYES)
        return 1;

    return 0;
}
#endif


void I_SysQuit (void)
{
    timeEndPeriod(1);
	exit (EXIT_SUCCESS);
}


//
// interfaces for system specifics
//

/*
==================== 
==================== 
*/
void I_InitVideo (void)
{
#if (defined (_WIN32) || defined(WIN32)) && !defined(__CONSOLE_APP__)
    win_InitVideo();
#endif
}


/*
==================== 
==================== 
*/
void I_ShutdownVideo (void)
{
#if (defined (_WIN32) || defined(WIN32)) && !defined(__CONSOLE_APP__)
    win_ShutdownVideo ();
#endif
}

/*
==================== 
==================== 
*/
void I_CheckEventQueue (void)
{
#if (defined (_WIN32) || defined(WIN32)) && !defined(__CONSOLE_APP__)
    win_CheckEventQueue();
#endif
}

/*
==================== 
==================== 
*/
int I_GetKey (int k)
{
	int rc = -1;

//#if defined (WIN32) || defined (_WIN32)
#if (defined (_WIN32) || defined(WIN32)) && !defined(__CONSOLE_APP__)
    rc = win_GetKey (k);
#endif

	return rc;
}

/*
==================== 
for use before video is initialized
==================== 
*/
void I_SetFullscreen (gbool fullscreenflag) {
#if (defined (_WIN32) || defined(WIN32)) && !defined(__CONSOLE_APP__)
    win_SetFullscreen(fullscreenflag);
#endif
}

/*
==================== 

- for use after video is initialized
==================== 
*/
void I_ToggleFullscreen (int w, int h)
{
    // silence compiler
    w = w + h;
    h = w + h;
}

/*
==================== 
 I_Refresh
==================== 
*/
void I_Refresh (void)
{
//#if defined (_WIN32) || defined (WIN32)
#if (defined (_WIN32) || defined(WIN32)) && !defined(__CONSOLE_APP__)
    win_Refresh();
#endif
}

void I_DoSounds (void)
{
}

void gsleep (int millisec)
{
    Sleep(millisec);
}
