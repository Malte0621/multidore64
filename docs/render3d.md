# 3D rendering - render3d

`render3d` is a fixed-point software 3D renderer. It provides a camera, projection, wireframe and flat-shaded polygon drawing, and a raycasting engine for first-person views - all in integer math, fast enough for real C64 frame rates.

```c
#include "multidore64/render3d.h"
#include "multidore64/renderlib.h"
```

!!! note
    render3d draws through renderlib, so switch to `RMODE_HIRES` (320x200) or `RMODE_MC_BITMAP` before rendering 3D content, and add `src/multidore64/render3d.c` to your build (see [Building](build.html)).

## Concepts

- All coordinates are 16-bit integers (`struct V3`), angles are degrees 0-359.
- Trigonometry returns 8.8 fixed-point; `R3D_DEG2FIX` (256) converts to integer pixels.
- A scene may hold up to `R3D_MAX_OBJECTS` (32) queued objects; polygons have up to `R3D_MAX_POLY` (8) vertices.

## Initialization and camera

```c
void render3d_init(void);                    // build sin/cos tables - call once

void render3d_cam_init(void);
void render3d_cam_pos(int x, int y, int z);
void render3d_cam_rot(int pitch, int yaw, int roll);   // degrees 0-359
void render3d_cam_focal(int focal);          // focal length = zoom / field of view
struct Camera3D render3d_cam_get(void);
```

```c
render3d_init();
render3d_cam_init();
render3d_cam_pos(0, 0, -300);
render3d_cam_rot(0, 0, 0);
render3d_cam_focal(256);
```

## Math helpers

```c
int render3d_sin(int angle);      // 8.8 fixed point
int render3d_cos(int angle);
void render3d_rot_x(struct V3 *v, int angle);
void render3d_rot_y(struct V3 *v, int angle);
void render3d_rot_z(struct V3 *v, int angle);
unsigned char render3d_project(const struct V3 *cam_space,
                               int *screen_x, int *screen_y,
                               const struct Camera3D *cam);
void render3d_transform(const struct V3 *world, struct V3 *camera_space,
                        const struct Camera3D *cam);
```

`render3d_project()` returns 0 when the point is behind the camera - always check before drawing to the screen.

## Wireframe drawing

```c
void render3d_line(int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color);
void render3d_poly_outline(const int *verts, int n, unsigned char color);
void render3d_box(int cx, int cy, int cz, int hx, int hy, int hz, unsigned char color);
void render3d_sphere(int cx, int cy, int cz, int r, unsigned char color);
```

`verts` is a flat array of `n * 3` integers: `x0,y0,z0, x1,y1,z1, ...`

## Flat-shaded polygons

```c
void render3d_poly_fill(const int *verts, int n, unsigned char color);
void render3d_triangle(int x1, int y1, int z1, int x2, int y2, int z2,
                       int x3, int y3, int z3, unsigned char color);
```

Both perform backface culling and depth testing. A spinning triangle:

```c
int tri[9] = { 0,-50,0,  50,50,0,  -50,50,0 };
int yaw = 0;

while (1)
{
    renderlib_clear(color_black);

    int v[9];
    for (int i = 0; i < 3; i++)
    {
        struct V3 p = { tri[i*3], tri[i*3+1], tri[i*3+2] };
        render3d_rot_y(&p, yaw);
        v[i*3] = p.x;  v[i*3+1] = p.y;  v[i*3+2] = p.z;
    }
    render3d_poly_fill(v, 3, color_light_green);

    yaw = (yaw + 3) & 0xff;
}
```

## Raycasting (first-person)

```c
void render3d_ray_init(void);
void render3d_ray_render(const unsigned char *map, int map_w, int map_h,
                         int player_x, int player_y, int player_dir);
```

Renders a Wolfenstein-style view of a byte map (`0` = empty, `>0` = wall type) across the full 320x200 screen. `player_dir` is in degrees.

```c
const unsigned char maze[8][8] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,1,0,2,2,0,1},
    {1,0,1,0,0,2,0,1},
    {1,0,0,0,0,0,0,1},
    {1,2,2,0,1,0,0,1},
    {1,0,0,0,1,0,0,1},
    {1,1,1,1,1,1,1,1},
};

renderlib_setmode(RMODE_HIRES);
render3d_ray_init();

int px = 4 * 64, py = 4 * 64, dir = 0;   // position and heading

while (1)
{
    render3d_ray_render(&maze[0][0], 8, 8, px, py, dir);
    if (controller_joy_left(0))  dir = (dir + 4) & 0xff;
    if (controller_joy_right(0)) dir = (dir - 4) & 0xff;
}
```

## Scene management

```c
void render3d_scene_clear(void);
void render3d_scene_render(void);
```

Queue objects during your update phase, then flush them in one `render3d_scene_render()` call - the clean split for a fixed-timestep game loop.
