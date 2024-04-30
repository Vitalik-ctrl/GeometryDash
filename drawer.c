#include "drawer.h"

// Draw background
void draw_back(unsigned short *fb, unsigned short color) {
  for (int ptr = 0; ptr < SCREEN_HEIGHT * SCREEN_WIDTH; ptr++) fb[ptr] = color;
}
 
 // Draw pixel with specified color 
void draw_pixel(unsigned short *fb, int x, int y, unsigned short color) {
  if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) 
    fb[x+480*y] = color;
}

// Draw square
void draw_square(unsigned short *fb, int x, int y, int size, unsigned short color) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++) {
      draw_pixel(fb, i + x, j + y, color);
    }
  }
}

// Draw base
void draw_base(unsigned short *fb, unsigned short color) {
  for (int j = SCREEN_HEIGHT; j > BASE; j--) {
    for (int i = 0; i < SCREEN_WIDTH; i++) {
      draw_pixel(fb, i, j, color);
    }
  }
}

// Draw triangle
void draw_triangle(unsigned short *fb, int start_pos, int end_pos, unsigned short color) {
  for (int j = BASE; j > OBSTACLE_HEIGHT; j--) {
    for (int i = start_pos; i < end_pos; i++) {
      draw_pixel(fb, i, j, color);
    }
    start_pos++;
    end_pos--;
  }
}

void draw_line(unsigned short *fb) {
    for (int i = 40; i < SCREEN_WIDTH - 40; i++) {
      draw_pixel(fb, i, BASE, 0xd3d3d3);
    }
    for (int i = 70; i < SCREEN_WIDTH - 70; i++) {
      draw_pixel(fb, i, BASE, 0xffff);
    }
}