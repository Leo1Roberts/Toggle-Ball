#include "main.h"
#include "Colors.h"
#include "TextBoxManager.h"
#include "ButtonManager.h"
#include "Fonts.h"

const int MAX_TEXT_BOXES = 100;

int TextBoxManager::focusedBoxIndex = -1;
bool TextBoxManager::anythingChanged = false;
std::string TextBoxManager::text;
short TextBoxManager::cursorPos;
long TextBoxManager::start_ms;
bool TextBoxManager::selected;
bool TextBoxManager::updateCursorPos;
float TextBoxManager::pointerPosX;
TextBox TextBoxManager::textBoxes[MAX_TEXT_BOXES];
int TextBoxManager::numTextBoxes;

void TextBoxManager::addTextBox(float l, float r, float t, float b,
								const std::string& value,
								byte restriction,
								byte property,
								void (*onFocus)(),
								void (*onTyped)(byte),
                                bool stateful) {
	if (numTextBoxes == MAX_TEXT_BOXES) return;

	textBoxes[numTextBoxes] = {
			value,
			restriction,
			property,
			onFocus,
			onTyped
	};

	bool active = numTextBoxes == focusedBoxIndex;

	const ButtonStyle* style;
	if (active)
		style = stateful ? &BS_TEXT_BOX_STATEFUL_ACTIVE : &BS_TEXT_BOX_ACTIVE;
	else
		style = stateful ? &BS_TEXT_BOX_STATEFUL : &BS_TEXT_BOX;
	ButtonManager::addButton(l, r, t, b,
	                         focusTextBox,
	                         numTextBoxes,
	                         *style,
							 "",
							 false, false, !active);

	std::string textToUse = active ? text : value;

	float buttonHeight = t - b;
	float fontSize = buttonHeight * 0.8f;
	float marginY = (buttonHeight - fontSize) * 0.5f;
	float textWidth = Text::calculateWidth(textToUse, COURIER_NEW, fontSize);

	float textStartX;
	if (restriction == TR_FLOAT) {
		size_t dotIndex = textToUse.find('.');
		if (dotIndex == std::string::npos)
			dotIndex = textToUse.length();
		textStartX = r - Text::calculateWidth(textToUse.substr(0, dotIndex), COURIER_NEW, fontSize)
		             - Text::calculateWidth(".00", COURIER_NEW, fontSize) - COURIER_NEW.charSpacing
		             - TEXT_BOX_RIGHT_MARGIN;
	} else
		textStartX = 0.5f * (l + r) - 0.5f * textWidth;

	Text::addText(textStartX,
				  t - marginY,
				  textToUse,
				  COURIER_NEW,
				  fontSize,
				  active ? TEXT_ACTIVE : TEXT_INACTIVE);

	if (active) {
		if (selected)
			ButtonManager::addButton(textStartX, textStartX + textWidth, t - marginY, b + marginY,
									 nullptr, 0, BS_TEXT_HIGHLIGHT, "", false, true, false);

		if (updateCursorPos) {
			cursorPos = -1;
			for (short i = 0; i < (short)textToUse.length(); i++) {
				float xPos = textStartX;
				xPos += Text::calculateWidth(textToUse.substr(0, i + 1), COURIER_NEW, fontSize)
						- 0.5f * Text::calculateWidth(textToUse.substr(i, 1), COURIER_NEW, fontSize);
				if (pointerPosX < xPos) {
					cursorPos = i;
					break;
				}
			}
			if (cursorPos == -1)
				cursorPos = (short)textToUse.length();
			updateCursorPos = false;
		}

		if (((now_ms() - start_ms) / 500 % 2 || now_ms() - start_ms < 500)) {
			float xPos = textStartX;
			xPos += Text::calculateWidth(textToUse.substr(0, cursorPos), COURIER_NEW, fontSize);
			xPos -= 0.5f * Text::calculateWidth("|", COURIER_NEW, fontSize);
			Text::addText(xPos, t - marginY, "|", COURIER_NEW, fontSize, TEXT_ACTIVE);
		}
	}

	numTextBoxes++;
}

void TextBoxManager::focusTextBox(int index) {
	if (focusedBoxIndex == index) { // Move cursor
		updateCursorPos = true;
		selected = false;
	} else {
		focusedBoxIndex = index;
		textBoxes[index].onFocus();
		text = textBoxes[index].value;
		cursorPos = (short) text.length();
		selected = true;
	}
	start_ms = now_ms();
}
void TextBoxManager::focusTextBoxByProperty(byte property) {
	for (int i = 0; i < numTextBoxes; i++) {
		if (textBoxes[i].property == property) {
			focusTextBox(i);
			return;
		}
	}
}

void TextBoxManager::typed(char c) {
	typed(std::string(1, c));
}
void TextBoxManager::typed(const std::string& str) {
	bool anyValidChars = false;
	for (char c : str) {
		if ((textBoxes[focusedBoxIndex].restriction == TR_FLOAT && c != '.' && c != '-' && (c < '0' || c > '9')) ||
			(textBoxes[focusedBoxIndex].restriction == TR_FILE_NAME && c != ' ' && c != '-' && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9')))
			continue; // Invalid character

		anyValidChars = true;

		if (selected) {
			text = c;
			selected = false;
			cursorPos = 1;
		} else {
			text.insert(cursorPos, 1, c);
			cursorPos += 1;
		}
	}

	if (anyValidChars) {
		textBoxes[focusedBoxIndex].onTyped(textBoxes[focusedBoxIndex].property);
		start_ms = now_ms();
	}
}