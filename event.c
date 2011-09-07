/***********************************************************************
*
* event.c - event handling - keys & defaults
*
***********************************************************************/

#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h> // NULL
#include <string.h> // memcpy?

#include "common.h" // gbool

#include "event.h"  

#include "ticcmd.h"

#include "player.h"
#include "menu.h" // M_Responder
#include "pacman.h" // sys_shutdown_confirm, C_WriteLog, ...





#define MAXCHEATS 3

int eventhead = 0;
int eventtail = 0;

extern gbool punishment_mode;

event_t events[MAXEVENTS];

gbool gamekeydown[MAXKEYS];

ticcmd_t __ticcmd_default_s = __ticcmd_default_d;

extern player_t pl;

// an array of these _would_ be for networking
ticcmd_t ticcmds[1];


int key_left;
int key_right;
int key_up;
int key_down;
int key_fullscreen;
int key_pause;
int key_use;
int key_activate;
int key_attack1;
int key_attack2;
int key_quit;
int key_menu;
int key_font;
int key_rotate;
int key_cheat;
int key_loadlevel;
int key_i;
int key_t;
int key_s;
int key_c;
int key_b;
int key_h;
int key_z;
int key_d;


timer_t maintimer;

extern pacengine_t pe;

extern char gamemsgbuf[128];
extern int gamemsgend;

