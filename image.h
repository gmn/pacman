
#ifndef __IMAGE_H__
#define __IMAGE_H__

#if !defined(__BYTE__) || !defined(__UINT__)
# include "common.h"
#endif

typedef enum {
    IMG_NONE,
    IMG_BMP,
    IMG_TGA,
    IMG_GIF,
    IMG_JPG,
    IMG_PCX,
    IMG_PPM,
    IMG_PNG,
    TOTAL_IMAGE_TYPES
} imagetypes_t;


typedef struct
{
    // size of ID field that follows 18 byte header (0 usually)
    unsigned char indentsize;
    // type of color map 0=none, 1=has palette
    unsigned char colormaptype;
    // type of image 0=none, 1=indexed, 2=rgb, 3=grey, +8=rle packed
    unsigned char imagetype;
    // first color map entry in palette
    unsigned short colormapstart;
    // number of colors in palette
    unsigned short colormaplength;
    // number of bits per palette entry 15, 16, 24, 32
    unsigned char colormapbits;
    // image x origin
    unsigned short xstart;
    // image y origin
    unsigned short ystart;
    // image width in pixels
    unsigned short width;
    // image height in pixels
    unsigned short height;
    // image bits per pixel (8, 16, 24, 32)
    unsigned char pixel_size;
    // image descriptor in bits (vh flip bits)
    unsigned char descriptor;
} tga_header_t;

/*
// Windows GDI Bitmap header
typedef struct tagBITMAP {
  LONG   bmType; 
  LONG   bmWidth; 
  LONG   bmHeight; 
  LONG   bmWidthBytes; 
  WORD   bmPlanes; 
  WORD   bmBitsPixel; 
  LPVOID bmBits; 
} BITMAP, *PBITMAP; 
*/

typedef struct { 
  unsigned int      biSize; 
  int               biWidth; 
  int               biHeight; 
  unsigned short    biPlanes; 
  unsigned short    biBitCount; 
  unsigned int      biCompression; 
  unsigned int      biSizeImage; 
  int               biXPelsPerMeter; 
  int               biYPelsPerMeter; 
  unsigned int      biClrUsed; 
  unsigned int      biClrImportant; 
} bmp_info_t;


typedef struct { 
  unsigned short    magic; 
  unsigned int      totalBytes; 
  unsigned short    reserved1; 
  unsigned short    reserved2; 
  unsigned int      dataOffsetBytes; 
} bmp_header_t;


typedef struct 
{
    imagetypes_t type;

    union
    {
        tga_header_t tga;
        bmp_header_t bmp;
    } header;

    uint h, w;
    uint numbytes;
    uint bpp; // bytes per pixel
    byte *data;

    byte *compiled;
    uint texhandle;

    // the pixel width and height in rendered view 
    point2_t sz;

} image_t;

image_t * IMG_readfileTGA ( const char *, image_t * );
void IMG_MakeGLTexture ( image_t * );
void IMG_compileGLImage ( image_t * );

void IMG_compileImageFile ( const char *, image_t ** );

#endif


