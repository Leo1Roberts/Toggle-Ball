#include "EditorMode.h"

EditorMode::EditorMode() {
	editorScreen = std::make_unique<EditorScreen>(LevelDescriptor::load("Level 1"));
	activeScreen = editorScreen.get();
}