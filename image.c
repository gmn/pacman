
#pragma warning ( disable : 4244 4668 4820 4255 )

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
# include <windows.h>
#endif
#include <gl/gl.h>
#include <gl/glu.h>
#include "common.h"
#include "image.h"
#include "pacman.h"
#include "virtmem.h"

int total_gl_tex = 0;


gbool IMG_getBmpInfo (image_t *img, bmp_info_t *info)
{
    img->w = info->biWidth;
    img->h = info->biHeight;
    img->bpp = info->biBitCount / 8;

    // check for stuff we dont want
    if (img->bpp != 3 && img->bpp != 4) {
        sys_error_warn("bitmap only supports 24 & 32-bit formats! : %d\n", img->bpp);
        return gfalse;
    }
    
    return gtrue;
}


image_t *IMG_readfileBMP ( const char *fullpath, image_t *img )
{
    FILE *fp;
    int i, bytesread;
    int datasize;
    int infosize;
    byte buf[128];
	bmp_header_t *h;
    bmp_info_t *info;
	int fuck_you;
    register byte temp;

    // open
    if ((fp = fopen(fullpath, "rb")) == NULL) {
        sys_error_warn("cant read %s \n", fullpath);
        return NULL;
    }

    // read header straight in
    if (fread(buf, 14, 1, fp) == NULL) {
        sys_error_warn("error reading header\n");
        return NULL;
    }

	h = &img->header.bmp;
	h->magic = (unsigned short) *(unsigned short *)buf;
	h->totalBytes = (unsigned int) *(unsigned int *)&buf[2];
	h->reserved1 = h->reserved2 = 0;
	h->dataOffsetBytes = (unsigned int) *(unsigned int *)&buf[10];

    // check magic number
    if (img->header.bmp.magic != 19778) {
        sys_error_warn("Not a Bitmap File\n");
        return NULL;
    }

    // read info portion of header
    memset( buf , 0, sizeof(buf) );
    //infosize = img->header.bmp.dataOffsetBytes - sizeof(bmp_header_t);
    infosize = img->header.bmp.dataOffsetBytes - 14;

    info = (bmp_info_t *) buf;

    if (fread(info, infosize, 1, fp) == NULL) {
        sys_error_warn("error reading bitmap info from file\n");
        return NULL;
    } 

    // record info
    if (! IMG_getBmpInfo(img, info) ) {
        return NULL;
    }
    
    // malloc data portion
	
	//   2 bytes padding (0x00, 0x00) on the end of bmp
    datasize = img->header.bmp.totalBytes - img->header.bmp.dataOffsetBytes;
    img->data = V_malloc (datasize, PU_STATIC, 0);
    memset (img->data, 0, datasize);
    img->numbytes = datasize;
	img->type = IMG_BMP;

    // get data
    fseek ( fp, img->header.bmp.dataOffsetBytes, SEEK_SET );

	/*
    do {
        i = fread(img->data, 1, 1024, fp);
        bytesread += i;
        if (i <= 0)
            break;
    } while(1);
	*/

	// read image data
    bytesread = 0;
	while ((i = fread(&img->data[bytesread], 1, 8192, fp)) > 0)
		bytesread += i;

    if ( bytesread != datasize ) {
        sys_error_warn("bitmap: incorrect datasize: %d\n", bytesread);
        return NULL;
    }

    /* swap red and blue (bgr -> rgb) */ 
    /*
    if (img->bpp == 3) {
    for (i = 0; i < bmp->size; i+= 3)
    {
        temp = handle[i+2];
        handle[i+2] = handle[i]; 
        handle[i] = temp;
    }    
    } */


    /* swap rows too */
   //  
   /*
    i = bmp->h;
    
    for (i = 0; i < ((int)(bmp->h/2)); i++)
    {
        memcpy(t, &handle[i*bmp->pitch], bmp->pitch);
        memcpy(&handle[i*bmp->pitch], 
                &handle[(bmp->h-1-i)*bmp->pitch], 
                bmp->pitch);
        memcpy(&handle[(bmp->h-1-i)*bmp->pitch], t, bmp->pitch);
    }
        
    */

    return img;
}





void IMG_MakeGLTexture (image_t *img)
{
	GLenum err = GL_NO_ERROR;

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (1, &img->texhandle);

    glBindTexture(GL_TEXTURE_2D, img->texhandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 
                  0, GL_RGBA, GL_UNSIGNED_BYTE, img->compiled);

	err = glGetError();
	if (err != GL_NO_ERROR)
		C_WriteLog("gl error: %s texnum: %d\n", gluErrorString(err), total_gl_tex);
}


