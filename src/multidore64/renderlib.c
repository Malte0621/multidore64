/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#include <c64/vic.h>
#include "renderlib.h"

/* Character and color RAM (C64 text mode). Pointers let oscar64 emit
   direct loads/stores instead of cc65 PEEK/POKE calls. */
static volatile char * const charram = (volatile char *)0x0400;
static volatile char * const colram = (volatile char *)0xd800;

/* Palette at $D021-$D030 */
static volatile char * const palette = (volatile char *)0xd021;

static unsigned char currentMode = RMODE_TEXT;
static unsigned char hasBeenInitialized = 0;

/* Iterative floodfill stack (max 320*200=64000 pixels in hires, but we
   limit to 40*25=1000 cells in text mode. Use 2000 entries for safety.) */
static unsigned char ff_stack[4000];
static unsigned int ff_sp = 0;

/*
----------------------------------------------------------
Internal pixel access (mode-aware)
----------------------------------------------------------
*/

/* Plot a pixel in hires bitmap mode (320x200) */
static void plot_hires(unsigned char x, unsigned char y, unsigned char color)
{
    unsigned int addr = (y >> 3) * 40 + (x >> 3);
    unsigned char row = y & 7;
    unsigned char bit = 7 - (x & 7);
    if (color)
        charram[addr + row] |= (1 << bit);
    else
        charram[addr + row] &= ~(1 << bit);
}

/* Get a pixel in hires bitmap mode */
static unsigned char get_hires(unsigned char x, unsigned char y)
{
    unsigned int addr = (y >> 3) * 40 + (x >> 3);
    unsigned char row = y & 7;
    unsigned char bit = 7 - (x & 7);
    return (charram[addr + row] >> bit) & 1;
}

/* Plot a pixel in multicolor bitmap mode (160x200) */
static void plot_mc(unsigned char x, unsigned char y, unsigned char color)
{
    unsigned int addr = (y >> 3) * 40 + (x >> 2);
    unsigned char row = y & 7;
    unsigned char shift = 6 - ((x & 3) << 1);
    volatile char *p = &charram[addr + row];
    *p = (*p & ~(3 << shift)) | ((color & 3) << shift);
}

/* Get a pixel in multicolor bitmap mode */
static unsigned char get_mc(unsigned char x, unsigned char y)
{
    unsigned int addr = (y >> 3) * 40 + (x >> 2);
    unsigned char row = y & 7;
    unsigned char shift = 6 - ((x & 3) << 1);
    return (charram[addr + row] >> shift) & 3;
}

/* Plot a pixel in text mode (40x25) - sets the character cell color */
static void plot_text(unsigned char x, unsigned char y, unsigned char color)
{
    if (x >= 40 || y >= 25) return;
    colram[y * 40 + x] = color;
}

/* Get a pixel in text mode - returns the character cell color */
static unsigned char get_text(unsigned char x, unsigned char y)
{
    if (x >= 40 || y >= 25) return 0;
    return colram[y * 40 + x];
}

/*
----------------------------------------------------------
Public pixel access
----------------------------------------------------------
*/

void renderlib_plot(unsigned char x, unsigned char y, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    switch (currentMode)
    {
        case RMODE_HIRES:
            plot_hires(x, y, color);
            break;
        case RMODE_MC_BITMAP:
            plot_mc(x, y, color);
            break;
        default:
            plot_text(x, y, color);
            break;
    }
}

unsigned char renderlib_getpixel(unsigned char x, unsigned char y)
{
    if (hasBeenInitialized == 0) return 0;
    switch (currentMode)
    {
        case RMODE_HIRES:
            return get_hires(x, y);
        case RMODE_MC_BITMAP:
            return get_mc(x, y);
        default:
            return get_text(x, y);
    }
}

void renderlib_clear(unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    if (currentMode == RMODE_HIRES)
    {
        /* Clear all 1000 character bytes */
        for (unsigned int i = 0; i < 1000; i++)
            charram[i] = color ? 0xFF : 0x00;
    }
    else if (currentMode == RMODE_MC_BITMAP)
    {
        for (unsigned int i = 0; i < 1000; i++)
            charram[i] = (color & 3) * 0x55; /* 01010101 pattern for color 1, etc. */
    }
    else
    {
        /* Text mode: clear color RAM */
        for (unsigned int i = 0; i < 1000; i++)
            colram[i] = color;
        /* Clear character RAM to spaces */
        for (unsigned int i = 0; i < 1000; i++)
            charram[i] = ' ';
    }
}

