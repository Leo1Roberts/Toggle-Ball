#include "PlayScreen.h"

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


PlayScreen::PlayScreen(const LevelDescriptor& levelToPlay, const std::function<void()>& browseLevelsCallback)
	: game(levelToPlay) {
	auto rootNode = std::make_unique<UINode>();

	auto levelCompleteBackground = std::make_unique<UIButton>();
	levelCompleteBackground->hide();
	levelCompleteDisplay = rootNode->addChild(std::move(levelCompleteBackground));
	levelCompleteDisplay->setOnClick([&] {
		// ReSharper disable once CppDFANullDereference
		levelCompleteDisplay->deactivate();
	});

	auto levelCompleteButton = std::make_unique<UIButton>("Level complete!", Theme::SuccessButton);
	levelCompleteButton->layout = {
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 280.f, .height = 80.f,
	};
	levelCompleteButton->setOnClick([&] {
		levelCompleteDisplay->deactivate();
		game.start();
	});

	levelCompleteDisplay->addChild(std::move(levelCompleteButton));
	levelCompleteDisplay->deactivate();

	auto restartButton = std::make_unique<UIButton>("Restart", Theme::PrimaryButton);
	restartButton->layout = {
		.anchor = Anchor::TopRight,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 100.f, .height = 60.f,
		.offset = {-20.f, 20.f}
	};
	restartButton->setOnClick([&] {
		levelCompleteDisplay->deactivate();
		game.start();
	});
	rootNode->addChild(std::move(restartButton));

	auto levelButton = std::make_unique<UIButton>(levelToPlay.getName(), Theme::SecondaryOutline);
	levelButton->layout = {
		.anchor = Anchor::BottomLeft,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 100.f, .height = 60.f,
		.offset = {20.f, -20.f}
	};
	levelButton->setOnClick(browseLevelsCallback);
	rootNode->addChild(std::move(levelButton));

	uiManager.setRootNode(std::move(rootNode));

	game.start();
}


void PlayScreen::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	game.processEvent(event);
}

void PlayScreen::update(microseconds dt) {
	bool wasComplete = game.levelIsComplete();
	game.update(dt);
	if (!wasComplete && game.levelIsComplete())
		levelCompleteDisplay->activate();
}

void PlayScreen::render() {
	game.render();
	uiManager.render();
}


void PlayScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	game.resize(width, height);
}