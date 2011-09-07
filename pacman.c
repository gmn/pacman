/*
============================================================
 pacman.c
============================================================
*/

#pragma warning ( disable : 4244 4668 4820 4255 )

#include <assert.h>

#include <math.h>
#include "common.h"
#include "event.h"
#include "i_system.h"
#include "virtmem.h"
#include "gameboard.h"
#include "ticcmd.h"
#include "pacman.h"
#include "player.h"

#include "tetris.h"


extern player_t pl;
pacengine_t pe;
drawinfo_t di;

#define DEFAULT_MAP MAPDIR"/pacman1.map"



static void P_Display (void)
{
    float fractional_frames;
    int discreet_frames;

    // haven't gotten a new logic frame yet, dont update
    if (pe.lock_rfps_to_logic && pe.logicframe == di.lastLogicFrameDrawn)
        return;

    di.thisFrameMsec = I_CurrentMillisecond();

    fractional_frames = (di.thisFrameMsec - di.lastFrameMsec)/pe.msecPerRenderFrame + di.leftOverFrames;

    discreet_frames = floor ( fractional_frames );

    if ( pe.render_cap_to_fps ) {
        if ( pe.capfps_turnedOn ) {
            di.leftOverFrames = 1.0;
            pe.capfps_turnedOn = gfalse;
            fractional_frames = 100;
            pe.render_newframe = 0;
            pe.render_starttic = di.thisFrameMsec;
        } else 
            if ( discreet_frames < 1 ) 
                return;
    }

    di.lastLogicFrameDrawn = pe.logicframe;

    di.lastFrameMsec = di.thisFrameMsec;

    di.leftOverFrames = fractional_frames - 1.0;

    // for multi-player
    //  R_RenderFrame (&players[displayplayer]);

    ++pe.renderframe;
    ++pe.render_newframe;

    R_RenderFrame ();

    I_Refresh ();
}

ticcmd_t cmd;

extern ticcmd_t ticcmds[];

static void P_RunLogicOnce (void)
{
    // menu ticker
    M_Ticker (); 

    // game tickers
    G_Ticker ();
}

#define COPYVEC2(x,y) \
        (x)[0]=(y)[0]; \
        (x)[1]=(y)[1]

/*
====================
 P_TryRunLogic()
====================
*/
#define L_SAMPLE_INTERVAL 8
static void P_TryRunLogic ( void )
{
    static int lframe = 0;
    static int discreet_save = 1;
    static float correction = 1.0;

    float dt;
    float LFPS = 1.0;
    int discreet_frames = 1;

    int counts;
    int availabletics;
    int newtics;
    static int lasttics = 0;


    /* radio: "author: Joe Keenen" */


    pe.thisLogicMsec = I_CurrentMillisecond();


//    dt = (float)(pe.thisLogicMsec - pe.lastLogicMsec);

//correction = 0;

    //newtics = (pe.thisLogicMsec*92/1000);
    newtics = I_GetTicks();

//    if ( dt + correction < pe.msecPerLogicFrame ) 
//        return;

    // save this Msec for dt next time through
    pe.lastLogicMsec = pe.thisLogicMsec;


#if 0
    lframe = (lframe + 1) % L_SAMPLE_INTERVAL;
    LFPS = ((float)pe.logicframe) / (((float)(pe.thistic - pe.starttic))/1000.0);

    discreet_frames = discreet_save;

    if (!lframe) 
    {
        if ( LFPS >  pe.logic_fps + 0.09 )
            correction = 0.0;
        else
            correction = 1.0;
        //C_WriteLog("correction: %f\n", correction);
        

        // adjust catchup frames
        //  note: larger margin like 10, helps dampen occilations
        if (LFPS < pe.logic_fps - 10.0 )
            ++discreet_save;
        else if (LFPS > pe.logic_fps + 10.0)
            --discreet_save;

        if (discreet_save > 4) 
            discreet_save = 4;
        if (discreet_save < 1)
            discreet_save = 1;
    } 
#endif

    // save player prevCoords
    COPYVEC2(pl.prev.sq, pl.sq);
    COPYVEC2(pl.prev.pix, pl.pix);
    COPYVEC2(pl.lastdrawpix, pl.drawpix);

//C_WriteLog("LFPS %f discreet_frames: %d\n", LFPS, discreet_frames);
//discreet_frames = 1;

#if 0
    while (discreet_frames--) {
        P_RunLogicOnce();
        ++pe.logicframe;
    }
#endif

    if (lasttics == 0) {
        lasttics = newtics;
        return;
    }

    counts = newtics - lasttics;

    if (counts <= 0)
        return;

    lasttics = newtics;

if (counts > 1)
C_WriteLog("running %d counts\n", counts);

    while (counts--) {
        P_RunLogicOnce();
        ++pe.logicframe;
    }

    pe.lastlogictime = pe.logictime;
    pe.logictime = I_CurrentMillisecond();
}


