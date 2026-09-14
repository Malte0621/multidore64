/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#include "render3d.h"
#include "renderlib.h"
#include <string.h>

/*
----------------------------------------------------------
Internal State
----------------------------------------------------------
*/
static int sin_table[256];   /* Sine table, 8.8 fixed-point */
static int cos_table[256];   /* Cosine table, 8.8 fixed-point */
static int tables_init = 0;

static struct Camera3D camera;

/*
----------------------------------------------------------
Initialization
----------------------------------------------------------
*/

void render3d_init(void)
{
    if (tables_init) return;
    
    /* Precompute sin/cos tables (256 entries, 0-359 degrees)
       Using a simple approximation: sin(x) ≈ x for small x,
       but we need the full range. Use a polynomial approximation
       or a lookup with interpolation.
       
       For the C64, we'll use a simple approach:
       Compute sin/cos using the identity and a small number
       of iterations. Since we're in C and oscar64 will optimize,
       we can use a simple algorithm.
       
       Actually, for 8.8 fixed-point, we can use:
       sin(angle) where angle is 0-255 (mapping to 0-360 degrees)
       
       Let's use a simple table generated at init time.
       We'll use the Taylor series or a simple approximation.
       
       For simplicity and speed, use a coarse table with
       linear interpolation. */
    
    for (int i = 0; i < 256; i++)
    {
        /* Convert to radians: angle_rad = (i / 256.0) * 2 * PI
           In fixed-point: angle_rad_8.8 = (i * 2 * PI * 256) / 256
           2 * PI * 256 ≈ 1608.5 */
        int angle_rad = (i * 1608) / 256;  /* 8.8 fixed-point radians */
        
        /* Simple sine approximation using polynomial
           sin(x) ≈ x - x^3/6 + x^5/120
           For 8.8 fixed-point, this needs careful scaling.
           
           Actually, let's use a simpler approach:
           Generate the table using a recursive formula or
           just hardcode a coarse table.
           
           For the C64, the fastest approach is a precomputed
           table. Let's generate it using a simple algorithm. */
        
        /* Use the identity: sin(x) = cos(90 - x)
           And for cos, use a polynomial approximation.
           
           Simplest: use a lookup with 16 entries per quadrant
           and linear interpolation. */
        
        /* For now, use a simple approximation:
           sin(i) where i is 0-255, mapped to 0-360 degrees
           
           We'll use: sin_deg = sin(i * 360 / 256)
           
           For the C64, let's just use a coarse table.
           Generate using the recurrence:
           sin(n+1) = sin(n)*cos(1) + cos(n)*sin(1)
           cos(n+1) = cos(n)*cos(1) - sin(n)*sin(1)
           
           In 8.8 fixed-point:
           cos(1 degree) ≈ 255 (0.996 * 256)
           sin(1 degree) ≈ 4 (0.0175 * 256) */
        
        if (i == 0)
        {
            sin_table[0] = 0;
            cos_table[0] = 256;  /* 1.0 in 8.8 */
        }
        else
        {
            /* Recurrence: sin(i) = sin(i-1)*cos(1°) + cos(i-1)*sin(1°)
               cos(i) = cos(i-1)*cos(1°) - sin(i-1)*sin(1°)
               
               cos(1°) ≈ 255/256, sin(1°) ≈ 4/256
               
               sin(i) = (sin(i-1) * 255 + cos(i-1) * 4) / 256
               cos(i) = (cos(i-1) * 255 - sin(i-1) * 4) / 256 */
            int s_prev = sin_table[i-1];
            int c_prev = cos_table[i-1];
            sin_table[i] = (s_prev * 255 + c_prev * 4) / 256;
            cos_table[i] = (c_prev * 255 - s_prev * 4) / 256;
        }
    }
    
    tables_init = 1;
    
    /* Initialize camera to default */
    render3d_cam_init();
}

/*
----------------------------------------------------------
Trigonometry
----------------------------------------------------------
*/

int render3d_sin(int angle)
{
    if (!tables_init) render3d_init();
    angle = angle % 256;
    if (angle < 0) angle += 256;
    return sin_table[angle];
}

