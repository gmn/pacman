
#pragma warning( disable : 4706 )

#define _CRT_SECURE_NO_DEPRECATE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pacman.h"
#include "gameboard.h"
#include "virtmem.h"


gameboard_t gameboard = __gameboard_default;

// unique ids are handed out to all entities
int next_entity_id = 111;


int strip_white_space (char *buf, int in, int max)
{
    int i = in;
    while ((buf[i] == ' '  || 
            buf[i] == '\t' ||
            buf[i] == '\n' ||
            buf[i] == '\r' ) && i < max-1)
        ++i;
    return i;
}
#define stripWhiteSpace() (i = strip_white_space(buf, i, sz))

void consume_comment_line (char *buf, int *in, int max)
{
    while (buf[*in] != '\n')
    {
        if (*in < max-1)
            ++(*in);
        if (*in >= max-1)
            break;
    }
}

// generalized version of the static tag checks below
static int check_tag (const char *tag, char *buf, int in, int max)
{
    int i = in;
    int j;
    int len = strlen (tag);
    if ( i + len > max - 1 )
        sys_error_fatal("try to read past file end: %d", __LINE__);
    for (j=0; j < len; j++)
        if ( buf[i+j] != tag[j] )
            return 0;
    
    return len;
}
#define checkTag(x) check_tag((x), buf, i, sz)


static int check_name_tag (char *buf, int in, int max)
{
    int i = in;
    if (i+4 > max - 1)
        sys_error_fatal("try to read past array end: %d", __LINE__);
    if ( buf[i+1] == 'a' &&
         buf[i+2] == 'm' &&
         buf[i+3] == 'e' &&
         buf[i+4] == ':') 
        return 1;
    
    return 0;
}
#define checkNameTag() check_name_tag(buf, i, sz)

static int check_section_tag (char *buf, int in, int max)
{
    int i = in;
    if (i+7 > max - 1)
        sys_error_fatal("try to read past array end: %d", __LINE__);
    if (buf[i+1] == 'e' &&
        buf[i+2] == 'c' &&
        buf[i+3] == 't' &&
        buf[i+4] == 'i' &&
        buf[i+5] == 'o' &&
        buf[i+6] == 'n' &&
        buf[i+7] == ':') 
    {
        return 1;
    }
    return 0;
}
#define checkSectionTag() check_section_tag (buf, i, sz)

static int check_squares_section (char *buf, int in, int max)
{
    int i = in;
    if (i+6 > max-1)
        sys_error_fatal("try to read past array end: %d", __LINE__);
    if (buf[i+0] == 's' &&
        buf[i+1] == 'q' &&
        buf[i+2] == 'u' &&
        buf[i+3] == 'a' &&
        buf[i+4] == 'r' &&
        buf[i+5] == 'e' &&
        buf[i+6] == 's')
    {
        return 1;
    }
    return 0;
}
#define checkSquaresSection() check_squares_section(buf, i, sz)


static int check_entities_section (char *buf, int in, int max)
{
    int i = in;
    if (i+7 > max-1)
        sys_error_fatal("try to read past array end: %d", __LINE__);
    if (buf[i+0] == 'e' &&
        buf[i+1] == 'n' &&
        buf[i+2] == 't' &&
        buf[i+3] == 'i' &&
        buf[i+4] == 't' &&
        buf[i+5] == 'i' &&
        buf[i+6] == 'e' && 
        buf[i+7] == 's')
    {
        return 1;
    }
    return 0;
}
#define checkEntitiesSection() check_entities_section(buf, i, sz)


static int check_total_tag(char *buf, int in, int max)
{
    int i = in;
    if (i+5 >= max)
        sys_error_fatal("try to read past array end: %d", __LINE__);
    if (buf[i+0] == 't' &&
        buf[i+1] == 'o' &&
        buf[i+2] == 't' &&
        buf[i+3] == 'a' &&
        buf[i+4] == 'l' &&
        buf[i+5] == ':')
    {
        return 1;
    }
    return 0;
}
#define checkTotalTag() check_total_tag(buf, i, sz)





// **version number can be single digit only
static int get_version(char *buf, int *in)
{
    char b[2];
    if (buf[0] == 'V' &&
        buf[1] == 'e' &&
        buf[2] == 'r' &&
        buf[3] == 's' &&
        buf[4] == 'i' &&
        buf[5] == 'o' &&
        buf[6] == 'n' &&
        buf[7] == ' ')
    {
        *in = 7;
        *in = strip_white_space (buf, *in, 100000000);
        b[0] = buf[*in];
        b[1] = '\0';
		(*in)++;
        return atoi(b);
    }
    return 0;
}

