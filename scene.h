#ifndef SCENE_H
#define SCENE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "drawer.h"
#include "collisions.h"

#define GRAVITY 4
#define JUMP_CONSTANT 30
#define BASE_LINE 260
#define PLAYER_HIGHT 60

void activate_scene(unsigned short *fb, font_descriptor_t *fdes, 
                unsigned char *parlcd_mem_base, unsigned char *mem_base, int speed_level);

#endif