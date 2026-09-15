/*
----------------------------------------------------------
This file is a part of MultiDore 64 and was made to
demonstrate the library in action.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

/*
Territory capture game in the style of Splix.io / Paper.io:

- Two players glide continuously across a 40x24 grid (row 0 is HUD).
- Leaving your own territory draws a trail behind you.
- Returning to your territory closes the loop: everything the trail
  encloses (empty cells AND enemy territory) becomes yours.
- Crossing your own trail or the arena wall kills you.
- Crossing the ENEMY's trail cuts it: the enemy dies.
- Enclosing the enemy's head inside a capture kills them too.
- Dying wipes all your cells; you respawn with a fresh 3x3 base.
- Round ends after ROUND_TICKS (~48 s) or when a player owns WIN_PCT.
  Most territory wins.

Controls:
- Player 1: W/A/S/D or joystick port 1.
- Player 2: cursor keys or joystick port 2.
- Q quits, space/fire starts a round from the title/game-over screen.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
#include "multidore64/colorlib.h"
#include "multidore64/controllerlib.h"
#include "multidore64/utilslib.h"

#define TICK_FRAMES   4     /* one game tick every 4 frames (~12.5 Hz) */
#define ROUND_TICKS   600   /* 600 ticks * 4 frames = ~48 s round */
#define WIN_PCT       65    /* instant win when a player owns >= 65 % */
#define RESPAWN_TICKS 10    /* death-to-respawn delay in ticks */
#define ARENA_TOP     1     /* row 0 is the HUD line */
#define ARENA_BOT     24
#define ARENA_W       40
#define ARENA_CELLS   960   /* 40 * 24 */

/* Cell values stored in map[][] (ownership, NOT colors) */
#define CELL_EMPTY    0
#define CELL_P1_LAND  1
#define CELL_P1_TRAIL 2
#define CELL_P2_LAND  3
#define CELL_P2_TRAIL 4

/* Direction codes: 0 = stopped, 1 = up, 2 = down, 3 = left, 4 = right */
static const signed char dirX[5] = { 0,  0,  0, -1,  1 };
static const signed char dirY[5] = { 0, -1,  1,  0,  0 };
static const unsigned char opposite[5] = { 0, 2, 1, 4, 3 };

/* Colors per map value: empty, p1 land, p1 trail, p2 land, p2 trail */
static const unsigned char cellColor[5] = { 0x00, 0x02, 0x0A, 0x06, 0x0E };
#define HEAD_COLOR_P1 0x01   /* white */
#define HEAD_COLOR_P2 0x07   /* yellow */

/* Player state, index 0 = player 1, 1 = player 2 */
static signed char px[2], py[2];
static unsigned char pdir[2];       /* 0 stopped, 1..4 direction */
static unsigned char palive[2];
static unsigned char ptrail[2];     /* 1 while a trail is drawn */
static unsigned char prespawn[2];   /* countdown ticks until respawn */
static unsigned char pkills[2];

/* Direction requested by a keypress this tick (0 = none). */
static unsigned char p1key, p2key;

int timeLeft = ROUND_TICKS;         /* global so tests can poke it */

/* Playfield ownership mirror + capture scratch, at fixed addresses:
   the compiled program stays below $4000 (SID tune staging area) and
   $C000-$CFFF is free RAM below the I/O area. */
typedef unsigned char MapRow[25];
static MapRow * const map = (MapRow *)0xC7D0;          /* 40x25 = 1000 bytes */
static MapRow * const visited = (MapRow *)0xCBB8;      /* BFS marker */
static unsigned char * const queue = (unsigned char *)0xC000; /* BFS queue, 2 bytes/cell */

#define landOf(p)   ((p) ? CELL_P2_LAND  : CELL_P1_LAND)
#define trailOf(p)  ((p) ? CELL_P2_TRAIL : CELL_P1_TRAIL)
#define headColor(p) ((p) ? HEAD_COLOR_P2 : HEAD_COLOR_P1)

