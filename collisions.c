
#include "collisions.h"

int rects_intersect(int x1, int y1, int s1, int x2, int y2, int w2, int h2) {
  int within_width = x1 > x2 && x1 + s1 < x2 + w2;
  int within_height = y1 + s1 > h2;
  return within_width && within_height;
}

// Function to check collision between a point and a triangle
bool CheckCollisionPointTriangle(Vector2 point, Vector2 v0, Vector2 v1, Vector2 v2) {
    // Compute vectors
    Vector2 v0v1 = {v1.x - v0.x, v1.y - v0.y};
    Vector2 v0v2 = {v2.x - v0.x, v2.y - v0.y};
    Vector2 vp = {point.x - v0.x, point.y - v0.y};

    // Compute dot products
    float dot00 = v0v1.x * v0v1.x + v0v1.y * v0v1.y;
    float dot01 = v0v1.x * v0v2.x + v0v1.y * v0v2.y;
    float dot02 = v0v1.x * vp.x + v0v1.y * vp.y;
    float dot11 = v0v2.x * v0v2.x + v0v2.y * v0v2.y;
    float dot12 = v0v2.x * vp.x + v0v2.y * vp.y;

    // Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

bool CheckCollisionSquareTriangle(Vector2 squarePos, float squareSize, Vector2 v0, Vector2 v1, Vector2 v2) {
    // Check each corner of the square against the triangle
    Vector2 corners[4] = {
        {squarePos.x, squarePos.y},
        {squarePos.x + squareSize, squarePos.y},
        {squarePos.x, squarePos.y + squareSize},
        {squarePos.x + squareSize, squarePos.y + squareSize}
    };

    for (int i = 0; i < 4; i++) {
        if (CheckCollisionPointTriangle(corners[i], v0, v1, v2)) {
            return true;
        }
        for (int i = 0; i < SQUARE_SIZE; i++) {
          Vector2 base_pix = {squarePos.x + i, squarePos.y};
          if (CheckCollisionPointTriangle(base_pix, v0, v1, v2)) {
            return true;
          }
        }
    }
    return false;
}