
/* common.c
 *  * cmd line handling/checking
 *  * zone memory management 
 *  * sys_error_fatal, sys_error_warn
 */

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
# include <process.h>
#endif


#include "i_system.h"
#include "virtmem.h"
#include "pacman.h"
#include "player.h"

#define EXECUTABLE_NAME "pacman"
#define DEFAULT_LOGFILE_NAME "debug.log"


/*
====================
  sys_error_fatal
====================
*/

void sys_error_fatal (char *fmt, ...)
{
	va_list args;
#ifndef __CONSOLE_APP__
    char msg[256];
#endif

	va_start(args, fmt);
#ifdef __CONSOLE_APP__
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
#else
    C_strncpy (msg, "error: ", 8);
    vsprintf(&msg[7], fmt, args);
//    MessageBox (NULL, msg, "Error!", MB_OK|MB_ICONEXCLAMATION);
    I_WinMsgBox( msg, "Error!" );
#endif
	va_end(args);

    // return mem to system
    V_virtmem_shutdown();

    // calls exit
    I_SysQuit();
}

/*
==================== 
 sys_error_warn
==================== 
*/
void sys_error_warn (char *fmt, ...)
{
	va_list args;
#ifndef __CONSOLE_APP__
    char msg[256];
#endif

	va_start(args, fmt);
#ifdef __CONSOLE_APP__
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
#else
    C_strncpy (msg, "warning: ", 10);
    vsprintf(&msg[9], fmt, args);
//    MessageBox (NULL, msg, "Warning!", MB_OK|MB_ICONEXCLAMATION);
    I_WinMsgBox ( msg, "Warning!" );
#endif
	va_end(args);
}


int C_strncmp (const char *s, const char *n, int sz)
{
    int diff = 0;
    int i = 0, se = -1, ne = -1;
    while (*s || *n)
    {
        if (i > sz)
            break;

        if (se != -1 && ne != -1)
            break;

        if (!*s) 
            se = i;
        if (!*n) 
            ne = i;

        if (se > -1 && ne > -1)
            return diff;

        if (se > -1)
            diff += -(*n);
        else if (ne > -1)
            diff += -(*s);
        else
            diff += *s - *n;

        if (se == -1)
            ++s; 
        if (ne == -1)
            ++n;
        ++i;
    }
    return diff;
}

/*
======================
 C_strncasecmp
  - case insensitive string compare
======================
*/
int C_strncasecmp (const char *s, const char *n, int sz)
{
    int i;
    int t, tot;

    // 'a' - 'A' ==> 97 - 65 ==> 32
    // 'z' - 'Z' ==> 122 - 90
    

    tot = i = 0;


    while (i < sz)
    {

        if (n[i] == 0) {
            if (s[i] == 0)
                break;
            else
                tot -= s[i];
        } else if (s[i] == 0) {
                tot += n[i];
        } 
        else 
        {
            t = n[i] - s[i];

            // both are char and are same case
            if ((n[i] >= 97 && n[i] <= 122  && s[i] >= 97 && s[i] <= 122) ||
                (n[i] >= 65 && n[i] <= 90   && s[i] >= 65 && s[i] <= 90))
            {
                tot += t;
            } 
            else 
            {
                // n is char & lower
                if (n[i] >= 97 && n[i] <= 122) 
                {
                    // s is char & upper
                    if (s[i] >= 65 && s[i] <= 90) 
                    {
                        tot += n[i] - (s[i] + 32);
                    }
                    // s is not char (s cant be lower)
                    else
                    {
                        tot += t;
                    }
                } 
                // n is char & is upper
                else if (n[i] >= 65 && n[i] <= 90) 
                {
                    // s is char & lower
                    if (s[i] >= 97 && s[i] <= 122) 
                    {
                        tot += n[i] - (s[i] - 32);
                    }
                    // s is not char (s cant be upper)
                    else
                    {
                        tot += t;
                    }
                }
                else
                {
                    tot += t;
                }
            }
        }

        ++i;
    }

    return tot;
}





/*
======================
  C_strncpy

  ensure against overruns, but also to make sure string is null 
   terminated no matter what
======================
*/
int C_strncpy (char *to, const char *from, int sz)
{
    int i = 0;
    int nullset = 0;

//    while (1)
    for (;;)
    {
        if (i >= sz)
            break;

        to[i] = from[i];

        if (from[i] == '\0') {
            nullset = 1;
            break;
        }

        ++i;
    }

    if (!nullset)
        to[sz-1] = '\0';

    if (i >= sz)
        return -1;

    return sz;
}


