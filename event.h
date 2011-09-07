
#ifndef __EVENT_H__
#define __EVENT_H__

//
// event handling types
//

typedef enum {
	ev_keydown,
	ev_keyup,
	ev_mouse,
	ev_joystick
} evtype_t;

typedef struct event_s {
	evtype_t type;
	int data1;		// button press
	int data2;		// key/joystick x move
	int data3;		// key/joystick y move
} event_t;

#define MAXEVENTS 	512

/* directions */
typedef enum {
	DIR_NONE = 			0x0,
	DIR_UP = 			0x1,	
	DIR_DOWN = 			0x2,
	DIR_LEFT = 			0x4,
	DIR_RIGHT = 		0x8,
	DIR_DOWN_LEFT = 	0x6,
	DIR_UP_LEFT = 		0x5,
	DIR_DOWN_RIGHT = 	0xA,
	DIR_UP_RIGHT = 		0x9,
    DIR_UP_AND_DOWN =   0x800,
    DIR_LEFT_AND_RIGHT = 0x2000,
} direction_t;



typedef enum {
    KEY_NULLKEY,
	KEY_UPARROW,
	KEY_DOWNARROW,
	KEY_LEFTARROW,
	KEY_RIGHTARROW,
    KEY_0 = 48,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
	KEY_A = 65,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_ESCAPE,
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_SPACEBAR,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_TAB,
    KEY_RETURN,
    KEY_CAPSLOCK,
    KEY_MINUS,
    KEY_EQUALS,
    KEY_TILDA,
    KEY_BACKSPACE,
    KEY_DELETE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_CEILING_VAL

} keyboard_t;

#define MAXKEYS 256

void E_KeyResponder (event_t *);
void E_PostEvent (event_t *);

#if !defined ( __GBOOL__ )
# include "common.h"
#endif

// timers are a list of finite events that start and run for a duration
//  once that duration is reached they are shutdown and discarded
typedef struct timer_s {
    char *name;
    int id;
    int start;
    int duration;

    // selects which func to call
    int action;
    // args for the func stored in order
    void *args[3];
    // the funcs
    void (*action0) (void);
    void (*action1) (void *);
    void (*action2) (void *, void *);
    void (*action3) (void *, void *, void *);
    // to disable set on to gfalse
    gbool on;
    struct timer_s *prev;
    struct timer_s *next;
} timer_t;

#define _default_timer { NULL, 0, 0, 0, 0, { NULL, NULL, NULL }, NULL, \
                        NULL, NULL, NULL, gtrue, NULL, NULL }


#endif __EVENT_H__

// in pacman.h instead
//void E_ProcessEvents (void);
//void E_LoadDefaults (void);


