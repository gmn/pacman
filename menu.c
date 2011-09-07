//
// menu.c
//
#include "common.h"
#include "event.h"

#include "game.h"
#include "pacman.h"

// only using for debug right now5
#include "player.h"
extern player_t pl;

extern gamestate_t gamestate;

gbool menukeydown[MAXKEYS];
static gbool menu_uninitialized = gtrue;

#include "i_system.h"

#define CHECKKEY(X) (menukeydown[(X)]) 
#define RESETKEY(X) menukeydown[(X)] = gfalse
#define RESETALLKEYS() for (i = 0; i < MAXKEYS; i++) \
                        menukeydown[i] = gfalse

int menu_esc;
int menu_quit;
int menu_select;
int menu_next;
int menu_prev;
int menu_1player;
int menu_2player;
int menu_space;
int menu_back;
int prev_option;
int next_option;

typedef struct {
    char *name;
    int *location;
    int defaultval;
} menu_default_t; 

menu_default_t menudefs[] =
{
    {"menu_prev",   &menu_prev,      KEY_DOWNARROW },
    {"menu_next",   &menu_next,      KEY_UPARROW }, 

    {"prev_option", &prev_option,    KEY_LEFTARROW },
    {"next_option", &next_option,    KEY_RIGHTARROW },

    {"menu_esc",    &menu_esc,       KEY_ESCAPE },
    {"menu_quit",   &menu_quit,      KEY_Q },
    {"menu_select", &menu_select,    KEY_RETURN },
    {"menu_1player", &menu_1player,  KEY_1 },
    {"menu_2player", &menu_2player,  KEY_2 },
    {"menu_space",  &menu_space,     KEY_SPACEBAR },
    {"menu_back",   &menu_back,      KEY_BACKSPACE },
};


int nummenudefs;
void M_SetMenuDefaults (void)
{
    int i;

    nummenudefs = sizeof(menudefs)/sizeof(menudefs[0]);
    for (i = 0; i < nummenudefs; i++)
    {
        *menudefs[i].location = menudefs[i].defaultval;
    }

    for (i = 0; i < MAXKEYS; i++)
    {
        menukeydown[i] = gfalse;
    }
}


highscore_t hs = { 0, 20000, gfalse, {'A',' ',' ',0}, 0, gfalse, -1 };

#define INISIZE 28
// A-Z ' ' '.'
char inis[INISIZE] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '!' };
                    

