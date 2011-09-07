
#pragma warning ( disable : 4244 4668 4820 4255 )

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <windows.h>		
#include <GL/gl.h>
#include <GL/glu.h>

#include "pacman.h" // sys_error_warn
#include "virtmem.h" // v_malloc
#include "common.h" // byte



int textures[128];

byte * pix_buffer;
#define PIX_BUFFER_SIZE 50000

void F_MakeFontDisplaylist ( FT_Face face, char ch, int list_base, int * tex_base )
{
    FT_Glyph glyph;
    FT_BitmapGlyph bitmap_glyph;
	FT_Bitmap *bitmap; 
    int width, height, i, j;
    float x, y;


	//Load the Glyph for our character.
	if (FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
		sys_error_warn("FT_Load_Glyph failed");

	//Move the face's glyph into a Glyph object.
    if (FT_Get_Glyph( face->glyph, &glyph ))
		sys_error_warn("FT_Get_Glyph failed");

	//Convert the glyph to a bitmap.
	FT_Glyph_To_Bitmap ( &glyph, ft_render_mode_normal, 0, 1 );
    bitmap_glyph = (FT_BitmapGlyph) glyph;

	bitmap = &bitmap_glyph->bitmap;

//    width =  (bitmap->width + 3) & ~3;
//    height = (bitmap->rows + 3) & ~3;

    width = 1;
    while (width < bitmap->width) width <<= 1;
    height = 1;
    while (height < bitmap->rows) height <<= 1;
    
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if (2*(i+j*width)+1 > PIX_BUFFER_SIZE)
				sys_error_fatal("illegal memory access: %s line %d", __FILE__, __LINE__);
			pix_buffer[2*(i+j*width)] = pix_buffer[2*(i+j*width)+1] = 
				(i >= bitmap->width || j >= bitmap->rows) ?  0 : bitmap->buffer[i + bitmap->width*j];
		}
	}

	//Now we just setup some texture paramaters.
    glBindTexture ( GL_TEXTURE_2D, tex_base[ch] );
	glTexParameteri ( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );

	//Here we actually create the texture itself, notice
	//that we are using GL_LUMINANCE_ALPHA to indicate that
	//we are using 2 channel data.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
		  0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pix_buffer );


	//So now we can create the display list
	glNewList(list_base + ch, GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D, tex_base[ch]);

	//first we need to move over a little so that
	//the character has the right amount of space
	//between it and the one before it.
	glTranslatef((GLfloat)bitmap_glyph->left, 0, 0);

	//Now we move down a little in the case that the
	//bitmap extends past the bottom of the line 
	//(this is only true for characters like 'g' or 'y'.
	glPushMatrix();
	glTranslatef(0, (GLfloat)(bitmap_glyph->top - bitmap->rows), 0);

	//Now we need to account for the fact that many of
	//our textures are filled with empty padding space.
	//We figure what portion of the texture is used by 
	//the actual character and store that information in 
	//the x and y variables, then when we draw the
	//quad, we will only reference the parts of the texture
	//that we contain the character itself.
	x = (float)bitmap->width / (float)width;
	y = (float)bitmap->rows  / (float)height;

	//Here we draw the texturemaped quads.
	//The bitmap that we got from FreeType was not 
	//oriented quite like we would like it to be,
	//so we need to link the texture to the quad
	//so that the result will be properly aligned.
	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex2f(0, bitmap->rows );
	glTexCoord2d(0,y); glVertex2f(0, 0);
	glTexCoord2d(x,y); glVertex2f((GLfloat)bitmap->width,0 );
	glTexCoord2d(x,0); glVertex2f((GLfloat)bitmap->width, (GLfloat)bitmap->rows );
	glEnd();
	glPopMatrix();
	glTranslatef((GLfloat)(face->glyph->advance.x >> 6), 0, 0);

	//increment the raster position as if we were a bitmap font.
	//(only needed if you want to calculate text length)
	//glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);

	//Finnish the display list
	glEndList();
}



void F_InitFont (int h, int *dlist, const char *path)
{
    FT_Library  library;
    FT_Face     face;
    int i, err, h64;

    h64 = 64 * h;

    if (FT_Init_FreeType( &library )) {
        sys_error_warn("unable to init Freetype!");
        return;
    }

    err = FT_New_Face ( library,
                        path,
                        0, 
                        &face );

    if ( err == FT_Err_Unknown_File_Format )
    {
        // the font file could be opened and read by appears
        //  that its format is unsupported
        sys_error_warn("unknown font file format");
    }
    else if ( err )
    {
        // another error code means that the font file could
        //  not be opened or read, or is just broken, or peice of shit.
        sys_error_warn("couldn't open freetype font");
        return;
    }

	// Freetype measures font size in terms of 1/64th pixel.  
	FT_Set_Char_Size( face, h64, h64, 96, 96);

	*dlist = glGenLists(128);

	glGenTextures( 128, (GLuint *)textures );

	for (i = 0; i < 128; i++)
		F_MakeFontDisplaylist (face, i, *dlist, textures);

	FT_Done_Face(face);

	FT_Done_FreeType(library);
}

