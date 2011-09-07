
#ifndef __BYTESPRITE_H__
#define __BYTESPRITE_H__

#if !defined(__BYTE__) || !defined(__USHORT__) || !defined(__GBOOL__) || !defined(__UINT__) || !defined(__VECTYPES__)
# include "common.h"
#endif


typedef struct bytesprite_s {
    int texhandle;
    int h;
    int w;
    vec3_t c;
    byte *compiled;
    byte *data;
} bytesprite_t; 

typedef struct spritelist_s {
    char *name;
    bytesprite_t *bytesprite;
    ushort dimension;
} spritelist_t;

typedef struct bitarray_s {
    char *name;
    uint *data;
    ushort length;
    byte rle;
} bitarray_t;

bytesprite_t * BS_getpacdude (void);

#endif


