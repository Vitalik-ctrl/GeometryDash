#define _POSIX_C_SOURCE 200112L

#include "scene.h"

void draw_level(unsigned short *fb, int *floor, int shift);

void activate_scene(unsigned short *fb, font_descriptor_t *fdes, 
          unsigned char *parlcd_mem_base, unsigned char *mem_base, int speed_level) {

  printf("Speed is: %d\n", speed_level);
  int i, j;
  int ptr;

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000 * 1000;
  uint32_t progress = 1 << 31;

  player_t cube;
  cube.coords.x = 0; // Center of the screen
  cube.coords.y = BASE_LINE - PLAYER_HIGHT; // Initial y position
  cube.size = PLAYER_HIGHT; // Size of the cube
  cube.rotation = 0; // Initial rotation angle
  cube.movement_x = 0;
  cube.movement_y = 0;

  input_t key;
  key.R_jump = false;

  size_t a = 0;
  size_t shift = 0;
  int floor = BASE_LINE;
  int old_floor = BASE_LINE;
  bool start_button_is_still_pressed = false;

    uint32_t current_progress_unit = progress;
    int iterator = 0;
    int last_position = PLAYER_HIGHT * 100 + END_SPACE;
    while (1) {
    iterator++;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = progress; 
    if (iterator%(last_position/speed_level/32) == 0) {
        progress += current_progress_unit >> 1; 
        current_progress_unit >>= 1;
    }

    // reading from knobs and ending the loop if knob is pressed
    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if (((r>>24)&R_KNOB_o) != 0 && !(key.R_jump) && shift == 0) {
      // this is to prevent the jump caused by pressing START
      start_button_is_still_pressed = true;
    }
    else if (((r>>24)&R_KNOB_o) != 0 && !(key.R_jump) && !start_button_is_still_pressed) {
      cube.movement_y = - JUMP_CONSTANT;
      key.R_jump = true;
    }
    else if (((r>>24)&R_KNOB_o) == 0 && start_button_is_still_pressed) {
      start_button_is_still_pressed = false;
    }
    if (((r>>24)&B_KNOB_o) != 0) {
      break;
    }


    // compute pos of PLAYER
    player_compute_pos(GRAVITY, r, &cube);
    // compute colision with floor
    if (CheckCollisionPlayerFloor(floor, old_floor, &cube, &key)) {
      printf("seems like we got a collision with a vertical wall!\n");
    }
    // set pixels' values in the buffer
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        fb[ptr] = 0u;
    }

    // draw obstacless
    old_floor = floor;
    floor = BASE_LINE;
    draw_level(fb, &floor, shift);

    handle_loss(fb, shift);

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
    shift += speed_level;
  }
}

void handle_loss(unsigned short *fb, int shift) {
    char *game_over = "Game over :(";
    font_descriptor_t *fdes;
    fdes = &font_winFreeSystem14x16;
    int x = 50;
    int y = 40;

    for (int i = 0; i < strlen(game_over); i++) {
      char ch = game_over[i];
      draw_char(fdes, fb, x, y, ch, 0xb3ffff);
      x += (char_width(fdes, game_over[i]) * 3.5 + 5);
  }
  
}

void draw_level(unsigned short *fb, int *floor, int shift) {
  int level_size = 100;
  int level_map[level_size];
  int step = PLAYER_HIGHT;


  draw_square(fb, 5 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 6 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 7 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 8 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 8 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 9 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 10 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 10 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 11 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 12 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 13 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 13 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 14 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 14 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, 0x7ff);
  draw_square(fb, 14 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, 0x7ff);



  //for (int i = 0; i < level_size; i++) {
  //  if (i % 3 == 1) {
  //    draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  //    draw_square(fb, i * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, 0x7ff);
  //  } else {
  //    draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, 0x7ff);
  //  }
  //}
}
