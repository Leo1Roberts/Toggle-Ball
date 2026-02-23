#include "Sizes.h"

void updateSizes(float windowWidth, float windowHeight) {
	WINDOW_WIDTH = windowWidth;
	WINDOW_HEIGHT = windowHeight;
	RATIO = windowWidth / windowHeight;

	LEFT = -RATIO;
	RIGHT = RATIO;
}

float WINDOW_WIDTH, WINDOW_HEIGHT;
float RATIO;

float LEFT, RIGHT;

float HALF_HEIGHT;
float OUTLINE_WIDTH_WORLD;