#include "game/PlayScreen.h"

#include "opengl/Shader.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"


PlayScreen::PlayScreen(const LevelDescriptor& levelToPlay, const std::function<void()>& browseLevelsCallback)
	: game(levelToPlay) {
	levelCompleteDisplay = uiManager.addNode<UIButton>();
	levelCompleteDisplay->hide();
	levelCompleteDisplay->setOnTrigger([this] { levelCompleteDisplay->deactivate(); });

	auto levelCompleteButton = levelCompleteDisplay->addChild<UIButton>("Level complete!", Theme::SuccessButton);
	levelCompleteButton->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Absolute, .width = 280.f,
		.heightMode = SizingMode::Absolute, .height = 80.f,
	});
	levelCompleteButton->setOnTrigger([this] {
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
	restartButton->setOnTrigger([this] {
		levelCompleteDisplay->deactivate();
		game.start();
	});

	auto levelButton = uiManager.addNode(std::make_unique<UIButton>(levelToPlay.name, Theme::SecondaryOutline));
	levelButton->setLayout({
		.anchor = Anchor::BottomLeft,
		.widthMode = SizingMode::Absolute, .width = 100.f,
		.heightMode = SizingMode::Absolute, .height = 60.f,
		.margin = glm::vec2(20.f)
	});
	levelButton->setOnTrigger(browseLevelsCallback);

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

	uiManager.update(dt);
}

void PlayScreen::render() {
	game.render();
	uiManager.render();
}


void PlayScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	game.resize((float)width, (float)height);
}