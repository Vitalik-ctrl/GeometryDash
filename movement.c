#include "movement.h"

void player_compute_pos(int gravity, int r, player_t *player) {
  // int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) in the main loop
  player->x = (((r&0xff)*480)/256) + player->movement_x;
  player->movement_y += gravity;
  player->y += player->movement_y; // yy = (((r>>8)&0xff)*320)/256;
}

void player_compute_pos(int gravity, int r, player_t *player) {
  // int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) in the main loop
  player->x = (((r&0xff)*480)/256) + player->movement_x;
  player->movement_y += gravity;
  player->y += player->movement_y; // yy = (((r>>8)&0xff)*320)/256;
}
