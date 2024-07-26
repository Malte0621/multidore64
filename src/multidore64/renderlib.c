/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#include "renderlib.h"
#include "utilslib.h"
#include <c64.h>
#include <cbm.h>
#include <conio.h>
#include <peekpoke.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgi.h>

// #include "colorlib.h"

#define tgi_sprite(spr) tgi_ioctl(0, (void *)(spr))
#define tgi_flip() tgi_ioctl(1, (void *)0)
#define tgi_setbgcolor(bgcol) tgi_ioctl(2, (void *)(bgcol))
#define tgi_setframerate(rate) tgi_ioctl(3, (void *)(rate))
#define tgi_busy() tgi_ioctl(4, (void *)0)
#define tgi_updatedisplay() tgi_ioctl(4, (void *)1)

#define BASE 0x3c00 // 8192

const unsigned char color_black1 = 0x00;
const unsigned char color_white1 = 0x01;
const unsigned char color_light_blue1 = 0x0E;
const unsigned char color_blue1 = 0x06;

unsigned short renderlib_screen_width = 40, renderlib_screen_height = 25;

char hasBeenInitialized = 0;
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
unsigned char renderMode = 0;

const unsigned char renderlib_mode_text = 0;
const unsigned char renderlib_mode_graphics = 1;

unsigned char drawPage = 0;
#endif

unsigned short get_renderlib_screen_width() {
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    return tgi_getmaxx();
  }
#endif

  return renderlib_screen_width;
}

unsigned short get_renderlib_screen_height() {
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    return tgi_getmaxy();
  }
#endif
  return renderlib_screen_height;
}

void msg(char *msg) { printf("[MD64]: %s", msg); }

void initErrorMsg() { msg("Renderlib has not been initialized!"); }

void initErrorMsg2() { msg("Renderlib has already been initialized!"); }

void renderlib_clear() {
  unsigned char color = PEEK(53280);
  if (hasBeenInitialized == 0) {
    printf("%c", 0x93);
    return;
  }
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    tgi_clear();
  } else {
#endif

    renderlib_fillrect(0, 0, 39, 23, color);

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
/*
Colors:
0 = black
1 = white
2 = red
3 = cyan
4 = purple
5 = green
6 = blue
7 = yellow
8 = orange
9 = brown
10 = light red
11 = dark grey
12 = grey
13 = light green
14 = light blue
15 = light grey
*/
// The lower 4 bits represent the color intensity of the blue channel, the next
// 3 bits represent the color intensity of the green channel, and the highest
// bit represents the color intensity of the red channel.
const unsigned char tgi_static_palette[] = {0x00, 0xff, 0x02, 0x03, 0x04, 0x05,
                                            0x06, 0x0e, 0x08, 0x10, 0x0c, 0x1c,
                                            0x0e, 0x14, 0x19, 0x1f};

void renderlib_draw() {
  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

  if (renderMode == renderlib_mode_graphics) {
    while (tgi_busy()) {
      sleep(1);
    }
    drawPage = !drawPage;
    tgi_setdrawpage(drawPage);
    tgi_setviewpage(!drawPage);
    tgi_updatedisplay();
  }
}

void renderlib_setmode(unsigned char state) {
  if (state == 0) {
    // Revert to text mode
    tgi_uninstall();
    tgi_unload();

    renderlib_screen_width = 40;
    renderlib_screen_height = 25;
  } else {
    // Enter graphics mode
    tgi_install(tgi_static_stddrv);
    tgi_init();
    tgi_setpalette(tgi_static_palette);
    tgi_clear();

    tgi_setdrawpage(drawPage);
    tgi_setviewpage(!drawPage);

    tgi_updatedisplay();

    // renderlib_setcolor(color_black1, color_white1);

    renderlib_screen_width = tgi_getxres();
    renderlib_screen_height = tgi_getyres();
  }
  renderMode = state;
}
#endif

void renderlib_setcolor(unsigned char background, unsigned char foreground) {
  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

  // Set the color of the pixel at position (x, y) to the desired color
  POKE(53280, background);
  POKE(53281, foreground);
}

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
#include "font8x8_basic.h"
#endif

void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color,
                        unsigned char c) {
  /*
  #ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
      char *c2 = malloc(2);
      c2[0] = c;
      c2[1] = '\0';
  #endif
  */

  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

  // Set the color of the pixel at position (x, y) to the desired color

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    // Draw the character using pixels
    unsigned char i, j;
    for (i = 0; i < 8; i++) {
      for (j = 0; j < 8; j++) {
        if ((font8x8_basic[c][i] >> j) & 1) {
          renderlib_setpixel(x * 8 + j, y * 8 + i, color);
        }
      }
    }
    // tgi_setcolor(color);
    // tgi_outtextxy(x, y, c2);
  } else {
#endif
    // Draw the character
    POKE(55296 + y * 40 + x, color);
    POKE(1024 + y * 40 + x, c);
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

unsigned char renderlib_getchar(unsigned char x, unsigned char y) {
  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return 0;
  }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    msg("Cannot get character in graphics mode!");
    return 0;
  }