int render3d_cos(int angle)
{
    if (!tables_init) render3d_init();
    angle = angle % 256;
    if (angle < 0) angle += 256;
    return cos_table[angle];
}

/*
----------------------------------------------------------
3D Rotation
----------------------------------------------------------
*/

void render3d_rot_x(struct V3 *v, int angle)
{
    if (!tables_init) render3d_init();
    angle = angle % 256;
    if (angle < 0) angle += 256;
    
    int s = sin_table[angle];
    int c = cos_table[angle];
    
    int y = v->y;
    int z = v->z;
    
    /* Rotate around X: y' = y*cos - z*sin, z' = y*sin + z*cos
       In 8.8 fixed-point: divide by 256 after multiply */
    v->y = (y * c - z * s) / 256;
    v->z = (y * s + z * c) / 256;
}

void render3d_rot_y(struct V3 *v, int angle)
{
    if (!tables_init) render3d_init();
    angle = angle % 256;
    if (angle < 0) angle += 256;
    
    int s = sin_table[angle];
    int c = cos_table[angle];
    
    int x = v->x;
    int z = v->z;
    
    /* Rotate around Y: x' = x*cos + z*sin, z' = -x*sin + z*cos */
    v->x = (x * c + z * s) / 256;
    v->z = (-x * s + z * c) / 256;
}

void render3d_rot_z(struct V3 *v, int angle)
{
    if (!tables_init) render3d_init();
    angle = angle % 256;
    if (angle < 0) angle += 256;
    
    int s = sin_table[angle];
    int c = cos_table[angle];
    
    int x = v->x;
    int y = v->y;
    
    /* Rotate around Z: x' = x*cos - y*sin, y' = x*sin + y*cos */
    v->x = (x * c - y * s) / 256;
    v->y = (x * s + y * c) / 256;
}

/*
----------------------------------------------------------
Camera Transform
----------------------------------------------------------
*/

void render3d_transform(const struct V3 *world, struct V3 *camera_space, const struct Camera3D *cam)
{
    /* Translate: subtract camera position */
    camera_space->x = world->x - cam->pos_x;
    camera_space->y = world->y - cam->pos_y;
    camera_space->z = world->z - cam->pos_z;
    
    /* Rotate: apply camera rotation (inverse)
       Order: Y (yaw), X (pitch), Z (roll) */
    render3d_rot_y(camera_space, -cam->rot_y);
    render3d_rot_x(camera_space, -cam->rot_x);
    render3d_rot_z(camera_space, -cam->rot_z);
}

/*
----------------------------------------------------------
Perspective Projection
----------------------------------------------------------
*/

unsigned char render3d_project(const struct V3 *cam_space, int *screen_x, int *screen_y, const struct Camera3D *cam)
{
    /* Check if point is in front of camera */
    if (cam_space->z <= 0) return 0;
    
    /* Perspective projection:
       screen_x = (cam_space->x * focal) / cam_space->z + screen_center_x
       screen_y = (cam_space->y * focal) / cam_space->z + screen_center_y
       
       Screen center: 160, 100 (for 320x200) */
    int sx = (cam_space->x * cam->focal) / cam_space->z + 160;
    int sy = (cam_space->y * cam->focal) / cam_space->z + 100;
    
    *screen_x = sx;
    *screen_y = sy;
    return 1;
}

/*
----------------------------------------------------------
Camera Control
----------------------------------------------------------
*/

void render3d_cam_init(void)
{
    camera.pos_x = 0;
    camera.pos_y = 0;
    camera.pos_z = -100;  /* Start 100 units back */
    camera.rot_x = 0;
    camera.rot_y = 0;
    camera.rot_z = 0;
    camera.focal = 100;   /* Default focal length */
}

void render3d_cam_pos(int x, int y, int z)
{
    camera.pos_x = x;
    camera.pos_y = y;
    camera.pos_z = z;
}

void render3d_cam_rot(int pitch, int yaw, int roll)
{
    camera.rot_x = pitch % 256;
    camera.rot_y = yaw % 256;
    camera.rot_z = roll % 256;
}