static int _isdig (int a)
{
    if (a >= '0' && a <= '9')
        return 1;
    return 0;
}

int parseInt (char *buf, int *in)
{
    int i = 0;
    char b[16];

    //while (buf[*in] != ' ' && buf[*in] != '-' && buf[*in] != ',') {
    while (buf[*in] >= '0' && buf[*in] <= '9') {
        b[i] = buf[*in];
        if (i == 15) {
            sys_error_warn ("warning: input digit exceeds maximum size!");
            if (*in != ' ' && *in != '-')
                while ((*in)++ != ' ') 
                    ;
            break;
        }
        ++i;
        ++(*in);
    }
    b[i] = '\0';
    return atoi(b);
}

#define P3TUI(x) (((x)[0]<<24)&((x)[1]<<16)&((x)[2]<8)&0xff)
#define P3TV3(v,p)  (v)[0]=((float)(p)[0])/255.0; \
                    (v)[1]=((float)(p)[1])/255.0; \
                    (v)[2]=((float)(p)[2])/255.0
#define V3ToV3(v1,v2) (v1)[0]=(v2)[0]; \
                      (v1)[1]=(v2)[1]; \
                      (v1)[2]=(v2)[2]  

void get_gamesquare_data (char *buf, int *in)
{
    int i = *in;
    gamesquare_t *gs;
    int in_parent = 0;
    int j = 0;
    int parsing = 1;
    int values[7];
    int k = 0;
    int c = gameboard.current;
    int x_comma[32], y_comma[32];
    int comma_looking = 0;
    int x_c = 0, y_c = 0;

    if (gameboard.current > gameboard.numsquares-1)  {
        sys_error_warn("square data exceeds number of squares specified.  excess discarded");
        while (buf[i++] != '}') 
            ;
        *in = i;
        return;
    }

    gs = gameboard.squares;

    for (j = 0; j < 7; j++)
        values[j] = -1;

    //   0 1 2     3  4 5  6
    // ( 0 0 0 ) ( 0-39 0-30 ) ||
    // ( 1 0 1 ) ( 10,12,18,20 13-28 ) 

    parsing = 1;

    while (buf[i] != '}' && parsing)
    {
		assert(&buf[i]);

        switch (buf[i]) {
        case '(':
            in_parent = 1;
            ++i;
            break;
        case ')':
            in_parent = 0;
            ++i;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':

            if ( comma_looking ) {
				
				if (k < 5) {
					if (x_c < 31)
						x_comma[++x_c] = parseInt(buf, &i);
				} else {
					if (y_c < 31)
						y_comma[++y_c] = parseInt(buf, &i);
				}

                comma_looking = 0 ;
				assert(&buf[i]);
			} else {
				if (k < 5) 
					x_comma[x_c] = values[k++] = parseInt(buf, &i);					 					
				else 
					y_comma[y_c] = values[k++] = parseInt(buf, &i);
			}	
            
            if (k == 4) {
                if (buf[i] == '-') { 
                    ++i;
                } else if (buf[i] == ',') {
                    comma_looking = 1;
                    ++i;
                } else {
                    k++;
                }
            }

            if (k == 6) {
				
                if (buf[i] == '-') {
                    ++i;
                } else if ( buf[i] == ',') {
                    comma_looking = 1;
                    ++i;
                }
				assert(&buf[i]);
            }

            break;

        case '}':
            parsing = 0;
			break;
		case '/':
			// comment
			if (buf[i] == '/' && buf[i+1] == '/') {
				consume_comment_line(buf, &i, 100000000);
				i = strip_white_space(buf, i, 100000000);
			}
			break;
        default:
            ++i;
            break;
        }
    }


    if (k < 6)
        sys_error_fatal("not enough square parameters!");

    // re-arrange values (to allow for decending hyphenated sets)
    if (values[4] != -1) { 
        if (values[3] > values[4])
        {
            j         = values[4];
            values[4] = values[3];
            values[3] = j;
        }
    }
    if (values[6] != -1) {
        if (values[5] > values[6]) 
        {
            j         = values[6];
            values[6] = values[5];
            values[5] = j;
        }
    }

	// map-boundary warnings
    if (values[3] >= MAX_X || values[4] >= MAX_X)
        sys_error_warn("map x coords exceed max value");
    if (values[5] >= MAX_Y || values[6] >= MAX_Y)
        sys_error_warn("map y coord exceeds max value");


    // comma'd values detected
    if (x_c > 0 || y_c > 0)
    {

        // test for large map values
        for (j = 0; j <= x_c; j++) 
            if (x_comma[j] >= MAX_X)
                sys_error_warn("map x coords exceed max value");
        for (j = 0; j <= y_c; j++)
            if (y_comma[j] >= MAX_Y) 
                sys_error_warn("map x coords exceed max value");

        // both sides are comma'd
        if (x_c > 0 && y_c > 0) {
            for (j = 0; j <= x_c; j++) {
                for (k = 0; k <= y_c; k++) {
                    gs[c].type = values[0];
                    gs[c].open = values[1];
                    gs[c].wall = values[2];
                    gs[c].x    = x_comma[j];
                    gs[c].y    = y_comma[k];
                    V3ToV3(gs[c].color, gameboard.defaultcolorv);
                    ++c;
                }
            }
        // only x
        } else if (x_c > 0) {
            // w/ y span
            if (values[6] != -1) {
                for (j = 0; j <= x_c; j++) {
                    for (k = values[5]; k <= values[6]; k++) {
                        gs[c].type = values[0];
                        gs[c].open = values[1];
                        gs[c].wall = values[2];
                        gs[c].x    = x_comma[j];
                        gs[c].y    = k;
                        V3ToV3(gs[c].color, gameboard.defaultcolorv);
                        ++c;
                    }
                }
            // with just 1 y
            } else {
                for (j = 0; j <= x_c; j++) {
                    gs[c].type = values[0];
                    gs[c].open = values[1];
                    gs[c].wall = values[2];
                    gs[c].x    = x_comma[j];
                    gs[c].y    = values[5];
                    V3ToV3(gs[c].color, gameboard.defaultcolorv);
                    ++c;
                }
            }
        // only y comma'd
        } else if (y_c > 0) {
            // w/ x span
            if (values[4] != -1) { 
                for (k = 0; k <= y_c; k++) {
                    for (j = values[3]; j <= values[4]; j++) {
                        gs[c].type = values[0];
                        gs[c].open = values[1];
                        gs[c].wall = values[2];
                        gs[c].x    = j;
                        gs[c].y    = y_comma[k];
                        V3ToV3(gs[c].color, gameboard.defaultcolorv);
                        ++c;
                    }
                }
            // just y comma's
            } else {
                for (k = 0; k <= y_c; k++) {
                    gs[c].type = values[0];
                    gs[c].open = values[1];
                    gs[c].wall = values[2];
                    gs[c].x    = values[3];
                    gs[c].y    = y_comma[k];
                    V3ToV3(gs[c].color, gameboard.defaultcolorv);
                    ++c;
                }
            }
        }
    }



	// regular, non-comma'd values
    else if (values[4] != -1 && values[6] == -1)
    {
        for (j = values[3]; j <= values[4]; j++)
        {
            gs[c].type = values[0];
            gs[c].open = values[1];
            gs[c].wall = values[2];
            gs[c].x    = j;
            gs[c].y    = values[5];
            V3ToV3(gs[c].color, gameboard.defaultcolorv);
            ++c;
        }
    } 
    else if (values[4] == -1 && values[6] != -1)
    {
        for (k = values[5]; k <= values[6]; k++)
        {
            gs[c].type = values[0];
            gs[c].open = values[1];
            gs[c].wall = values[2];
            gs[c].x    = values[3];
            gs[c].y    = k;
            V3ToV3(gs[c].color, gameboard.defaultcolorv);
            ++c;
        }
    } 
    else if (values[4] != -1 && values[6] != -1)
    {
        for (j = values[3]; j <= values[4]; j++)
        {
            for (k = values[5]; k <= values[6]; k++)
            {
                gs[c].type = values[0];
                gs[c].open = values[1];
                gs[c].wall = values[2];
                gs[c].x    = j;
                gs[c].y    = k;
                V3ToV3(gs[c].color, gameboard.defaultcolorv);
                ++c;
            }
        }
    }
    else 
    {   
        gs[c].type = values[0];
        gs[c].open = values[1];
        gs[c].wall = values[2];
        gs[c].x    = values[3];
        gs[c].y    = values[5];
        V3ToV3(gs[c].color, gameboard.defaultcolorv);
		++c;
    }

    gameboard.current = c;

    *in = i;
}