/*
----------------------------------------------------------
Mode management
----------------------------------------------------------
*/

void renderlib_setmode(unsigned char mode)
{
    if (hasBeenInitialized == 0) return;
    currentMode = mode;
    switch (mode)
    {
        case RMODE_TEXT:
            vic_setmode(VICM_TEXT, (const char *)0x0400, (const char *)0x0000);
            break;
        case RMODE_HIRES:
            vic_setmode(VICM_HIRES, (const char *)0x0400, (const char *)0x0000);
            break;
        case RMODE_MC_CHAR:
            vic_setmode(VICM_TEXT_MC, (const char *)0x0400, (const char *)0x0000);
            break;
        case RMODE_MC_BITMAP:
            vic_setmode(VICM_HIRES_MC, (const char *)0x0400, (const char *)0x0000);
            break;
        case RMODE_ECM:
            vic_setmode(VICM_TEXT_ECM, (const char *)0x0400, (const char *)0x0000);
            break;
    }
}

unsigned char renderlib_getmode(void)
{
    return currentMode;
}

unsigned int renderlib_screen_w(void)
{
    switch (currentMode)
    {
        case RMODE_HIRES: return 320;
        case RMODE_MC_BITMAP: return 160;
        default: return 40;
    }
}

unsigned int renderlib_screen_h(void)
{
    switch (currentMode)
    {
        case RMODE_HIRES:
        case RMODE_MC_BITMAP: return 200;
        default: return 25;
    }
}

/*
----------------------------------------------------------
Drawing primitives
----------------------------------------------------------
*/

void renderlib_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int sx = dx < 0 ? -1 : 1;
    int sy = dy < 0 ? -1 : 1;
    int err = adx - ady;
    int x = x1, y = y1;
    unsigned char sw = renderlib_screen_w();
    unsigned char sh = renderlib_screen_h();
    for (;;)
    {
        if (x < sw && y < sh)
            renderlib_plot((unsigned char)x, (unsigned char)y, color);
        if (x == x2 && y == y2) break;
        int e2 = err << 1;
        if (e2 > -ady) { err -= ady; x += sx; }
        if (e2 < adx)  { err += adx; y += sy; }
    }
}

void renderlib_rect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    renderlib_line(x, y, x + w - 1, y, color);
    renderlib_line(x, y + h - 1, x + w - 1, y + h - 1, color);
    renderlib_line(x, y, x, y + h - 1, color);
    renderlib_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    for (unsigned char j = 0; j < h; j++)
    {
        for (unsigned char i = 0; i < w; i++)
        {
            renderlib_plot(x + i, y + j, color);
        }
    }
}

