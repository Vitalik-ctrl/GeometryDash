#define _POSIX_C_SOURCE 200112L

#include "scene.h"

void activate_scene(unsigned short *fb, font_descriptor_t *fdes, 
          unsigned char *parlcd_mem_base, unsigned char *mem_base, int speed_level) {

  printf("Speed is: %d\n", speed_level);
  int i, j;
  int ptr;

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 100 * 1000;

  player_t cube;
  cube.coords.x = 0; // Center of the screen
  cube.coords.y = BASE_LINE - PLAYER_HIGHT; // Initial y position
  cube.size = 60; // Size of the cube
  cube.rotation = 0; // Initial rotation angle
  cube.movement_x = 0;
  cube.movement_y = 0;

  input_t key;
  key.R_jump = false;

  size_t a = 0;

    while (1) {
    // reading from knobs and ending the loop if knob is pressed
    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if (((r>>24)&R_KNOB_o) != 0 && !(key.R_jump)) {
      cube.movement_y = -JUMP_CONSTANT;
      key.R_jump = true;
    }
    else if (((r>>24)&B_KNOB_o) != 0) {
      break;
    }

    // compute pos of PLAYER
    player_compute_pos(GRAVITY, r, &cube);

    // compute colision with floor
    CheckCollisionPlayerFloor(BASE_LINE, &cube, &key);


    // set pixels' values in the buffer
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        fb[ptr] = 0u;
    }
    // draw LINE
    for (j=0; j < SCREEN_WIDTH; j++) {
      draw_pixel(fb, j, BASE_LINE, 0x7ff);
    }
    // draw PLAYER
    for (j=0; j < PLAYER_HIGHT; j++) {
      for (i=0; i < PLAYER_HIGHT; i++) {
        draw_pixel(fb, i + cube.coords.x, j + cube.coords.y, 0x7ff);
      }
    }

    // write the data from buffer to the lcd display
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
    // FPS
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    a++;
  }
}
