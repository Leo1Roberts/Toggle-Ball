#include "ui/FPSOverlay.h"
#include "ui/UIStyle.h"
#include "ui/UIText.h"

#include <format>

FPSOverlay::FPSOverlay()
	: UIText("--- fps") {
	textStyle = {
		.font = FontId::CourierNew,
		.fontSize = 20.f,
		.color = {0, 255, 128, 255},
		.alignHorizontal = TextAlignHorizontal::Left,
		.alignVertical = TextAlignVertical::Top
	};
}


void FPSOverlay::doUpdate(microseconds dt) {
	timeAccumulator += dt;
	frameCount++;

	if (timeAccumulator >= 250000) {
		float seconds = toSeconds(timeAccumulator);
		int fps = (int)std::round((float)frameCount / seconds);

		std::string fpsString = std::format("{:3}", fps);
		setText(fpsString + " fps");

		timeAccumulator = 0;
		frameCount = 0;
	}
}