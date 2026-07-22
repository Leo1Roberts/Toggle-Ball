#include "LevelBrowserScreen.h"

#include "AssetManager.h"
#include "Theme.h"
#include "UIButton.h"


LevelBrowserScreen::LevelBrowserScreen(const std::function<void(std::string)>& playLevelCallback) {
	auto rootNode = std::make_unique<UIPanel>(Theme::DarkCard);
	rootNode->layout = { .offset = {25.f, 20.f} };

	auto levels = AssetManager::getFileList("levels", ".lvl");
	float yOffset = 60.f;
	for (const auto& levelName : levels) {
		auto button = std::make_unique<UIButton>(levelName, Theme::PrimaryButton);
		button->layout = {
			.anchor = Anchor::TopCentre,
			.widthMode = SizingMode::Relative, .heightMode = SizingMode::Absolute,
			.width = 1.f, .height = 50.f,
			.offset = {60.f, yOffset},
		};

		button->setOnClick([=] { playLevelCallback(levelName); });

		rootNode->addChild(std::move(button));
		yOffset += 60.f;
	}

	uiManager.setRootNode(std::move(rootNode));
}


void LevelBrowserScreen::processEvent(const Event& event) {
	uiManager.processEvent(event);
}

void LevelBrowserScreen::render() {
	uiManager.render();
}


void LevelBrowserScreen::doResize(int screenWidth, int screenHeight, float screenDPIScale) {
	uiManager.resize(screenWidth, screenHeight, screenDPIScale);
}