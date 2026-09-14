/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#ifndef RENDER3D_H
#define RENDER3D_H

/*
----------------------------------------------------------
3D Vector (16-bit integer coordinates)
----------------------------------------------------------
*/
struct V3 {
    int x, y, z;
};

/*
----------------------------------------------------------
3D Point (same as vector, for positions)
----------------------------------------------------------
*/
typedef struct V3 P3;

/*
----------------------------------------------------------
Camera / View State
----------------------------------------------------------
*/
struct Camera3D {
    int pos_x, pos_y, pos_z;   /* Camera position */
    int rot_x, rot_y, rot_z;   /* Rotation in degrees (0-359) */
    int focal;                 /* Focal length for projection (pixels) */
};

/*
----------------------------------------------------------
3D Math Constants
----------------------------------------------------------
*/
#define R3D_DEG2FIX     256    /* Fixed-point scale (8.8 format) */
#define R3D_MAX_POLY    8      /* Max vertices per polygon */
#define R3D_MAX_OBJECTS 32     /* Max objects in scene */
#define R3D_RAY_WIDTH   160    /* Raycast screen width (half of 320) */
#define R3D_RAY_HEIGHT  200    /* Raycast screen height */

/*
----------------------------------------------------------
3D Math Functions
----------------------------------------------------------
*/

/* Initialize 3D rendering (precompute sin/cos tables) */
void render3d_init(void);

/* Get sine of angle (degrees, 0-359). Returns 8.8 fixed-point (-128 to 127). */
int render3d_sin(int angle);

/* Get cosine of angle (degrees, 0-359). Returns 8.8 fixed-point (-128 to 127). */
int render3d_cos(int angle);

/* Rotate a 3D point around X axis by angle (degrees) */
void render3d_rot_x(struct V3 *v, int angle);

/* Rotate a 3D point around Y axis by angle (degrees) */
void render3d_rot_y(struct V3 *v, int angle);

/* Rotate a 3D point around Z axis by angle (degrees) */
void render3d_rot_z(struct V3 *v, int angle);

/* Transform a 3D point: translate by camera position, then rotate by camera rotation */
void render3d_transform(const struct V3 *world, struct V3 *camera_space, const struct Camera3D *cam);

/* Project a camera-space point to screen coordinates.
   Returns 1 if point is in front of camera (z > 0), 0 if behind. */
unsigned char render3d_project(const struct V3 *cam_space, int *screen_x, int *screen_y, const struct Camera3D *cam);

/*
----------------------------------------------------------
Camera Control
----------------------------------------------------------
*/

/* Initialize camera to default position */
void render3d_cam_init(void);

/* Set camera position */
void render3d_cam_pos(int x, int y, int z);

/* Set camera rotation (degrees, 0-359) */
void render3d_cam_rot(int pitch, int yaw, int roll);

/* Set camera focal length (affects field of view) */
void render3d_cam_focal(int focal);

/* Get current camera state */
struct Camera3D render3d_cam_get(void);

/*
----------------------------------------------------------
Wireframe Rendering
----------------------------------------------------------
*/

/* Draw a 3D line (world coordinates) */
void render3d_line(int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color);

/* Draw a 3D polygon outline (world coordinates) */
void render3d_poly_outline(const int *verts, int n, unsigned char color);
/* verts: array of n*3 integers (x0,y0,z0, x1,y1,z1, ...) */

/* Draw a 3D box (wireframe) centered at (cx,cy,cz) with half-extents (hx,hy,hz) */
void render3d_box(int cx, int cy, int cz, int hx, int hy, int hz, unsigned char color);

/* Draw a 3D sphere (wireframe, simplified as box + diagonals) */
void render3d_sphere(int cx, int cy, int cz, int r, unsigned char color);

/*
----------------------------------------------------------
Flat-Shaded Polygon Rendering
----------------------------------------------------------
*/

/* Draw a flat-shaded 3D polygon (world coordinates).
   Performs backface culling and depth testing. */
void render3d_poly_fill(const int *verts, int n, unsigned char color);

/* Draw a 3D triangle (flat-shaded) */
void render3d_triangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, unsigned char color);

/*
----------------------------------------------------------
Raycasting (First-Person 3D)
----------------------------------------------------------
*/

/* Initialize raycasting renderer */
void render3d_ray_init(void);

/* Render a raycast view of a 2D map.
   map: 2D array (map_w * map_h bytes), 0 = empty, >0 = wall type
   map_w, map_h: map dimensions
   player_x, player_y: player position (fixed-point 8.8)
   player_dir: player direction in degrees (0-359)
   Renders to the full screen (320x200 in hires mode) */
void render3d_ray_render(const unsigned char *map, int map_w, int map_h,
                         int player_x, int player_y, int player_dir);

/*
----------------------------------------------------------
Scene Management
----------------------------------------------------------
*/

/* Clear the 3D scene (remove all queued objects) */
void render3d_scene_clear(void);

/* Render all queued 3D objects (call after adding objects) */
void render3d_scene_render(void);

#endif
