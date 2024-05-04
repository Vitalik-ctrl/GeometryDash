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
#include "collisions.h"

unsigned short *fb;

rect_t generate_obstacle() {
  rect_t rect;
  rect.coords.x = 0;
  rect.coords.y = 0;
  rect.width = 70;
  rect.height = 70;
  rect.color = 0x0;
  return rect;
}

int main(int argc, char *argv[]) {

  int end = 0;
  uint32_t progress = 1 << 31;

  unsigned char *parlcd_mem_base, *mem_base;

  fb  = (unsigned short *)malloc(SCREEN_HEIGHT * SCREEN_WIDTH * 2);
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL) exit(1);
 
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL) exit(1);
 
  parlcd_hx8357_init(parlcd_mem_base);
  parlcd_write_cmd(parlcd_mem_base, 0x2c);

  // allocate all pixels and set them to 0
  int pixels_count = SCREEN_HEIGHT * SCREEN_WIDTH;
  for (int ptr = 0; ptr < pixels_count; ptr++) {
    fb[ptr] = 0;
    parlcd_write_data(parlcd_mem_base, fb[ptr++]);
  }
 
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;
  int step = 0;

  Vector2 v0 = {260, BASE};
  Vector2 v1 = {340, BASE};
  Vector2 v2 = {300, 180};
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

    if (CheckCollisionSquareTriangle(squarePos, SQUARE_SIZE, v0, v1, v2)) {
      printf("Trinagle & Square colliosion\n");
    }
    
    draw_triangle(fb, v0.x-=4, v1.x-=4, 0x0);
    rect_t obs = generate_obstacle();
    draw_rect(fb, obs.coords, obs.width, obs.height, obs.color);

    // Send pixels on LCD screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    step++;

    if (end) break;
  }
 
  printf("Goodbye world\n");
 
  return 0;
}
