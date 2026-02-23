#ifndef SIZES_H
#define SIZES_H

// Measurements for UI elements
// Y ranges from bottom (-1.0f) to top (1.0f)
// X ranges from left (-RATIO) to right (RATIO)

// Call every time the window changes size
extern void updateSizes(float windowWidth, float windowHeight);

extern float WINDOW_WIDTH, WINDOW_HEIGHT;
extern float RATIO; // WINDOW_WIDTH / WINDOW_HEIGHT

const float TOP = 1.0f;
const float BOTTOM = -1.0f;
extern float LEFT;
extern float RIGHT;

extern float HALF_HEIGHT;

const float OUTLINE_WIDTH = 0.003f;
extern float OUTLINE_WIDTH_WORLD;
const float BLOB_RADIUS = 0.01f;
const float PIVOT_RADIUS = OUTLINE_WIDTH * 2;

const float TOGGLE_RADIUS = 0.03f;
const float TOGGLE_BLOB_RADIUS = 0.02f;
const float TOGGLE_WIDTH = TOGGLE_RADIUS * 4;

const float TEXT_BOX_RIGHT_MARGIN = 0.04f;
const float SEMI_STATEFUL_BLOB_RADIUS = 0.008f;

#endif // SIZES_H