/* Redraw one cell from the ownership map (single source of truth). */
static void renderCell(unsigned char x, unsigned char y)
{
    renderlib_plot(x, y, cellColor[map[x][y]]);
}

static void drawHead(unsigned char p)
{
    renderlib_plot(px[p], py[p], headColor(p));
}

/* ------------------------------------------------------------------ */
/* Scoring                                                            */
/* ------------------------------------------------------------------ */

static unsigned int landCount(unsigned char p)
{
    unsigned char x, y;
    unsigned int n = 0;
    unsigned char land = landOf(p);
    for (x = 0; x < ARENA_W; x++)
        for (y = ARENA_TOP; y <= ARENA_BOT; y++)
            if (map[x][y] == land)
                n++;
    return n;
}

static unsigned char landPct(unsigned char p)
{
    return (unsigned char)(landCount(p) * 100 / ARENA_CELLS);
}

/* ------------------------------------------------------------------ */
/* Spawning / dying                                                   */
/* ------------------------------------------------------------------ */

/* Give a player a fresh 3x3 base around (cx, cy). */
static void placeBase(unsigned char p, unsigned char cx, unsigned char cy)
{
    signed char x, y;
    px[p] = cx;
    py[p] = cy;
    pdir[p] = 0;
    ptrail[p] = 0;
    palive[p] = 1;
    for (x = cx - 1; x <= cx + 1; x++)
        for (y = cy - 1; y <= cy + 1; y++)
        {
            map[x][y] = landOf(p);
            renderCell(x, y);
        }
    drawHead(p);
}

/* Respawn at a random spot whose 3x3 surroundings are completely empty. */
static void spawn(unsigned char p)
{
    unsigned char x, y, i;
    for (i = 0; i < 64; i++)
    {
        x = 2 + (unsigned char)(rand() % 36);
        y = ARENA_TOP + 1 + (unsigned char)(rand() % 21);
        if (map[x - 1][y - 1] == CELL_EMPTY && map[x][y - 1] == CELL_EMPTY &&
            map[x + 1][y - 1] == CELL_EMPTY && map[x - 1][y] == CELL_EMPTY &&
            map[x][y] == CELL_EMPTY && map[x + 1][y] == CELL_EMPTY &&
            map[x - 1][y + 1] == CELL_EMPTY && map[x][y + 1] == CELL_EMPTY &&
            map[x + 1][y + 1] == CELL_EMPTY)
        {
            placeBase(p, x, y);
            return;
        }
    }
    placeBase(p, p ? 34 : 5, 12);   /* fallback if the arena is crowded */
}

/* credit: 0/1 = that player scores a kill, 2 = nobody (wall/self/head-on). */
static void die(unsigned char p, unsigned char credit)
{
    unsigned char x, y;
    unsigned char land = landOf(p);
    unsigned char trail = trailOf(p);
    for (x = 0; x < ARENA_W; x++)
        for (y = ARENA_TOP; y <= ARENA_BOT; y++)
            if (map[x][y] == land || map[x][y] == trail)
            {
                map[x][y] = CELL_EMPTY;
                renderCell(x, y);
            }
    if (credit < 2)
        pkills[credit]++;
    palive[p] = 0;
    ptrail[p] = 0;
    pdir[p] = 0;
    prespawn[p] = RESPAWN_TICKS;
}

/* ------------------------------------------------------------------ */
/* Capture (loop closure)                                             */
/* ------------------------------------------------------------------ */

/* The player just re-entered their own land with a trail drawn.
   1. The trail itself becomes territory.
   2. Flood fill from the arena border through cells that are NOT the
      player's land; everything the fill cannot reach is enclosed.
   3. All enclosed cells (empty, enemy land, enemy trail) become the
      player's land. An enemy head caught inside dies. */
