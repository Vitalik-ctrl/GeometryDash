#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "util.h"
#include "drawer.h"
#include "stdbool.h"

int rects_intersect(int x1, int y1, int s1, int x2, int y2, int w2, int h2);

bool CheckCollisionPointTriangle(Vector2 point, Vector2 v0, Vector2 v1, Vector2 v2);

bool CheckCollisionSquareTriangle(Vector2 squarePos, float squareSize, Vector2 v0, Vector2 v1, Vector2 v2);

bool CheckCollisionPlayerFloor(int floor, int old_floor, player_t *player, input_t *input, int *in_air);

#endif