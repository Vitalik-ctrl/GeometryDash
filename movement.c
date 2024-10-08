#include "movement.h"

void player_compute_pos(int gravity, int r, player_t *player) {
  player->coords.x -= player->movement_x;
  player->movement_y += gravity;
  player->coords.y += player->movement_y; // yy = (((r>>8)&0xff)*320)/256;
}
