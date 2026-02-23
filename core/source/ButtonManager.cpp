#include "main.h"
#include "Colors.h"
#include "Sizes.h"
#include "Fonts.h"
#include "MatrixUtilities.h"
#include "Cursor.h"
#include "TextBoxManager.h"
#include "ButtonManager.h"

const float BUTTON_TEXTURE_INSET = 0.0625f;
const int MAX_BUTTONS = 1000; // WARNING: changing this may require changing the type of batchSize
const int MAX_BATCHES = 5;

bool ButtonManager::isPressed;
vec2 ButtonManager::pressedPos = { NAN, NAN };
vec2 ButtonManager::pointerPos = { NAN, NAN };

int ButtonManager::focusedButtonIndex;

Button ButtonManager::buttons[MAX_BUTTONS];
int ButtonManager::numButtons;
int ButtonManager::numButtonsDrawn;
short ButtonManager::batchToFill;
short ButtonManager::batchSize[MAX_BATCHES];

GLuint ButtonManager::program;
GLint ButtonManager::position;
GLint ButtonManager::uv;
GLint ButtonManager::color;
GLint ButtonManager::outlineColor;
GLint ButtonManager::outlineRadius;
GLint ButtonManager::projectionMatrix;

GLuint ButtonManager::vao;
GLuint ButtonManager::vertex_buffer;
GLuint ButtonManager::index_buffer;
ButtonVertex ButtonManager::vertices[MAX_BUTTONS * 24];
Index ButtonManager::indices[MAX_BUTTONS * 78];

bool focusedButtonIsPressed;
void ButtonManager::findFocusedButton() {
	for (int i = numButtons - 1; i >= 0; i--) {
		const Button& b = buttons[i];
		if (pointerPos.x >= b.l && pointerPos.x <= b.r &&
		    pointerPos.y <= b.t && pointerPos.y >= b.b &&
			!b.pointerPassThrough) {
			if (isPressed &&
			    pressedPos.x >= b.l && pressedPos.x <= b.r &&
			    pressedPos.y <= b.t && pressedPos.y >= b.b)
				focusedButtonIsPressed = true;
			else
				focusedButtonIsPressed = false;
			focusedButtonIndex = i;
			return;
		}
	}
	focusedButtonIndex = -1; // The pointer is not on any buttons
}

