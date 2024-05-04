#ifndef DRAWER_H
#define DRAWER_H

#include "util.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define COLOR 256
#define BASE 250
#define OBSTACLE_HEIGHT 130
#define SQUARE_SIZE 60

void draw_back(unsigned short *fb, unsigned short color);

void draw_pixel(unsigned short *fb, int x, int y, unsigned short color);

void draw_square(unsigned short *fb, int x, int y, int size, unsigned short color);

void draw_base(unsigned short *fb, unsigned short color);

void draw_triangle(unsigned short *fb, int start_pos, int end_pos, unsigned short color);

void draw_line(unsigned short *fb);

void draw_rect(unsigned short *fb, Vector2 coords, int width, int height, unsigned short color);

#endif /* DRAWER_H */
