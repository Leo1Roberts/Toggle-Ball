#include "game/LevelBrowserScreen.h"

#include "utilities/AssetManager.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"
#include "ui/UIList.h"


LevelBrowserScreen::LevelBrowserScreen(const std::function<void(std::string)>& playLevelCallback) {
	auto list = uiManager.addNode(std::make_unique<UIVerticalList>(10.f));
	list->setLayout({ .padding = glm::vec2(60.f), });

	auto levels = AssetManager::getFileList("levels", ".lvl");
	for (const auto& levelName : levels) {
		auto button = list->addChild<UIButton>(levelName, Theme::PrimaryButton);
		button->setLayout({
			.widthMode = SizingMode::Stretch,
			.heightMode = SizingMode::Wrap,
			.padding = {0.f, 15.f}
		});

		button->setOnClick([=] { playLevelCallback(levelName); });
	}
}