#include "editor/PlayTestScreen.h"

#include "ui/Theme.h"
#include "ui/UIButton.h"


PlayTestScreen::PlayTestScreen(const LevelDescriptor& levelToPlay)
	: game(levelToPlay) {
	auto rootNode = std::make_unique<UINode>();

	auto levelCompleteBackground = std::make_unique<UIButton>();
	levelCompleteBackground->hide();
	levelCompleteDisplay = rootNode->addChild(std::move(levelCompleteBackground));
	levelCompleteDisplay->setOnClick([this] { levelCompleteDisplay->deactivate(); });

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
	rootNode->addChild(std::move(restartButton));
	
	uiManager.setRootNode(std::move(rootNode));

	game.start();
}


void PlayTestScreen::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	game.processEvent(event);
}

void PlayTestScreen::update(microseconds dt) {
	bool wasComplete = game.levelIsComplete();
	game.update(dt);
	if (!wasComplete && game.levelIsComplete())
		levelCompleteDisplay->activate();
}

void PlayTestScreen::render() {
	game.render();
	uiManager.render();
}


void PlayTestScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	game.resize(width, height);
}