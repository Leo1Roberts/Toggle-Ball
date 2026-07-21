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
struct TextStyle {
	FontId font = FontId::Bahnschrift;
	float fontSize = 20.f;
	col color = Color::Black;
};


#endif // UI_STYLE_H