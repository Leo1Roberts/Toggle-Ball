#include "EditorMode.h"

#include "Level.h"

EditorMode::EditorMode() {
	editorScreen = std::make_unique<EditorScreen>(LevelDescriptor::load("Level 1"));
	activeScreen = editorScreen.get();
}


void EditorMode::processEvent(const Event& event) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::TestLevel:
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


void EditorMode::resumeEditing() {
	resizeToMatchActiveScreen(editorScreen.get());
	activeScreen = editorScreen.get();
}

void EditorMode::testLevel() {
	playTestScreen = std::make_unique<PlayTestScreen>(*editorScreen->getLevel());
	resizeToMatchActiveScreen(playTestScreen.get());
	activeScreen = playTestScreen.get();
}