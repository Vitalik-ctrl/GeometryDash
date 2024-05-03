#include <stdint.h>
#include <stdbool.h>

typedef struct Vector2 {
    int x;
    int y;
} Vector2;

typedef struct {
  bool R_jump; // red knob is for jumping
  uint32_t R_jump_o; // offset of the red knob
} input_t;

typedef struct {
    int size;
    int rotation; // rotation angle in degrees
    int jump_height;
    int x; // x pos of the center
    int y; // y pos of the center
    int movement_x; // movement in the direction of x
    int movement_y;
} player_t;