void renderlib_circle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    int x = r, y = 0;
    int err = 1 - r;
    while (x >= y)
    {
        renderlib_plot(cx + x, cy + y, color);
        renderlib_plot(cx - x, cy + y, color);
        renderlib_plot(cx + x, cy - y, color);
        renderlib_plot(cx - x, cy - y, color);
        renderlib_plot(cx + y, cy + x, color);
        renderlib_plot(cx - y, cy + x, color);
        renderlib_plot(cx + y, cy - x, color);
        renderlib_plot(cx - y, cy - x, color);
        y++;
        if (err < 0)
            err += 2 * y + 1;
        else
        {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void renderlib_fillcircle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    int x = r, y = 0;
    int err = 1 - r;
    while (x >= y)
    {
        /* Draw horizontal lines for each octant pair */
        for (int i = cx - x; i <= cx + x; i++)
        {
            if (i >= 0)
            {
                renderlib_plot((unsigned char)i, (unsigned char)(cy + y), color);
                renderlib_plot((unsigned char)i, (unsigned char)(cy - y), color);
            }
        }
        for (int i = cx - y; i <= cx + y; i++)
        {
            if (i >= 0)
            {
                renderlib_plot((unsigned char)i, (unsigned char)(cy + x), color);
                renderlib_plot((unsigned char)i, (unsigned char)(cy - x), color);
            }
        }
        y++;
        if (err < 0)
            err += 2 * y + 1;
        else
        {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void renderlib_ellipse(unsigned char cx, unsigned char cy, unsigned char rx, unsigned char ry, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    if (rx == 0 || ry == 0) return;
    int x = 0, y = ry;
    int rx2 = rx * rx, ry2 = ry * ry;
    int d = ry2 - rx2 * ry + rx2 / 4;
    while (rx2 * y >= ry2 * x)
    {
        renderlib_plot(cx + x, cy + y, color);
        renderlib_plot(cx - x, cy + y, color);
        renderlib_plot(cx + x, cy - y, color);
        renderlib_plot(cx - x, cy - y, color);
        y--;
        if (d < 0)
            d += ry2 * (2 * x + 3);
        else
            d += ry2 * (2 * x + 3) - rx2 * (2 * y - 1);
        x++;
    }
    x = rx;
    y = 0;
    d = ry2 * (2 * x - 1) * (2 * x - 1) + rx2 * 4 * y * y - 4 * rx2 * ry2;
    while (y <= x)
    {
        renderlib_plot(cx + x, cy + y, color);
        renderlib_plot(cx - x, cy + y, color);
        renderlib_plot(cx + x, cy - y, color);
        renderlib_plot(cx - x, cy - y, color);
        y++;
        if (d < 0)
            d += ry2 * (2 * x + 3) * 4;
        else
            d += ry2 * (2 * x + 3) * 4 - rx2 * (2 * y - 1) * 4;
        x--;
    }
}

void renderlib_fillellipse(unsigned char cx, unsigned char cy, unsigned char rx, unsigned char ry, unsigned char color)
{
    if (hasBeenInitialized == 0) return;
    if (rx == 0 || ry == 0) return;
    int x = 0, y = ry;
    int rx2 = rx * rx, ry2 = ry * ry;
    int d = ry2 - rx2 * ry + rx2 / 4;
    while (rx2 * y >= ry2 * x)
    {
        for (int i = cx - x; i <= cx + x; i++)
        {
            if (i >= 0)
            {
                renderlib_plot((unsigned char)i, (unsigned char)(cy + y), color);
                renderlib_plot((unsigned char)i, (unsigned char)(cy - y), color);
            }
        }
        y--;
        if (d < 0)
            d += ry2 * (2 * x + 3);
        else
            d += ry2 * (2 * x + 3) - rx2 * (2 * y - 1);
        x++;
    }
    x = rx;
    y = 0;
    d = ry2 * (2 * x - 1) * (2 * x - 1) + rx2 * 4 * y * y - 4 * rx2 * ry2;
    while (y <= x)
    {
        for (int i = cx - x; i <= cx + x; i++)
        {
            if (i >= 0)
            {
                renderlib_plot((unsigned char)i, (unsigned char)(cy + y), color);
                renderlib_plot((unsigned char)i, (unsigned char)(cy - y), color);
            }
        }
        y++;
        if (d < 0)
            d += ry2 * (2 * x + 3) * 4;
        else
            d += ry2 * (2 * x + 3) * 4 - rx2 * (2 * y - 1) * 4;
        x--;
    }
}

void renderlib_polygon(const struct RPoint *pts, unsigned char n, unsigned char color)
{
    if (hasBeenInitialized == 0 || n < 3) return;
    for (unsigned char i = 0; i < n; i++)
    {
        unsigned char j = (i + 1) % n;
        renderlib_line(pts[i].x, pts[i].y, pts[j].x, pts[j].y, color);
    }
}

void renderlib_fillpolygon(const struct RPoint *pts, unsigned char n, unsigned char color)
{
    if (hasBeenInitialized == 0 || n < 3) return;
    /* Scanline fill */
    unsigned char minY = 255, maxY = 0;
    for (unsigned char i = 0; i < n; i++)
    {
        if (pts[i].y < minY) minY = pts[i].y;
        if (pts[i].y > maxY) maxY = pts[i].y;
    }
    for (unsigned char y = minY; y <= maxY; y++)
    {
        /* Find intersections with all edges */
        unsigned char intersections[16];
        unsigned char count = 0;
        for (unsigned char i = 0; i < n && count < 16; i++)
        {
            unsigned char j = (i + 1) % n;
            unsigned char y1 = pts[i].y, y2 = pts[j].y;
            if (y1 == y2) continue;
            if (y1 > y2) { unsigned char t = y1; y1 = y2; y2 = t; }
            if (y >= y1 && y < y2)
            {
                int x1 = pts[i].x, x2 = pts[j].x;
                int dy = pts[j].y - pts[i].y;
                int dx = x2 - x1;
                int ix = x1 + (int)((y - pts[i].y) * dx / dy);
                if (ix >= 0 && ix < 256)
                    intersections[count++] = (unsigned char)ix;
            }
        }
        /* Sort intersections (insertion sort, small n) */
        for (unsigned char i = 1; i < count; i++)
        {
            unsigned char key = intersections[i];
            int j = i - 1;
            while (j >= 0 && intersections[j] > key)
            {
                intersections[j + 1] = intersections[j];
                j--;
            }
            intersections[j + 1] = key;
        }
        /* Fill between pairs */
        for (unsigned char i = 0; i + 1 < count; i += 2)
        {
            for (unsigned char x = intersections[i]; x <= intersections[i + 1]; x++)
                renderlib_plot(x, y, color);
        }
    }
}

void renderlib_blit(unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *data)
{
    if (hasBeenInitialized == 0) return;
    for (unsigned char j = 0; j < h; j++)
    {
        for (unsigned char i = 0; i < w; i++)
        {
            renderlib_plot(x + i, y + j, data[j * w + i]);
        }
    }
}

/*
----------------------------------------------------------
Floodfill / Region
----------------------------------------------------------
*/

void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor)
{
    if (hasBeenInitialized == 0) return;
    unsigned char currentColor = renderlib_getpixel(x, y);
    if (currentColor == stopColor || currentColor == color) return;

    ff_sp = 0;
    ff_stack[ff_sp++] = x;
    ff_stack[ff_sp++] = y;

    while (ff_sp > 0)
    {
        unsigned char py = ff_stack[--ff_sp];
        unsigned char px = ff_stack[--ff_sp];
        unsigned char pc = renderlib_getpixel(px, py);
        if (pc == stopColor || pc == color) continue;
        renderlib_plot(px, py, color);
        unsigned char sw = renderlib_screen_w();
        unsigned char sh = renderlib_screen_h();
        if (px > 0)         { unsigned char c = renderlib_getpixel(px - 1, py); if (c != stopColor && c != color) { ff_stack[ff_sp++] = px - 1; ff_stack[ff_sp++] = py; } }
        if (px + 1 < sw)    { unsigned char c = renderlib_getpixel(px + 1, py); if (c != stopColor && c != color) { ff_stack[ff_sp++] = px + 1; ff_stack[ff_sp++] = py; } }
        if (py > 0)         { unsigned char c = renderlib_getpixel(px, py - 1); if (c != stopColor && c != color) { ff_stack[ff_sp++] = px; ff_stack[ff_sp++] = py - 1; } }
        if (py + 1 < sh)    { unsigned char c = renderlib_getpixel(px, py + 1); if (c != stopColor && c != color) { ff_stack[ff_sp++] = px; ff_stack[ff_sp++] = py + 1; } }
    }
}

char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX, unsigned char *outY)
{
    if (hasBeenInitialized == 0) return 0;
    /* Find the bounding box of the connected region, return center */
    unsigned char minX = x, maxX = x, minY = y, maxY = y;
    unsigned char color = renderlib_getpixel(x, y);
    if (color == 0) return 0;

    ff_sp = 0;
    ff_stack[ff_sp++] = x;
    ff_stack[ff_sp++] = y;

    while (ff_sp > 0)
    {
        unsigned char py = ff_stack[--ff_sp];
        unsigned char px = ff_stack[--ff_sp];
        unsigned char pc = renderlib_getpixel(px, py);
        if (pc != color) continue;
        renderlib_plot(px, py, 0); /* mark as visited */
        if (px < minX) minX = px;
        if (px > maxX) maxX = px;
        if (py < minY) minY = py;
        if (py > maxY) maxY = py;
        unsigned char sw = renderlib_screen_w();
        unsigned char sh = renderlib_screen_h();
        if (px > 0)    { if (renderlib_getpixel(px - 1, py) == color) { ff_stack[ff_sp++] = px - 1; ff_stack[ff_sp++] = py; } }
        if (px + 1 < sw) { if (renderlib_getpixel(px + 1, py) == color) { ff_stack[ff_sp++] = px + 1; ff_stack[ff_sp++] = py; } }
        if (py > 0)    { if (renderlib_getpixel(px, py - 1) == color) { ff_stack[ff_sp++] = px; ff_stack[ff_sp++] = py - 1; } }
        if (py + 1 < sh) { if (renderlib_getpixel(px, py + 1) == color) { ff_stack[ff_sp++] = px; ff_stack[ff_sp++] = py + 1; } }
    }

    /* Restore the region */
    for (unsigned char j = minY; j <= maxY; j++)
    {
        for (unsigned char i = minX; i <= maxX; i++)
        {
            if (renderlib_getpixel(i, j) == 0)
                renderlib_plot(i, j, color);
        }
    }

    *outX = (minX + maxX) / 2;
    *outY = (minY + maxY) / 2;
    return 1;
}

void renderlib_invert(unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
    if (hasBeenInitialized == 0) return;
    for (unsigned char j = 0; j < h; j++)
    {
        for (unsigned char i = 0; i < w; i++)
        {
            unsigned char c = renderlib_getpixel(x + i, y + j);
            renderlib_plot(x + i, y + j, c ? 0 : 1);
        }
    }
}

void renderlib_copy(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char w, unsigned char h)
{
    if (hasBeenInitialized == 0) return;
    /* Simple copy - no overlap handling (caller's responsibility) */
    for (unsigned char j = 0; j < h; j++)
    {
        for (unsigned char i = 0; i < w; i++)
        {
            renderlib_plot(x2 + i, y2 + j, renderlib_getpixel(x1 + i, y1 + j));
        }
    }
}

/*
----------------------------------------------------------
Palette / Colors
----------------------------------------------------------
*/

void renderlib_setpalette(unsigned char index, unsigned char color)
{
    if (index >= RPALETTE_SIZE) return;
    palette[index] = color;
}

unsigned char renderlib_getpalette(unsigned char index)
{
    if (index >= RPALETTE_SIZE) return 0;
    return palette[index];
}

void renderlib_setbg(unsigned char color)
{
    vic.color_back = color;
}

void renderlib_setborder(unsigned char color)
{
    vic.color_border = color;
}

void renderlib_setcolor(unsigned char background, unsigned char foreground)
{
    if (hasBeenInitialized == 0) return;
    vic.color_border = background;
    vic.color_back = foreground;
}

/*
----------------------------------------------------------
Hardware sprites
----------------------------------------------------------
*/

void renderlib_sprite_enable(unsigned char n, unsigned char enabled)
{
    if (n >= RSPIRTE_COUNT) return;
    if (enabled)
        vic.spr_enable |= (1 << n);
    else
        vic.spr_enable &= ~(1 << n);
}

void renderlib_sprite_pos(unsigned char n, unsigned char x, unsigned char y)
{
    if (n >= RSPIRTE_COUNT) return;
    vic.spr_pos[n].x = x;
    vic.spr_pos[n].y = y;
    if (x & 0x80)
        vic.spr_msbx |= (1 << n);
    else
        vic.spr_msbx &= ~(1 << n);
}

void renderlib_sprite_color(unsigned char n, unsigned char color)
{
    if (n >= RSPIRTE_COUNT) return;
    vic.spr_color[n] = color;
}

void renderlib_sprite_multicolor(unsigned char n, unsigned char enabled)
{
    if (n >= RSPIRTE_COUNT) return;
    if (enabled)
        vic.spr_multi |= (1 << n);
    else
        vic.spr_multi &= ~(1 << n);
}

void renderlib_sprite_expand(unsigned char n, unsigned char x2, unsigned char y2)
{
    if (n >= RSPIRTE_COUNT) return;
    if (x2)
        vic.spr_expand_x |= (1 << n);
    else
        vic.spr_expand_x &= ~(1 << n);
    if (y2)
        vic.spr_expand_y |= (1 << n);
    else
        vic.spr_expand_y &= ~(1 << n);
}

void renderlib_sprite_data(unsigned char n, unsigned char pointer)
{
    if (n >= RSPIRTE_COUNT) return;
    /* Sprite character data is at $0800 + pointer*8, or $0800 + (pointer-256)*8 for bank 1 */
    /* The VIC sprite pointer register is at $D700-n (for sprite n) */
    /* Actually, sprite pointers are at $D7F0-7 to $D7F0-0 for sprites 1-8 */
    /* In the VIC struct, we access them via the raw register addresses */
    volatile char *spr_ptr = (volatile char *)(0xd7f0 - (RSPIRTE_COUNT - 1 - n));
    *spr_ptr = pointer;
}

void renderlib_sprite_all_enable(unsigned char enabled)
{
    vic.spr_enable = enabled ? 0xFF : 0x00;
}

/*
----------------------------------------------------------
Character operations (text modes)
----------------------------------------------------------
*/

void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c)
{
    if (hasBeenInitialized == 0) return;
    if (x >= 40 || y >= 25) return;
    unsigned int idx = y * 40 + x;
    colram[idx] = color;
    charram[idx] = c;
}

unsigned char renderlib_getchar(unsigned char x, unsigned char y)
{
    if (hasBeenInitialized == 0) return 0;
    if (x >= 40 || y >= 25) return 0;
    return charram[y * 40 + x];
}

void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, const char *str)
{
    if (hasBeenInitialized == 0) return;
    if (y >= 25) return;
    unsigned int idx = y * 40 + x;
    while (*str)
    {
        if (idx >= 1000) break;
        colram[idx] = color;
        charram[idx] = *str;
        idx++;
        str++;
    }
}