static void capture(unsigned char p)
{
    unsigned char x, y, cx, cy, myland = landOf(p), mytrail = trailOf(p);
    unsigned char other = 1 - p;
    unsigned int qh = 0, qt = 0;

    /* 1. trail becomes land */
    for (x = 0; x < ARENA_W; x++)
        for (y = ARENA_TOP; y <= ARENA_BOT; y++)
            if (map[x][y] == mytrail)
            {
                map[x][y] = myland;
                renderCell(x, y);
            }

    /* 2. BFS from the border through non-my-land cells */
    memset((void *)visited, 0, 1000);
    for (x = 0; x < ARENA_W; x++)
    {
        if (map[x][ARENA_TOP] != myland && !visited[x][ARENA_TOP])
        { visited[x][ARENA_TOP] = 1; queue[qt++] = x; queue[qt++] = ARENA_TOP; }
        if (map[x][ARENA_BOT] != myland && !visited[x][ARENA_BOT])
        { visited[x][ARENA_BOT] = 1; queue[qt++] = x; queue[qt++] = ARENA_BOT; }
    }
    for (y = ARENA_TOP; y <= ARENA_BOT; y++)
    {
        if (map[0][y] != myland && !visited[0][y])
        { visited[0][y] = 1; queue[qt++] = 0; queue[qt++] = y; }
        if (map[ARENA_W - 1][y] != myland && !visited[ARENA_W - 1][y])
        { visited[ARENA_W - 1][y] = 1; queue[qt++] = ARENA_W - 1; queue[qt++] = y; }
    }
    while (qh < qt)
    {
        cx = queue[qh++];
        cy = queue[qh++];
        if (cx > 0 && !visited[cx - 1][cy] && map[cx - 1][cy] != myland)
        { visited[cx - 1][cy] = 1; queue[qt++] = cx - 1; queue[qt++] = cy; }
        if (cx < ARENA_W - 1 && !visited[cx + 1][cy] && map[cx + 1][cy] != myland)
        { visited[cx + 1][cy] = 1; queue[qt++] = cx + 1; queue[qt++] = cy; }
        if (cy > ARENA_TOP && !visited[cx][cy - 1] && map[cx][cy - 1] != myland)
        { visited[cx][cy - 1] = 1; queue[qt++] = cx; queue[qt++] = cy - 1; }
        if (cy < ARENA_BOT && !visited[cx][cy + 1] && map[cx][cy + 1] != myland)
        { visited[cx][cy + 1] = 1; queue[qt++] = cx; queue[qt++] = cy + 1; }
    }

    /* 3. enclosed cells become mine */
    for (x = 0; x < ARENA_W; x++)
        for (y = ARENA_TOP; y <= ARENA_BOT; y++)
            if (!visited[x][y] && map[x][y] != myland)
            {
                if (palive[other] && px[other] == (signed char)x &&
                    py[other] == (signed char)y)
                    die(other, p);
                map[x][y] = myland;
                renderCell(x, y);
            }
    ptrail[p] = 0;
}

/* ------------------------------------------------------------------ */
/* One game tick for one player                                       */
/* ------------------------------------------------------------------ */

static void handlePlayer(unsigned char p)
{
    signed char nx, ny, om, k;
    unsigned char other = 1 - p;

    if (!palive[p])
    {
        if (--prespawn[p] == 0)
            spawn(p);
        return;
    }

    /* steering: keys captured this tick + live joystick state */
    controller_poll(p);
    k = p ? p2key : p1key;
    if (k == 0)
    {
        if (controller_joy_up(p)) k = 1;
        else if (controller_joy_down(p)) k = 2;
        else if (controller_joy_left(p)) k = 3;
        else if (controller_joy_right(p)) k = 4;
    }
    if (k)
    {
        if (pdir[p] == 0 || k != opposite[pdir[p]])
            pdir[p] = k;
    }
    if (pdir[p] == 0)
        return;                     /* waiting for the first steering input */

    /* one step in the current direction */
    nx = px[p] + dirX[pdir[p]];
    ny = py[p] + dirY[pdir[p]];

    if (nx < 0 || nx >= ARENA_W || ny < ARENA_TOP || ny > ARENA_BOT)
    {
        die(p, 2);                  /* wall */
        return;
    }

    om = map[nx][ny];
    if (om == trailOf(p))
    {
        die(p, 2);                  /* crossed own trail */
        return;
    }
    if (palive[other] && nx == px[other] && ny == py[other])
    {
        die(other, 2);              /* head-on: both die, no credit */
        die(p, 2);
        return;
    }
    if (om == trailOf(other))
        die(other, p);              /* cut the enemy's trail */

    /* leaving the current cell draws the trail (only outside own land) */
    if (map[px[p]][py[p]] != landOf(p))
    {
        map[px[p]][py[p]] = trailOf(p);
        ptrail[p] = 1;
    }
    renderCell(px[p], py[p]);       /* undraw the head overlay */

    px[p] = nx;
    py[p] = ny;
    if (map[nx][ny] == landOf(p) && ptrail[p])
        capture(p);                 /* loop closed */
    drawHead(p);
}

