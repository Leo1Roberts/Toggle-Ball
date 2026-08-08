#include "game/PlayScreen.h"

#include "opengl/Shader.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"


PlayScreen::PlayScreen(const LevelDescriptor& levelToPlay, const std::function<void()>& browseLevelsCallback)
	: game(levelToPlay) {
	auto levelCompleteBackground = std::make_unique<UIButton>();
	levelCompleteBackground->hide();
	levelCompleteDisplay = uiManager.addNode(std::move(levelCompleteBackground));
	levelCompleteDisplay->setOnClick([this] {
		levelCompleteDisplay->deactivate();
	});

	auto levelCompleteButton = std::make_unique<UIButton>("Level complete!", Theme::SuccessButton);
	levelCompleteButton->layout = {
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 280.f, .height = 80.f,
	};
	levelCompleteButton->setOnClick([this] {
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
	restartButton->setOnClick([this] {
		levelCompleteDisplay->deactivate();
		game.start();
	});
	uiManager.addNode(std::move(restartButton));

	auto levelButton = std::make_unique<UIButton>(levelToPlay.name, Theme::SecondaryOutline);
	levelButton->layout = {
		.anchor = Anchor::BottomLeft,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 100.f, .height = 60.f,
		.offset = {20.f, -20.f}
	};
	levelButton->setOnClick(browseLevelsCallback);
	uiManager.addNode(std::move(levelButton));

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