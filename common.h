
//=============================
//=============================
#ifndef __COMMON_H__
#define __COMMON_H__	1


//--------------------
#ifndef __BYTE__
#define __BYTE__		1
typedef unsigned char byte;
#endif
//--------------------


//--------------------
#ifndef __GBOOL__
#define __GBOOL__		1

typedef enum {gfalse,gtrue} gbool;


#endif // __GBOOL__
//--------------------


//--------------------
#ifndef __UINT__
#define __UINT__
typedef unsigned int uint;
#endif
//--------------------


//--------------------
#ifndef __USHORT__
#define __USHORT__
typedef unsigned short ushort;
#endif
//--------------------

#ifndef __POINT__
#define __POINT__
typedef struct point_s {
    int x, y;
} point_t;
#define point_default { 0, 0 }
typedef int point2_t[2];
typedef int point3_t[3];
typedef int point4_t[4];
typedef int point5_t[5];
#endif

// dope quake stuff
#ifndef __VECTYPES__
#define __VECTYPES__
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];
#endif



//--------------------
#define FILENAME_SZ     64
#define FULLPATH_SZ     256
#define MAX_FILES       256


#define SCREENWIDTH     640
#define SCREENHEIGHT    480
#define SCREENPITCH     1920


/*
#define FRAMES_PER_SECOND 76
// (1000 ms/s) / (40 f/s) == 25 ms/f
#define MILLISECONDS_PER_FRAME 13
#define FRAMES_PER_SECOND 67
#define MILLISECONDS_PER_FRAME 15
*/


//#define LOGIC_FPS 60
//#define LOGIC_FPS 40
//#define LOGIC_FPS 92
#define LOGIC_FPS 35


#if 0
#define TICSPERSEC	    70
#define TICSPERFRAME    2
#define FRAMERATE       (TICSPERSEC / TICSPERFRAME)
#endif

#ifndef M_PI
# define M_PI 3.1415926535898
#endif

#define START_PAUSE 2000
//--------------------


typedef struct {
    int score;
    char initials[4];
} scorelist_t;

scorelist_t * C_GetScoreListP (void);

typedef struct highscore_s {
    int start;
    int dur;
    gbool on;
    char initials[4];
    int index;
    gbool finished;
    int rank;
} highscore_t;


typedef struct pacengine_s {

    // frames logic runs per sec
    float logic_fps; 
    float msecPerLogicFrame;  // 1000 / logic_fps

    // whether to sleep in main-loop
    gbool throttle;

    // how much to sleep if throttle is on
    int ms_sleep; /* the smallest timeslice on windows is 10ms, 
                     so any value <= 10 will probably have the same effect */

    // time keeping
    int thistic, lasttic, starttic;
    float dt;

    // game frames (absolute)
    int logicframe;
    int renderframe;

    // interpolation times
    float lastlogictime;
    float logictime;
    float drawtime;

    // turn interpolation on/off
    gbool interpolate;

    float render_fps;
    float msecPerRenderFrame;
    gbool render_cap_to_fps;
    gbool capfps_turnedOn;

    gbool showfps;

    // non-absolute : for use in displaying RFPS stat
    int render_starttic;
    int render_newframe;

    // prevent display update unless logic has run
    gbool lock_rfps_to_logic;

    // used to calculate dt
    int thisLogicMsec;
    int lastLogicMsec;

    float logicFrameExtra;

    gbool drawPaths;

} pacengine_t;

// used by P_Display
typedef struct drawinfo_s {
    int lastLogicFrameDrawn;
    int lastFrameMsec;
    int thisFrameMsec;
    float leftOverFrames;
} drawinfo_t;


// Default Locations
#define IMGDIR      "data/img"
#define FONTDIR     "data/font"
#define MAPDIR      "data/maps"


#endif //__COMMON_H__
//=============================
//=============================



