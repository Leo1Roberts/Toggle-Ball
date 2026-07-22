#include "GameMode.h"

GameMode::GameMode() {
	playScreen = std::make_unique<PlayScreen>(LevelDescriptor::load("Level 1").get());
	activeScreen = playScreen.get();
}