void render3d_cam_focal(int focal)
{
    if (focal > 0) camera.focal = focal;
}

struct Camera3D render3d_cam_get(void)
{
    return camera;
}

/*
----------------------------------------------------------
Wireframe Rendering
----------------------------------------------------------
*/

void render3d_line(int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color)
{
    if (!tables_init) render3d_init();
    
    struct V3 p1 = {x1, y1, z1};
    struct V3 p2 = {x2, y2, z2};
    struct V3 c1, c2;
    
    render3d_transform(&p1, &c1, &camera);
    render3d_transform(&p2, &c2, &camera);
    
    int sx1, sy1, sx2, sy2;
    unsigned char v1 = render3d_project(&c1, &sx1, &sy1, &camera);
    unsigned char v2 = render3d_project(&c2, &sx2, &sy2, &camera);
    
    /* Draw line if at least one endpoint is visible */
    if (v1 || v2)
    {
        /* Clip: if one point is behind camera, project to z=1 */
        if (!v1)
        {
            /* Interpolate to find where line crosses z=0 plane */
            /* For simplicity, just use the visible endpoint */
            sx1 = sx2;
            sy1 = sy2;
        }
        if (!v2)
        {
            sx2 = sx1;
            sy2 = sy1;
        }
        
        renderlib_line((unsigned char)sx1, (unsigned char)sy1, (unsigned char)sx2, (unsigned char)sy2, color);
    }
}

void render3d_poly_outline(const int *verts, int n, unsigned char color)
{
    if (!tables_init) render3d_init();
    if (n < 2) return;
    
    struct V3 cam_pts[R3D_MAX_POLY];
    int screen_x[R3D_MAX_POLY];
    int screen_y[R3D_MAX_POLY];
    unsigned char visible[R3D_MAX_POLY];
    
    for (int i = 0; i < n && i < R3D_MAX_POLY; i++)
    {
        struct V3 world = {verts[i*3], verts[i*3+1], verts[i*3+2]};
        render3d_transform(&world, &cam_pts[i], &camera);
        visible[i] = render3d_project(&cam_pts[i], &screen_x[i], &screen_y[i], &camera);
    }
    
    /* Draw lines between consecutive vertices */
    for (int i = 0; i < n - 1 && i < R3D_MAX_POLY - 1; i++)
    {
        if (visible[i] && visible[i+1])
        {
            renderlib_line((unsigned char)screen_x[i], (unsigned char)screen_y[i], (unsigned char)screen_x[i+1], (unsigned char)screen_y[i+1], color);
        }
    }
    /* Close the polygon */
    if (n > 2 && visible[0] && visible[n-1])
    {
        renderlib_line((unsigned char)screen_x[n-1], (unsigned char)screen_y[n-1], (unsigned char)screen_x[0], (unsigned char)screen_y[0], color);
    }
}

void render3d_box(int cx, int cy, int cz, int hx, int hy, int hz, unsigned char color)
{
    /* 8 corners of the box */
    int verts[24] = {
        cx-hx, cy-hy, cz-hz,  cx+hx, cy-hy, cz-hz,
        cx+hx, cy+hy, cz-hz,  cx-hx, cy+hy, cz-hz,
        cx-hx, cy-hy, cz+hz,  cx+hx, cy-hy, cz+hz,
        cx+hx, cy+hy, cz+hz,  cx-hx, cy+hy, cz+hz
    };
    
    /* 12 edges: (0,1), (1,2), (2,3), (3,0), (4,5), (5,6), (6,7), (7,4), (0,4), (1,5), (2,6), (3,7) */
    render3d_line(verts[0], verts[1], verts[2], verts[3], verts[4], verts[5], color);
    render3d_line(verts[3], verts[4], verts[5], verts[6], verts[7], verts[8], color);
    render3d_line(verts[6], verts[7], verts[8], verts[9], verts[10], verts[11], color);
    render3d_line(verts[9], verts[10], verts[11], verts[0], verts[1], verts[2], color);
    render3d_line(verts[12], verts[13], verts[14], verts[15], verts[16], verts[17], color);
    render3d_line(verts[15], verts[16], verts[17], verts[18], verts[19], verts[20], color);
    render3d_line(verts[18], verts[19], verts[20], verts[21], verts[22], verts[23], color);
    render3d_line(verts[21], verts[22], verts[23], verts[12], verts[13], verts[14], color);
    render3d_line(verts[0], verts[1], verts[2], verts[12], verts[13], verts[14], color);
    render3d_line(verts[3], verts[4], verts[5], verts[15], verts[16], verts[17], color);
    render3d_line(verts[6], verts[7], verts[8], verts[18], verts[19], verts[20], color);
    render3d_line(verts[9], verts[10], verts[11], verts[21], verts[22], verts[23], color);
}

