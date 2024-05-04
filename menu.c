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

#define M_PI 3.1415

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

unsigned short fb[320*480*2];
font_descriptor_t *fdes;
int scale = 4;

void draw_pixel_big(int x, int y, unsigned short color) {
  int i, j;
  for (i = 0; i < scale; i++) {
    for (j = 0; j < scale; j++) {
      draw_pixel(fb, x + i, y + j, color);
    }
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

void draw_rect(int x, int y, int width, int height, unsigned short color) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      draw_pixel(fb, x + i, y + j, color);
    }
  }
}

int main(int argc, char *argv[]) {

  unsigned char *parlcd_mem_base, *mem_base;
  int i, j;
  int first_time = 1;

  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL) exit(1);

  parlcd_hx8357_init(parlcd_mem_base);
  fdes = &font_winFreeSystem14x16;

  // Initialize mem_base
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL) exit(1);

  char *logo = "GeometryDash";
  char *play = "Play";
  char *settings = "Settings";
  unsigned int col = hsv2rgb_lcd(0, 255, 255);

  // Square code from the other snippet
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;
  int xx=0, yy=0;

  while (1) {
  int x = 20;
  int y = 60;

  // Draw logo
  for (int i = 0; i < strlen(logo); i++) {
    char ch = logo[i];
    draw_char(x, y, ch, col);
    x += (char_width(logo[i]) * 3.5 + 5);
    // parlcd_write_cmd(parlcd_mem_base, 0x2c);
    // for (int ptr = 0; ptr < 480 * 320; ptr++) parlcd_write_data(parlcd_mem_base, fb[ptr]);
    // sleep(0.5);
  }

  // Draw "Play" label and gray rectangle
  x = 20;
  draw_rect(15, y + 80, 2.5 * strlen(play) * fdes->maxwidth, fdes->height * scale, 0x8410); // Gray rectangle
  if (rects_intersect(xx, yy, 20, 15, y + 80, 2.5 * strlen(play) * fdes->maxwidth, fdes->height * scale)) {
    printf("Play button is active!\n");
  }
  for (int i = 0; i < strlen(play); i++) {
    char ch = play[i];
    draw_char(x, y + 80, ch, col);
    x += (char_width(play[i]) * 3.5 + 5);
    // parlcd_write_cmd(parlcd_mem_base, 0x2c);
    // for (int ptr = 0; ptr < 480 * 320; ptr++) parlcd_write_data(parlcd_mem_base, fb[ptr]);
    // sleep(0.5);
  }

  // Draw "Settings" label and gray rectangle
  x = 20;
  draw_rect(15, y + 160, 2.5 * strlen(settings) * fdes->maxwidth, fdes->height * scale, 0x8410); // Gray rectangle
  if (rects_intersect(xx, yy, 20, 15, y + 160, 2.5 * strlen(settings) * fdes->maxwidth, fdes->height * scale)) {
      printf("Settings button is active!\n");
  }
  for (int i = 0; i < strlen(settings); i++) {
    char ch = settings[i];
    draw_char(x, y + 160, ch, col);
    x += (char_width(settings[i]) * 3.5 + 5);
    // parlcd_write_cmd(parlcd_mem_base, 0x2c);
    // for (int ptr = 0; ptr < 480 * 320; ptr++) parlcd_write_data(parlcd_mem_base, fb[ptr]);
    // sleep(0.5);
  }

    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if ((r&0x7000000)!=0) {
      break;
    }
    
    xx = ((r&0xff)*480)/256;
    yy = (((r>>8)&0xff)*320)/256;

    for (j=0; j<20; j++) {
      for (i=0; i<20; i++) {
        draw_pixel(fb, i+xx,j+yy,0x7ff);
      }
    }

    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int ptr = 0; ptr < 480*320 ; ptr++) {
      parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);

    for (int ptr = 0; ptr < 320*480 ; ptr++) {
      fb[ptr]=0u;
    }
  }

  sleep(20);

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
