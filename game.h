
#ifndef __GAME_H__
#define __GAME_H__ 1


typedef enum {
    GS_NONE,
    GS_LEVEL,
    GS_LEVELBEGIN,
    GS_LEVELEND,
    GS_NEWLEVEL,
    GS_INTERMISSION,
    GS_HIGHSCORES,
    GS_DEMOSCREEN,
    GS_INTRO,
    GS_LOSELIFE,
    GS_GAMEOVER,
    GS_HIGHSCORE,
	GS_NEWGAME,
    GS_PUNISHMENT,
    GS_LEVELEFFECTS,
    MAX_GAMESTATES
} gamestate_t;


#endif

/* 
 * INTRO - when game boots/starts , 
 * LEVELBEGIN - when you're starting,
 * LEVEL - during game , 
 * INTERMISSION - in between levels , 
 * LOSELIFE - when a guy gets caught
 *
 * INTRO --> LEVELBEGIN --> LEVEL --> LOSELIFE --> GS_GAMEOVER
 *           ^  ^             |          |
 *           |__|_____________|__________|
 *              |             +--> LEVELWIN -+ --> VICTORY (not reached)
 *              |                            |
 *              |   INTERMISSION <-----------+
 *              |         |
 *              |_________|
 */
