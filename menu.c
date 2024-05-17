#include "menu.h"

int scale = 4;

void activate_menu(unsigned short *fb, font_descriptor_t *fdes, unsigned int col, unsigned char *parlcd_mem_base, unsigned char *mem_base) {

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 15 * 100 * 100;
  int speed_level = 1;
  uint32_t led_loading = 15;

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

    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = led_loading; 
    led_loading = led_loading << 1;
    if (led_loading > 1<<31) {
      led_loading = 15;
    }

  // Draw logo
  for (int i = 0; i < strlen(logo); i++) {
    char ch = logo[i];
    draw_char(fdes, fb, x, y, ch, 0xb3ffff);
    x += (char_width(fdes, logo[i]) * 3.5 + 5);
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
    draw_char(fdes, fb, x, y + 80, ch, 0xb3ffff);
    x += (char_width(fdes, play[i]) * 3.5 + 5);
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
    draw_char(fdes, fb, x, y + 160, ch, 0xb3ffff);
    x += (char_width(fdes, settings[i]) * 3.5 + 5);
  }

    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if (((r>>24)&R_KNOB_o) != 0 && play_active) {
        printf("Game activated\n\n\n");
        activate_scene(fb, fdes, parlcd_mem_base, mem_base, speed_level);
    } else if (((r>>24)&R_KNOB_o) != 0 && pref_active) {
        speed_level = set_speed_level(activate_settings(fb, fdes, parlcd_mem_base, mem_base));
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

int set_speed_level(int speed) {
  if (speed > 200) {
    return 3;
  } else if (speed > 100) {
    return 2;
  } else {
    return 1;
  }
}