void E_BuildTiccmd(ticcmd_t *cmd)
{
    extern int currentfont;
    extern int total_fonts;
    extern gbool enable_rotation;




    // reset the cmd
    *cmd = __ticcmd_default_s;

    /*
     * the players ticcmd is capable of combining
     *  multiple directions OR'd together
     */

    // shouldn't need
    // cmd->dirmoving = DIR_NONE;

	if (gamekeydown[key_up]) 
        cmd->dirmoving |= DIR_UP;
	if (gamekeydown[key_left])
        cmd->dirmoving |= DIR_LEFT;
	if (gamekeydown[key_down])
        cmd->dirmoving |= DIR_DOWN;
	if (gamekeydown[key_right])
        cmd->dirmoving |= DIR_RIGHT;


    /* check special keys */
    if (gamekeydown[key_quit] || gamekeydown[key_menu]) {
        sys_shutdown_confirm ();
        gamekeydown[key_menu] = gfalse;
        gamekeydown[key_quit] = gfalse;
	} 

    if (gamekeydown[key_rotate]) {
        enable_rotation = !enable_rotation;
        gamekeydown[key_rotate] = gfalse;
    }

    if (gamekeydown[key_loadlevel]) {
        gamekeydown[key_loadlevel] = gfalse;
        G_InstantLoadLevel();
    }

    if (gamekeydown[key_font]) {
        currentfont = (++currentfont) % total_fonts;
        gamekeydown[key_font] = gfalse;
    }

    if (gamekeydown[key_pause]) {
        if (G_CheckPause()) {
            G_UnsetPause();
        } else {
            G_SetPause(0);
        }
        gamekeydown[key_pause] = gfalse;
    }

    // extra life button
    if (gamekeydown[key_cheat]) 
    {
        if ( !pl.discoveredCheat ) {
            C_strncpy (gamemsgbuf, "player discovered cheat", 128);
            gamemsgend = I_CurrentMillisecond() + 1500;
            pl.discoveredCheat = gtrue;
        }

        if (pl.lives <= 6) {
            ++pl.lives;
            R_PostExtraLife();
            C_WriteLog("player totally cheated.  Lives: %d\n", pl.lives);
            gamekeydown[key_cheat] = gfalse;
            ++pl.times_cheated;
        }

        if (!punishment_mode && (pl.times_cheated > MAXCHEATS))
            G_DoPunishmentMode ();
    }


    // interpolate 
    if (gamekeydown[key_i]) {
        pe.interpolate = ( pe.interpolate ) ? gfalse : gtrue;
        gamekeydown[key_i] = gfalse;
        if (pe.interpolate) {
            C_strncpy (gamemsgbuf, "interpolate is on", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        } else {
            C_strncpy (gamemsgbuf, "interpolate is off", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        }
    }
    // throttle
    if (gamekeydown[key_t]) {
        pe.throttle = ( pe.throttle ) ? gfalse : gtrue;
        gamekeydown[key_t] = gfalse;
        if (pe.throttle) {
            C_strncpy (gamemsgbuf, "throttle is on", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        } else {
            C_strncpy (gamemsgbuf, "throttle is off", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        }
    }
    // showfps
    if (gamekeydown[key_s]) {
        pe.showfps = ( pe.showfps ) ? gfalse : gtrue;
        gamekeydown[key_s] = gfalse;
        if (pe.showfps) {
            C_strncpy (gamemsgbuf, "showfps is on", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        } else {
            C_strncpy (gamemsgbuf, "showfps is off", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        }
    }
    // fps cap to render_fps
    if (gamekeydown[key_b]) {
        pe.render_cap_to_fps = ( pe.render_cap_to_fps ) ? gfalse : gtrue;
        gamekeydown[key_b] = gfalse;
        if (pe.render_cap_to_fps) {
            pe.capfps_turnedOn = gtrue;
            C_snprintf (gamemsgbuf, 128, "renderfps cap is on at %d fps", (int)pe.render_fps);
            gamemsgend = I_CurrentMillisecond() + 1000;
        } else {
            C_strncpy (gamemsgbuf, "renderfps cap is off", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        }
    }
    if (gamekeydown[key_h]) {
        pe.lock_rfps_to_logic = ( pe.lock_rfps_to_logic ) ? gfalse : gtrue;
        gamekeydown[key_h] = gfalse;
        if (pe.lock_rfps_to_logic) {
            C_strncpy (gamemsgbuf, "renderfps is locked", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        } else {
            C_strncpy (gamemsgbuf, "renderfps is un-locked", 128);
            gamemsgend = I_CurrentMillisecond() + 1000;
        }
    }
    if (gamekeydown[key_attack2]) {
        gamekeydown[key_attack2] = gfalse;
        S_ToggleSlideShow();
    }
    // 
    if (gamekeydown[key_d]) {
        gamekeydown[key_d] = gfalse;
        pe.drawPaths = (pe.drawPaths) ? gfalse : gtrue;
    }



    /* save for later when I actually have some
	else { 
		
		if (gamekeydown[key_activate])
		cmd->special |= S_ACTIVATE;
		if (gamekeydown[key_use])
		cmd->special |= S_USE;
		if (gamekeydown[key_attack1])
			cmd->special |= S_ATTACK1;
	} */


    // should've been done elsewhere 
//    memcpy (&ticcmds[0], cmd, sizeof(ticcmd_t));
}


/*
==================== 
 G_Responder 
 
 - The Game Responder decides what to do with an event
   based on the data that the event holds
==================== 
*/
gbool G_Responder (event_t *ev)
{
	switch (ev->type) {
	case ev_keydown:
		if (ev->data1 < KEY_CEILING_VAL && ev->data1 >= 0)
			gamekeydown[ev->data1] = gtrue;
		return gtrue; 
	case ev_keyup:
		if (ev->data1 < KEY_CEILING_VAL && ev->data1 >= 0)
			gamekeydown[ev->data1] = gfalse;
		return gfalse; 
	default :
		break;
	}
	return gfalse;
}

//
// E_PostEvent 
// called by the I/O functions when input is detected
//
void E_PostEvent(event_t *ev)
{
	events[eventhead] = *ev;
	eventhead = (++eventhead)&(MAXEVENTS-1);
}

//
// E_ProcessEvents
// send all events of a given timestamp down the responder chain ...
//
void E_ProcessEvents (void)
{
	event_t *ev;

	for ( ; eventtail != eventhead; eventtail = (++eventtail)&(MAXEVENTS-1))
	{
		ev = &events[eventtail];

		if (M_Responder(ev))
			continue;			// menu ate event
		G_Responder(ev);
	}
}


typedef struct {
    char    *name;
    int     *location;
    int     defaultvalue;
} default_t;

default_t defaults[] =
{
    {"key_left",        &key_left,          KEY_LEFTARROW},
    {"key_right",       &key_right,         KEY_RIGHTARROW},
    {"key_down",        &key_down,          KEY_DOWNARROW},
    {"key_up",          &key_up,            KEY_UPARROW},
    {"key_fullscreen",  &key_fullscreen,    KEY_V},
    {"key_quit",        &key_quit,          KEY_Q},
    {"key_attack1",     &key_attack1,       KEY_LCONTROL},
    {"key_attack2",     &key_attack2,       KEY_SPACEBAR},
    {"key_pause",       &key_pause,         KEY_P},
    {"key_activate",    &key_activate,      KEY_TAB},
    {"key_use",         &key_use,           KEY_U},
    {"key_menu",        &key_menu,          KEY_ESCAPE},
    {"key_rotate",      &key_rotate,        KEY_R},
    {"key_font",        &key_font,          KEY_F},
    {"key_cheat",       &key_cheat,         KEY_C},
    {"key_loadlevel",     &key_loadlevel,       KEY_L},
    {"key_i",           &key_i,             KEY_I},
    {"key_t",           &key_t,             KEY_T},
    {"key_s",           &key_s,             KEY_S},
    {"key_b",           &key_b,             KEY_B},
    {"key_h",           &key_h,             KEY_H},
    {"key_z",           &key_z,             KEY_Z},
    {"key_d",           &key_d,             KEY_D}
};

int numdefaults;
void E_LoadDefaults (void)
{
    int i;

    numdefaults = sizeof(defaults)/sizeof(defaults[0]);
    for (i=0; i<numdefaults; i++)
    {
        *defaults[i].location = defaults[i].defaultvalue;
    }

    for (i=0; i<MAXKEYS; i++)
    {
        gamekeydown[i] = gfalse;
    }
}

void E_InitTimers (void)
{
    maintimer.next = maintimer.prev = &maintimer;
}

void E_RunTimers (void)
{
}

/*
timer_t * E_StartTimer (int start, int duration, void *func, void **args, int nump)
{
    timer_t *new;
    new = V_malloc(sizeof(*new), PU_STATIC, 0);
    *new = _default_timer;

    new->start = start;
    new->duration = dur;
    new->action

    switch (nump
        
}
*/


