#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <termios.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"
#include "collisions.h"
#include "drawer.h"

#define R_KNOB_o 0x4
#define G_KNOB_o 0x2
#define B_KNOB_o 0x1
#define GRAVITY 4
#define JUMP_CONSTANT 30
#define BASE_LINE 260
#define PLAYER_HIGHT 60
#define SCREEN_WIDTH 480
#define SCREEN_HIGHT 320

unsigned short *fb;
font_descriptor_t *fdes;
int scale = 4;

void activate_menu(unsigned int col, unsigned char *parlcd_mem_base, unsigned char *mem_base);
unsigned int hsv2rgb_lcd(int hue, int saturation, int value);
void draw_pixel_big(int x, int y, unsigned short color);
int char_width(int ch);
void draw_char(int x, int y, char ch, unsigned short color);

void draw_rect_borders(unsigned short *fb, Vector2 coords, int width, int height, int border_width, unsigned short color) {
    // Drawing top and bottom borders
    for (int j = 0; j < border_width; j++) {
        for (int i = 0; i < width; i++) {
            draw_pixel(fb, i + coords.x, coords.y + j, color); // Top border
            draw_pixel(fb, i + coords.x, coords.y + height - 1 - j, color); // Bottom border
        }
    }

    // Drawing left and right borders (excluding corners)
    for (int j = border_width; j < height - border_width; j++) {
        for (int i = 0; i < border_width; i++) {
            draw_pixel(fb, coords.x + i, j + coords.y, color); // Left border
            draw_pixel(fb, coords.x + width - 1 - i, j + coords.y, color); // Right border
        }
    }
}

void drawCircle(unsigned short *fb, int centerX, int centerY, int radius, int width, int height) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Drawing circle points using symmetry
        fb[(centerX + x) + (centerY + y) * width] = 0xFFFF; // Assuming white color
        fb[(centerX + y) + (centerY + x) * width] = 0xFFFF;
        fb[(centerX - y) + (centerY + x) * width] = 0xFFFF;
        fb[(centerX - x) + (centerY + y) * width] = 0xFFFF;
        fb[(centerX - x) + (centerY - y) * width] = 0xFFFF;
        fb[(centerX - y) + (centerY - x) * width] = 0xFFFF;
        fb[(centerX + y) + (centerY - x) * width] = 0xFFFF;
        fb[(centerX + x) + (centerY - y) * width] = 0xFFFF;

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }

        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}


int main(int argc, char *argv[]) {

  unsigned char *parlcd_mem_base, *mem_base;
  int i, j;
  fb = (unsigned short *)malloc(SCREEN_HIGHT * SCREEN_WIDTH * 2);

  unsigned int col = hsv2rgb_lcd(0, 255, 255);

  printf("Hello world\n");

  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL) exit(1);

  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL) exit(1);

  parlcd_hx8357_init(parlcd_mem_base);
  fdes = &font_winFreeSystem14x16;

  activate_menu(col, parlcd_mem_base, mem_base);

  // Clear the screen
  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (i = 0; i < 320; i++) {
    for (j = 0; j < 480; j++) {
      fb[i + 480 * j] = 0;
      parlcd_write_data(parlcd_mem_base, fb[i + 480 * j]);
    }
  }

  printf("Goodbye\n");
  return 0;
}

void draw_pixel_big(int x, int y, unsigned short color) {
  int i, j;
  for (i = 0; i < scale; i++) {
    for (j = 0; j < scale; j++) {
      draw_pixel(fb, x + i, y + j, color);
    }
  }
}

void activate_settings(unsigned char *parlcd_mem_base, unsigned char *mem_base) {
  int i, j;
  int ptr;

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 100 * 1000;

    int xx = 0;
    int yy = 0;

    while (1) {
    // reading from knobs and ending the loop if knob is pressed
    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if ((r>>24)&B_KNOB_o) {
      break;
    }

    // set pixels' values in the buffer
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HIGHT; ptr++) {
        fb[ptr] = 0u;
    }
    char* speed_label = "Speed";

    int x = 20;
    int y = 70;

    for (int i = 0; i < strlen(speed_label); i++) {
      char ch = speed_label[i];
      draw_char(x, y, ch, 0xb3ffff);
      x += (char_width(speed_label[i]) * 3.5 + 5);
    }

    // Speed line
    for (j=70; j < SCREEN_WIDTH-70; j++) {
      draw_pixel(fb, j, y+95, 0x7ff);
    }

    // Drawing a small circle at center
    int centerX = 70;
    int centerY = y+95;
    int radius = 10;

    xx = ((r>>8&0xff)*480)/256;
    printf("%d\n", xx);

    drawCircle(fb, centerX + xx, centerY, radius, SCREEN_WIDTH, SCREEN_HIGHT);

    for (j=0; j<12; j++) {
      for (i=0; i<12; i++) {
        draw_pixel(fb, i+xx,j+yy,0x0);
      }
    }
    

    // write the data from buffer to the lcd display
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HIGHT ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
    // FPS
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
}

void activate_scene(unsigned char *parlcd_mem_base, unsigned char *mem_base) {

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
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HIGHT; ptr++) {
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
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HIGHT ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
    // FPS
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    a++;
  }
}

