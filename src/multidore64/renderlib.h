/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#ifndef RENDERLIB_H
#define RENDERLIB_H

/* Display modes */
enum RenderMode {
    RMODE_TEXT = 0,       /* 40x25 text mode */
    RMODE_HIRES = 1,      /* 320x200 hires bitmap */
    RMODE_MC_CHAR = 2,    /* 40x25 multicolor character */
    RMODE_MC_BITMAP = 3,  /* 160x200 multicolor bitmap */
    RMODE_ECM = 4         /* 40x25 extended color mode */
};

/* Screen dimensions per mode */
#define RMODE_TEXT_W    40
#define RMODE_TEXT_H    25
#define RMODE_HIRES_W   320
#define RMODE_HIRES_H   200
#define RMODE_MC_W      160
#define RMODE_MC_H      200

/* Palette indices (VIC-II) */
#define RPALETTE_SIZE   16

/* Hardware sprites */
#define RSPIRTE_COUNT   8

/* Point (pixel coordinates) */
struct RPoint {
    unsigned char x;
    unsigned char y;
};

/* Rectangle */
struct RRect {
    unsigned char x, y;
    unsigned char w, h;
};

/*
----------------------------------------------------------
Initialization / Mode
----------------------------------------------------------
void renderlib_init(void);
void renderlib_unload(void);
void renderlib_setmode(unsigned char mode);
unsigned char renderlib_getmode(void);
unsigned int renderlib_screen_w(void);
unsigned int renderlib_screen_h(void);

/*
----------------------------------------------------------
Pixel access (mode-aware)
----------------------------------------------------------
*/
void renderlib_plot(unsigned char x, unsigned char y, unsigned char color);
unsigned char renderlib_getpixel(unsigned char x, unsigned char y);
void renderlib_clear(unsigned char color);

/*
----------------------------------------------------------
Drawing primitives
----------------------------------------------------------
*/
void renderlib_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char color);
void renderlib_rect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color);
void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color);
void renderlib_circle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color);
void renderlib_fillcircle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color);
void renderlib_ellipse(unsigned char cx, unsigned char cy, unsigned char rx, unsigned char ry, unsigned char color);
void renderlib_fillellipse(unsigned char cx, unsigned char cy, unsigned char rx, unsigned char ry, unsigned char color);
void renderlib_polygon(const struct RPoint *pts, unsigned char n, unsigned char color);
void renderlib_fillpolygon(const struct RPoint *pts, unsigned char n, unsigned char color);
void renderlib_blit(unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *data);

/*
----------------------------------------------------------
Floodfill / Region
----------------------------------------------------------
*/
void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor);
char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX, unsigned char *outY);
void renderlib_invert(unsigned char x, unsigned char y, unsigned char w, unsigned char h);
void renderlib_copy(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char w, unsigned char h);

/*
----------------------------------------------------------
Palette / Colors
----------------------------------------------------------
*/
void renderlib_setpalette(unsigned char index, unsigned char color);
unsigned char renderlib_getpalette(unsigned char index);
void renderlib_setbg(unsigned char color);
void renderlib_setborder(unsigned char color);
void renderlib_setcolor(unsigned char background, unsigned char foreground);

/*
----------------------------signed char n, unsigned char enabled);
void renderlib_sprite_expand(unsigned char n, unsigned char x2, unsigned char y2);
void renderlib_sprite_data(unsigned char n, unsigned char pointer);
void renderlib_sprite_all_enable(unsigned char enabled);

/*
----------------------------------------------------------
Character operations (text modes)
----------------------------------------------------------
*/
void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c);
unsigned char renderlib_getchar(unsigned char x, unsigned char y);
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, const char *str);
void renderlib_setcharset(const unsigned char *data); /* 256 chars x 8 bytes = 2048 bytes */

/*
----------------------------------------------------------
Screen operations
----------------------------------------------------------
*/
void renderlib_scroll(unsigned char dir); /* 0=up, 1=down, 2=left, 3=right */
void renderlib_toggle_rendering(unsigned char state);

/*
----------------------------------------------------------
Initialization / Mode
----------------------------------------------------------
*/
void renderlib_init(void);
void renderlib_unload(void);
void renderlib_setmode(unsigned char mode);
unsigned char renderlib_getmode(void);
unsigned int renderlib_screen_w(void);
unsigned int renderlib_screen_h(void);



#endif
