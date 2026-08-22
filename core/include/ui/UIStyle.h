#ifndef UI_STYLE_H
#define UI_STYLE_H

#include "Color.h"
#include "Font.h"


struct PanelStyle {
	col fillColor = Color::White;
	col strokeColor = Color::Black;
	float cornerRadius = 0.f;
	float strokeWidth = 0.f;

	[[nodiscard]] static PanelStyle mix(const PanelStyle& x, const PanelStyle& y, float a) {
		return {
			glm::mix((glm::vec4)x.fillColor,    (glm::vec4)y.fillColor,    a),
			glm::mix((glm::vec4)x.strokeColor,  (glm::vec4)y.strokeColor,  a),
			glm::mix(           x.cornerRadius,            y.cornerRadius, a),
			glm::mix(           x.strokeWidth,             y.strokeWidth,  a),
		};
	}
};

struct LineStyle {
	col primaryColor = Color::Black;
	col secondaryColor = Color::White; // Only used if dashLength > 0.f
	float width = 1.f;
	float dashLength = 0.f;
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
	TextStyle disabledText;

	PanelStyle cursor;
	PanelStyle highlight;
};

struct ToggleStyle {
	PanelStyle normalTrackOff;
	PanelStyle hoveredTrackOff;
	PanelStyle disabledTrackOff;

	PanelStyle normalTrackOn;
	PanelStyle hoveredTrackOn;
	PanelStyle disabledTrackOn;

	PanelStyle handle;
};

struct SegmentedControlStyle {
	PanelStyle track;
	ButtonStyle selectedOption;
	ButtonStyle option;
};

struct DropDownListStyle {
	ButtonStyle mainButton;
	PanelStyle listBackground;
	ButtonStyle option;
	ButtonStyle selectedOption;
};

#endif // UI_STYLE_H