int current_entity = 0;

void get_mapentity_data (char *buf, int *in) 
{
    int i = *in;
    int k;
    int in_parent;
    int parsing;
    int values[5] = { 0,0,0,0,0 };
    //static int current = 0;

    if (!gameboard.entities)
    {
        sys_error_warn ("attempted to get entities w/o total");
        return;
    }

    parsing = 1;
    k = 0;
    while (buf[i] != '}' && parsing)
    {
        switch (buf[i]) {
        case '(':
            in_parent = 1;
            ++i;
            break;
        case ')':
            in_parent = 0;
            ++i;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            values[k++] = parseInt(buf, &i);
            break;
        case '}':
            parsing = 0;
			break;
		case '/':
			// comment
			if (buf[i] == '/' && buf[i+1] == '/') {
				consume_comment_line(buf, &i, 100000000);
				i = strip_white_space(buf, i, 100000000);
			}
			break;
        default:
            ++i;
            break;
        }
    }

    if (k != 5)
        sys_error_fatal("bad arg num in entities section: %d", k);

    gameboard.entities[current_entity].x    = values[3];
    gameboard.entities[current_entity].y    = values[4];
    gameboard.entities[current_entity].type = values[0]; 
    gameboard.entities[current_entity].pix[0] = values[1];

	switch (gameboard.entities[current_entity].type) {
	case ME_GHOSTSPAWN:
		gameboard.entities[current_entity].ghostcolor = values[2];
		gameboard.entities[current_entity].pix[1] = 0;
		break;
	default:
		gameboard.entities[current_entity].pix[1] = values[2];
		break;
	}

    gameboard.entities[current_entity].id   = next_entity_id++;
    gameboard.entities[current_entity].spawn[0] = values[3];
    gameboard.entities[current_entity].spawn[1] = values[4];
//    gameboard.entities[current].respawn[0] = values[5];
//    gameboard.entities[current].respawn[1] = values[6];


	

    ++current_entity;

    *in = i;
}