void M_HighScoreScreen (void)
{
    int i;

    // just starting
    if (!hs.on) {
        hs.start = I_CurrentMillisecond();
        hs.dur = 20000;
        hs.on = gtrue;
        hs.initials[0] = 'A';
        hs.initials[1] = 'A';
        hs.initials[2] = 'A';
        hs.initials[3] = 0x0;
        hs.index = 0;
        hs.finished = gfalse;
        hs.rank = -1;
        RESETALLKEYS();
        return;
    }

    if (I_CurrentMillisecond() - hs.start < 400)
    {
        RESETALLKEYS();
        return;
    }

    if (hs.finished)
    {
        // time runout 
        if (I_CurrentMillisecond() - hs.start >= hs.dur) {
            // SET INITIALS HERE ***
            hs.rank = C_AddGameScore (P_GetScore(), &hs.initials[0]);
            C_WriteScoreFile ();
		    G_SetPause(2400);
            for (i = 0; i < MAXKEYS; i++)
                menukeydown[i] = gfalse;
            hs.on = gfalse;
            gamestate = GS_INTERMISSION;
            return;
        }

        // unset finished mode
        if (CHECKKEY(menu_esc) || CHECKKEY(menu_back)) {
            hs.dur = 20000;
            hs.finished = gfalse;
            hs.start = I_CurrentMillisecond();
            for (i = 0; i < MAXKEYS; i++)
                menukeydown[i] = gfalse;
        }

        // after 1/4 second just set it if they keep pressing ret
        if (CHECKKEY(menu_select))
        {
            if (I_CurrentMillisecond() - hs.start > 250) 
            {    
                // SET INITIALS HERE ***
                hs.rank = C_AddGameScore (P_GetScore(), &hs.initials[0]);
                C_WriteScoreFile ();
		        G_SetPause(2400);
                for (i = 0; i < MAXKEYS; i++)
                    menukeydown[i] = gfalse;
                hs.on = gfalse;
                gamestate = GS_INTERMISSION;
                return;
            }
        }
    }

    // let time run out, goto finished screen
    if (I_CurrentMillisecond() - hs.start >= hs.dur) {
        hs.rank = C_AddGameScore (P_GetScore(), &hs.initials[0]);
        C_WriteScoreFile ();
        G_SetPause(2400);
        hs.finished = gtrue;
        hs.dur = 10000;
        hs.start = I_CurrentMillisecond();
        RESETALLKEYS();
//        hs.on = gfalse;
//        gamestate = GS_INTERMISSION;
        return;
    }

    // exit keys
    if (menukeydown[menu_esc]) {
        hs.rank = C_AddGameScore (P_GetScore(), &hs.initials[0]);
        C_WriteScoreFile ();
        G_SetPause(2400);
        hs.on = gfalse;
        gamestate = GS_INTERMISSION;
        RESETALLKEYS();
        return;
    }

    // more time
    if (CHECKKEY(menu_space)) {
        hs.dur += 10000;
        RESETKEY(menu_space);
    }

    // next letter index
    if (CHECKKEY(next_option)) {
        if (hs.index < 2) 
            ++hs.index;
        else if (hs.index == 2)
            hs.index = 0;
        RESETKEY(next_option);
    }

    // prev letter index
    if (CHECKKEY(prev_option) || CHECKKEY(menu_back)) {
        if (hs.index > 0) 
            --hs.index;
        else if (hs.index <= 0)
            hs.index = 2;
        RESETKEY(prev_option);
        RESETKEY(menu_back);
    }

    // change letter forward
    if (CHECKKEY(menu_next)) {
        i = 0;
        while (inis[i] != hs.initials[hs.index]) ++i;
        ++i;
        if (i >= INISIZE) 
            i = 0;
        hs.initials[hs.index] = inis[i];
        RESETKEY(menu_next);
    }

    // change letter previous
    if (CHECKKEY(menu_prev)) {
        i = 0;
        while (inis[i] != hs.initials[hs.index]) ++i;
        --i;
        if (i < 0)
            i = INISIZE-1;
        hs.initials[hs.index] = inis[i];
        RESETKEY(menu_prev);
    }

    if (menukeydown[menu_select]) {
        if (hs.index < 2) 
            ++hs.index;
        else if (hs.index == 2) {
            hs.finished = gtrue;
            hs.dur = 10000;
            hs.start = I_CurrentMillisecond();
        }
    }

    RESETALLKEYS();

}


/*
====================
 M_Ticker()
 - run once a logic frame, if gamestate doesn't pertain
 to menu stuff, exits and does nothing
====================
*/
void M_Ticker (void)
{
    if (menu_uninitialized) {
        M_SetMenuDefaults();
        menu_uninitialized = gfalse;
    }
  
    switch (gamestate) {
    case GS_INTERMISSION:
        if (menukeydown[menu_esc])
            sys_shutdown_safe();
        if (menukeydown[menu_quit])
            sys_shutdown_safe();
        if (menukeydown[menu_select]) {
            G_StartNewGame();
            G_SetPause(2600);
            gamestate = GS_LEVELEFFECTS;
            RESETKEY(menu_select);
        }
        break;
    case GS_HIGHSCORE:
        M_HighScoreScreen();
        break;
    default:
        break;
    }

}


/*
==================== 
 M_Responder()

 - responds to events, and turns them into menukeydown[]'s 
==================== 
*/
gbool M_Responder (event_t *ev)
{

    if (menu_uninitialized) {
        M_SetMenuDefaults();
        menu_uninitialized = gfalse;
    }

    switch (gamestate) {
    case GS_LEVEL:
        return gfalse;
    default:
        break;
    }
  
	switch (ev->type) {
	case ev_keydown:
		if (ev->data1 < KEY_CEILING_VAL && ev->data1 >= 0)
			menukeydown[ev->data1] = gtrue;
		return gtrue; 
	case ev_keyup:
		if (ev->data1 < KEY_CEILING_VAL && ev->data1 >= 0)
			menukeydown[ev->data1] = gfalse;
		return gfalse; 
	default :
		break;
	}

    return gfalse;
}

gbool M_InMenu ( void )
{
    if (gamestate == GS_INTERMISSION || gamestate == GS_HIGHSCORE)
        return gtrue;
    return gfalse;
}