void render3d_sphere(int cx, int cy, int cz, int r, unsigned char color)
{
    /* Simplified sphere: draw a box + 3 circles (great circles)
       For the C64, this is a cheap approximation. */
    render3d_box(cx, cy, cz, r, r, r, color);
    
    /* Draw 3 "great circles" as polygons */
    int n = 8;  /* 8-segment circles */
    int verts[R3D_MAX_POLY * 3];
    
    /* Circle in XY plane */
    for (int i = 0; i < n; i++)
    {
        int angle = (i * 256) / n;
        verts[i*3]   = cx + (r * render3d_cos(angle)) / 256;
        verts[i*3+1] = cy + (r * render3d_sin(angle)) / 256;
        verts[i*3+2] = cz;
    }
    render3d_poly_outline(verts, n, color);
    
    /* Circle in XZ plane */
    for (int i = 0; i < n; i++)
    {
        int angle = (i * 256) / n;
        verts[i*3]   = cx + (r * render3d_cos(angle)) / 256;
        verts[i*3+1] = cy;
        verts[i*3+2] = cz + (r * render3d_sin(angle)) / 256;
    }
    render3d_poly_outline(verts, n, color);
    
    /* Circle in YZ plane */
    for (int i = 0; i < n; i++)
    {
        int angle = (i * 256) / n;
        verts[i*3]   = cx;
        verts[i*3+1] = cy + (r * render3d_cos(angle)) / 256;
        verts[i*3+2] = cz + (r * render3d_sin(angle)) / 256;
    }
    render3d_poly_outline(verts, n, color);
}

/*
----------------------------------------------------------
Flat-Shaded Polygon Rendering
----------------------------------------------------------
*/

