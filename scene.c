#define _POSIX_C_SOURCE 200112L

#include "scene.h"

void draw_level3(unsigned short *fb, int *floor, int *floor_level, player_t *player, int shift);
void draw_level2(unsigned short *fb, int *floor, int *floor_level, player_t *player, int shift);

int activate_scene(unsigned short *fb, font_descriptor_t *fdes, 
          unsigned char *parlcd_mem_base, unsigned char *mem_base, int speed_level, int level) {

  printf("Speed is: %d\n", speed_level);
  int i, j;
  int ptr;

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000 * 1000;
  uint32_t progress = 1 << 31;

  player_t cube;
  cube.coords.x = START_POS_X; // Center of the screen
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
  bool in_air = false;
  int lose = 0;
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
      if (((r>>24)&R_KNOB_o) != 0 && !(key.R_jump) && !in_air && shift == 0) {
        // this is to prevent the jump caused by pressing START
        start_button_is_still_pressed = true;
      }
      else if (((r>>24)&R_KNOB_o) != 0 && !(key.R_jump) && !in_air && !start_button_is_still_pressed) {
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
      if (!lose && CheckCollisionPlayerFloor(floor, old_floor, &cube, &key, &in_air)) {
        printf("seems like we got a collision with a vertical wall!\n");
        cube.movement_y = - JUMP_CONSTANT;
        lose = 1;
        cube.movement_x = speed_level + 3;
        speed_level *= 2;
      } 
      printf("player lower part hight %d\n", (cube.coords.y + PLAYER_HIGHT));
      // set pixels' values in the buffer
      for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        fb[ptr] = 0u;
      }

      // draw obstacless and handle the floor
      if (!lose) {
        old_floor = floor;
        floor = BASE_LINE;
      }

      switch (level) {
        case 1:
          draw_level3(fb, &floor, &floor_level, &cube, shift);
        default:
          draw_level2(fb, &floor, &floor_level, &cube, shift);
      }

      if (lose) {
        floor = -100;
      }
  
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
        for (j=0; j < PLAYER_HIGHT; j++) {
          for (i=0; i < PLAYER_HIGHT; i++) {
            draw_pixel(fb, i + cube.coords.x, j + cube.coords.y, 0x714);
          }
        }
        draw_rect_borders(fb, cube.coords, PLAYER_HIGHT, PLAYER_HIGHT, 24, 0x0);
        draw_rect_borders(fb, cube.coords, PLAYER_HIGHT, PLAYER_HIGHT, 12, 0x714);
        handle_loss(fb, shift);
      }


      if ((progress >= (1<<32)-1) && !lose) {
          level++;
          handle_win(fb, shift);
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
    return level;
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


  void draw_level2(unsigned short *fb, int *floor, int *floor_level, player_t *player, int shift) {
    int level_size = 100;
    int level_map[level_size];
    int step = PLAYER_HIGHT;

    draw_square(fb, 10 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 13 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x9fe0);
    draw_square(fb, 16 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x07ff);

    draw_square(fb, 18 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xff20);
    draw_square(fb, 20 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 21 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 23 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 23 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 25 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 26 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 27 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 27 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 28 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 29 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 29 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 31 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 33 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 33 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 34 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 36 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 36 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 38 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 38 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 40 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 40 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 43 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 43 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 45 * step - shift, BASE_LINE - 3 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 45 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 45 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 47 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 47 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 49 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 51 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);
    draw_square(fb, 51 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 53 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0xf800);

    draw_square(fb, 55 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0xa1fb00);

    for (int i = 60; i < level_size; i++) {
      if (i * step - shift >= - PLAYER_HIGHT && i * step - shift <= SCREEN_WIDTH) {
        if (i % 3 == 1) {
          draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
          draw_square(fb, i * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
        } else if (i%4 == 0) {
          draw_square(fb, i * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
        }
      }
    }
  }

  void draw_level3(unsigned short *fb, int *floor, int *floor_level, player_t *player, int shift) {
    int level_size = 100;
    int level_map[level_size];
    int step = PLAYER_HIGHT;

    // Colors for the rainbow effect
    unsigned short colors[] = {0xF800, 0xF800, 0x07E0, 0x07E0, 0x07E0, 0xF81F, 0xF81F}; // Red, Orange, Yellow, Green, Cyan, Blue, Violet
    int color_index = 0;

    draw_square(fb, 10 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 11 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 12 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 12 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 13 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 13 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 14 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 14 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 14 * step - shift, BASE_LINE - 3 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 15 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 15 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 15 * step - shift, BASE_LINE - 3 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 18 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 18 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 19 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 19 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 20 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 20 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 21 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 22 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);



    for (int i = 26; i < 56; i++) {
      int x = i * step - shift;
      
      if (x >= -PLAYER_HIGHT && x <= SCREEN_WIDTH) {
          if (i % 8 == 0 || i % 8 == 1) {
              // Double block high obstacle
              draw_square(fb, x, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
              draw_square(fb, x, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
          } else if (i % 8 == 2 || i % 8 == 3) {
              // Single block high obstacle
              draw_square(fb, x, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
          } else if (i % 8 == 4) {
              // Small gap
              // No block
          } else if (i % 8 == 5 || i % 8 == 6) {
              // Double block high obstacle
              draw_square(fb, x, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
              draw_square(fb, x, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
          } else if (i % 8 == 7) {
              // Single block high obstacle
              draw_square(fb, x, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
          }
      }
    }
    // Original provided pattern
    draw_square(fb, 62 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 65 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 65 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 68 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 68 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 71 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 72 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 75 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 78 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 78 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    // Extending the pattern manually up to 100 blocks
    draw_square(fb, 81 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 84 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 84 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 87 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 87 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 90 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 91 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 94 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 97 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
    draw_square(fb, 97 * step - shift, BASE_LINE - 2 * step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);

    draw_square(fb, 100 * step - shift, BASE_LINE - step, PLAYER_HIGHT, floor, floor_level, player, 0x7ff);
  }