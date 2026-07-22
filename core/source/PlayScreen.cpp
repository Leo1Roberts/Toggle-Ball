#include "PlayScreen.h"

#include "KeyBindings.h"
#include "Settings.h"
#include "Shader.h"
#include "Theme.h"
#include "UIButton.h"

#include <glm/glm.hpp>
#include <ranges>

const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
const glm::vec3 sunDirection = glm::normalize(glm::vec3(2, 2, 3));


PlayScreen::PlayScreen(const LevelDescriptor* levelToPlay, const std::function<void()>& browseLevelsCallback)
	: game(*levelToPlay) {
	auto rootNode = std::make_unique<UINode>();

	auto restartButton = std::make_unique<UIButton>("Restart", Theme::PrimaryButton);
	restartButton->layout = {
		.anchor = Anchor::TopRight,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 100.f, .height = 60.f,
		.offset = {-20.f, 20.f}
	};
	restartButton->setOnClick([&] { game.start(); });

	auto levelButton = std::make_unique<UIButton>(levelToPlay->getName(), Theme::SecondaryOutline);
	levelButton->layout = {
		.anchor = Anchor::BottomRight,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 100.f, .height = 60.f,
		.offset = {-20.f, -20.f}
	};
	levelButton->setOnClick(browseLevelsCallback);

	rootNode->addChild(std::move(restartButton));
	rootNode->addChild(std::move(levelButton));

	uiManager.setRootNode(std::move(rootNode));

	game.start();
}


void PlayScreen::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (game.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Down) {
			switch (pointer->button) {
			default:;
			}
		}
	}
}

void PlayScreen::update(microseconds dt) {
	game.update(dt);
	if (game.levelIsComplete()) { /* Do something */ }
}

void PlayScreen::render() {
	game.render();
	uiManager.render();
}


void PlayScreen::doResize(int width, int height, float dpiScale) {
	uiManager.resize(width, height, dpiScale);
	game.resize(width, height);
}