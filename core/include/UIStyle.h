#ifndef UI_STYLE_H
#define UI_STYLE_H

#include "Colors.h"
#include "Font.h"


struct PanelStyle {
	col fillColor = Color::White;
	col strokeColor = Color::Black;
	float cornerRadius = 0.f;
	float strokeWidth = 0.f;
};


struct Font;
enum class TextAlignHorizontal { Left, Centre, Right };
enum class TextAlignVertical { Top, Middle, Bottom };

struct TextStyle {
	FontId font = FontId::Bahnschrift;
	float fontSize = 20.f;
	col color = Color::Black;

	TextAlignHorizontal alignHorizontal = TextAlignHorizontal::Left;
	TextAlignVertical alignVertical = TextAlignVertical::Top;
};

struct ButtonStyle {
	PanelStyle normalPanel;
	PanelStyle hoveredPanel;
	PanelStyle pressedPanel;
	PanelStyle disabledPanel;

	TextStyle normalText;
	TextStyle hoveredText;
	TextStyle pressedText;
	TextStyle disabledText;
};

struct TextBoxStyle {
	PanelStyle normalPanel;
	PanelStyle hoveredPanel;
	PanelStyle focusedPanel;
	PanelStyle disabledPanel;

	TextStyle normalText;
	TextStyle hoveredText;
	TextStyle focusedText;
	TextStyle highlightedText;
	TextStyle disabledText;

	col highlight = Color::SoftBlue;
};

#endif // UI_STYLE_H