/***********************************************************************
*  pacman.h
\***********************************************************************/

#pragma warning ( disable : 4244 4668 4820 4255 4711 )

#if !defined(__UINT__) || !defined(__GBOOL__)
# include "common.h"
#endif


// common.c
const char ** M_CheckParm(char *);

void sys_error_fatal (char *fmt, ...);
void sys_error_warn (char *fmt, ...);
void sys_shutdown_safe (void);
void sys_shutdown_confirm (void);

const char * C_GetExeName (void);
int ParseWinMainArgs(const char *);
int sys_file_size (char *);

int C_strncpy (char *, const char *, int);
int C_snprintf (char *, int, char *, ...);
int C_strncasecmp (const char *, const char *, int);
int C_strncmp (const char *, const char *, int);

void C_OpenLogFile (const char *);
void C_WriteLog (const char *, ...);
void C_CloseLog (void);
void C_Srand (void);
int  C_Rand (int);

void C_LoadScoreFile (void);
int C_AddGameScore (int, const char *);
int  C_GetHighScore (void);
void C_WriteScoreFile (void);


// pacman.c
int Pacman_Main (const char *);

// menu.c
void M_Ticker (void);
gbool M_InMenu (void);

// player.c
void P_PlayerDefaults (void);
void P_PlayerThink (void);
void P_Ticker (void);
int P_GetScore (void);
void P_ResetPlayerCoordinates (void);
void P_ResetPlayerDraws (void);
void P_DotsRedraw (void);
void P_GobbleDots(void);
void P_SetupPlayerAnimations(void);


// render.c
void GL_ResizeScene (int, int);
int  GL_InitGL (void);
int  GL_DrawScene (void);
void R_RenderFrame (void);
void R_InitGraphics (void);
void R_PostFloatingText (const char *text, int len);
void R_DrawHighScores (void);
void GL_setup_gameboard (void);
void R_PostExtraLife (void);
void R_PostPurgatoryMsg (void);
void R_DrawPurgMsg (void);
void R_PostLoseGuy (void);
void R_SpinBoardOut (int,int);
void R_SpinBoardIn (int,int);


// game.c
void G_Ticker (void);
void G_StartNewGame (void);
void G_CheckGameState (void);

gbool G_CheckPause (void);
void G_SetPause(int);
void G_UnsetPause(void);
void G_DoPunishmentMode (void);
void G_LoadNextLevel (void);
void G_InstantLoadLevel (void);

// gameboard.c
void readGameBoard (char *);

// event.c
void E_ProcessEvents (void);
void E_LoadDefaults (void);
void E_ResetMoveKeys (void);

// bytesprite.c
void BS_compile_spritelist (void);

// sector.c
// uint S_GetSectorNumber (int, int);

// image.h

// thinker.c
void T_Ticker (void);
void T_InitThinkers (void);
void T_InitThinkerData (void);
void T_InitThinkerGraphics (void);
void T_ResetThinkerCoordinates (void);
void T_ResetDraws (void);
void T_SetGhostSpeeds (int);
int  T_GetNumEaten (void);
void T_StartChaseMode (void);
void T_ResetThinkerDirections(void);
void T_ResetThinkers (void);
void T_IncGhostChasedDuration(int);
void T_RebuildThinkers (void);

// random.c
int getrand (int);

// font.c
void F_InitFreetype (void);
void F_Printf (int, int, int, uint, const char *, ...);

// pathfind.c
void PF_InitPathFind (void);

// splash.c
void S_LoadSplashData( void );
void S_ToggleSlideShow( void );
void S_NextImage( void );