/*
======================
  C_snprintf
======================
*/
int C_snprintf (char *buf, int sz, char *fmt, ...)
{
	va_list args;
	char *p, *sval;
	int ival;
	double dval;
    char tmp[256];
    int i;
    int stoploop = 1;

    char *buf_p = buf;

    // the maximum pointer position exclusive
    int maxp = (int)buf + sz;



	/* make args point to first unamed arg */
	va_start( args, fmt );

    /* loop through format string */
    /* while not at the end of the string and 
     * src buffer not full */
	for (p = fmt; *p && (int)buf_p < maxp; p++) 
    {

        if (*p != '%') {
            *buf_p++ = *p;
			continue;
        }

		switch (*++p) {
		case 'd':
			ival = va_arg (args, int);
			sprintf(tmp, "%d", ival);
            i = 0;
            stoploop = 1;
            while (tmp[i] && stoploop) 
            {
                *buf_p++ = tmp[i++];
                if ((int)buf_p >= maxp)
                    stoploop = 0;
            }
            if (!stoploop)
                continue;
            
			break;
		case 'f':
			dval = va_arg (args, double);
			sprintf(tmp, "%f", dval);
            i = 0;
            stoploop = 1;
            while (tmp[i] && stoploop) 
            {
                *buf_p++ = tmp[i++];
                if ((int)buf_p >= maxp)
                    stoploop = 0;
            }
            if (!stoploop)
                continue;

			break;
		case 's':
			for (sval = va_arg (args, char *); *sval && stoploop; sval++) {
                *buf_p++ = *sval;
                if ((int)buf_p >= maxp)
                    stoploop = 0;
            }
            if (!stoploop)
                continue;

			break;
		default:
            *buf_p++ = *p;
			break;
		}
	}

	va_end(args);

    if ((int)buf_p > maxp) {
        sys_error_warn ("overrun in C_snprintf");
        buf[sz-1] = '\0';
        return -1;
    } else if ((int)buf_p == maxp) 
        buf[sz-1] = '\0';
    else 
        *buf_p = '\0';

    return (int)buf_p - (int)buf;
}



/*
====================
 Cmdline Arguments
====================
*/

int     myargc;
char ** myargv;

int ParseWinMainArgs (const char *cmdline)
{
    char *p, *arg;
    int i;
	int sawchar = 0;

    myargc = 1;

    p = cmdline;

    // eat up any prepend spaces
    while (*p == ' ')
        ++p;

    //while (1)
    for (;;)
    {
		if (*p != ' ' && *p != '\0') {
            ++p;
			sawchar++;
		}
        else
        {
            if (*p == ' ') {
                ++myargc;

                // eat up extra space til next arg
                while (*++p == ' ' )
                    ;
            } 
            else if (*p == '\0')
            {
				if (sawchar)
	                ++myargc;
                break;
            }
        }
    }



	myargv = (char **) V_malloc (myargc * sizeof (char *), PU_STATIC, NULL);
	
	

    for (i = 0; i < myargc; i++)
    {
        myargv[i] = (char *) V_malloc (FULLPATH_SZ, PU_STATIC, NULL);
    }


	C_strncpy (myargv[0], EXECUTABLE_NAME, FULLPATH_SZ);


    p = arg = cmdline;
    // eat up any prepend spaces
    while (*p == ' ') {
        ++arg;
        ++p;
    }

    i = 1;
	sawchar = 0;
    //while (1)
    for(;;)
    {
		if (*p != ' ' && *p != '\0') {
            ++p;
			sawchar++;
		}
        else
        {
            if (*p == ' ') {
                C_snprintf (myargv[i], p - arg + 1, "%s" , arg);
                myargv[i][p - arg + 1] = '\0';

                ++i;

                // eat up any extra space til next arg
                while ( *++p == ' ' )
                    ;
                arg = p;
            } 
            else if (*p == '\0')
            {
				if (sawchar)
	                C_strncpy (myargv[i++], arg, FULLPATH_SZ);
                break;
            }
        }
    }

    if (i == myargc)
        return 0;

    return -1;
}

/*
====================
  M_CheckParm()
====================
*/
const char ** M_CheckParm(char *arg)
{
	int i;
	for (i = 1; i < myargc; i++) {
		if (! C_strncasecmp(arg, myargv[i], FULLPATH_SZ)) 
			return &myargv[i];
	}

	return ((const char **)0);
}

/*
==================== 
 C_GetExeName
==================== 
*/
const char * C_GetExeName(void)
{
    static char buf[FULLPATH_SZ];
    C_strncpy (buf, *myargv, FULLPATH_SZ);
    return (const char *) buf;
}

int sys_file_size (const char *filename)
{
	return I_FileSize(filename);
}


/*
====================
 C_PrintStats
 - pacengine timing stats
====================
*/
extern pacengine_t pe;
void C_PrintStats (void)
{
    float t = pe.thistic - pe.starttic;
    C_WriteLog("total tics: %d\n", pe.thistic - pe.starttic);
    C_WriteLog("Secs played: %f\n", (t /= 1000.0));
    C_WriteLog("logic fps: %f\n", ((float)pe.logicframe)/ t);
    C_WriteLog("render fps: %f\n", ((float)pe.renderframe)/ t);
}


void sys_shutdown_safe (void)
{
    C_PrintStats();
    I_ShutdownVideo();
    V_virtmem_shutdown();
    C_CloseLog();
    I_SysQuit();
}


