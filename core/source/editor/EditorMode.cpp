#include "editor/EditorMode.h"

#include "level/Level.h"


void EditorMode::processEvent(const Event& event) {
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

bool EditorMode::requestQuit(const std::function<void()>& quitCallback) {
	return editorScreen ? editorScreen->requestQuit(quitCallback) : true;
}


void EditorMode::openEntryScreen() {
	editorScreen.reset();
	editorEntryScreen = std::make_unique<EditorEntryScreen>([this](const std::string& levelName) { startEditing(levelName); });
	resizeToMatchActiveScreen(editorEntryScreen.get());
	activeScreen = editorEntryScreen.get();
}

void EditorMode::startEditing(const std::string& levelName) {
	editorScreen = std::make_unique<EditorScreen>(LevelDescriptor::load(levelName), [this] { testLevel(); }, [this] { openEntryScreen(); });
	resizeToMatchActiveScreen(editorScreen.get());
	activeScreen = editorScreen.get();
}

void EditorMode::resumeEditing() {
	resizeToMatchActiveScreen(editorScreen.get());
	activeScreen = editorScreen.get();
}

void EditorMode::testLevel() {
	playTestScreen = std::make_unique<PlayTestScreen>(*editorScreen->getLevel(), [this] { resumeEditing(); });
	resizeToMatchActiveScreen(playTestScreen.get());
	activeScreen = playTestScreen.get();
}