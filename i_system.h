
#include "common.h" //byte

byte *I_ZoneBase (int *);
void I_FreeZoneBase (byte *);

void I_InitTimers (void);
int I_GetStartMillisecond (void);
int I_CurrentMillisecond (void);
int I_GetTicks (void);

char *I_GetTimeStamp(void);

void I_SysQuit (void);

int I_FileSize (const char *);
int I_WinMsgBox (char *, char *);
int I_WinConfirmBox (char *, char *);


/*
#if defined (WIN32) || defined (_WIN32)
# include "win32/win_global.h" 
#endif
*/


void I_InitVideo (void);
void I_ShutdownVideo (void);
void I_CheckEventQueue (void);
void I_GetKeysym (void);
void I_ToggleFullscreen (int w, int h);
void I_Refresh (void);
void I_DoSounds (void);

void gsleep (int);

void I_SetFullscreen(gbool);