typedef enum {
    S_NONE = 0,
    S_SQUARES = 1,
    S_ENTITIES = 2
} section_t;


static void build_index_array (void)
{
    int i, j, z;
    static gbool have_malloced = gfalse;
    point_t w;
    static gamesquare_t **index;
    gamesquare_t *gs;
    gameboard_t *gb;
    static byte *dots;
	
    int used[MAX_X*MAX_Y];

    for (i = 0; i < MAX_X*MAX_Y; i++)
        used[i] = 0;

    if (!have_malloced) {
        index = V_malloc (sizeof(gamesquare_t *) * MAX_X*MAX_Y, PU_STATIC, 0);
        dots = V_malloc (sizeof(byte) * MAX_X*MAX_Y, PU_STATIC, 0);
        have_malloced = gtrue;
    }

    // initialize/reset 
    for (i = 0; i < MAX_X*MAX_Y; i++) {
        index[i] = NULL;
        dots[i] = OT_NULL;
    }

    gs = gameboard.squares;
    gb = &gameboard;

	for (j = 0; j < MAX_Y; j++)
    {
		for (i = 0; i < MAX_X; i++)
		{
            for (z = 0; z < gameboard.numsquares; z++)
            {
                if (used[z])
					continue;

                if (gs[z].x == i && gs[z].y == j)
                {
                    index[MAX_X*j+i] = &gs[z];
					used[z] = 1;
                    if (gs[z].type == ST_OPEN) {
                        dots[MAX_X*j+i] = gs[z].open;
                        if (gs[z].open == OT_LITTLE || gs[z].open == OT_BIG) {
                            ++gameboard.currentdots;
                            ++gameboard.numdots;
                        }
                    }
                    ++gameboard.maxindex;
					break;
                }
            }

            for (z = 0; z < gameboard.numsquares; z++)
            {
                if (used[z])
					continue;

                if (gs[z].x == i && gs[z].y == j)
                    C_WriteLog ("found duplicated map square: (%2d, %2d) type: %d\n",i,j, gs[z].type);
            }
        }
    }

	//
	// setup metadot information
	// 
	// ...


    w.x = w.y = 0;

    // allot the left-over allocated squares to null spaces
    while (gameboard.maxindex < gameboard.numsquares) 
    {
        // look for a free index slot
        z = 1;
        for (i = gb->llcorner.x; i < (gb->width+gb->llcorner.x) && z; i++) {
            z = 1;
            for (j = gb->llcorner.y; j < (gb->height+gb->llcorner.y); j++) {
                if (!index[j*MAX_X+i]) {
                    w.x = i; w.y = j;
                    z = 0;
                    break;
                }
            }
        }

        // look for an unused gameboard square
        for (z = 0; z < gameboard.numsquares; z++)
            if (!used[z])
                break;

        // if we found both, allot the square to the index & mark
        if ( !index[w.y*MAX_X+w.x] && !used[z] ) {
            index[w.y*MAX_X+w.x] = &gs[z];
            gs[z].x = w.x;
            gs[z].y = w.y;
            gs[z].open = OT_NULL;
            used[z] = 1;
        }

        ++gameboard.maxindex;
    }

    gameboard.index = index;
    gameboard.dots  = dots;
}