void ButtonManager::drawButtons(byte batch) {
	if (batch == 0) {
		findFocusedButton();
		batchToFill = 0;
	}

	const int nextBatchStart = numButtonsDrawn + batchSize[batch];
	for (int i = numButtonsDrawn; i < nextBatchStart; i++) {
		const Button& b = buttons[i];

		col mainCol;
		if (i == focusedButtonIndex) {
			if (focusedButtonIsPressed)
				mainCol = b.style->pressedColor;
			else
				mainCol = b.style->hoverColor;
		} else mainCol = b.style->baseColor;

		const col outlineCol = b.style->outlineColor;

		const float CS = b.style->cornerRadius;

		const float outlineThickness = b.style->outlineWidth;
		float outlineRad, inset;
		if (CS > outlineThickness) {
			outlineRad = (CS - outlineThickness) / CS;
			inset = 0;
		} else {
			outlineRad = 0;
			inset = outlineThickness - CS;
		}

		// Top row
		vertices[i * 24 + 0] = {{ b.l, b.t },			{ 1, 1 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 1] = {{ b.l + CS, b.t },		{ 0, 1 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 2] = {{ b.r - CS, b.t },		{ 0, 1 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 3] = {{ b.r, b.t },			{ 1, 1 }, mainCol, outlineCol, outlineRad };
		// Second row
		vertices[i * 24 + 4] = {{ b.l, b.t - CS },		{ 1, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 5] = {{ b.l + CS, b.t - CS },	{ 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 6] = {{ b.r - CS, b.t - CS },	{ 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 7] = {{ b.r, b.t - CS },		{ 1, 0 }, mainCol, outlineCol, outlineRad };
		// Third row
		vertices[i * 24 + 8] = {{ b.l, b.b + CS },		{ 1, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 9] = {{ b.l + CS, b.b + CS },	{ 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 10] = {{ b.r - CS, b.b + CS },{ 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 11] = {{ b.r, b.b + CS },		{ 1, 0 }, mainCol, outlineCol, outlineRad };
		// Bottom row
		vertices[i * 24 + 12] = {{ b.l, b.b },			{ 1, 1 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 13] = {{ b.l + CS, b.b },		{ 0, 1 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 14] = {{ b.r - CS, b.b },		{ 0, 1 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 15] = {{ b.r, b.b },			{ 1, 1 }, mainCol, outlineCol, outlineRad };
		// Inset centre quad
		vertices[i * 24 + 16] = {{ b.l + CS + inset, b.t - CS - inset }, { 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 17] = {{ b.l + CS + inset, b.b + CS + inset }, { 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 18] = {{ b.r - CS - inset, b.b + CS + inset }, { 0, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 19] = {{ b.r - CS - inset, b.t - CS - inset }, { 0, 0 }, mainCol, outlineCol, outlineRad };
		// Inset centre quad (for joining to bordering quads)
		vertices[i * 24 + 21] = {{ b.l + CS + inset, b.b + CS + inset }, { 0.005f, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 22] = {{ b.r - CS - inset, b.b + CS + inset }, { 0.005f, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 23] = {{ b.r - CS - inset, b.t - CS - inset }, { 0.005f, 0 }, mainCol, outlineCol, outlineRad };
		vertices[i * 24 + 20] = {{ b.l + CS + inset, b.t - CS - inset }, { 0.005f, 0 }, mainCol, outlineCol, outlineRad };

		for (int j = 0; j < 9; j++) {
			if (j == 4) {
				indices[i * 78 + j * 6 + 0] = i * 24 + 16 + 0;
				indices[i * 78 + j * 6 + 1] = i * 24 + 16 + 1;
				indices[i * 78 + j * 6 + 2] = i * 24 + 16 + 2;
				indices[i * 78 + j * 6 + 3] = i * 24 + 16 + 0;
				indices[i * 78 + j * 6 + 4] = i * 24 + 16 + 2;
				indices[i * 78 + j * 6 + 5] = i * 24 + 16 + 3;
			} else {
				indices[i * 78 + j * 6 + 0] = i * 24 + j / 3 * 4 + j % 3;
				indices[i * 78 + j * 6 + 1] = i * 24 + j / 3 * 4 + j % 3 + 4;
				indices[i * 78 + j * 6 + 2] = i * 24 + j / 3 * 4 + j % 3 + 5;
				indices[i * 78 + j * 6 + 3] = i * 24 + j / 3 * 4 + j % 3;
				indices[i * 78 + j * 6 + 4] = i * 24 + j / 3 * 4 + j % 3 + 5;
				indices[i * 78 + j * 6 + 5] = i * 24 + j / 3 * 4 + j % 3 + 1;
			}
		}

		// Trapezium insetting quads
		// Top
		indices[i * 78 + 54 +  0] = i * 24 +  5;
		indices[i * 78 + 54 +  1] = i * 24 + 20;
		indices[i * 78 + 54 +  2] = i * 24 + 23;
		indices[i * 78 + 54 +  3] = i * 24 +  5;
		indices[i * 78 + 54 +  4] = i * 24 + 23;
		indices[i * 78 + 54 +  5] = i * 24 +  6;
		// Left
		indices[i * 78 + 54 +  6] = i * 24 +  5;
		indices[i * 78 + 54 +  7] = i * 24 +  9;
		indices[i * 78 + 54 +  8] = i * 24 + 21;
		indices[i * 78 + 54 +  9] = i * 24 +  5;
		indices[i * 78 + 54 + 10] = i * 24 + 21;
		indices[i * 78 + 54 + 11] = i * 24 + 20;
		// Bottom
		indices[i * 78 + 54 + 12] = i * 24 + 21;
		indices[i * 78 + 54 + 13] = i * 24 +  9;
		indices[i * 78 + 54 + 14] = i * 24 + 10;
		indices[i * 78 + 54 + 15] = i * 24 + 21;
		indices[i * 78 + 54 + 16] = i * 24 + 10;
		indices[i * 78 + 54 + 17] = i * 24 + 22;
		// Right
		indices[i * 78 + 54 + 18] = i * 24 + 23;
		indices[i * 78 + 54 + 19] = i * 24 + 22;
		indices[i * 78 + 54 + 20] = i * 24 + 10;
		indices[i * 78 + 54 + 21] = i * 24 + 23;
		indices[i * 78 + 54 + 22] = i * 24 + 10;
		indices[i * 78 + 54 + 23] = i * 24 +  6;
	}

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ButtonVertex) * nextBatchStart * 24, vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * (nextBatchStart - numButtonsDrawn) * 78, indices + numButtonsDrawn * 78, GL_DYNAMIC_DRAW);

	// Draw as indexed triangles
	glDrawElements(GL_TRIANGLES, (GLsizei)(nextBatchStart - numButtonsDrawn) * 78, GL_UNSIGNED_SHORT, nullptr);

	numButtonsDrawn = nextBatchStart;
	batchSize[batch] = 0;
}

bool ButtonManager::initButtonShader(
		const std::string& vertexSource,
		const std::string& fragmentSource,
		const std::string& positionAttributeName,
		const std::string& uvAttributeName,
		const std::string& colorAttributeName,
		const std::string& outlineColorAttributeName,
		const std::string& outlineRadiusAttributeName,
		const std::string& projectionMatrixUniformName) {
	GLuint vertexShader = 0;
	vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
	if (!vertexShader)
		return false;

	GLuint fragmentShader = 0;
	fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (!fragmentShader) {
		glDeleteShader(vertexShader);
		return false;
	}

	GLuint iProgram = glCreateProgram();
	if (iProgram) {
		glAttachShader(iProgram, vertexShader);
		glAttachShader(iProgram, fragmentShader);

		glLinkProgram(iProgram);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(iProgram, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint logLength = 0;
			glGetProgramiv(iProgram, GL_INFO_LOG_LENGTH, &logLength);

			// If we fail to link the shader program, log the result for debugging
			if (logLength) {
				GLchar* log = new GLchar[logLength];
				glGetProgramInfoLog(iProgram, logLength, nullptr, log);
				delete[] log;
			}

			glDeleteProgram(iProgram);
		} else {
			GLint positionAttribute = glGetAttribLocation(iProgram, positionAttributeName.c_str());
			GLint uvAttribute = glGetAttribLocation(iProgram, uvAttributeName.c_str());
			GLint colorAttribute = glGetAttribLocation(iProgram, colorAttributeName.c_str());
			GLint outlineColorAttribute = glGetAttribLocation(iProgram, outlineColorAttributeName.c_str());
			GLint outlineRadiusAttribute = glGetAttribLocation(iProgram, outlineRadiusAttributeName.c_str());
			GLint projectionMatrixUniform = glGetUniformLocation(iProgram, projectionMatrixUniformName.c_str());

			if (positionAttribute != -1
			    && uvAttribute != -1
			    && colorAttribute != -1
				&& outlineColorAttribute != -1
				&& outlineRadiusAttribute != -1
			    && projectionMatrixUniform != -1) {
				program = iProgram;
				position = positionAttribute;
				uv = uvAttribute;
				color = colorAttribute;
				outlineColor = outlineColorAttribute;
				outlineRadius = outlineRadiusAttribute;
				projectionMatrix = projectionMatrixUniform;
			} else {
				glDeleteProgram(iProgram);
			}
		}
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	activateShader();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	// The position attribute is 2 floats
	glVertexAttribPointer(
		position, // attrib
		2, // elements
		GL_FLOAT, // of type float
		GL_FALSE, // don't normalize
		sizeof(ButtonVertex), // stride is ButtonVertex bytes
		nullptr // pull from the start of the vertex data
	);
	glEnableVertexAttribArray(position);

	// The uv attribute is 2 floats
	glVertexAttribPointer(
		uv, // attrib
		2, // elements
		GL_FLOAT, // of type float
		GL_FALSE, // don't normalize
		sizeof(ButtonVertex), // stride is ButtonVertex bytes
		(uint8_t*) sizeof(vec2) // offset vec2 from the start
	);
	glEnableVertexAttribArray(uv);

	// The color attribute is 4 bytes
	glVertexAttribPointer(
		color, // attrib
		4, // elements
		GL_UNSIGNED_BYTE, // of type byte
		GL_TRUE, // normalize
		sizeof(ButtonVertex), // stride is ButtonVertex bytes
		(uint8_t*) (sizeof(vec2) * 2) // offset 2 * vec2 from the start
	);
	glEnableVertexAttribArray(color);

	// The outline color attribute is 4 bytes
	glVertexAttribPointer(
		outlineColor, // attrib
		4, // elements
		GL_UNSIGNED_BYTE, // of type byte
		GL_TRUE, // normalize
		sizeof(ButtonVertex), // stride is ButtonVertex bytes
		(uint8_t*)(sizeof(vec2) * 2 + sizeof(col)) // offset 2 * vec2 + col from the start
	);
	glEnableVertexAttribArray(outlineColor);

	// The uv attribute is 1 float
	glVertexAttribPointer(
		outlineRadius, // attrib
		1, // elements
		GL_FLOAT, // of type float
		GL_FALSE, // don't normalize
		sizeof(ButtonVertex), // stride is ButtonVertex bytes
		(uint8_t*)(sizeof(vec2) * 2 + sizeof(col) * 2) // offset 2 * vec2 + 2 * col from the start
	);
	glEnableVertexAttribArray(outlineRadius);

	return true;
}

void ButtonManager::updateProjectionMatrix() {
	mat4 projMat;
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	glUniformMatrix4fv(projectionMatrix, 1, false, projMat.m16);
}

bool ButtonManager::pressed() {
	for (int i = numButtons - 1; i >= 0; i--) {
		const Button& b = buttons[i];
		if (pointerPos.x >= b.l && pointerPos.x <= b.r &&
		    pointerPos.y <= b.t && pointerPos.y >= b.b &&
			!b.pointerPassThrough) {
			isPressed = true;
			pressedPos = pointerPos;
			if (b.callbackImmediately)
				b.onClick(b.data);
			if (b.style->category != CAT_TEXT)
				TextBoxManager::focusedBoxIndex = -1; // Unfocus text box if clicking on a regular button
			return true;
		}
	}
	return false;
}

bool ButtonManager::released() {
	bool ret = false;
	if (isPressed) {
		isPressed = false;
		for (int i = numButtons - 1; i >= 0; i--) {
			const Button& b = buttons[i];
			if (pointerPos.x >= b.l && pointerPos.x <= b.r &&
			    pointerPos.y <= b.t && pointerPos.y >= b.b) {
				ret = true;
				if (pressedPos.x >= b.l && pressedPos.x <= b.r &&
				    pressedPos.y <= b.t && pressedPos.y >= b.b &&
				    !b.callbackImmediately &&
				    !b.pointerPassThrough) {
					if (b.onClick)
						b.onClick(b.data);
					break;
				}
			}
		}
#ifndef WINDOWS_VERSION
		pointerPos = { NAN, NAN };
#endif
	} else {
		for (int i = numButtons - 1; i >= 0; i--) {
			const Button& b = buttons[i];
			if (pointerPos.x >= b.l && pointerPos.x <= b.r &&
			    pointerPos.y <= b.t && pointerPos.y >= b.b)
				ret = true;
		}
	}
	return ret;
}

void ButtonManager::addButton(float l, float r, float t, float b,
	void (*onClick)(int data), int data,
	const ButtonStyle& style,
	const std::string& text,
	bool callBackImmediately,
	bool pointerPassThrough,
	bool affectsCursor) {
	if (numButtons == MAX_BUTTONS) return;

	buttons[numButtons++] = {
			l, r, t, b,
			onClick, data,
			&style,
			callBackImmediately,
			pointerPassThrough,
			affectsCursor
	};
	batchSize[batchToFill]++;

	if (!text.empty()) {
		float fontSize = (t - b) * style.fontSize;

		size_t dotIndex = text.find('.');
		float x, y;
		if (dotIndex == std::string::npos) { // No decimal point
			float textWidth = Text::calculateWidth(text, style.font, fontSize);
			x = (l + r - textWidth) * 0.5f;
		} else { // Centralise decimal point
			float leftWidth = Text::calculateWidth(text.substr(0, dotIndex), style.font, fontSize) + 0.5f * Text::calculateWidth(".", style.font, fontSize);
			x = (l + r) * 0.5f - leftWidth;
		}
		y = t - (t - b - fontSize) * 0.5f;
		Text::addText(x, y, text, style.font, fontSize, style.fontColor);
	}
}

void ButtonManager::addToggleButton(float l, float t, float state, void (*onClick)(int data)) {
	addButton(l, l + TOGGLE_WIDTH, t, t - TOGGLE_RADIUS * 2, onClick, 0, BS_TOGGLE, "");
	float start = l + TOGGLE_RADIUS - TOGGLE_BLOB_RADIUS + state * (TOGGLE_WIDTH - TOGGLE_RADIUS * 2);
	addButton(start, start + TOGGLE_BLOB_RADIUS * 2, t - TOGGLE_RADIUS + TOGGLE_BLOB_RADIUS, t - TOGGLE_RADIUS - TOGGLE_BLOB_RADIUS, nullptr, 0, BS_TOGGLE_BLOB, "", false, true);
}
