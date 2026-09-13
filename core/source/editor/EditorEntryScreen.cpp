#include "editor/EditorEntryScreen.h"

#include "level/Level.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"
#include "ui/UIList.h"
#include "utilities/AssetManager.h"

#include <ranges>


EditorEntryScreen::EditorEntryScreen(const std::function<void(const std::string&)>& editLevelCallback) {
	auto list = uiManager.addNode(std::make_unique<UIVerticalList>(10.f));
	list->setLayout({ .padding = glm::vec2(60.f) });

	auto addButton = list->addChild<UIButton>("+", Theme::SuccessButton);
	addButton->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Absolute, .width = Theme::SuccessButton.normalText.fontSize + 30.f,
		.heightMode = SizingMode::Wrap,
		.padding = {0.f, 15.f}
	});
	addButton->setOnTrigger([=] {
		auto newLevel = LevelDescriptor();
		newLevel.name = AssetManager::findAvailableFileName("levels", "New level", ".lvl");
		if (newLevel.save())
			editLevelCallback(newLevel.name);
	});

	auto levels = AssetManager::getFileList("levels", ".lvl");
	for (const auto& levelName : std::views::reverse(levels)) {
		auto item = list->addChild<UIHorizontalList>(10.f, 0.f);
		item->setLayout({ .heightMode = SizingMode::Wrap });

		auto mainButton = item->addChild<UIButton>(levelName, Theme::PrimaryButton);
		mainButton->setLayout({
			.widthMode  = SizingMode::Stretch,
			.heightMode = SizingMode::Wrap,
		});
		mainButton->setTextLayout({ .margin = glm::vec2(15.f) });
		mainButton->setOnTrigger([=] { editLevelCallback(levelName); });

		auto deleteButton = mainButton->addChild<UIButton>("X", Theme::NegativeButton);
		deleteButton->setLayout({
			.anchor = Anchor::CentreRight,
			.widthMode  = SizingMode::Absolute, .width = Theme::NegativeButton.normalText.fontSize + 20.f,
			.heightMode = SizingMode::Wrap,
			.padding = glm::vec2(10.f),
			.margin = glm::vec2(10.f)
		});
		deleteButton->setOnTrigger([this, levelName, item] {
			if (AssetManager::remove("levels/" + levelName + ".lvl"))
				uiManager.removeNode(item);
		});
	}
}