#ifndef SIZES_H
#define SIZES_H

// Measurements for UI elements
// Y ranges from bottom (-1.0f) to top (1.0f)
// X ranges from left (-RATIO) to right (RATIO)

// Call every time the window changes size
extern void updateSizes(float windowWidth, float windowHeight);

extern float WINDOW_WIDTH, WINDOW_HEIGHT;
extern float RATIO; // WINDOW_WIDTH / WINDOW_HEIGHT

constexpr float TOP = 1.0f;
constexpr float BOTTOM = -1.0f;
extern float LEFT;
extern float RIGHT;

extern float HALF_HEIGHT;

constexpr float OUTLINE_WIDTH = 0.003f;
extern float OUTLINE_WIDTH_WORLD;
constexpr float BLOB_RADIUS = 0.01f;
constexpr float PIVOT_RADIUS = OUTLINE_WIDTH * 2;

constexpr float TOGGLE_RADIUS = 0.03f;
constexpr float TOGGLE_BLOB_RADIUS = 0.02f;
constexpr float TOGGLE_WIDTH = TOGGLE_RADIUS * 4;

constexpr float TEXT_BOX_RIGHT_MARGIN = 0.04f;
constexpr float SEMI_STATEFUL_BLOB_RADIUS = 0.008f;

#endif // SIZES_H
