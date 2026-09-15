#include "editor/EditorMode.h"

#include "level/Level.h"
#include "ui/Theme.h"
#include "ui/UIButton.h"
#include "ui/UIDialogue.h"
#include "ui/UIList.h"


EditorMode::EditorMode(const std::function<void()>& quitCallback) :
	quitCallback(quitCallback), confirmQuitDialogue(uiManager.addNode<UIDialogue>()) {
	auto closeDialogue = [this] {
		confirmQuitDialogue->deactivate();
	};

	confirmQuitDialogue->setOnReturn(closeDialogue);
	confirmQuitDialogue->setOnConfirm(closeDialogue);
	confirmQuitDialogue->setLayout({
		.widthMode  = SizingMode::Absolute, .width = 400.f,
		.heightMode = SizingMode::Wrap,
		.padding = {15.f, 20.f}
	});

	auto content = confirmQuitDialogue->addChild<UIVerticalList>(20.f, 0.f);
	content->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
	});

	auto title = content->addChild<UIText>("There are unsaved changes", TextStyle{
		.font = FontId::Bahnschrift,
		.fontSize = 32.f,
		.color = Color::LightGrey,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle
	});
	title->setLayout({ .heightMode = SizingMode::Wrap });

	auto buttonsRow = content->addChild<UIHorizontalList>(0.f, 0.f);
	buttonsRow->setLayout({
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
	});

	auto quit = buttonsRow->addChild<UIButton>("Quit anyway", Theme::SecondaryOutline);
	quit->setLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(10.f)
	});
	quit->setTextLayout({
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});
	quit->setOnTrigger(quitCallback);

	buttonsRow->addChild<UIContainer>(); // Spacer

	auto cancel = buttonsRow->addChild<UIButton>("Cancel", Theme::PrimaryButton);
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

	confirmQuitDialogue->deactivate();


	openEntryScreen();
}


void EditorMode::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::TestOrEditLevel:
					if (activeScreen == editorScreen.get())
						testLevel();
					else if (activeScreen == playTestScreen.get())
						resumeEditing();
				default:;
				}
			}
		}
	}

	activeScreen->processEvent(event);
}

bool EditorMode::requestQuit() {
	if (editorScreen) {
		if (editorScreen->canQuit())
			return true;

		confirmQuitDialogue->activate();
		uiManager.changeFocus(confirmQuitDialogue, true);
		return false;
	}

	return true;
}


std::optional<Cursor> EditorMode::queryCursor() const {
	if (auto c = uiManager.queryCursor())
		return c;
	return AppMode::queryCursor();
}


void EditorMode::tick(microseconds dt) {
	if (scheduledScreenChange) {
		scheduledScreenChange();
		scheduledScreenChange = nullptr;
	}

	if (activeScreen) {
		activeScreen->update(dt);
		activeScreen->render();
	}

	uiManager.update(dt);
	uiManager.render();
}


void EditorMode::resize(int windowWidth, int windowHeight, float windowDPI) {
	uiManager.resize(windowWidth, windowHeight, windowDPI);
	AppMode::resize(windowWidth, windowHeight, windowDPI);
}


void EditorMode::openEntryScreen() {
	editorEntryScreen = std::make_unique<EditorEntryScreen>(
		[this](const std::string& levelName) { 	scheduledScreenChange = [this, levelName] { startEditing(levelName); }; });
	resizeToMatchActiveScreen(editorEntryScreen.get());
	editorScreen.reset();
	activeScreen = editorEntryScreen.get();
}

void EditorMode::startEditing(const std::string& levelName) {
	editorScreen = std::make_unique<EditorScreen>(LevelDescriptor::load(levelName),
		[this] { scheduledScreenChange = [this] { testLevel(); }; },
		[this] { scheduledScreenChange = [this] { openEntryScreen(); }; });
	resizeToMatchActiveScreen(editorScreen.get());
	activeScreen = editorScreen.get();
}

void EditorMode::resumeEditing() {
	resizeToMatchActiveScreen(editorScreen.get());
	playTestScreen.reset();
	activeScreen = editorScreen.get();
}

void EditorMode::testLevel() {
	playTestScreen = std::make_unique<PlayTestScreen>(*editorScreen->getLevel(),
		[this] { scheduledScreenChange = [this] { resumeEditing(); }; });
	resizeToMatchActiveScreen(playTestScreen.get());
	activeScreen = playTestScreen.get();
}