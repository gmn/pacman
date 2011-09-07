
#include <stdlib.h>

#include "common.h"
#include "image.h"
#include "virtmem.h"
#include "i_system.h"

static struct {
    char *path;
    image_t *img;
} imagefiles[] = 
{
    { IMGDIR"/splash/house.bmp", NULL } ,
    { IMGDIR"/splash/IMG_0512.tga", NULL } ,
    { IMGDIR"/splash/m106_block_full1024x1024.tga", NULL } ,
    /*
    { IMGDIR"/splash/geneticlight512x512.tga", NULL } ,
    { IMGDIR"/splash/greenhouseeffect512x512.tga", NULL } ,
    { IMGDIR"/splash/jupiterstormsystems512x512.tga", NULL } ,
    { IMGDIR"/splash/lifebearmoons512x512.tga", NULL } ,
    { IMGDIR"/splash/longwayhome512x512.tga", NULL } ,
    { IMGDIR"/splash/lookingfuture512x512.tga", NULL } ,
    { IMGDIR"/splash/moonsaturn512x512.tga", NULL } ,
    { IMGDIR"/splash/newearth512x512.tga", NULL } ,
    { IMGDIR"/splash/newspacecity512x512.tga", NULL } ,
    { IMGDIR"/splash/pastfuturemars512x256.tga", NULL } ,
    { IMGDIR"/splash/seedsofmars512x512.tga", NULL } ,
    */
};

static struct {
    int currentImage;
    int slideDelay;

    int lastFrame_ms;
    gbool on;
    gbool autoCycle;
} sa;

#define NUM_IMAGE_FILES (sizeof(imagefiles)/sizeof(*imagefiles))

static void S_InitLocal ( void )
{
    sa.currentImage = 0;
    sa.slideDelay = 10000;
    sa.lastFrame_ms = I_CurrentMillisecond();
    sa.on = gfalse;
    sa.autoCycle = gtrue;
}

void S_LoadSplashData ( void )
{
    int i;

    // so I can read in debugger
    //int nf = NUM_IMAGE_FILES;

    for (i = 0; i < NUM_IMAGE_FILES; i++) 
    {
        /*
        imagefiles[i].img = V_malloc(sizeof(image_t), PU_STATIC, 0);

        IMG_readfileTGA( imagefiles[i].path, imagefiles[i].img ); 

        IMG_compileGLImage( imagefiles[i].img );

        V_free( imagefiles[i].img->data );

        IMG_MakeGLTexture( imagefiles[i].img );

        V_free( imagefiles[i].img->compiled );
        */


        IMG_compileImageFile( imagefiles[i].path, &imagefiles[i].img );
        // tile display size
		imagefiles[i].img->sz[0] = 640; 
        imagefiles[i].img->sz[1] = 480; 
    }

    S_InitLocal();
}

void S_ToggleSlideShow ( void )
{
    sa.on = (sa.on) ? gfalse : gtrue;
}

void S_NextImage ( void )
{
    sa.currentImage = (sa.currentImage+1) % NUM_IMAGE_FILES;
}

image_t * S_GetSplashImage ( void )
{
    int now;
    if (!sa.on)
        return NULL;

    now = I_CurrentMillisecond();
    if (sa.autoCycle && now - sa.lastFrame_ms > sa.slideDelay)
    {
        S_NextImage();
        sa.lastFrame_ms = now;
    }

    return imagefiles[sa.currentImage].img;
}




