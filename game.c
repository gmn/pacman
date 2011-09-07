

#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h> 
#include <string.h> // memcpy


#include "common.h"
#include "game.h"
#include "ticcmd.h"
#include "player.h"
#include "pacman.h" // P_Ticker, T_ResetThinkers, R_PostExtraLife
#include "i_system.h"

#include "gameboard.h"
extern gameboard_t gameboard;
#include "thinker.h"
extern thinker_t thinkercap;

extern player_t pl;
extern ticcmd_t ticcmds[];


gbool punishment_mode = gfalse;
gbool escaping_purgatory = gfalse;

gamestate_t gamestate = GS_INTERMISSION;

int levelstate = 0;


gbool paused = gfalse;
int pause_start = 0;
int pause_length = 0;
uint pause_type = 0;
#define PAUSETYPE_TIMED 1
#define PAUSETYPE_SPECIFIED 2

#define DEATH_TIMEOUT 2000

int death_wait_start = 0;
gbool death_wait = gfalse;



void G_SetPause (int amount)
{
    paused = gtrue;
    pause_type = (amount > 0) ? PAUSETYPE_TIMED : PAUSETYPE_SPECIFIED;
    pause_start = I_CurrentMillisecond();
    pause_length = amount;
}

void G_UnsetPause (void)
{
    pause_type = 0;
    pause_length = 0;
    pause_start = 0;
    paused = gfalse;
}

gbool G_CheckPause (void)
{
    if (!paused)
        return gfalse;

    if (pause_type == PAUSETYPE_TIMED)
    {
        if (I_CurrentMillisecond() - pause_start > pause_length) {
            G_UnsetPause();
            return gfalse;
        } else {
			return gtrue;
        }
    } 
    else if (pause_type == PAUSETYPE_SPECIFIED) {
        return gtrue;
    }

    return gfalse;
}

void G_DotsRedraw (void)
{
    int i;

    for (i = 0; i < MAX_X*MAX_Y; i++)
    {
        if (gameboard.index[i] && gameboard.index[i]->type == ST_OPEN)
        {
            gameboard.dots[i] = gameboard.index[i]->open;
        }
    }
    gameboard.currentdots = gameboard.numdots;
}

void G_DeathTicker (void)
{
    if (I_CurrentMillisecond() - death_wait_start > DEATH_TIMEOUT) {
        death_wait = gfalse;
        gamestate = GS_LEVEL;
        P_ResetPlayerCoordinates();
        T_ResetThinkerCoordinates();
	}
}

void G_CheckGamestateKeys (void)
{
}

void G_IntermissionTicker (void)
{
    G_CheckGamestateKeys();
}

char *mapfilenames[3] = {   MAPDIR"/pacman1.map", 
                            MAPDIR"/mspacman2.map", 
                            MAPDIR"/argus.map" };


/*
==================== 
 G_StartNewGame
==================== 
*/
static gbool newgamestart = gtrue;
void G_StartNewGame (void)
{
	// only reload the first map if it is not the first game played 
	//  (in case of user specified map)
    if (!newgamestart && C_strncmp(gameboard.name, "Original Pacman L1", 128)) {
        readGameBoard(mapfilenames[0]);
        BS_compile_spritelist ();
        GL_setup_gameboard ();
    }

    P_PlayerDefaults();
    T_RebuildThinkers();

    C_Srand();
    G_DotsRedraw ();

    punishment_mode = gfalse;
	death_wait = gfalse;
	newgamestart = gfalse;

    R_SpinBoardIn(0, 1000);
/*    G_SetPause(3600);
    gamestate = GS_LEVELEFFECTS;
    */
}

/*
==================== 
 G_LoadNextLevel
==================== 
*/
void G_LoadNextLevel (void)
{

    // getting out of punishment_mode
    if (!punishment_mode)
        ++pl.levnum;

    C_WriteLog("New Level.  pl.levnum: %d\n", pl.levnum);

    // load a new map every two 
    if (punishment_mode || (pl.levnum > 2 && (pl.levnum % 2)))
    {
        readGameBoard(mapfilenames[(((pl.levnum-1)/2)%3)]);
        C_WriteLog("gameboard file %s read\n", mapfilenames[((pl.levnum-1)/2)%3]);
        BS_compile_spritelist ();
        GL_setup_gameboard ();
    }

    // if getting out, turn off punishment_mode
    if (escaping_purgatory) {
        punishment_mode = gfalse;
        escaping_purgatory = gfalse;
    }

    G_DotsRedraw ();

    P_ResetPlayerCoordinates();

    // decides _how_ to reset the thinkers
    T_ResetThinkers();

    G_SetPause(2000);

	death_wait = gfalse;
}



#define LISTABLE_SCORES 12

