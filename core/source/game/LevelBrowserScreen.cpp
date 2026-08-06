#include "game/LevelBrowserScreen.h"

#include "utilities/AssetManager.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"
#include "ui/UIVerticalList.h"


LevelBrowserScreen::LevelBrowserScreen(const std::function<void(std::string)>& playLevelCallback) {
	auto rootNode = std::make_unique<UINode>();
	auto list = rootNode->addChild(std::make_unique<UIVerticalList>(glm::vec2(60.f), 10.f));

	auto levels = AssetManager::getFileList("levels", ".lvl");
	for (const auto& levelName : levels) {
		auto button = std::make_unique<UIButton>(levelName, Theme::PrimaryButton);
		button->layout = {
			.anchor = Anchor::TopCentre,
			.widthMode = SizingMode::Relative, .heightMode = SizingMode::Absolute,
			.width = 1.f, .height = 50.f
		};

		button->setOnClick([=] { playLevelCallback(levelName); });

		list->addChild(std::move(button));
	}

	uiManager.setRootNode(std::move(rootNode));
}


void LevelBrowserScreen::processEvent(const Event& event) {
	uiManager.processEvent(event);
}

void LevelBrowserScreen::render() {
	uiManager.render();
}


void LevelBrowserScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
}