/* ------------------------------------------------------------------ */
/* HUD / screens                                                      */
/* ------------------------------------------------------------------ */

static unsigned char addStr(char *b, unsigned char i, const char *s)
{
    while (*s)
        b[i++] = *s++;
    return i;
}

static unsigned char addNum2(char *b, unsigned char i, unsigned char v)
{
    b[i++] = '0' + ((v / 10) % 10);
    b[i++] = '0' + (v % 10);
    return i;
}

static unsigned char addNum3(char *b, unsigned char i, unsigned int v)
{
    b[i++] = '0' + ((v / 100) % 10);
    b[i++] = '0' + ((v / 10) % 10);
    b[i++] = '0' + (v % 10);
    return i;
}

static void updateHUD(void)
{
    char buf[40];
    unsigned char i = 0, len;
    i = addStr(buf, i, "P1 ");
    i = addNum2(buf, i, landPct(0));
    i = addStr(buf, i, "% K");
    i = addNum2(buf, i, pkills[0]);
    i = addStr(buf, i, "     ");
    i = addNum3(buf, i, (unsigned int)(timeLeft / 12));
    i = addStr(buf, i, "S     P2 ");
    i = addNum2(buf, i, landPct(1));
    i = addStr(buf, i, "% K");
    i = addNum2(buf, i, pkills[1]);
    buf[i] = 0;
    len = i;
    renderlib_drawstring((ARENA_W - len) / 2, 0, color_white, buf);
}

static void drawTitle(void)
{
    renderlib_drawstring(8, 6, color_yellow, "MULTIDORE 64");
    renderlib_drawstring(3, 9, color_white, "TAKE THE LAND - CLOSE LOOPS - LIVE");
    renderlib_drawstring(2, 12, color_light_red, "P1: WASD OR JOYSTICK 1");
    renderlib_drawstring(2, 13, color_light_blue, "P2: CURSOR KEYS OR JOYSTICK 2");
    renderlib_drawstring(1, 16, color_grey, "WALLS AND YOUR OWN TRAIL KILL");
    renderlib_drawstring(3, 17, color_grey, "CUT TRAILS TO KILL ENEMIES");
    renderlib_drawstring(3, 20, color_white, "PRESS SPACE/FIRE TO START");
}

/* returns 1 = start round, 0 = quit */
static unsigned char waitStart(void)
{
    while (1)
    {
        soundlib_update();
        controller_poll(0);
        controller_poll(1);
        if (controller_joy_fire(0) || controller_joy_fire(1))
            return 1;
        if (kbhit())
        {
            char k = getch();
            if (k == 0x20 || k == 0x0D)
                return 1;
            if (k == 0x51 || k == 0x71)
                return 0;
        }
    }
}

