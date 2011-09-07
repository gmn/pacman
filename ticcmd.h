
#ifndef __TICCMD__
# define __TICCMD__ 

# if !defined(__BYTE__) || !defined(__USHORT__)
#  include "common.h" 
# endif

typedef struct {
	byte dirmoving;
    byte buttons;
    byte checksum;
    byte chatchar;
	byte special;
    byte pad1;
    byte pad2;
    byte pad3;
} ticcmd_t;

# define __ticcmd_default_d { 0, 0, 0, 0, 0, 0, 0, 0 }

void E_BuildTiccmd (ticcmd_t *);

#endif


