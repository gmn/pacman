
#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__ 1

#include <io.h>
#include "tgaviewer.h"


#define FT_UNKNOWN      (0)
#define FT_NORMAL       _A_NORMAL
#define FT_FOLDER       _A_SUBDIR
#define FT_DIRECTORY    FT_FOLDER
#define FT_HIDDEN       _A_HIDDEN
#define FT_SYSTEM       _A_SYSTEM
#define FT_RDONLY       _A_RDONLY


// 
// struct dirent is a list of directories
//  or list of contents of one directory
//
typedef struct dirent_s {
    char    name[FILENAME_SZ];              // name of this file
    int     type;                           // type of this file
    int     size;                           // size of this file in bytes
    char    fullname[FULLPATH_SZ];          // full path name

    struct dirent_s *next_p;
    struct dirent_s *prev_p;
    struct dirent_s *contents_first_p;
    struct dirent_s *contents_current_p;
    int num_files;  // num files read into this dirent's contents_*_p
    int num_dirent; // num of allocated dirent structs under contents_first_p
} dirent_t;

#define __dirent_default  { "", 0, 0, "", NULL, NULL, NULL, NULL, 0, 0 }

//
// struct dirlist is a headnode to a list of directories
// 
typedef struct dirlist_s {
    int total;
    char cwd[FULLPATH_SZ];
    dirent_t * first_p;
    dirent_t * current_p;
} dirlist_t;

#define __dirlist_default { 0, "", NULL, NULL } 


#endif // __FILESYSTEM_H__


dirlist_t *D_NewDirlist(void);
void T_GetDirlist (int , char **, dirlist_t *);
void D_AddDirent_dirlist (dirlist_t *);
dirent_t * D_AddDirent_dirent (dirent_t *);
void D_ReadDirlistContents (dirlist_t *);



