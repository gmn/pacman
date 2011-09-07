

typedef enum {
    FT_NONE,
    FT_BMP,
    FT_FREETYPE
} fonttype_t;


typedef enum {
    FONT_DEFAULT
} font_t;


// a text group 
typedef struct textchunk_s {
    int x, y;
    char *text;
    int strlen;
    unsigned int color;
    fonttype_t type;
    font_t font;
} textchunk_t ;

#define textchunk_default { 0, 0, NULL, 0, 0, FT_FREETYPE, FONT_DEFAULT }
