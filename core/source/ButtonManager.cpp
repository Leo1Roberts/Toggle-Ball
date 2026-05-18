#include <Shader.h>
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

void ButtonVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ButtonVertex), (void*)offsetof(ButtonVertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ButtonVertex), (void*)offsetof(ButtonVertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ButtonVertex), (void*)offsetof(ButtonVertex, fillColor));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ButtonVertex), (void*)offsetof(ButtonVertex, outlineColor));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ButtonVertex), (void*)offsetof(ButtonVertex, outlineRadius));
	glEnableVertexAttribArray(4);
}

bool ButtonManager::isPressed;
vec2 ButtonManager::pressedPos = { NAN, NAN };
vec2 ButtonManager::pointerPos = { NAN, NAN };

int ButtonManager::focusedButtonIndex;

Button ButtonManager::buttons[MAX_BUTTONS];
int ButtonManager::numButtons;
int ButtonManager::numButtonsDrawn;
short ButtonManager::batchToFill;
short ButtonManager::batchSize[MAX_BATCHES];

std::vector<ButtonVertex> ButtonManager::vertices;
std::vector<Index> ButtonManager::indices;

std::unique_ptr<Mesh<ButtonVertex>> ButtonManager::mesh;

void ButtonManager::init() {
	mesh = std::make_unique<Mesh<ButtonVertex>>(GL_DYNAMIC_DRAW);

	vertices.reserve(MAX_BUTTONS * 24);
	indices.reserve(MAX_BUTTONS * 78);
}

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

	vertices.clear();
	indices.clear();

	for (int i = 0; i < batchSize[batch]; i++) {
		const Button& b = buttons[i + numButtonsDrawn];

		col mainCol;
		if (i + numButtonsDrawn == focusedButtonIndex) {
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
		vertices.emplace_back(vec2(b.l, b.t),           vec2(1, 1), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.l + CS, b.t),      vec2(0, 1), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r - CS, b.t),      vec2(0, 1), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r, b.t),           vec2(1, 1), mainCol, outlineCol, outlineRad);
		// Second row
		vertices.emplace_back(vec2(b.l, b.t - CS),      vec2(1, 0), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.l + CS, b.t - CS), vec2(0, 0), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r - CS, b.t - CS), vec2(0, 0), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r, b.t - CS),      vec2(1, 0), mainCol, outlineCol, outlineRad);
		// Third row
		vertices.emplace_back(vec2(b.l, b.b + CS),      vec2(1, 0), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.l + CS, b.b + CS), vec2(0, 0), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r - CS, b.b + CS), vec2(0, 0), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r, b.b + CS),      vec2(1, 0), mainCol, outlineCol, outlineRad);
		// Bottom row
		vertices.emplace_back(vec2(b.l, b.b),			vec2(1, 1), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.l + CS, b.b),		vec2(0, 1), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r - CS, b.b),		vec2(0, 1), mainCol, outlineCol, outlineRad);
		vertices.emplace_back(vec2(b.r, b.b),			vec2(1, 1), mainCol, outlineCol, outlineRad);
		// Inset centre quad
		vertices.emplace_back(vec2(b.l + CS + inset, b.t - CS - inset), vec2(0, 0), mainCol, outlineCol, outlineRad); // 16: TL
		vertices.emplace_back(vec2(b.l + CS + inset, b.b + CS + inset), vec2(0, 0), mainCol, outlineCol, outlineRad); // 17: BL
		vertices.emplace_back(vec2(b.r - CS - inset, b.b + CS + inset), vec2(0, 0), mainCol, outlineCol, outlineRad); // 18: BR
		vertices.emplace_back(vec2(b.r - CS - inset, b.t - CS - inset), vec2(0, 0), mainCol, outlineCol, outlineRad); // 19: TR
		// Inset centre quad (for joining to bordering quads)
		vertices.emplace_back(vec2(b.l + CS + inset, b.t - CS - inset), vec2(0.005f, 0), mainCol, outlineCol, outlineRad); // 20: TL
		vertices.emplace_back(vec2(b.l + CS + inset, b.b + CS + inset), vec2(0.005f, 0), mainCol, outlineCol, outlineRad); // 21: BL
		vertices.emplace_back(vec2(b.r - CS - inset, b.b + CS + inset), vec2(0.005f, 0), mainCol, outlineCol, outlineRad); // 22: BR
		vertices.emplace_back(vec2(b.r - CS - inset, b.t - CS - inset), vec2(0.005f, 0), mainCol, outlineCol, outlineRad); // 23: TR

		for (int j = 0; j < 9; j++) {
			if (j == 4) {
				indices.push_back(i * 24 + 16 + 0);
				indices.push_back(i * 24 + 16 + 1);
				indices.push_back(i * 24 + 16 + 2);
				indices.push_back(i * 24 + 16 + 0);
				indices.push_back(i * 24 + 16 + 2);
				indices.push_back(i * 24 + 16 + 3);
			} else {
				indices.push_back(i * 24 + j / 3 * 4 + j % 3);
				indices.push_back(i * 24 + j / 3 * 4 + j % 3 + 4);
				indices.push_back(i * 24 + j / 3 * 4 + j % 3 + 5);
				indices.push_back(i * 24 + j / 3 * 4 + j % 3);
				indices.push_back(i * 24 + j / 3 * 4 + j % 3 + 5);
				indices.push_back(i * 24 + j / 3 * 4 + j % 3 + 1);
			}
		}

		// Trapezium insetting quads
		// Top
		indices.push_back(i * 24 +  5);
		indices.push_back(i * 24 + 20);
		indices.push_back(i * 24 + 23);
		indices.push_back(i * 24 +  5);
		indices.push_back(i * 24 + 23);
		indices.push_back(i * 24 +  6);
		// Left
		indices.push_back(i * 24 +  5);
		indices.push_back(i * 24 +  9);
		indices.push_back(i * 24 + 21);
		indices.push_back(i * 24 +  5);
		indices.push_back(i * 24 + 21);
		indices.push_back(i * 24 + 20);
		// Bottom
		indices.push_back(i * 24 + 21);
		indices.push_back(i * 24 +  9);
		indices.push_back(i * 24 + 10);
		indices.push_back(i * 24 + 21);
		indices.push_back(i * 24 + 10);
		indices.push_back(i * 24 + 22);
		// Right
		indices.push_back(i * 24 + 23);
		indices.push_back(i * 24 + 22);
		indices.push_back(i * 24 + 10);
		indices.push_back(i * 24 + 23);
		indices.push_back(i * 24 + 10);
		indices.push_back(i * 24 +  6);
	}

	Shaders::button->use();

	mesh->setData(vertices, indices);
	mesh->draw();

	numButtonsDrawn += batchSize[batch];
	batchSize[batch] = 0;
}

void ButtonManager::updateProjectionMatrix() {
	mat4 projMat;
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	Shaders::button->setMat4("uProjection2D", projMat);
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
