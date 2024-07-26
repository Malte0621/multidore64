/*
----------------------------------------------------------
This file is a part of MultiDore 64 and was made to
demonstrate the library in action.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2024 by Malte0621
*/

#include "multidore64/include.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  struct renderlib3d_camera *camera;
  struct renderlib3d_object *cube;

  renderlib_init();
  renderlib3d_init();
  soundlib_init();
  controller_init();

  camera = renderlib3d_getcam();

  cube = renderlib3d_createobj(0);
  cube->pos->z = 5;
  cube->scl->x = 4;
  cube->scl->y = 4;
  cube->scl->z = 4;

  sleep(50);

  while (1) {
    renderlib3d_render();

    cube->rot->x = (cube->rot->x + 1) % 360;
    cube->rot->y = (cube->rot->y + 1) % 360;
    cube->rot->z = (cube->rot->z + 1) % 360;

    // camera->pos->z -= 1;

    sleep(5000);
  }

  renderlib3d_clear();
  renderlib_unload();
  return EXIT_SUCCESS;
}