#define TOTAL_FONTS 7
int total_fonts = TOTAL_FONTS;
int font_dlist[TOTAL_FONTS];

#define DEFAULT_POINT_SIZE 18

void F_InitFreetype(void) 
{
    pix_buffer = V_malloc(PIX_BUFFER_SIZE, PU_STATIC, 0);
    memset ( pix_buffer, 0, PIX_BUFFER_SIZE );
    F_InitFont (DEFAULT_POINT_SIZE, &font_dlist[0], FONTDIR"/DS-DIGIB.TTF");
    F_InitFont (DEFAULT_POINT_SIZE, &font_dlist[1], FONTDIR"/Computerfont.ttf");
    F_InitFont (DEFAULT_POINT_SIZE, &font_dlist[2], FONTDIR"/ARCADECLASSIC.TTF");
    F_InitFont (12,                 &font_dlist[3], FONTDIR"/ARCADECLASSIC.TTF");
    F_InitFont (DEFAULT_POINT_SIZE, &font_dlist[4], FONTDIR"/Share-Regular.ttf");
    F_InitFont (DEFAULT_POINT_SIZE, &font_dlist[5], FONTDIR"/BEBAS___.TTF");
    F_InitFont (DEFAULT_POINT_SIZE, &font_dlist[6], FONTDIR"/suede.ttf");
	V_free(pix_buffer);
}

void pushScreenCoordinateMatrix(void) {
	GLint viewport[4];
	glPushAttrib (GL_TRANSFORM_BIT);
	glGetIntegerv (GL_VIEWPORT, viewport);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	gluOrtho2D (viewport[0],viewport[2],viewport[1],viewport[3]);
	glPopAttrib ();
}

void popScreenCoordinateMatrix(void) {
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}



#define TEXT_SIZE 512
/*
==================================
  F_Printf (fontnum, text)

==================================
 */
void F_Printf (int x, int y, int fnum, uint color, const char *fmt, ...)
{
    char text[TEXT_SIZE];
    va_list ap;
    int sz;
	float modelview_matrix[16];	
    float r, g, b, a;


    r = ((float) (0xFF & ((color & 0xFF000000) >> 24))) / 255.0;
    g = ((float) (0xFF & ((color & 0x00FF0000) >> 16))) / 255.0;
    b = ((float) (0xFF & ((color & 0x0000FF00) >>  8))) / 255.0;
    a = ((float) (color & 0x000000FF)) / 255.0;

//C_WriteLog("colors: %f %f %f %f\n", r, g, b, a);


    memset (text, 0, sizeof(text));
  //  pushScreenCoordinateMatrix();

    if (fmt == NULL)
        *text = 0;
    else {
        va_start(ap, fmt);
        vsprintf(text, fmt, ap);
        va_end(ap);
    }

	//glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);	
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
    glColor4f(r, g, b, a);
	glEnable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);
//	glDisable(GL_DEPTH_TEST);
    glColor4f(r, g, b, a);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

    if (fnum >= total_fonts) 
        fnum = 0; 
	glListBase(font_dlist[fnum]);

	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

    sz = 0;
    while (text[sz] && sz < TEXT_SIZE-1) ++sz;

	//This is where the text display actually happens.
	//For each line of text we reset the modelview matrix
	//so that the line's text will start in the correct position.
	//Notice that we need to reset the matrix, rather than just translating
	//down by h. This is because when each character is
	//draw it modifies the current matrix so that the next character
	//will be drawn immediatly after it.  

	glPushMatrix();
	glLoadIdentity();
    glColor4f(r, g, b, a);
	glTranslatef((GLfloat)x, (GLfloat)(y - DEFAULT_POINT_SIZE), 0);
    glMultMatrixf(modelview_matrix);

	//  The commented out raster position stuff can be useful if you need to
	//  know the length of the text that you are creating.
	//  If you decide to use it make sure to also uncomment the glBitmap command
	//  in make_dlist().
	//	glRasterPos2f(0,0);
    glCallLists(sz, GL_UNSIGNED_BYTE, text);
	//	float rpos[4];
	//	glGetFloatv(GL_CURRENT_RASTER_POSITION ,rpos);
	//	float len=x-rpos[0];

    glPopMatrix();

	glPopAttrib();		

//	popScreenCoordinateMatrix();
    
}


/*
 
   So, how does a font printer work?  I guess if you want a persistant
   piece of text say on the top of the screen
   the print function should be called each time from the GL display 
   function.

   for pacman , its simple enough that you should just compute what to
   write once each frame.
 
   anything more complicated than that, and I'll have to hosue the print
   management from within some more code
 
 */ 