int startFrame      = 0; 
int endFrame        = 0;
float deltaFrame    = 0.0;
int CurrentSecond   = 0;

extern char gamemsgbuf2[];
extern int gamemsgend2;

static void Pacman_Mainloop (void)
{
    float tl, tr;


    for (;;)
    { 
		startFrame = I_CurrentMillisecond();



		//
		// Events()
		//
        I_CheckEventQueue ();
        E_ProcessEvents   ();
        E_BuildTiccmd (&ticcmds[0]);



        // used by stats
        pe.thistic = I_CurrentMillisecond();





		//
		// Logic Frame
		//
        P_TryRunLogic();



        // things to do once a second
        if ( I_CurrentMillisecond() > CurrentSecond ) {
            CurrentSecond = I_CurrentMillisecond() + 1000;
            if (pe.showfps) {
                gamemsgend2 = CurrentSecond;
                tl = ((float)(pe.thistic - pe.starttic)) / 1000.0;
                tr = ((float)(pe.thistic - pe.render_starttic)) / 1000.0;
                C_snprintf (gamemsgbuf2, 128, "logicfps: %f   renderfps: %f", ((float)pe.logicframe)/tl, ((float)pe.render_newframe)/tr);
            }
        }


    
		//
		// Render
		//
        P_Display ();



		//
		// Sound
		//
        I_DoSounds ();


        pe.lasttic = pe.thistic;


        // sleeping
        if (G_CheckPause() || M_InMenu())
            gsleep(10);
        else if (pe.throttle) 
            gsleep(pe.ms_sleep);
		

		endFrame = I_CurrentMillisecond();
		deltaFrame = endFrame - startFrame;

    }
}

static void P_InitGameState (void)
{
    ///////////////
    // pacengine
    pe.logic_fps = LOGIC_FPS;
    pe.msecPerLogicFrame = 1000.0 / pe.logic_fps;

    C_WriteLog("Logic FPS Set at: %f\n", pe.logic_fps);
    C_WriteLog("msecPerLogicFrame: %f\n", pe.msecPerLogicFrame);

    // would be nice to have on as default, but until there is code
    //  to make this decision it is better left off, to ensure performance
    //  across disparate platforms.
    pe.throttle = gfalse;

    pe.ms_sleep = 4;
    pe.starttic = pe.thistic = pe.lasttic = I_CurrentMillisecond();
    pe.dt = 0;
    pe.logicframe = 1;
    pe.renderframe = 1;
    pe.logictime = pe.lastlogictime = I_CurrentMillisecond();
    pe.interpolate = gtrue;

    pe.render_fps = 120;
    pe.render_cap_to_fps = gtrue;
    pe.msecPerRenderFrame = 1000.0 / pe.render_fps;
    pe.lock_rfps_to_logic = gfalse;

    pe.showfps = gfalse;
    pe.capfps_turnedOn = gfalse;
    pe.render_newframe = 0;
    pe.render_starttic = pe.starttic;

    pe.thisLogicMsec = pe.lastLogicMsec = I_CurrentMillisecond();
    pe.logicFrameExtra = 0.0;
    pe.drawPaths = gfalse;


    ///////////////////
    // drawinfo
    di.lastFrameMsec = di.thisFrameMsec = I_CurrentMillisecond();
    di.leftOverFrames = 0.0;
    di.lastLogicFrameDrawn = 0;

    if (pe.throttle)
        C_WriteLog("throttle is on\n");
    if (pe.render_cap_to_fps)
        C_WriteLog("capping render fps to %d\n",(int)pe.render_fps);
}

/*
======================
 P_PacmanInit 

 - things that only have to be initialized once in the beginning
======================
 */
static void P_PacmanInit (const char *winmain_argv)
{
    char **p;

#ifdef __DEBUG__
    C_OpenLogFile("debug_pacman.log");
#endif

    I_InitTimers();

    V_virtmem_init ();

	ParseWinMainArgs ( winmain_argv );

    E_LoadDefaults ();

    // supports only literal file path right now
    if ((p = M_CheckParm("-m"))) 
        readGameBoard(*++p);
    else
        readGameBoard(DEFAULT_MAP);

	P_PlayerDefaults ();

    T_InitThinkers ();

    if (M_CheckParm("-fs") || M_CheckParm("--fullscreen"))
        I_SetFullscreen(gtrue);
    else
        I_SetFullscreen(gfalse);

    R_InitGraphics ();

    C_Srand ();

    F_InitFreetype ();

    C_LoadScoreFile ();

    P_InitGameState ();

    PF_InitPathFind ();

    C_WriteLog("memory used: %d bytes\n", V_getMemUsed());
}

static void P_Shutdown (void)
{
    I_ShutdownVideo ();
    V_virtmem_shutdown ();
    C_CloseLog();
}


int Pacman_Main (const char *winmain_argv)
{
    P_PacmanInit(winmain_argv);

    //Tetris_MainLoop ();
    Pacman_Mainloop ();

    P_Shutdown();

    return 0;
}