void renderlib_setcharset(const unsigned char *data)
{
    /* Copy 2048 bytes of character data to $0000 (default charset location) */
    volatile char *charset = (volatile char *)0x0000;
    for (unsigned int i = 0; i < 2048; i++)
        charset[i] = data[i];
}

/*
----------------------------------------------------------
Screen operations
----------------------------------------------------------
*/

void renderlib_scroll(unsigned char dir)
{
    if (hasBeenInitialized == 0) return;
    unsigned char sw = renderlib_screen_w();
    unsigned char sh = renderlib_screen_h();
    switch (dir)
    {
        case 0: /* up */
            for (unsigned char y = 0; y < sh - 1; y++)
                for (unsigned char x = 0; x < sw; x++)
                    renderlib_plot(x, y, renderlib_getpixel(x, y + 1));
            for (unsigned char x = 0; x < sw; x++)
                renderlib_plot(x, sh - 1, 0);
            break;
        case 1: /* down */
            for (unsigned char y = sh - 1; y > 0; y--)
                for (unsigned char x = 0; x < sw; x++)
                    renderlib_plot(x, y, renderlib_getpixel(x, y - 1));
            for (unsigned char x = 0; x < sw; x++)
                renderlib_plot(x, 0, 0);
            break;
        case 2: /* left */
            for (unsigned char y = 0; y < sh; y++)
                for (unsigned char x = 0; x < sw - 1; x++)
                    renderlib_plot(x, y, renderlib_getpixel(x + 1, y));
            for (unsigned char y = 0; y < sh; y++)
                renderlib_plot(sw - 1, y, 0);
            break;
        case 3: /* right */
            for (unsigned char y = 0; y < sh; y++)
                for (unsigned char x = sw - 1; x > 0; x--)
                    renderlib_plot(x, y, renderlib_getpixel(x - 1, y));
            for (unsigned char y = 0; y < sh; y++)
                renderlib_plot(0, y, 0);
            break;
    }
}

void renderlib_toggle_rendering(unsigned char state)
{
    if (state == 0)
        vic.ctrl1 &= ~VIC_CTRL1_DEN;
    else
        vic.ctrl1 |= VIC_CTRL1_DEN;
}

/*
----------------------------------------------------------
Init / Unload
----------------------------------------------------------
*/

void renderlib_init(void)
{
    if (hasBeenInitialized == 1) return;
    hasBeenInitialized = 1;
    currentMode = RMODE_TEXT;
    vic_setmode(VICM_TEXT, (const char *)0x0400, (const char *)0x0000);
    renderlib_clear(0);
    renderlib_setcolor(0, 0);
}

void renderlib_unload(void)
{
    if (hasBeenInitialized == 0) return;
    renderlib_setcolor(0x0E, 0x06);
    hasBeenInitialized = 0;
    renderlib_clear(0);
}
