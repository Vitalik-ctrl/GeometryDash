#include "settings.h"

int activate_settings(unsigned short *fb, font_descriptor_t *fdes, unsigned char *parlcd_mem_base, unsigned char *mem_base) {
  int i, j;
  int ptr;

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 100 * 1000;

    int xx = 0;
    int yy = 0;

    while (1) {
    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if ((r>>24)&B_KNOB_o) {
      break;
    }

    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        fb[ptr] = 0u;
    }
    char* speed_label = "Speed";

    int x = 20;
    int y = 70;

    for (int i = 0; i < strlen(speed_label); i++) {
      char ch = speed_label[i];
      draw_char(fdes, fb, x, y, ch, 0xb3ffff);
      x += (char_width(fdes, speed_label[i]) * 3.5 + 5);
    }

    // Speed line
    for (j=70; j < SCREEN_WIDTH-70; j++) {
      draw_pixel(fb, j, y+95, 0x7ff);
    }

    // Drawing a small circle at center
    int centerX = 70;
    int centerY = y+95;
    int radius = 10;

    if (((r>>8&0xff)*480)/256 < 333 && ((r>>8&0xff)*480)/256 > 10) xx = ((r>>8&0xff)*480)/256;
    printf("%d\n", xx);

    drawCircle(fb, centerX + xx, centerY, radius, SCREEN_WIDTH, SCREEN_HEIGHT);

    for (j=0; j<12; j++) {
      for (i=0; i<12; i++) {
        draw_pixel(fb, i+xx,j+yy,0x0);
      }
    }

    // write the data from buffer to the lcd display
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < SCREEN_WIDTH * SCREEN_HEIGHT; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
    // FPS
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
  return xx;
}