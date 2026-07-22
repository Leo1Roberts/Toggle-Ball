#include "FPSOverlay.h"
#include "UIStyle.h"
#include "UIText.h"

#include <format>

FPSOverlay::FPSOverlay() {
	hitTestable = false;

	TextStyle fpsStyle{
		.font = FontId::CourierNew,
		.fontSize = 20.f,
		.color = {0, 255, 128, 255},
		.alignHorizontal = TextAlignHorizontal::Left,
		.alignVertical = TextAlignVertical::Top
	};

	auto text = std::make_unique<UIText>("--- fps", fpsStyle);

	labelNode = addChild(std::move(text));
}


void FPSOverlay::update(microseconds dt) {
	timeAccumulator += dt;
	frameCount++;

	if (timeAccumulator >= 250000) {
		float seconds = toSeconds(timeAccumulator);
		int fps = (int)std::round((float)frameCount / seconds);

		std::string fpsString = std::format("{:3}", fps);
		labelNode->text = fpsString + " fps";

		timeAccumulator = 0;
		frameCount = 0;
	}
}