void render3d_poly_fill(const int *verts, int n, unsigned char color)
{
    if (!tables_init) render3d_init();
    if (n < 3) return;
    
    struct V3 cam_pts[R3D_MAX_POLY];
    int screen_x[R3D_MAX_POLY];
    int screen_y[R3D_MAX_POLY];
    unsigned char visible[R3D_MAX_POLY];
    
    for (int i = 0; i < n && i < R3D_MAX_POLY; i++)
    {
        struct V3 world = {verts[i*3], verts[i*3+1], verts[i*3+2]};
        render3d_transform(&world, &cam_pts[i], &camera);
        visible[i] = render3d_project(&cam_pts[i], &screen_x[i], &screen_y[i], &camera);
    }
    
    /* Check if all vertices are visible */
    for (int i = 0; i < n && i < R3D_MAX_POLY; i++)
    {
        if (!visible[i]) return;  /* Polygon partially behind camera, skip */
    }
    
    /* Backface culling: compute normal using cross product of first 3 vertices
       If normal points away from camera, cull. */
    int ax = cam_pts[1].x - cam_pts[0].x;
    int ay = cam_pts[1].y - cam_pts[0].y;
    int az = cam_pts[1].z - cam_pts[0].z;
    int bx = cam_pts[2].x - cam_pts[0].x;
    int by = cam_pts[2].y - cam_pts[0].y;
    int bz = cam_pts[2].z - cam_pts[0].z;
    
    /* Cross product: normal = a x b */
    int nx = ay * bz - az * by;
    int ny = az * bx - ax * bz;
    int nz = ax * by - ay * bx;
    
    /* Vector from camera to polygon center */
    int cx = (cam_pts[0].x + cam_pts[1].x + cam_pts[2].x) / 3;
    int cy = (cam_pts[0].y + cam_pts[1].y + cam_pts[2].y) / 3;
    int cz = (cam_pts[0].z + cam_pts[1].z + cam_pts[2].z) / 3;
    
    /* Dot product: if normal . view_dir < 0, backface */
    int dot = nx * cx + ny * cy + nz * cz;
    if (dot > 0) return;  /* Backface, cull */
    
    /* Draw filled polygon using renderlib */
    /* For the C64, use a simple scanline fill */
    /* For now, draw outline + fill using renderlib_fillrect approximation */
    
    /* Find bounding box */
    int min_x = screen_x[0], max_x = screen_x[0];
    int min_y = screen_y[0], max_y = screen_y[0];
    for (int i = 1; i < n && i < R3D_MAX_POLY; i++)
    {
        if (screen_x[i] < min_x) min_x = screen_x[i];
        if (screen_x[i] > max_x) max_x = screen_x[i];
        if (screen_y[i] < min_y) min_y = screen_y[i];
        if (screen_y[i] > max_y) max_y = screen_y[i];
    }
    
    /* Draw outline */
    for (int i = 0; i < n - 1 && i < R3D_MAX_POLY - 1; i++)
    {
        renderlib_line((unsigned char)screen_x[i], (unsigned char)screen_y[i], (unsigned char)screen_x[i+1], (unsigned char)screen_y[i+1], color);
    }
    if (n > 2)
    {
        renderlib_line((unsigned char)screen_x[n-1], (unsigned char)screen_y[n-1], (unsigned char)screen_x[0], (unsigned char)screen_y[0], color);
    }
    
    /* Simple fill: draw horizontal lines within bounding box
       This is a rough approximation - proper polygon fill is complex
       on the C64. For now, fill the bounding box. */
    /* Note: This is a placeholder - proper scanline polygon fill
       would be needed for production. */
}

void render3d_triangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, unsigned char color)
{
    int verts[9] = {x1, y1, z1, x2, y2, z2, x3, y3, z3};
    render3d_poly_fill(verts, 3, color);
}

/*
----------------------------------------------------------
Raycasting (First-Person 3D)
----------------------------------------------------------
*/

void render3d_ray_init(void)
{
    if (!tables_init) render3d_init();
}

