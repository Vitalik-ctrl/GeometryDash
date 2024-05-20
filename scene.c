#define _POSIX_C_SOURCE 200112L

#include "scene.h"

void draw_level(unsigned short *fb, int *floor, int *floor_level, int shift);

void activate_scene(unsigned short *fb, font_descriptor_t *fdes, 
          unsigned char *parlcd_mem_base, unsigned char *mem_base, int speed_level) {

  printf("Speed is: %d\n", speed_level);
  int i, j;
  int ptr;

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000 * 1000;
  uint32_t progress = 1 << 31;
  int lose = 0;

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
  int floor_level = 0;
  bool start_button_is_still_pressed = false;

    uint32_t current_progress_unit = progress;
    int iterator = 0;
    int last_position = PLAYER_HIGHT * 30 + END_SPACE;
    while (1) {
    iterator++;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = progress; 
    if (iterator%(last_position/speed_level/32) == 0) {
        progress += current_progress_unit >> 1; 
        current_progress_unit >>= 1;
    }

    if (progress > (1<<10)-1) {
        printf("Win");
        handle_win(fb, shift);
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

    player_compute_pos(GRAVITY, r, &cube);

    // compute colision with floor
    if (CheckCollisionPlayerFloor(floor, old_floor, &cube, &key)) {
      lose = 1;
      //speed_level += 5;
    }
    // set pixels' values in the buffer
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        fb[ptr] = 0u;
    }

    // draw obstacless
    old_floor = floor;
    floor = BASE_LINE;
    draw_level2(fb, &floor, &floor_level, shift);
    printf("level of a block %d\n", floor_level);

    // draw LINE
    for (j=0; j < SCREEN_WIDTH; j++) {
      draw_pixel(fb, j, BASE_LINE, 0x7ff);
    }
    // draw PLAYER
    if (!lose) {
      for (j=0; j < PLAYER_HIGHT; j++) {
        for (i=0; i < PLAYER_HIGHT; i++) {
          draw_pixel(fb, i + cube.coords.x, j + cube.coords.y, 0x714);
        }
      }
      draw_rect_borders(fb, cube.coords, PLAYER_HIGHT, PLAYER_HIGHT, 24, 0x0);
      draw_rect_borders(fb, cube.coords, PLAYER_HIGHT, PLAYER_HIGHT, 12, 0x714);
    } else {
          handle_loss(fb, shift);
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

void draw_scene_text(unsigned short *fb, int shift, char *text, Vector2 position) {
    font_descriptor_t *fdes;
    fdes = &font_winFreeSystem14x16;

    for (int i = 0; i < strlen(text); i++) {
      char ch = text[i];
      draw_char(fdes, fb, position.x, position.y, ch, 0xb3ffff);
      position.x += (char_width(fdes, text[i]) * 3.5 + 5);
  }
}

void handle_loss(unsigned short *fb, int shift) {
  Vector2 pos;
  pos.x = 60;
  pos.y = 40;
    draw_scene_text(fb, shift, "Game over :(", pos);
}

void handle_win(unsigned short *fb, int shift) {
  Vector2 pos;
  pos.x = 100;
  pos.y = 140;
  draw_scene_text(fb, shift, "You won !", pos);
}


void draw_level(unsigned short *fb, int *floor, int *floor_level, int shift) {
  int level_size = 100;
  int level_map[level_size];
  int step = PLAYER_HIGHT;

  for (int i = 8; i < level_size; i++) {
    if (i % 3 == 1) {
      draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, 0x7ff);
      draw_square(fb, i * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, 0x7ff);
    } else {
      draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, 0x7ff);
    }
  }
}

void draw_level2(unsigned short *fb, int *floor, int *floor_level, int shift) {
  int level_size = 30;
  int level_map[level_size];
  int step = PLAYER_HIGHT;

  for (int i = 8; i < level_size; i++) {
    if (i % 3 == 1) {
      draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, 0xa1fb00);
      draw_square(fb, i * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, 0xa1fb00);
    } else if (i % 4 == 0) {
      draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, 0xa1fb00);
    } 
  }
}