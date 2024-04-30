#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>    
#include <unistd.h>
#include <stdbool.h>
 
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "drawer.h"

typedef struct Vector2 {
    int x;
    int y;
} Vector2;
 
unsigned short *fb;

int rects_intersect(int x1, int y1, int s1, int x2, int y2, int w2, int h2) {
  int within_width = x1 > x2 && x1 + s1 < x2 + w2;
  int within_height = y1 + s1 > h2;
  return within_width && within_height;
}

// Function to check collision between a point and a triangle
bool CheckCollisionPointTriangle(Vector2 point, Vector2 v0, Vector2 v1, Vector2 v2) {
    // Compute vectors
    Vector2 v0v1 = {v1.x - v0.x, v1.y - v0.y};
    Vector2 v0v2 = {v2.x - v0.x, v2.y - v0.y};
    Vector2 vp = {point.x - v0.x, point.y - v0.y};

    // Compute dot products
    float dot00 = v0v1.x * v0v1.x + v0v1.y * v0v1.y;
    float dot01 = v0v1.x * v0v2.x + v0v1.y * v0v2.y;
    float dot02 = v0v1.x * vp.x + v0v1.y * vp.y;
    float dot11 = v0v2.x * v0v2.x + v0v2.y * v0v2.y;
    float dot12 = v0v2.x * vp.x + v0v2.y * vp.y;

    // Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

bool CheckCollisionSquareTriangle(Vector2 squarePos, float squareSize, Vector2 v0, Vector2 v1, Vector2 v2) {
    // Check each corner of the square against the triangle
    Vector2 corners[4] = {
        {squarePos.x, squarePos.y},
        {squarePos.x + squareSize, squarePos.y},
        {squarePos.x, squarePos.y + squareSize},
        {squarePos.x + squareSize, squarePos.y + squareSize}
    };

    for (int i = 0; i < 4; i++) {
        if (CheckCollisionPointTriangle(corners[i], v0, v1, v2)) {
            return true;
        }
        for (int i = 0; i < SQUARE_SIZE; i++) {
          Vector2 base_pix = {squarePos.x + i, squarePos.y};
          if (CheckCollisionPointTriangle(base_pix, v0, v1, v2)) {
            return true;
          }
        }
    }
    return false;
}

int main(int argc, char *argv[]) {

  int i,j;
  int ptr;
  unsigned int c;
  int end = 0;

  uint32_t progress = 1 << 31;

  unsigned char *parlcd_mem_base, *mem_base;

  fb  = (unsigned short *)malloc(SCREEN_HEIGHT * SCREEN_WIDTH * 2);
 
  printf("*************************\n");
  printf("-- Start Application --\n");
  printf("*************************\n");
 
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL) exit(1);
 
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL) exit(1);
 
  parlcd_hx8357_init(parlcd_mem_base);
 
  parlcd_write_cmd(parlcd_mem_base, 0x2c);

  // allocate all pixels and set them to 0
  ptr = 0;
  for (i = 0; i < SCREEN_HEIGHT; i++) 
  {
    for (j = 0; j < SCREEN_WIDTH; j++) 
    {
      c = 0;
      fb[ptr]= c;
      parlcd_write_data(parlcd_mem_base, fb[ptr++]);
    }
  }
 
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;

  int triangle_x, triangle_y;
  int step = 0;

  Vector2 v0 = {260, BASE};
  Vector2 v1 = {340, BASE};
  Vector2 v2 = {300, 150};
  Vector2 squarePos = {0, 0};

  uint32_t current_progress_unit = progress;
  while (1) {
    
    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    // progress bar (LED Line)
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = progress; 
    progress += current_progress_unit >> 1; 
    current_progress_unit >>= 1;

    // one of knobs were pushed
    if ( (r & 0x7000000) != 0 ) break;

    // change positon of x and y

    // blue knob
    squarePos.x = ((r & 0xff) * SCREEN_WIDTH) / COLOR;
    // green knob
    squarePos.y = (((r>>8)&0xff) * (SCREEN_HEIGHT-125)) / COLOR;

    // if (triangle_intersect(square_x, square_y, SQUARE_SIZE, 260-step, 340-step)) {
    //   draw_square(fb, square_x, square_y, SQUARE_SIZE, 0xff000);
    //   printf("There is intersection with triangle.\n");
    //   end = 1;
    // }

    if (rects_intersect(squarePos.x, squarePos.y, SQUARE_SIZE, 0, SCREEN_WIDTH, SCREEN_WIDTH, BASE)) {
      squarePos.y = BASE - SQUARE_SIZE+1;
      printf("There is base line.\n");
    }

    draw_back(fb, 0xffff);
    printf("square y position: %d\n", squarePos.y);

    // Entity
    draw_square(fb, squarePos.x, squarePos.y, SQUARE_SIZE, (short unsigned int) 0x0070ff);

    // Scene. Draws 3 triangles with coords-step: movement to the left
    draw_base(fb, 0x0);

    triangle_x = 260, triangle_y = 340;
    draw_triangle(fb, triangle_x, triangle_y, 0x0);

    printf("%d %d\n", squarePos.x, squarePos.y);
    printf("%d %d\n", v0.x, v0.y);
    printf("%d %d\n", v1.x, v1.y);
    printf("%d %d\n", v2.x, v2.y);
    if (CheckCollisionSquareTriangle(squarePos, SQUARE_SIZE, v0, v1, v2)) {
      printf("Trinagle & Square colliosion\n");
    }

    // triangle_x = 400-step, triangle_y = 480-step;
    // draw_triangle(fb, triangle_x, triangle_y, 0x0);
    // triangle_x = 800-step, triangle_y = 880-step;
    // draw_triangle(fb, triangle_x, triangle_y, 0x0);


    // Send pixels on LCD screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    step++;

    if (end) break;
  }
 
  printf("Goodbye world\n");
 
  return 0;
}