void render3d_ray_render(const unsigned char *map, int map_w, int map_h,
                         int player_x, int player_y, int player_dir)
{
    if (!tables_init) render3d_init();
    
    /* Wolfenstein-style raycasting
       For each column on screen, cast a ray and find the wall hit.
       
       player_x, player_y: fixed-point 8.8 (integer part = map coords)
       player_dir: direction in degrees (0-359)
       
       Screen: 320x200 (hires)
       We render to 160 columns (half resolution for speed) */
    
    int px = player_x / 256;  /* Integer part */
    int py = player_y / 256;
    
    /* Direction vector */
    int dir_angle = player_dir % 256;
    if (dir_angle < 0) dir_angle += 256;
    
    /* For each screen column, cast a ray */
    for (int col = 0; col < 160; col++)
    {
        /* Ray angle: spread across field of view
           FOV ≈ 60 degrees = 42.7 in our 256-unit system
           Ray angle = player_dir - 30 + (col / 160) * 60 */
        int ray_angle = dir_angle - 21 + (col * 42) / 160;
        ray_angle = ray_angle % 256;
        if (ray_angle < 0) ray_angle += 256;
        
        /* Ray direction (using sin/cos tables) */
        int ray_dx = render3d_cos(ray_angle);  /* 8.8 fixed-point */
        int ray_dy = render3d_sin(ray_angle);
        
        /* DDA raycasting algorithm */
        int map_x = px;
        int map_y = py;
        
        /* Step direction and side distance */
        int step_x, step_y;
        int side_dist_x, side_dist_y;
        
        if (ray_dx < 0)
        {
            step_x = -1;
            side_dist_x = ((player_x - map_x * 256) * 256) / (-ray_dx);
        }
        else
        {
            step_x = 1;
            side_dist_x = ((map_x * 256 + 256 - player_x) * 256) / ray_dx;
        }
        
        if (ray_dy < 0)
        {
            step_y = -1;
            side_dist_y = ((player_y - map_y * 256) * 256) / (-ray_dy);
        }
        else
        {
            step_y = 1;
            side_dist_y = ((map_y * 256 + 256 - player_y) * 256) / ray_dy;
        }
        
        /* Trace through map */
        int hit = 0;
        int side = 0;
        for (int step = 0; step < 64; step++)
        {
            if (side_dist_x < side_dist_y)
            {
                side_dist_x += 256;  /* 1.0 in 8.8 */
                map_x += step_x;
                side = 0;
            }
            else
            {
                side_dist_y += 256;
                map_y += step_y;
                side = 1;
            }
            
            /* Check if we hit a wall */
            if (map_x >= 0 && map_x < map_w && map_y >= 0 && map_y < map_h)
            {
                if (map[map_y * map_w + map_x] > 0)
                {
                    hit = 1;
                    break;
                }
            }
            else
            {
                hit = 1;  /* Out of bounds = wall */
                break;
            }
        }
        
        if (!hit) continue;
        
        /* Calculate wall height */
        int perp_dist;
        if (side == 0)
            perp_dist = side_dist_x - 256;
        else
            perp_dist = side_dist_y - 256;
        
        if (perp_dist < 1) perp_dist = 1;
        
        /* Wall height on screen: (screen_height * 256) / perp_dist
           screen_height = 200 */
        int wall_height = (200 * 256) / perp_dist;
        
        /* Clip to screen */
        if (wall_height > 200) wall_height = 200;
        
        int wall_top = (200 - wall_height) / 2;
        int wall_bottom = wall_top + wall_height;
        
        /* Wall color: shade based on side and distance */
        unsigned char color;
        int wall_type = 0;
        if (map_x >= 0 && map_x < map_w && map_y >= 0 && map_y < map_h)
            wall_type = map[map_y * map_w + map_x];
        
        /* Base color from wall type */
        switch (wall_type)
        {
            case 1: color = 0x03; break;  /* Red */
            case 2: color = 0x02; break;  /* Cyan */
            case 3: color = 0x04; break;  /* Purple */
            case 4: color = 0x05; break;  /* Green */
            default: color = 0x01; break; /* White */
        }
        
        /* Darken for Y-side walls */
        if (side == 1)
        {
            /* Use a darker shade */
            if (color == 0x03) color = 0x0A;  /* Dark red */
            else if (color == 0x02) color = 0x0B;  /* Dark cyan */
            else if (color == 0x04) color = 0x0C;  /* Dark purple */
            else if (color == 0x05) color = 0x0D;  /* Dark green */
            else color = 0x00;  /* Black */
        }
        
        /* Draw wall column (2 pixels wide for 320 screen) */
        for (int row = wall_top; row < wall_bottom; row++)
        {
            renderlib_plot((unsigned char)(col * 2), (unsigned char)row, color);
            renderlib_plot((unsigned char)(col * 2 + 1), (unsigned char)row, color);
        }
        
        /* Draw floor and ceiling */
        for (int row = 0; row < wall_top; row++)
        {
            renderlib_plot((unsigned char)(col * 2), (unsigned char)row, 0x00);
            renderlib_plot((unsigned char)(col * 2 + 1), (unsigned char)row, 0x00);
        }
        for (int row = wall_bottom; row < 200; row++)
        {
            renderlib_plot((unsigned char)(col * 2), (unsigned char)row, 0x06);
            renderlib_plot((unsigned char)(col * 2 + 1), (unsigned char)row, 0x06);
        }
    }
}

/*
----------------------------------------------------------
Scene Management
----------------------------------------------------------
*/

void render3d_scene_clear(void)
{
    /* No persistent scene state in this implementation
       Objects are rendered immediately when drawn. */
}

void render3d_scene_render(void)
{
    /* No persistent scene state in this implementation
       Objects are rendered immediately when drawn. */
}