/* returns 1 = rematch, 0 = quit */
static unsigned char gameOverScreen(void)
{
    unsigned char pct1 = landPct(0), pct2 = landPct(1);
    char buf[40];
    unsigned char i, len;
    while (kbhit())
        getch();                    /* swallow stale keys */

    renderlib_clear(0);
    if (pct1 > pct2)
        renderlib_drawstring(13, 8, color_light_red, "PLAYER 1 WINS");
    else if (pct2 > pct1)
        renderlib_drawstring(13, 8, color_light_blue, "PLAYER 2 WINS");
    else
        renderlib_drawstring(17, 8, color_white, "DRAW");

    i = 0;
    i = addNum2(buf, i, pct1);
    i = addStr(buf, i, "% - ");
    i = addNum2(buf, i, pct2);
    i = addStr(buf, i, "%");
    buf[i] = 0;
    renderlib_drawstring((ARENA_W - i) / 2, 11, color_white, buf);

    i = addStr(buf, 0, "KILLS ");
    i = addNum2(buf, i, pkills[0]);
    i = addStr(buf, i, " : ");
    i = addNum2(buf, i, pkills[1]);
    buf[i] = 0;
    renderlib_drawstring((ARENA_W - i) / 2, 12, color_white, buf);

    renderlib_drawstring(6, 16, color_white, "SPACE = REMATCH   Q = QUIT");
    len = 0; (void)len;

    while (1)
    {
        controller_poll(0);
        controller_poll(1);
        if (controller_joy_fire(0) || controller_joy_fire(1))
            return 1;
        if (kbhit())
        {
            char k = getch();
            if (k == 0x20 || k == 0x0D)
                return 1;
            if (k == 0x51 || k == 0x71)
                return 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Round setup                                                        */
/* ------------------------------------------------------------------ */

static void resetRound(void)
{
    unsigned char x, y;
    for (x = 0; x < 40; x++)
        for (y = 0; y < 25; y++)
            map[x][y] = CELL_EMPTY;
    px[0] = px[1] = py[0] = py[1] = 0;
    pdir[0] = pdir[1] = 0;
    palive[0] = palive[1] = 0;
    ptrail[0] = ptrail[1] = 0;
    prespawn[0] = prespawn[1] = 0;
    pkills[0] = pkills[1] = 0;
    p1key = p2key = 0;
    timeLeft = ROUND_TICKS;

    renderlib_clear(0);
    placeBase(0, 10, 12);
    placeBase(1, 29, 12);
    updateHUD();
}

/* ------------------------------------------------------------------ */
/* Main                                                               */
/* ------------------------------------------------------------------ */

/* Drain all buffered keys. Sets p1key/p2key to the last direction key
   seen for each player this tick; returns 1 if Q (quit) was pressed. */
static unsigned char readKeys(void)
{
    while (kbhit())
    {
        char k = getch();
        switch (k)
        {
        case 0x57: case 0x77: p1key = 1; break;    /* W */
        case 0x53: case 0x73: p1key = 2; break;    /* S */
        case 0x41: case 0x61: p1key = 3; break;    /* A */
        case 0x44: case 0x64: p1key = 4; break;    /* D */
        case 0x91: p2key = 1; break;               /* cursor up */
        case 0x11: p2key = 2; break;               /* cursor down */
        case 0x9D: p2key = 3; break;               /* cursor left */
        case 0x1D: p2key = 4; break;               /* cursor right */
        case 0x51: case 0x71: return 1;            /* Q */
        default: break;
        }
    }
    return 0;
}

int main(void)
{
    renderlib_init();
    soundlib_init();
    controller_init();

    /* Title screen: let the drive settle after the autostart LOAD
       before touching the IEC bus, then start the music. */
    sleep(50);
    drawTitle();
    soundlib_play_file("song.bin");

    if (!waitStart())
        goto quit;
    soundlib_stop();

    while (1)
    {
        resetRound();
        while (1)
        {
            if (readKeys())
                goto quit;
            handlePlayer(0);
            handlePlayer(1);
            p1key = p2key = 0;
            updateHUD();
            sleep(TICK_FRAMES);
            if (landPct(0) >= WIN_PCT || landPct(1) >= WIN_PCT)
                break;
            if (--timeLeft == 0)
                break;
        }
        if (!gameOverScreen())
            break;
    }

quit:
    renderlib_unload();
    return EXIT_SUCCESS;
}
