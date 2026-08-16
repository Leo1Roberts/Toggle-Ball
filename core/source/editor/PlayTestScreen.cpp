#include "editor/PlayTestScreen.h"

#include "ui/Theme.h"
#include "ui/UIButton.h"


PlayTestScreen::PlayTestScreen(const LevelDescriptor& levelToPlay)
	: game(levelToPlay) {
	levelCompleteDisplay = uiManager.addNode<UIButton>();
	levelCompleteDisplay->hide();
	levelCompleteDisplay->setOnClick([this] { levelCompleteDisplay->deactivate(); });

	auto levelCompleteButton = levelCompleteDisplay->addChild<UIButton>("Level complete!", Theme::SuccessButton);
	levelCompleteButton->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Absolute, .width = 280.f,
		.heightMode = SizingMode::Absolute, .height = 80.f,
	});
	levelCompleteButton->setOnClick([this] {
		levelCompleteDisplay->deactivate();
		game.start();
	});

	levelCompleteDisplay->deactivate();


	auto restartButton = uiManager.addNode(std::make_unique<UIButton>("Restart", Theme::PrimaryButton));
	restartButton->setLayout({
		.anchor = Anchor::TopRight,
		.widthMode = SizingMode::Absolute, .width = 100.f,
		.heightMode = SizingMode::Absolute, .height = 60.f,
		.margin = glm::vec2(20.f)
	});
	restartButton->setOnClick([this] {
		levelCompleteDisplay->deactivate();
		game.start();
	});

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

	uiManager.update(dt);
}

void PlayTestScreen::render() {
	game.render();
	uiManager.render();
}


void PlayTestScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	game.resize((float)width, (float)height);
}