#endif

  // Get the character at position (x, y)
  return PEEK(1024 + y * 40 + x);
}

void renderlib_setpixel(unsigned char x, unsigned char y, unsigned char color) {
  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

  if (x >= get_renderlib_screen_width() || y >= get_renderlib_screen_height())
    return;

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    // Set the color of the pixel at position (x, y) to the desired color
    // POKE(55296 + y * 40 + x, color);
    tgi_setcolor(color);
    tgi_setpixel(x, y);
    return;
  } else {

#endif

    // Set the color of the pixel at position (x, y) to the desired color
    renderlib_drawchar(x, y, color, 224);

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

unsigned char renderlib_getpixel(unsigned char x, unsigned char y) {
  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return 0;
  }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    // Get the color of the pixel at position (x, y)
    return tgi_getpixel(x, y);
  } else {
#endif

    // Get the color of the pixel at position (x, y)
    return PEEK(55296 + y * 40 + x);
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color,
                          const char *str2) {
  char *str = malloc(10); // DEBUG

  // Get the length of the string
  unsigned char len = strlen(str);
  // Loop through the string
  unsigned char i = 0;

  sprintf(str, "%d", tgi_getmaxcolor()); // DEBUG

  /*
  #ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
      if (renderMode == renderlib_mode_graphics)
      {
          tgi_setcolor(color);
          tgi_outtextxy(x, y, str);
          return;
      }
  #endif
  */

  while (i < len) {
    // Draw the character
    renderlib_drawchar(x + i, y, color, str[i]);
    i++;
  }
}

void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color,
                         unsigned char stopColor) {
  // Get the shape around x, y
  // Fill outwards until we reach the edge of the shape

  // Get the color of the pixel at position (x, y)
  unsigned char currentColor = renderlib_getpixel(x, y);

  // Check if the color is the same as the stop color
  if (currentColor == stopColor) {
    // Return
    return;
  }

  // Set the color of the pixel at position (x, y) to the desired color
  renderlib_setpixel(x, y, color);

  // Check if the pixel is on the left edge
  if (x > 0) {
    // Fill the pixel to the left
    renderlib_floodfill(x - 1, y, color, stopColor);
  }

  // Check if the pixel is on the right edge
  if (x < 40) {
    // Fill the pixel to the right
    renderlib_floodfill(x + 1, y, color, stopColor);
  }

  // Check if the pixel is on the top edge
  if (y > 0) {
    // Fill the pixel above
    renderlib_floodfill(x, y - 1, color, stopColor);
  }

  // Check if the pixel is on the bottom edge
  if (y < 25) {
    // Fill the pixel below
    renderlib_floodfill(x, y + 1, color, stopColor);
  }
}

// Find the center of a filled shape using one pixel from the edge of it.
char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX,
                          unsigned char *outY) {
  unsigned char diff;
  unsigned char tmp;
  char useTemp;
  unsigned char i;
  unsigned char currentColor = renderlib_getpixel(x, y);

  for (i = x; i < 40; i++) {
    // Check if the color of the pixel at position (i, j) is the same as the
    // current color
    if (renderlib_getpixel(i, y) == currentColor) {
      break;
    } else {
      diff++;
    }
  }
  tmp = diff;
  useTemp = 1;
  for (i = x; i > 0; i--) {
    // Check if the color of the pixel at position (i, j) is the same as the
    // current color
    if (renderlib_getpixel(i, y) == currentColor) {
      break;
    } else {
      if (tmp <= 0) {
        return 0;
      } else {
        tmp--;
      }
    }
  }
  (*outX) = x + diff - tmp;
  // Do the same for the y axis
  diff = 0;
  tmp = 0;
  useTemp = 1;
  for (i = y; i < 25; i++) {
    // Check if the color of the pixel at position (i, j) is the same as the
    // current color
    if (renderlib_getpixel(x, i) == currentColor) {
      break;
    } else {
      diff++;
    }
  }
  tmp = diff;
  for (i = y; i > 0; i--) {
    // Check if the color of the pixel at position (i, j) is the same as the
    // current color
    if (renderlib_getpixel(x, i) == currentColor) {
      break;
    } else {
      if (tmp <= 0) {
        return 0;
      } else {
        tmp--;
      }
    }
  }
  (*outY) = y + diff - tmp;
  return 1;
}

