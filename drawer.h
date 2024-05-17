#ifndef DRAWER_H
#define DRAWER_H

#include "util.h"
#include "font_types.h"

#include <stdio.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define COLOR 256
#define BASE 250
#define OBSTACLE_HEIGHT 130
#define SQUARE_SIZE 60
#define START_POS_X 60
#define PLAYER_HIGHT 60
#define BASE_LINE 260
#define SCALE 4

void draw_back(unsigned short *fb, unsigned short color);

void draw_pixel(unsigned short *fb, int x, int y, unsigned short color);

void draw_square(unsigned short *fb, int x, int y, int size, int *floor, unsigned short color);

void draw_base(unsigned short *fb, unsigned short color);

void draw_triangle(unsigned short *fb, int start_pos, int end_pos, unsigned short color);

void draw_line(unsigned short *fb);

void draw_rect(unsigned short *fb, Vector2 coords, int width, int height, unsigned short color);

void drawCircle(unsigned short *fb, int centerX, int centerY, int radius, int width, int height);

void draw_rect_borders(unsigned short *fb, Vector2 coords, int width, int height, int border_width, unsigned short color);

int char_width(font_descriptor_t *fdes, int ch);

unsigned int hsv2rgb_lcd(int hue, int saturation, int value);

void draw_char(font_descriptor_t *fdes, unsigned short *fb, int x, int y, char ch, unsigned short color);

void draw_pixel_big(unsigned short *fb, int x, int y, unsigned short color);

#endif /* DRAWER_H */