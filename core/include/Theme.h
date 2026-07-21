#ifndef THEME_H
#define THEME_H

#include "UIStyle.h"


namespace Theme {
	inline constexpr PanelStyle LightCard {
		.fillColor = Color::GreyT,
		.strokeColor = Color::Black,
		.cornerRadius = 12.0f,
		.strokeWidth = 1.5f
	};

	inline constexpr TextStyle Body {
		.font = FontId::Bahnschrift,
		.fontSize = 20.f,
		.color = Color::Black
	};

	inline constexpr TextStyle TechStats {
		.font = FontId::CourierNew,
		.fontSize = 24.f,
		.color = Color::Green
	};
}


#endif // THEME_H