void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w,
                        unsigned char h, unsigned char color) {
  unsigned char i = 0;
  unsigned char j = 0;

  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    // Draw a filled rectangle
    tgi_setcolor(color);
    tgi_bar(x, y, x + w, y + h);
  } else {
#endif

    for (i = 0; i < w; i++) {
      for (j = 0; j < h; j++) {
        renderlib_setpixel(x + i, y + j, color);
      }
    }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2,
                        unsigned char y2, unsigned char thickness,
                        unsigned char color) {
  // Calculate the difference between the two points
  unsigned char dx = x2 - x1;
  unsigned char dy = y2 - y1;

  // Calculate the slope without using a float (as it is not supported by CC65)
  unsigned char m = dy / dx;

  // Calculate the thickness
  unsigned char t = thickness / 2;

  // Calculate the thickness
  unsigned char i = 0;
  unsigned char j = 0;

  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    // Draw a line
    tgi_setcolor(color);
    tgi_line(x1, y1, x2, y2);
  } else {
#endif

    // Loop through the thickness
    for (i = 0; i < t; i++) {
      // Loop through the x-axis
      for (j = 0; j < dx; j++) {
        // Calculate the y-axis
        unsigned char y = m * j + y1;
        // Draw the pixel
        renderlib_setpixel(x1 + j, y + i, color);
        renderlib_setpixel(x1 + j, y - i, color);
      }
    }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

void renderlib_fillcircle(unsigned char x, unsigned char y, unsigned char r,
                          unsigned char color) {
  unsigned char i = 0;
  unsigned char j = 0;

  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

  for (i = 0; i < r; i++) {
    for (j = 0; j < r; j++) {
      if (i * i + j * j <= r * r) {
        renderlib_setpixel(x + i, y + j, color);
        renderlib_setpixel(x - i, y + j, color);
        renderlib_setpixel(x + i, y - j, color);
        renderlib_setpixel(x - i, y - j, color);
      }
    }
  }
}

void renderlib_drawcirlce(unsigned char x, unsigned char y, unsigned char r,
                          unsigned char color) {
  unsigned char i = 0;
  unsigned char j = 0;
  int radiusSquared;

  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    // Draw a circle
    tgi_setcolor(color);
    tgi_circle(x, y, r);
  } else {
#endif

    radiusSquared = r * r;
    for (i = 0; i <= r; i++) {
      for (j = 0; j <= r; j++) {
        int distanceSquared = (i - r) * (i - r) + (j - r) * (j - r);
        if (distanceSquared >= radiusSquared - 2 * r &&
            distanceSquared <= radiusSquared + 2 * r) {
          renderlib_setpixel(x + i, y + j, color);
          renderlib_setpixel(x - i, y + j, color);
          renderlib_setpixel(x + i, y - j, color);
          renderlib_setpixel(x - i, y - j, color);
        }
      }
    }
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  }
#endif
}

void renderlib_drawsprite(unsigned char x, unsigned char y, unsigned char w,
                          unsigned char h, unsigned char *sprite) {
  unsigned char i = 0;
  unsigned char j = 0;

  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }

  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      renderlib_setpixel(x + i, y + j, sprite[i + j * w]);
    }
  }
}

void renderlib_unload() {
  if (hasBeenInitialized == 0) {
    initErrorMsg();
    return;
  }
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
  if (renderMode == renderlib_mode_graphics) {
    renderlib_setmode(renderlib_mode_text);
  }
#endif
  renderlib_setcolor(color_light_blue1, color_blue1);
  hasBeenInitialized = 0;
  renderlib_clear();
}

void renderlib_init() {
  if (hasBeenInitialized == 1) {
    initErrorMsg2();
    return;
  }
  // renderlib_setmode(renderlib_mode_text);
  renderlib_clear();
  hasBeenInitialized = 1;
  renderlib_setcolor(color_black1, color_black1);
}

#ifdef RENDERLIB3D_INCLUDED
// RenderLib 3D Implementation //

#include <string.h>