/*
====================
 G_CheckGameState
====================
*/
void G_CheckGameState( void )
{
    //
    // Player Dead
    //
    if (pl.lives <= 0) 
    {
        // 1st thing to do is a gameover pause
		G_SetPause(2400);
        gamestate = GS_GAMEOVER;
        return;



        /*
        if (P_GetScore() > ((C_GetScoreListP())[LISTABLE_SCORES-1]).score) {
            gamestate = GS_HIGHSCORE;
            return;
        } else
            gamestate = GS_GAMEOVER;


        C_AddGameScore (P_GetScore(), "NEW");
        C_WriteScoreFile ();
		G_SetPause(2400);
        
        return;
        */
    }

    //
    // Completed a Level
    // 
    if (gameboard.currentdots <= 0)
    {
        // coming out of punishment level
        if (punishment_mode) 
            escaping_purgatory = gtrue;

        levelstate = 1;
        gamestate = GS_NEWLEVEL;
        return;
    }

    //
    // extralives
    //
    if (pl.score / 1000 >= pl.nextguy) {
        // 1st @ 20
        if (pl.nextguy == 20) {
            pl.nextguy = 50;
        }
        // the rest every 50 starting at 50
        else {
            pl.nextguy += 50;
        }
        ++pl.lives;
        R_PostExtraLife();
        C_WriteLog("Extra Life at %d!  Total Lives: %d\n", pl.score, pl.lives);
    }
}

/*
==================== 
 G_CheckThinkerEncounters
 - 
==================== 
*/
void G_CheckThinkerEncounters (void)
{
    thinker_t *tp;
    char buf[32];
    int gscore;

    tp = thinkercap.next;
    while (tp != &thinkercap)
    {
        switch (tp->type) {
        case TT_REDGHOST:
        case TT_PINKGHOST:
        case TT_CYANGHOST:
        case TT_YELLOWGHOST:
            if (tp->sq[0] == pl.sq[0] && tp->sq[1] == pl.sq[1])  
            {
                if (tp->dead || tp->eaten)
                    break;

                if (tp->chased) /* ghost gets eaten */
                {
                    T_EatGhost(tp);
                    gscore = (1 << T_GetNumEaten()) * 100;
                    pl.score += gscore;
                    C_WriteLog("ghost eaten: #%d - score: %d\n", T_GetNumEaten(), pl.score);
                    C_snprintf(buf, 32, "%d", gscore);
                    R_PostFloatingText(buf, 2040);
                    G_SetPause(500);
                    // increase the chase timer to make up for the pause
                    T_IncGhostChasedDuration(500);
                }
                else /* player got eaten */
                {
                    if (death_wait)
                        break;

                    G_SetPause(DEATH_TIMEOUT+100);
				    death_wait = gtrue;
                    death_wait_start = I_CurrentMillisecond(); 
                    
                    R_PostLoseGuy ();
                    gamestate = GS_LOSELIFE;

                    pl.lives--;
                }
            }
            break;
        default:
            break;
        }
        tp = tp->next;
    }
}

/*
==================== 
 G_DoPunishmentMode
 - called from event.c when too many extra guys
==================== 
*/
void G_DoPunishmentMode (void)
{
    punishment_mode = gtrue;
    C_WriteLog("Starting Punishment Level\n"); 
    readGameBoard(MAPDIR"/openarea.map");
    C_WriteLog("gameboard file %s read\n", MAPDIR"/openarea.map");
    BS_compile_spritelist ();
    GL_setup_gameboard ();

    G_DotsRedraw ();

    P_ResetPlayerCoordinates();
    T_ResetThinkers();

    G_SetPause(2000);

    R_PostPurgatoryMsg ();

	death_wait = gfalse;
    gamestate = GS_LEVEL;
}


/*
==================== 
 G_Ticker
==================== 
*/
void G_Ticker (void)
{
    //
    // Do things to change the game state
    // 

    //
    // get commands
    //
    memcpy(&pl.cmd, &ticcmds[0], sizeof(ticcmd_t));

    //
    // do main actions
    //
	switch (gamestate)
	{
		case GS_INTRO:
            break;
		case GS_LEVELBEGIN:
            break;
        case GS_PUNISHMENT:
		case GS_LEVEL:
            if (G_CheckPause()) 
                break;
            
			P_Ticker ();
            T_Ticker ();
            G_CheckThinkerEncounters();
            G_CheckGameState();
		//	ST_Ticker (); //
		//	HU_Ticker (); // hud ticker
			break;
		case GS_INTERMISSION:
     //       G_IntermissionTicker ();
			break;
		case GS_LOSELIFE:
            G_DeathTicker();
			break;
		case GS_GAMEOVER:
            if (G_CheckPause()) 
                break;
            
            C_WriteLog("\n******************************\n**** Final Score: %7d ****\n******************************\n", P_GetScore());
            if (P_GetScore() > ((C_GetScoreListP())[LISTABLE_SCORES-1]).score) {
                gamestate = GS_HIGHSCORE;
                return;
            }
            gamestate = GS_INTERMISSION;
			break;
        case GS_NEWLEVEL:
            switch (levelstate) {
            case 1:
                G_SetPause(3600);

                // the display code calls G_LoadNextLevel
                R_SpinBoardOut(1600,2000);
                gamestate = GS_LEVELEFFECTS;
                levelstate = 2;
                break;
            case 2:
                G_SetPause(1600);
                levelstate = 0;
                gamestate = GS_LEVELEFFECTS;
                break;

			case 0:
            default:
                gamestate = GS_LEVEL;
                break;
            }
            break;

        case GS_LEVELEFFECTS:
            if (G_CheckPause())
                break;
            gamestate = GS_NEWLEVEL;
            break;
	}
}

void G_InstantLoadLevel (void)
{
    R_SpinBoardOut(0,2000);
    G_SetPause(2000);
    gamestate = GS_LEVELEFFECTS;
    levelstate = 2;
}
