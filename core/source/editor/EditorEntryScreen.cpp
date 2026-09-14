#include "editor/EditorEntryScreen.h"

#include "level/Level.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"
#include "ui/UIDialogue.h"
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
			requestDeleteLevel(levelName, item);
		});
	}
}


void EditorEntryScreen::requestDeleteLevel(const std::string& levelName, UINode* listItem) {
	auto dialogue = uiManager.addNode<UIDialogue>();

	auto closeDialogue = [this, dialogue] { uiManager.removeNode(dialogue); };
	auto deleteLevel = [this, levelName, listItem, dialogue] {
		if (AssetManager::remove("levels/" + levelName + ".lvl"))
			uiManager.removeNode(listItem);
		uiManager.removeNode(dialogue);
	};

	dialogue->setOnReturn(closeDialogue);
	dialogue->setOnConfirm(deleteLevel);
	dialogue->setLayout({
		.widthMode  = SizingMode::Absolute, .width = 400.f,
		.heightMode = SizingMode::Wrap,
		.padding = {15.f, 20.f}
	});

	auto content = dialogue->addChild<UIVerticalList>(20.f, 0.f);
	content->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
	});

	auto title = content->addChild<UIHorizontalList>(3.f, 0.f);
	title->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});
	auto title_p1 = title->addChild<UIText>("Delete ", TextStyle{
		.font = FontId::Bahnschrift,
		.fontSize = 32.f,
		.color = Color::LightGrey,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle
	});
	title_p1->setLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap
	});

	auto title_p2 = title->addChild<UIText>(levelName, TextStyle{
		.font = FontId::Bahnschrift,
		.fontSize = 32.f,
		.color = Color::White,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle
	});
	title_p2->setLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap
	});

	auto title_p3 = title->addChild<UIText>("?", TextStyle{
		.font = FontId::Bahnschrift,
		.fontSize = 32.f,
		.color = Color::LightGrey,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle
	});
	title_p3->setLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap
	});


	auto buttonsRow = content->addChild<UIHorizontalList>(0.f, 0.f);
	buttonsRow->setLayout({
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
	});

	auto cancel = buttonsRow->addChild<UIButton>("Cancel", Theme::SecondaryOutline);
	cancel->setLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(10.f)
	});
	cancel->setTextLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});
	cancel->setOnTrigger(closeDialogue);

	buttonsRow->addChild<UIContainer>(); // Spacer

	auto deleteButton = buttonsRow->addChild<UIButton>("Delete", Theme::NegativeButton);
	deleteButton->setLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(10.f)
	});
	deleteButton->setTextLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});
	deleteButton->setOnTrigger(deleteLevel);

	uiManager.changeFocus(dialogue, true);
}