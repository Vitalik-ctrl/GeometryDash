#ifndef MENU_H
#define MENU_H

#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "font_types.h"
#include "mzapo_regs.h"
#include "settings.h"
#include "scene.h"
#include "util.h"
#include "drawer.h"

void activate_menu(unsigned short *fb, font_descriptor_t *fdes, unsigned int col, unsigned char *parlcd_mem_base, unsigned char *mem_base);

#endif