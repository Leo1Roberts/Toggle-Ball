#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include "Model.h"
#include "ButtonStyles.h"

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
	vec2 pos;
	vec2 uv;
	col color;
	col outlineColor;
	float outlineRadius = 0;
};

struct ButtonManager {
	static bool isPressed;
	static vec2 pointerPos;

	static int focusedButtonIndex;
	static Button buttons[];

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

	static bool initButtonShader(
			const std::string& vertexSource,
			const std::string& fragmentSource,
			const std::string& positionAttributeName,
			const std::string& uvAttributeName,
			const std::string& colorAttributeName,
			const std::string& outlineColorAttributeName,
			const std::string& outlineRadiusAttributeName,
			const std::string& projectionMatrixUniformName);

	static void activateShader() { glUseProgram(program); }

	static void updateProjectionMatrix();

	static bool pressed(); // Returns true if the press was on a button
	static bool released(); // Returns true if the release was on a button

private:
	static vec2 pressedPos;

	static int numButtons;
	static int numButtonsDrawn;
	static short batchToFill;
	static short batchSize[]; // If MAX_BUTTONS > 256 this will need to be changed to a short

	static GLuint program;
	static GLint position;
	static GLint uv;
	static GLint color;
	static GLint outlineColor;
	static GLint outlineRadius;
	static GLint projectionMatrix;

	static GLuint vao;
	static GLuint vertex_buffer;
	static GLuint index_buffer;
	static ButtonVertex vertices[];
	static Index indices[];

	static void findFocusedButton();
};


#endif // BUTTON_MANAGER_H
