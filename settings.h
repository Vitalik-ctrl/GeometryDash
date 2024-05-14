#ifndef SETTINGS_H
#define SETTINGS_H

#include "font_types.h"
#include "mzapo_regs.h"
#include "drawer.h"

#include <stddef.h>
#include <string.h>
#include <time.h>

int activate_settings(unsigned short *fb, font_descriptor_t *fdes, unsigned char *parlcd_mem_base, unsigned char *mem_base);

#endif