void IMG_copyHeaderTGA (tga_header_t *tga, byte *buf_p)
{
    tga->indentsize     = *buf_p++;
    tga->colormaptype   = *buf_p++;
    tga->imagetype      = *buf_p++;
    tga->colormapstart  = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->colormaplength = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->colormapbits   = *buf_p++;
    tga->xstart         = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->ystart         = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->width          = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->height         = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->pixel_size     = *buf_p++;
    tga->descriptor     = *buf_p;
}


image_t * IMG_readfileTGA (const char *fullpath, image_t *img)
{
    FILE *fp;
    byte tga_header_buf[18];
    int sz;
    int i,j;

    if ((fp = fopen (fullpath, "rb")) == NULL)
    {
        sys_error_fatal ("couldn't open file: %s\n", fullpath);
    }

    if ( fread(tga_header_buf, 18, 1, fp) != 1)
    {
        sys_error_fatal ("couldn't read file: %s\n", fullpath);
    }

    if (( sz = sys_file_size(fullpath) ) < 0)
        sys_error_fatal ("couldn't get filesize: %s\n", fullpath);

    img->data = V_malloc (sz, PU_STATIC, 0);
    memset (img->data, 0, sz);

    IMG_copyHeaderTGA (&img->header.tga, tga_header_buf);

    i = 0;

	/*
    for(;;) 
    {
        j = fread(&img->data[i], 1, 8192, fp);
        if (j <= 0)
            break;
        i += j;
    } */

	
	while ((j = fread(&img->data[i], 1, 8192, fp)) > 0)
	{
		i += j;
	}
	


    fclose(fp);
	
	img->numbytes = i;
    img->type = IMG_TGA;
    
    img->h = img->header.tga.height;
    img->w = img->header.tga.width;
    img->bpp = img->header.tga.pixel_size / 8;

	return img;
}

/*
==================== 
  IMG_compileGLImage()
   - was T_gl_compile_img
==================== 
*/
void IMG_compileGLImage (image_t *img)
{
    int i, j, rgba_sz;

    byte *compiled;

    if (img->bpp != 3 && img->bpp != 4)
        sys_error_fatal ("only handling 24 or 32-bit right now");

    if (img->bpp == 3) {
	    rgba_sz = img->numbytes * 4;
	    rgba_sz /= 3;
    } else { 
        rgba_sz = img->numbytes;
    }

    rgba_sz += 16; // pad

    compiled = V_malloc (rgba_sz, PU_STATIC, 0);
    memset (compiled, 0, rgba_sz);

    if (img->bpp == 3) {
        for (i = 0, j = 0; i < (int)img->numbytes; i += img->bpp, j+=4)
        {
            if (img->data[i] || img->data[i+1] || img->data[i+2]) {
                compiled[j+2] = img->data[i+0];
                compiled[j+1] = img->data[i+1];
                compiled[j+0] = img->data[i+2];
                compiled[j+3] = 255;
            }
        }
    } else {
        for (i = 0, j = 0; i < (int)img->numbytes; i += img->bpp, j+=4)
        {
            compiled[j+2] = img->data[i+0];
            compiled[j+1] = img->data[i+1];
            compiled[j+0] = img->data[i+2];
            compiled[j+3] = img->data[i+3];
        }
    }
    img->compiled = compiled;
}

void IMG_compileImageFile ( const char *path, image_t **ip )
{
    int type;
    int i;

    *ip = V_malloc(sizeof(image_t), PU_STATIC, 0);

    // get type from filename extension
    for (i = 0; path[i] != '\0'; i++)
        ;

    do {
        --i;
    } while ( path[i] != '.' );
    ++i;


    type = IMG_NONE;
    if ( !C_strncasecmp(&path[i], "BMP", 3 ))
        type = IMG_BMP;
    else if ( !C_strncasecmp(&path[i], "TGA", 3 ))
        type = IMG_TGA;


    // 
    switch(type) {
    case IMG_TGA:
        IMG_readfileTGA( path, *ip ); 
        break;
    case IMG_BMP:
        IMG_readfileBMP( path, *ip );
        break;
    case IMG_GIF:
    case IMG_JPG:
    case IMG_PCX:
    case IMG_PPM:
    case IMG_PNG:
    default:
        sys_error_warn("image type: %d not supported\n", type);
        V_free(*ip);
        (*ip)=NULL;
        return;
    }

    IMG_compileGLImage( *ip );

    V_free( (*ip)->data );
    (*ip)->data = NULL;

    IMG_MakeGLTexture( *ip );

    V_free( (*ip)->compiled );
    (*ip)->compiled = NULL;
}