struct renderlib3d_camera *camera;

void renderlib3d_setcam(struct renderlib3d_camera *cam) { camera = cam; }
struct renderlib3d_camera *renderlib3d_getcam() { return camera; }

#define MAX_OBJECTS 32
struct renderlib3d_object *objects[MAX_OBJECTS];

void renderlib3d_clear() {
  unsigned char i;

  // Default values reset //
  for (i = 0; i < MAX_OBJECTS; i++) {
    if (objects[i] != NULL) {
      free(objects[i]->pos);
      free(objects[i]->rot);
      free(objects[i]->scl);
      free(objects[i]);
    }
    objects[i] = NULL;
  }
}

void renderlib3d_reset() { renderlib3d_clear(); }

void renderlib3d_init() {
  if (hasBeenInitialized == 0) {
    renderlib_init();
  }
  renderlib_setmode(renderlib_mode_graphics);

  // Set the camera
  camera = malloc(sizeof(struct renderlib3d_camera));
  camera->pos = malloc(sizeof(struct renderlib3d_vector3));
  camera->rot = malloc(sizeof(struct renderlib3d_vector3));
  camera->scl = malloc(sizeof(struct renderlib3d_vector3));

  camera->pos->x = 0;
  camera->pos->y = 0;
  camera->pos->z = 0;

  camera->rot->x = 0;
  camera->rot->y = 0;
  camera->rot->z = 0;

  camera->scl->x = 1;
  camera->scl->y = 1;
  camera->scl->z = 1;

  renderlib3d_reset();
}

struct renderlib3d_object *renderlib3d_createobj(unsigned char shape) {
  unsigned char cobj;
  struct renderlib3d_object *obj = malloc(sizeof(struct renderlib3d_object));

  if (obj == NULL) {
    return NULL;
  }

  // Find the first available object slot
  cobj = 0;
  while (objects[cobj] != NULL && cobj < MAX_OBJECTS) {
    cobj++;
  }

  if (cobj >= MAX_OBJECTS && objects[cobj] != NULL) {
    free(obj);
    return NULL;
  }

  obj->shape = shape;
  obj->color = color_white1;
  obj->pos = malloc(sizeof(struct renderlib3d_vector3));
  obj->rot = malloc(sizeof(struct renderlib3d_vector3));
  obj->scl = malloc(sizeof(struct renderlib3d_vector3));

  obj->pos->x = 0;
  obj->pos->y = 0;
  obj->pos->z = 0;

  obj->rot->x = 0;
  obj->rot->y = 0;
  obj->rot->z = 0;

  obj->scl->x = 1;
  obj->scl->y = 1;
  obj->scl->z = 1;

  obj->id = cobj;

  objects[obj->id] = obj;

  return obj;
}

void renderlib3d_destroyobj(struct renderlib3d_object *obj) {
  if (obj == NULL) {
    return;
  }

  free(obj->pos);
  free(obj->rot);
  free(obj->scl);

  // remove from objects array
  objects[obj->id] = NULL;

  free(obj);
}

void rotate_x(int *x, int *y, int *z, int angle) {
  int cos_angle = cos(angle);
  int sin_angle = sin(angle);

  int old_y = *y;
  int old_z = *z;

  *y = ((old_y * cos_angle - old_z * sin_angle) >> FIXED_POINT_SHIFT);
  *z = ((old_y * sin_angle + old_z * cos_angle) >> FIXED_POINT_SHIFT);
}

void rotate_y(int *x, int *y, int *z, int angle) {
  int cos_angle = cos(angle);
  int sin_angle = sin(angle);

  int old_x = *x;
  int old_z = *z;

  *x = ((old_x * cos_angle + old_z * sin_angle) >> FIXED_POINT_SHIFT);
  *z = ((-old_x * sin_angle + old_z * cos_angle) >> FIXED_POINT_SHIFT);
}

void rotate_z(int *x, int *y, int *z, int angle) {
  int cos_angle = cos(angle);
  int sin_angle = sin(angle);

  int old_x = *x;
  int old_y = *y;

  *x = ((old_x * cos_angle - old_y * sin_angle) >> FIXED_POINT_SHIFT);
  *y = ((old_x * sin_angle + old_y * cos_angle) >> FIXED_POINT_SHIFT);
}

void renderlib_project_point(int x, int y, int z, unsigned char *x2d,
                             unsigned char *y2d) {
  *x2d = (unsigned char)(x + WIDTH / 2);
  *y2d = (unsigned char)(y + HEIGHT / 2);
}