int char_width(int ch) {
  int width;
  if (!fdes->width) {
    width = fdes->maxwidth;
  } else {
    width = fdes->width[ch - fdes->firstchar];
  }
  return width;
}

unsigned int hsv2rgb_lcd(int hue, int saturation, int value) {
  hue = (hue % 360);
  float f = ((hue % 60) / 60.0);
  int p = (value * (255 - saturation)) / 255;
  int q = (value * (255 - (saturation * f))) / 255;
  int t = (value * (255 - (saturation * (1.0 - f)))) / 255;
  unsigned int r, g, b;
  if (hue < 60) {
    r = value; g = t; b = p;
  } else if (hue < 120) {
    r = q; g = value; b = p;
  } else if (hue < 180) {
    r = p; g = value; b = t;
  } else if (hue < 240) {
    r = p; g = q; b = value;
  } else if (hue < 300) {
    r = t; g = p; b = value;
  } else {
    r = value; g = p; b = q;
  }
  r >>= 3;
  g >>= 2;
  b >>= 3;
  return (((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f));
}

void draw_char(int x, int y, char ch, unsigned short color) {
  int w = char_width(ch);
  const font_bits_t *ptr;
  if ((ch >= fdes->firstchar) && (ch - fdes->firstchar < fdes->size)) {
    if (fdes->offset) {
      ptr = &fdes->bits[fdes->offset[ch - fdes->firstchar]];
    } else {
      int bw = (fdes->maxwidth + 15) / 16;
      ptr = &fdes->bits[(ch - fdes->firstchar) * bw * fdes->height];
    }
    int i, j;
    for (i = 0; i < fdes->height; i++) {
      font_bits_t val = *ptr;
      for (j = 0; j < w; j++) {
        if ((val & 0x8000) != 0) {
          draw_pixel_big(x + scale * j, y + scale * i, color);
        }
        val <<= 1;
      }
      ptr++;
    }
  }
}

void activate_menu(unsigned int col, unsigned char *parlcd_mem_base, unsigned char *mem_base) {

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;

  char *logo = "GeometryDash";
  char *play = "Play";
  char *settings = "Settings";
  int xx = 0;
  int yy = 0;
  int i, j;
  while (1) {
    int x = 20;
    int y = 40;
    bool play_active = false;
    bool pref_active = false;

  // Draw logo
  for (int i = 0; i < strlen(logo); i++) {
    char ch = logo[i];
    draw_char(x, y, ch, 0xb3ffff);
    x += (char_width(logo[i]) * 3.5 + 5);
  }

  // Draw "Play" label and gray rectangle
  x = 20;
  Vector2 rect_coords = {15, y + 80};
  draw_rect(fb, rect_coords, 2.5 * strlen(play) * fdes->maxwidth, fdes->height * scale, 0x264d00); // Gray rectangle
  if (rects_intersect(xx, yy, 20, 15, y + 80, 2.5 * strlen(play) * fdes->maxwidth, fdes->height * scale)) {
    draw_rect_borders(fb, rect_coords, 2.5 * strlen(play) * fdes->maxwidth, fdes->height * scale, 5, 0x0);
    play_active = true;
  }
  for (int i = 0; i < strlen(play); i++) {
    char ch = play[i];
    draw_char(x, y + 80, ch, 0xb3ffff);
    x += (char_width(play[i]) * 3.5 + 5);
  }

  // Draw "Settings" label and gray rectangle
  x = 20;
  Vector2 rect_coords2 = {15, y + 160};
  draw_rect(fb, rect_coords2, 2.5 * strlen(settings) * fdes->maxwidth, fdes->height * scale, 0x264d00); // Gray rectangle
  if (rects_intersect(xx, yy, 20, 15, y + 160, 2.5 * strlen(settings) * fdes->maxwidth, fdes->height * scale)) {
      draw_rect_borders(fb, rect_coords2, 2.5 * strlen(settings) * fdes->maxwidth, fdes->height * scale, 5, 0x0);
      pref_active = true;
  }
  for (int i = 0; i < strlen(settings); i++) {
    char ch = settings[i];
    draw_char(x, y + 160, ch, 0xb3ffff);
    x += (char_width(settings[i]) * 3.5 + 5);
    // print by chars with animation
    // parlcd_write_cmd(parlcd_mem_base, 0x2c);
    // for (int ptr = 0; ptr < 480 * 320; ptr++) parlcd_write_data(parlcd_mem_base, fb[ptr]);
    // sleep(0.5);
  }

    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if (((r>>24)&R_KNOB_o) != 0 && play_active) {
        printf("TADAAAM User clicked a button\n\n\n");
        activate_scene(parlcd_mem_base, mem_base);
    } else if (((r>>24)&R_KNOB_o) != 0 && pref_active) {
        activate_settings(parlcd_mem_base, mem_base);
    }

    xx = ((r&0xff)*480)/256;
    yy = (((r>>8)&0xff)*320)/256;

    for (j=0; j<12; j++) {
      for (i=0; i<12; i++) {
        draw_pixel(fb, i+xx,j+yy,0x0);
      }
    }

    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int ptr = 0; ptr < 480*320 ; ptr++) {
      parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);

    for (int ptr = 0; ptr < 320*480 ; ptr++) {
      fb[ptr] = 0x001a33;
    }
  }
}