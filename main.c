#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "drawer.h"
#include "menu.h"

unsigned short *fb;
font_descriptor_t *fdes;

int main(int argc, char *argv[]) {

  unsigned char *parlcd_mem_base, *mem_base;
  int i, j;
  fb = (unsigned short *)malloc(SCREEN_HEIGHT * SCREEN_WIDTH * 2);

  unsigned int col = hsv2rgb_lcd(0, 255, 255);

  printf("Hello world\n");

  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL) exit(1);

  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL) exit(1);

  parlcd_hx8357_init(parlcd_mem_base);
  fdes = &font_winFreeSystem14x16;

  activate_menu(fb, fdes, col, parlcd_mem_base, mem_base);

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