void sys_shutdown_confirm (void)
{
    if (! I_WinConfirmBox("Are you sure you want to exit?", "confirm exit"))
        return;

    C_WriteLog("player quit\n");
#ifndef __CONSOLE_APP__
    C_WriteLog("\n******************************\n**** Final Score: %7d ****\n******************************\n", P_GetScore());
#endif

    C_PrintStats();

    I_ShutdownVideo();
    V_virtmem_shutdown();
    C_CloseLog();
    I_SysQuit();
}

static FILE *logfp = NULL;

void C_OpenLogFile (const char *filename)
{
    char fname[FILENAME_SZ];

    if (! filename)
        C_strncpy ( fname, DEFAULT_LOGFILE_NAME, FILENAME_SZ ); 
    else
        C_strncpy ( fname, filename, FILENAME_SZ ); 

    if ((logfp = fopen( fname, "a" )) == NULL) {
        sys_error_warn ("couldn't open logfile for writing");
        return;
    }

    fprintf(logfp, "\nNew Game Started - %s", I_GetTimeStamp());
}

void C_WriteLog (const char *fmt, ...)
{
	va_list args;
    if (!logfp || !fmt)
        return;

	va_start(args, fmt);
	vfprintf(logfp, fmt, args);
	va_end(args);

    fflush(logfp);
}

void C_CloseLog(void)
{
    if (logfp) {
        fclose(logfp);
        logfp = NULL;
    }
}

//////////////////////////////////////////
//  
// stuff to maintain highscore list
//
//////////////////////////////////////////

#define SCOREFILE_NAME ".highscores"
#define TOTAL_HIGHSCORES 16
static scorelist_t highscores[TOTAL_HIGHSCORES];
#define HIGHSCORES_SZ (sizeof(highscores[0]) * TOTAL_HIGHSCORES)

void C_WriteScoreFile (void)
{
    FILE *fp = NULL;
    size_t w;

    if ((fp = fopen( SCOREFILE_NAME, "wb" )) == NULL) 
        sys_error_warn ("couldn't open scorefile");

    if ((w = fwrite(highscores, 1, HIGHSCORES_SZ, fp)) != HIGHSCORES_SZ) 
        sys_error_warn ("only wrote %d bytes to scorefile", (int)w);
    
    if (fp) 
        fclose(fp);
}

void C_LoadScoreFile (void)
{
    FILE *fp = NULL;
    size_t r;
    int i;

    // scorefile found
    if ((fp = fopen( SCOREFILE_NAME, "rb" )) != NULL) 
    {
        if ((r = fread (highscores, 1, HIGHSCORES_SZ, fp)) != HIGHSCORES_SZ)
            sys_error_warn ("only read %d bytes from scorefile", (int)r);

C_WriteLog("opened scorefile. read: %d\n", (int)r);
        fclose(fp);
    }
    else // scorefile not found, load default scorelist
    {
C_WriteLog("generating scorelist\n");
        i = 0;
        
        C_strncpy(highscores[i].initials, "PWN", 4);
        highscores[i++].score = 10000;
        C_strncpy(highscores[i].initials, "BUZ", 4);
        highscores[i++].score = 9000;
        C_strncpy(highscores[i].initials, "JOE", 4);
        highscores[i++].score = 8000;
        
        for (; i < TOTAL_HIGHSCORES; i++) {
            memset (highscores[i].initials, 0, 4);
            highscores[i].score = 0;
        }
    }
}

/* 
 * C_AddGameScore
 * this is where the scores are entered, the lowest discarded
 * The local scorelist_t 'highscores' should always be kept in
 * a sorted state.
 */
int C_AddGameScore (int score, const char *name)
{
    int snum = TOTAL_HIGHSCORES - 1;
    int i;

    // check score is even good enough to be added to list
    if (score <= highscores[TOTAL_HIGHSCORES-1].score)
        return -1;

    // find slot
    while (score > highscores[snum-1].score && snum > 0)
        --snum;
    
    // found slot, copy from this slot to end of list into the
    //  next highest slot, dropping the last one off the end.
    i = TOTAL_HIGHSCORES - 1;
    while (i != snum && i > 0) {
        highscores[i].score = highscores[i-1].score;
        C_strncpy (highscores[i].initials, highscores[i-1].initials, 4);
        --i;
    }

    // record the new score
    highscores[snum].score = score;
    C_strncpy(highscores[snum].initials, name, 4);

    return snum;
}

scorelist_t * C_GetScoreListP (void)
{
    return &highscores[0];
}

int C_GetHighScore (void)
{
    return highscores[0].score;
}

static int rand_next;

// returns from 0 to top minus one
int C_Rand (int top)
{
    return (int) (top * (rand() / (RAND_MAX + 1.0)));

    // another method
    rand_next = rand_next * 1103515245 + 12345;
    return ((unsigned) (rand_next/65536) % 32768);
}


void C_Srand (void)
{
#ifdef __CONSOLE_APP__
    int rand_index = 2;
#else
    extern int rand_index;
#endif

    srand(getpid()*time(0));
    rand_next = getpid() * time (0);
    rand_index = rand_next;
}