void GB_SetToDefaults (void)
{
    static int notfirsttime = 0;
    gameboard_t d = __gameboard_default;
    if (notfirsttime) {
        d.index = gameboard.index;
        d.dots = gameboard.dots;
        d.squares = gameboard.squares;
        d.entities = gameboard.entities;
    }
    ++notfirsttime;
    gameboard = d;

	current_entity = 0;
}

/*
====================
 readGameBoard
 - this is where all the gameboard loading happens.
====================
*/
void readGameBoard (char *filename)
{
    FILE *fp; 
    char *buf;
    static int old_squares_sz = 0;
    static int old_entities_sz = 0;

    // Important: non-general use vars
    int i; 
    int sz;

    // general use vars
    int j, i_tmp;

    int inside_bracket = 0;
    section_t current_section = S_NONE;
    //int linenum = 1;
    gamesquare_t _gs_def = __gamesquare_default;

    // 
    GB_SetToDefaults();

    // get file size from system
    if ((sz = sys_file_size(filename)) < 0)
        sys_error_fatal ("file not found: %s", filename );

    // alloc file buf
    buf = V_malloc(sz * 2, PU_STATIC, 0);

    // read in file data to buf
    if ((fp = fopen(filename, "r")) == NULL)
        sys_error_fatal("couldn't open gameboard file for reading");

    i = 0;
    for(;;)
    {
        j = fread(&buf[i], 1, 8192, fp);
        i += j;
        if (j <= 0)
            break;
    } 
    fclose(fp);

	// updated (actual) filesize
	sz = i;

    // get version
    i = 0;
    if (get_version(buf, &i) != 1)
        sys_error_fatal("gameboard supports version 1 only");


    // parsing loop
    while (i < sz)
    {
        stripWhiteSpace();

        // overruns
        if (i >= sz-1 || i < 0)
            break;

		assert(&buf[i]);

        // comment
        if (buf[i] == '/' && i < sz-1 && buf[i+1] == '/') {
            consume_comment_line(buf, &i, sz);
            stripWhiteSpace();
        }


        if (buf[i] == '{')
        {
            inside_bracket = 1;
            ++i;
        }  
        if (buf[i] == '}') 
        {
            inside_bracket = 0;
            ++i;
        }

		// optional (could be used to prematurely end a file)
		if (buf[i] == 'E' && buf[i+1] == 'O' && buf[i+2] == 'F')
			break;

        if (inside_bracket)
        {
            // possible tokens: name, section, total, '(', '}'
            //  unless gameboard.squares == NULL
            //  in which case only name or section are possible
            switch (buf[i]) {
            case 'n':
                if (checkNameTag())
                {
                    i+=5;
                    stripWhiteSpace();
					j = 0; 
					if (buf[i+j] != '"')
						sys_error_fatal("quotes should be around gameboard name");
                    ++i;
					while (buf[i+j] != '"' && j < 128-1)
						gameboard.name[j] = buf[i + j++];

	                gameboard.name[j] = '\0';
                    i += j+1;
                }
                C_WriteLog("Loading gameboard: \"%s\"\n", gameboard.name);
                break;

            case 's':
                if (checkSectionTag()) 
                {
                    i+=8;
                    stripWhiteSpace();

                    if (checkSquaresSection()) // && !gameboard.squares)
                    {
					    i+=7;
                        current_section = S_SQUARES;
                    } else if (checkEntitiesSection())
                    {
                        i+=8;
                        current_section = S_ENTITIES;
                    } 
                    else {
                        sys_error_fatal("expecting ``squares'' or ``entities'' token after section");
                    }
                }
                break;

            case 't':
                if (checkTotalTag())
                {
                    i += 6;
                    stripWhiteSpace();

                    switch (current_section) {
                    case S_SQUARES:
//                        if (!gameboard.squares)
  //                      {
                            gameboard.numsquares = parseInt(buf, &i);
                            if (gameboard.numsquares > old_squares_sz) {
                                if (old_squares_sz)
                                    V_free (gameboard.squares);
                                gameboard.squares = (gamesquare_t *) V_malloc (gameboard.numsquares * sizeof(gamesquare_t), PU_STATIC, NULL);
                                old_squares_sz = gameboard.numsquares;
                            }
                            for (j = 0; j < gameboard.numsquares; j++)
                                gameboard.squares[j] = _gs_def; 
    //                    }

                        break;
                    case S_ENTITIES:
      //                  if (!gameboard.entities)
        //                {
                            gameboard.numentities = parseInt(buf, &i);
                            if (gameboard.numentities > old_entities_sz) {
                                if (old_entities_sz)
                                    V_free(gameboard.entities);
                                gameboard.entities = V_malloc (gameboard.numentities * sizeof(entity_t), PU_STATIC, 0);
                                old_entities_sz = gameboard.numentities;
                            }
                            for (j = 0; j < gameboard.numentities; j++) {
                                gameboard.entities[j].x = -1;
                                gameboard.entities[j].y = -1;
                                gameboard.entities[j].type = ME_NONE;
                            }
          //              }
                    }
                }
                break;

            case 'h':
            case 'w':
            case 'v':
                if ((j=checkTag("height:")))
                {
                    i_tmp = i;
                    i+=j;
                    stripWhiteSpace();
                    gameboard.height = parseInt(buf, &i);
                    if (gameboard.height > MAX_Y) {
                        sys_error_warn("requested height: %d. gameboard only supports %d.  resetting to default.", gameboard.height, MAX_Y);
                        gameboard.height = MAX_Y;
                    }
                }
                else if ((j=checkTag("width:")))
                {
                    i_tmp = i;
                    i+=j;
                    stripWhiteSpace();
                    gameboard.width = parseInt(buf, &i);
                    if (gameboard.width > MAX_X) {
                        sys_error_warn("requested height: %d. gameboard only supports %d.  resetting to default.", gameboard.width, MAX_X);
                        gameboard.width = MAX_X;
                    }
                }
                else if ((j=checkTag("vshift:")))
                {
                    i_tmp = i;
                    i+=j;
                    stripWhiteSpace();
                    gameboard.shift.y = parseInt(buf, &i);
                }
                else if ((j=checkTag("hshift:")))
                {
                    i_tmp = i;
                    i+=j;
                    stripWhiteSpace();
                    gameboard.shift.x = parseInt(buf, &i);
                }

                break;

            case 'l':
            case 'c':
                if ((j=checkTag("llcorner:")))
                {
                    i+=j;
                    stripWhiteSpace();
                    gameboard.llcorner.x = parseInt(buf, &i);
                    stripWhiteSpace();
                    gameboard.llcorner.y = parseInt(buf, &i);
                }
                else if ((j=checkTag("color:")))
                {
                    i_tmp = i;
                    i+=j;
                    stripWhiteSpace();
                    gameboard.defaultcolori[0] = parseInt(buf, &i);
                    gameboard.defaultcolorv[0]=((float)gameboard.defaultcolori[0])/255.0;
                    stripWhiteSpace();
                    gameboard.defaultcolori[1] = parseInt(buf, &i);
                    gameboard.defaultcolorv[1]=((float)gameboard.defaultcolori[1])/255.0;
                    stripWhiteSpace();
                    gameboard.defaultcolori[2] = parseInt(buf, &i);
                    gameboard.defaultcolorv[2]=((float)gameboard.defaultcolori[2])/255.0;
                }
                break;

            case '(':
                switch (current_section) {
                case S_SQUARES:
                    get_gamesquare_data (buf, &i); 
                    
					stripWhiteSpace();
                    if (buf[i] == '}') {
                        inside_bracket = 0;
                        ++i;
                    }
                    break;
                case S_ENTITIES:
                    get_mapentity_data (buf, &i );

					stripWhiteSpace();
                    if (buf[i] == '}') {
                        inside_bracket = 0;
                        ++i;
                    }
                }
                break;

            case '}':
                inside_bracket = 0;
            case '\n':
            case '\0':
            case '\r':
            case ' ':
            case '/':
                break;
            default:
                stripWhiteSpace();
                break;
            }
        // NOT INSIDE BRACKETS
        } else {
            switch (buf[i]) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case '/':
                break;
            default:
				sys_error_fatal("lingering character: %c i: %d sz: %d\nbuf:\n%s", buf[i], i, sz, &buf[i]);
            }
        }
		if (i > sz-1 || i < 0)
			sys_error_fatal("gameboard: i == %d , sz == %d", i, sz);
    } // while (i < sz)

    // TODO:
    // check for redundant squares
    //  if they are the same, simply unset one
    //  if they are different, select an open over a wall,
    //  and post a warning.

    V_free(buf);
    buf = NULL;

    build_index_array ();
}

