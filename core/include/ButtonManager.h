#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include "ButtonStyles.h"
#include "Mesh.h"

struct Button {
	float l, r, t, b;

	void (*onClick)(int data);
	int data;

	const ButtonStyle* style;
	bool callbackImmediately;
	bool pointerPassThrough;
	bool affectsCursor;
};

struct ButtonVertex {
	vec2 position;
	vec2 uv;
	col fillColor;
	col outlineColor;
	float outlineRadius;

	ButtonVertex() = default;
	ButtonVertex(vec2 position, vec2 uv, col fillColor, col outlineColor, float outlineRadius) : position(position), uv(uv), fillColor(fillColor), outlineColor(outlineColor), outlineRadius(outlineRadius) {}

	static void setupLayout();
};

struct ButtonManager {
	static bool isPressed;
	static vec2 pointerPos;

	static int focusedButtonIndex;
	static Button buttons[];

	static void init();

	static void clearButtons() { // MUST be called each frame before adding buttons
		numButtons = 0;
		numButtonsDrawn = 0;
	}

	static void addButton(float l, float r, float t, float b,
	                      void (* onClick)(int data), int data,
	                      const ButtonStyle& style,
	                      const std::string& text = "",
						  bool callBackImmediately = false,
						  bool pointerPassThrough = false,
						  bool affectsCursor = true);

	static void addToggleButton(float l, float t, float state,
	                      void (* onClick)(int data));

	static inline void markEndOfBatch() { batchToFill++; }

	static void drawButtons(byte batch);

	static void updateProjectionMatrix();

	static bool pressed(); // Returns true if the press was on a button
	static bool released(); // Returns true if the release was on a button

private:
	static vec2 pressedPos;

	static int numButtons;
	static int numButtonsDrawn;
	static short batchToFill;
	static short batchSize[];

	static std::vector<ButtonVertex> vertices;
	static std::vector<Index> indices;

	static std::unique_ptr<Mesh<ButtonVertex>> mesh;

	static void findFocusedButton();
};


#endif // BUTTON_MANAGER_H
