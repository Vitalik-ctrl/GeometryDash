#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
} Vector2;

typedef struct {
  bool R_jump; // red knob is for jumping
  uint32_t R_jump_o; // offset of the red knob
} input_t;

// !! Player model struct !!
typedef struct {
    Vector2 coords;
    int size;
    int rotation; // rotation angle in degrees
    int jump_height;
    int movement_x; // movement in the direction of x
    int movement_y;
} player_t;

// !! Obstacles structs !!
typedef struct {
  Vector2 coords;
  int width;
  int height;
  unsigned short color;
} rect_t;

typedef struct {
  Vector2 coords_1; // lower left vertex
  Vector2 coords_2; // lower right vertex
  Vector2 coords_3; // top vertex
  unsigned short color;
} triangle_t;

#endif