void renderlib3d_render() {
  // Declare all variables at the beginning
  unsigned char shape;
  struct renderlib3d_vector3 *pos, *rot, *scl;
  int vertices[8][3];
  int v1[3], v2[3], v3[3], v4[3];
  unsigned char x2d[8], y2d[8];
  unsigned char edges[12][2];
  unsigned char color;
  int half_sclx, half_scly, half_sclz;
  int segments, angle_step;
  int phi1, phi2;
  int theta1, theta2;
  int base_x, base_y, base_z;
  int apex[3];
  unsigned char x2d_temp[3], y2d_temp[3];
  int i, j, e, k, v;
  int radius, height;
  int cos_theta1, sin_theta1, cos_theta2, sin_theta2, cos_phi1, sin_phi1,
      cos_phi2, sin_phi2;
  int tx, ty, tz;

  // Get camera position, rotation, scale
  struct renderlib3d_vector3 *cam_pos = camera->pos;
  struct renderlib3d_vector3 *cam_rot = camera->rot;
  struct renderlib3d_vector3 *cam_scl = camera->scl;

  // Clear the screen
  renderlib_clear();

  // Loop through all objects
  for (k = 0; k < MAX_OBJECTS; k++) {
    // Get the object
    struct renderlib3d_object *obj = objects[k];

    if (obj == NULL) {
      continue;
    }

    // Get the shape
    shape = obj->shape;

    // Get the position, rotation, scale
    pos = obj->pos;
    rot = obj->rot;
    scl = obj->scl;

    // Get the color (default or from object if available)
    color = obj->color; // Set a default color or get from object if available

    // Draw it!
    switch (shape) {
    case 0: // Cube
      half_sclx = scl->x / 2;
      half_scly = scl->y / 2;
      half_sclz = scl->z / 2;

      // Define cube vertices
      vertices[0][0] = pos->x - half_sclx;
      vertices[0][1] = pos->y - half_scly;
      vertices[0][2] = pos->z - half_sclz;
      vertices[1][0] = pos->x + half_sclx;
      vertices[1][1] = pos->y - half_scly;
      vertices[1][2] = pos->z - half_sclz;
      vertices[2][0] = pos->x + half_sclx;
      vertices[2][1] = pos->y + half_scly;
      vertices[2][2] = pos->z - half_sclz;
      vertices[3][0] = pos->x - half_sclx;
      vertices[3][1] = pos->y + half_scly;
      vertices[3][2] = pos->z - half_sclz;
      vertices[4][0] = pos->x - half_sclx;
      vertices[4][1] = pos->y - half_scly;
      vertices[4][2] = pos->z + half_sclz;
      vertices[5][0] = pos->x + half_sclx;
      vertices[5][1] = pos->y - half_scly;
      vertices[5][2] = pos->z + half_sclz;
      vertices[6][0] = pos->x + half_sclx;
      vertices[6][1] = pos->y + half_scly;
      vertices[6][2] = pos->z + half_sclz;
      vertices[7][0] = pos->x - half_sclx;
      vertices[7][1] = pos->y + half_scly;
      vertices[7][2] = pos->z + half_sclz;

      // Apply rotation to vertices
      for (v = 0; v < 8; v++) {
        tx = vertices[v][0] - pos->x;
        ty = vertices[v][1] - pos->y;
        tz = vertices[v][2] - pos->z;

        rotate_x(&tx, &ty, &tz, rot->x);
        rotate_y(&tx, &ty, &tz, rot->y);
        rotate_z(&tx, &ty, &tz, rot->z);

        vertices[v][0] = tx + pos->x;
        vertices[v][1] = ty + pos->y;
        vertices[v][2] = tz + pos->z;

        // Apply projection and adjust for screen center
        renderlib_project_point(vertices[v][0], vertices[v][1], vertices[v][2],
                                &x2d[v], &y2d[v]);
      }

      // Define cube edges
      edges[0][0] = 0;
      edges[0][1] = 1;
      edges[1][0] = 1;
      edges[1][1] = 2;
      edges[2][0] = 2;
      edges[2][1] = 3;
      edges[3][0] = 3;
      edges[3][1] = 0;
      edges[4][0] = 4;
      edges[4][1] = 5;
      edges[5][0] = 5;
      edges[5][1] = 6;
      edges[6][0] = 6;
      edges[6][1] = 7;
      edges[7][0] = 7;
      edges[7][1] = 4;
      edges[8][0] = 0;
      edges[8][1] = 4;
      edges[9][0] = 1;
      edges[9][1] = 5;
      edges[10][0] = 2;
      edges[10][1] = 6;
      edges[11][0] = 3;
      edges[11][1] = 7;

      for (e = 0; e < 12; e++) {
        renderlib_drawline(x2d[edges[e][0]], y2d[edges[e][0]], x2d[edges[e][1]],
                           y2d[edges[e][1]], 1, color);
      }
      break;

    case 1: // Sphere
      radius = scl->x / 2;
      segments = 8; // Number of segments to draw the sphere
      angle_step = 256 / segments;

      for (i = 0; i < segments; i++) {
        theta1 = i * angle_step;
        theta2 = (i + 1) * angle_step;

        cos_theta1 = cos(theta1 * M_PI / 128);
        sin_theta1 = sin(theta1 * M_PI / 128);
        cos_theta2 = cos(theta2 * M_PI / 128);
        sin_theta2 = sin(theta2 * M_PI / 128);

        for (j = 0; j < segments; j++) {
          phi1 = j * angle_step;
          phi2 = (j + 1) * angle_step;

          cos_phi1 = cos(phi1 * M_PI / 128);
          sin_phi1 = sin(phi1 * M_PI / 128);
          cos_phi2 = cos(phi2 * M_PI / 128);
          sin_phi2 = sin(phi2 * M_PI / 128);

          v1[0] = pos->x + radius * sin_theta1 * cos_phi1;
          v1[1] = pos->y + radius * sin_theta1 * sin_phi1;
          v1[2] = pos->z + radius * cos_theta1;

          v2[0] = pos->x + radius * sin_theta2 * cos_phi1;
          v2[1] = pos->y + radius * sin_theta2 * sin_phi1;
          v2[2] = pos->z + radius * cos_theta2;

          v3[0] = pos->x + radius * sin_theta2 * cos_phi2;
          v3[1] = pos->y + radius * sin_theta2 * sin_phi2;
          v3[2] = pos->z + radius * cos_theta2;

          v4[0] = pos->x + radius * sin_theta1 * cos_phi2;
          v4[1] = pos->y + radius * sin_theta1 * sin_phi2;
          v4[2] = pos->z + radius * cos_theta1;

          // Apply projection and adjust for screen center
          renderlib_project_point(v1[0], v1[1], v1[2], &x2d_temp[0],
                                  &y2d_temp[0]);
          renderlib_project_point(v2[0], v2[1], v2[2], &x2d_temp[1],
                                  &y2d_temp[1]);
          renderlib_project_point(v3[0], v3[1], v3[2], &x2d_temp[2],
                                  &y2d_temp[2]);
          renderlib_project_point(v4[0], v4[1], v4[2], &x2d_temp[3],
                                  &y2d_temp[3]);

          renderlib_drawline(x2d_temp[0], y2d_temp[0], x2d_temp[1], y2d_temp[1],
                             1, color);
          renderlib_drawline(x2d_temp[1], y2d_temp[1], x2d_temp[2], y2d_temp[2],
                             1, color);
          renderlib_drawline(x2d_temp[2], y2d_temp[2], x2d_temp[3], y2d_temp[3],
                             1, color);
          renderlib_drawline(x2d_temp[3], y2d_temp[3], x2d_temp[0], y2d_temp[0],
                             1, color);
        }
      }
      break;

    case 2: // Pyramid
      half_sclx = scl->x / 2;
      half_scly = scl->y / 2;
      height = scl->z;

      base_x = pos->x;
      base_y = pos->y;
      base_z = pos->z;
      apex[0] = base_x;
      apex[1] = base_y;
      apex[2] = base_z + height;

      // Define pyramid base vertices
      vertices[0][0] = base_x - half_sclx;
      vertices[0][1] = base_y - half_scly;
      vertices[0][2] = base_z - half_sclx;
      vertices[1][0] = base_x + half_sclx;
      vertices[1][1] = base_y - half_scly;
      vertices[1][2] = base_z - half_sclx;
      vertices[2][0] = base_x + half_sclx;
      vertices[2][1] = base_y + half_scly;
      vertices[2][2] = base_z - half_sclx;
      vertices[3][0] = base_x - half_sclx;
      vertices[3][1] = base_y + half_scly;
      vertices[3][2] = base_z - half_sclx;

      // Apply rotation to base vertices
      for (v = 0; v < 4; v++) {
        tx = vertices[v][0] - base_x;
        ty = vertices[v][1] - base_y;
        tz = vertices[v][2] - base_z;

        rotate_x(&tx, &ty, &tz, rot->x);
        rotate_y(&tx, &ty, &tz, rot->y);
        rotate_z(&tx, &ty, &tz, rot->z);

        vertices[v][0] = tx + base_x;
        vertices[v][1] = ty + base_y;
        vertices[v][2] = tz + base_z;
      }

      // Apply rotation to apex
      tx = apex[0] - base_x;
      ty = apex[1] - base_y;
      tz = apex[2] - base_z;

      rotate_x(&tx, &ty, &tz, rot->x);
      rotate_y(&tx, &ty, &tz, rot->y);
      rotate_z(&tx, &ty, &tz, rot->z);

      apex[0] = tx + base_x;
      apex[1] = ty + base_y;
      apex[2] = tz + base_z;

      // Apply projection and adjust for screen center
      for (v = 0; v < 4; v++) {
        renderlib_project_point(vertices[v][0], vertices[v][1], vertices[v][2],
                                &x2d[v], &y2d[v]);
      }
      renderlib_project_point(apex[0], apex[1], apex[2], &x2d_temp[0],
                              &y2d_temp[0]);

      // Draw pyramid edges
      for (v = 0; v < 4; v++) {
        renderlib_drawline(x2d[v], y2d[v], x2d[(v + 1) % 4], y2d[(v + 1) % 4],
                           1, color);
        renderlib_drawline(x2d[v], y2d[v], x2d_temp[0], y2d_temp[0], 1, color);
      }
      break;

    case 3: // Cylinder
      radius = scl->x / 2;
      height = scl->z;
      segments = 8; // Number of segments to draw the cylinder
      angle_step = 256 / segments;

      for (i = 0; i < segments; i++) {
        theta1 = i * angle_step;
        theta2 = (i + 1) * angle_step;

        cos_theta1 = cos(theta1 * M_PI / 128);
        sin_theta1 = sin(theta1 * M_PI / 128);
        cos_theta2 = cos(theta2 * M_PI / 128);
        sin_theta2 = sin(theta2 * M_PI / 128);

        v1[0] = pos->x + radius * cos_theta1 / 256;
        v1[1] = pos->y + radius * sin_theta1 / 256;
        v1[2] = pos->z;

        v2[0] = pos->x + radius * cos_theta2 / 256;
        v2[1] = pos->y + radius * sin_theta2 / 256;
        v2[2] = pos->z;

        v3[0] = v1[0];
        v3[1] = v1[1];
        v3[2] = pos->z + height;

        v4[0] = v2[0];
        v4[1] = v2[1];
        v4[2] = pos->z + height;

        // Apply rotations to vertices
        rotate_x(&v1[0], &v1[1], &v1[2], rot->x);
        rotate_y(&v1[0], &v1[1], &v1[2], rot->y);
        rotate_z(&v1[0], &v1[1], &v1[2], rot->z);

        rotate_x(&v2[0], &v2[1], &v2[2], rot->x);
        rotate_y(&v2[0], &v2[1], &v2[2], rot->y);
        rotate_z(&v2[0], &v2[1], &v2[2], rot->z);

        rotate_x(&v3[0], &v3[1], &v3[2], rot->x);
        rotate_y(&v3[0], &v3[1], &v3[2], rot->y);
        rotate_z(&v3[0], &v3[1], &v3[2], rot->z);

        rotate_x(&v4[0], &v4[1], &v4[2], rot->x);
        rotate_y(&v4[0], &v4[1], &v4[2], rot->y);
        rotate_z(&v4[0], &v4[1], &v4[2], rot->z);

        // Project vertices
        renderlib_project_point(v1[0], v1[1], v1[2], &x2d_temp[0],
                                &y2d_temp[0]);
        renderlib_project_point(v2[0], v2[1], v2[2], &x2d_temp[1],
                                &y2d_temp[1]);
        renderlib_project_point(v3[0], v3[1], v3[2], &x2d_temp[2],
                                &y2d_temp[2]);
        renderlib_project_point(v4[0], v4[1], v4[2], &x2d_temp[3],
                                &y2d_temp[3]);

        // Draw lines
        renderlib_drawline(x2d_temp[0], y2d_temp[0], x2d_temp[1], y2d_temp[1],
                           1, color);
        renderlib_drawline(x2d_temp[0], y2d_temp[0], x2d_temp[2], y2d_temp[2],
                           1, color);
        renderlib_drawline(x2d_temp[1], y2d_temp[1], x2d_temp[3], y2d_temp[3],
                           1, color);
        renderlib_drawline(x2d_temp[2], y2d_temp[2], x2d_temp[3], y2d_temp[3],
                           1, color);
      }
      break;
    case 4: // Cone
      segments = 8;
      angle_step = 256 / segments;

      // Draw base circle
      for (i = 0; i < segments; ++i) {
        theta1 = i * angle_step;
        theta2 = (i + 1) * angle_step;

        v1[0] = pos->x + (scl->x * cos(theta1)) / 256;
        v1[1] = pos->y + (scl->y - scl->y / 2);
        v1[2] = pos->z + (scl->z * sin(theta1)) / 256;

        v2[0] = pos->x + (scl->x * cos(theta2)) / 256;
        v2[1] = pos->y + (scl->y - scl->y / 2);
        v2[2] = pos->z + (scl->z * sin(theta2)) / 256;

        // Apply rotation to base vertices
        rotate_x(&v1[0], &v1[1], &v1[2], rot->x);
        rotate_y(&v1[0], &v1[1], &v1[2], rot->y);
        rotate_z(&v1[0], &v1[1], &v1[2], rot->z);

        rotate_x(&v2[0], &v2[1], &v2[2], rot->x);
        rotate_y(&v2[0], &v2[1], &v2[2], rot->y);
        rotate_z(&v2[0], &v2[1], &v2[2], rot->z);

        renderlib_project_point(v1[0], v1[1], v1[2], &x2d_temp[0],
                                &y2d_temp[0]);
        renderlib_project_point(v2[0], v2[1], v2[2], &x2d_temp[1],
                                &y2d_temp[1]);

        renderlib_drawline(x2d_temp[0], y2d_temp[0], x2d_temp[1], y2d_temp[1],
                           1, color);
      }

      // Draw cone lines
      for (i = 0; i < segments; ++i) {
        theta1 = i * angle_step;

        v1[0] = pos->x + (scl->x * cos(theta1)) / 256;
        v1[1] = pos->y + (scl->y - scl->y / 2);
        v1[2] = pos->z + (scl->z * sin(theta1)) / 256;

        // Apply rotation to cone vertex
        rotate_x(&v1[0], &v1[1], &v1[2], rot->x);
        rotate_y(&v1[0], &v1[1], &v1[2], rot->y);
        rotate_z(&v1[0], &v1[1], &v1[2], rot->z);

        renderlib_project_point(v1[0], v1[1], v1[2], &x2d_temp[0],
                                &y2d_temp[0]);
        renderlib_drawline(x2d_temp[0], y2d_temp[0], x2d_temp[0],
                           y2d_temp[0] - 16, 1, color);
      }
      break;
    case 5: // Plane
      base_x = scl->x / 2;
      base_y = scl->y / 2;

      // Define vertices
      vertices[0][0] = pos->x - base_x;
      vertices[0][1] = pos->y - base_y;
      vertices[0][2] = pos->z;

      vertices[1][0] = pos->x + base_x;
      vertices[1][1] = pos->y - base_y;
      vertices[1][2] = pos->z;

      vertices[2][0] = pos->x + base_x;
      vertices[2][1] = pos->y + base_y;
      vertices[2][2] = pos->z;

      vertices[3][0] = pos->x - base_x;
      vertices[3][1] = pos->y + base_y;
      vertices[3][2] = pos->z;

      // Apply rotation to vertices
      for (v = 0; v < 4; v++) {
        tx = vertices[v][0] - pos->x;
        ty = vertices[v][1] - pos->y;
        tz = vertices[v][2] - pos->z;

        rotate_x(&tx, &ty, &tz, rot->x);
        rotate_y(&tx, &ty, &tz, rot->y);
        rotate_z(&tx, &ty, &tz, rot->z);

        vertices[v][0] = tx + pos->x;
        vertices[v][1] = ty + pos->y;
        vertices[v][2] = tz + pos->z;

        renderlib_project_point(vertices[v][0], vertices[v][1], vertices[v][2],
                                &x2d[v], &y2d[v]);
      }

      // Draw edges
      for (e = 0; e < 4; e++) {
        renderlib_drawline(x2d[e], y2d[e], x2d[(e + 1) % 4], y2d[(e + 1) % 4],
                           1, color);
      }
      break;

    default:
      break;
    }
  }
}

#endif // RENDERLIB3D_INCLUDED