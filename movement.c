#include "movement.h"
#include "drawer.c"

void player_compute_pos(int gravity, int r, player_t *player) {
  player->coords.x = START_POS_X; // (((r&0xff)*480)/256) + player->movement_x;
  player->movement_y += gravity;
  player->coords.y += player->movement_y; // yy = (((r>>8)&0xff)*320)/256;
}