void cheepPrintGameBoard(void)
{
    int i;
    printf("name: %s\n", gameboard.name);
    printf("num squares: %d\n", gameboard.numsquares);
    printf("----------------\n");
    printf("walls: \n");
    for (i = 0; i < gameboard.numsquares; i++)
    {
        if (gameboard.squares[i].type == ST_WALL) {
            printf(" x, y: %d, %d \n", gameboard.squares[i].x, gameboard.squares[i].y);
		} else {
			printf("FUCK x, y: %d, %d \n", gameboard.squares[i].x, gameboard.squares[i].y);
		}
    }
    printf("\n");
    printf("open areas: \n");
    for (i = 0; i < gameboard.numsquares; i++) {
        if (gameboard.squares[i].type == ST_OPEN)
        {
            printf("x, y: %d, %d \n", gameboard.squares[i].x, gameboard.squares[i].y);
        }
    }
    printf ("\n");
}

void debugPrintGameBoard(void)
{
    int i, j, k;
    for (j = MAX_Y-1; j >= 0; j--) {
        for (i = 0; i < MAX_X; i++) {
            if (gameboard.index[j*MAX_X+i]) {
				if (gameboard.index[j*MAX_X+i]->type == ST_WALL)
                    printf ("x");
				else if (gameboard.index[j*MAX_X+i]->type == ST_OPEN) {
                    if (gameboard.index[j*MAX_X+i]->open == OT_LITTLE)
					    printf (".");
                    else if (gameboard.index[j*MAX_X+i]->open == OT_BIG)
                        printf ("O");
                    else
                        printf ("?");
	                // check entities
                    for (k = 0; k < gameboard.numentities; k++) {
                        if (gameboard.entities[k].x == i &&
                            gameboard.entities[k].y == j)
                        {
                            printf("\bE");
                        }
                    }            				    
                } else {
					printf ("?");
                }
            } else {
                printf (" ");
            }
        }
        printf("\n");
    }
}

