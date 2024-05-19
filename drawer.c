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
// which can also be a floor
void draw_square(unsigned short *fb, int x, int y, int size, int *floor, int *floor_level, unsigned short color) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++) {
      draw_pixel(fb, i + x, j + y, color);
    }
  }
  int current_level = (BASE_LINE - y) / PLAYER_HIGHT;
  // change the floor variable according to the pos of this square
  if (current_level >= *floor_level && (x >= START_POS_X && x <= START_POS_X + PLAYER_HIGHT) || (x + PLAYER_HIGHT >= START_POS_X && x + PLAYER_HIGHT <= START_POS_X + PLAYER_HIGHT)) {
    *floor_level = (BASE_LINE - y) / PLAYER_HIGHT;
    *floor = y;
  }
}

void draw_rect(unsigned short *fb, Vector2 coords, int width, int height, unsigned short color) {
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      draw_pixel(fb, i + coords.x, j + coords.y, color);
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
      draw_pixel(fb, i, BASE, (unsigned short) 0xd3d3d3);
    }
    for (int i = 70; i < SCREEN_WIDTH - 70; i++) {
      draw_pixel(fb, i, BASE, 0xffff);
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

int char_width(font_descriptor_t *fdes, int ch) {
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

void draw_char(font_descriptor_t *fdes, unsigned short *fb, int x, int y, char ch, unsigned short color) {
  int w = char_width(fdes, ch);
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
          draw_pixel_big(fb, x + SCALE * j, y + SCALE * i, color);
        }
        val <<= 1;
      }
      ptr++;
    }
  }
}

void draw_pixel_big(unsigned short *fb, int x, int y, unsigned short color) {
  int i, j;
  for (i = 0; i < SCALE; i++) {
    for (j = 0; j < SCALE; j++) {
      draw_pixel(fb, x + i, y + j, color);
    }
  }
}