void debugPrintIndexList (void)
{
	int i;
	int t = 0;
	for (i = 0; i < MAX_X*MAX_Y; i++)
	{
		if (gameboard.index[i]) {
			printf ("%4d: %4d %4d\n", i, gameboard.index[i]->x, gameboard.index[i]->y);
			++t;
		}
	}
	printf ("\ntotal found: %d\n", t);
}


#if defined(__CONSOLE_APP__) && defined(__MAPTEST__)
extern int myargc;
extern char **myargv;
int main (int argc, char *argv[])
{
    const char **p;
    V_virtmem_init();

    myargc = argc;
    myargv = argv;

    if (argc < 2 || M_CheckParm("-h") || M_CheckParm("--help")) {
        fprintf(stderr, "%s: -m <mapname>\n", argv[0]);
        return 1;
    }

    if ((p = M_CheckParm("-m")) && argc >= 2) {
	    ++p;
    } else {
        fprintf(stderr, "error: no map name given\n");
	return 1;
    }

#ifdef __DEBUG__
    C_OpenLogFile("debug_pacman.log");
#endif

    //readGameBoard("pacman1.map");
    
    readGameBoard (*p);

//    cheepPrintGameBoard();

    debugPrintGameBoard();

//	debugPrintIndexList();

    V_virtmem_shutdown();

    C_CloseLog();
    return 0;
}
#endif
