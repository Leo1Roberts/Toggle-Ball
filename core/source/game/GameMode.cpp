#include "game/GameMode.h"

GameMode::GameMode() {
	browseLevels();
}


void GameMode::browseLevels() {
	levelBrowserScreen = std::make_unique<LevelBrowserScreen>(
		[&](const std::string& levelName) { this->playLevel(levelName); } );
	resizeToMatchActiveScreen(levelBrowserScreen.get());
	activeScreen = levelBrowserScreen.get();
}

void GameMode::playLevel(const std::string& levelName) {
	playScreen = std::make_unique<PlayScreen>(*LevelDescriptor::load(levelName),
		[&] { this->browseLevels(); });
	resizeToMatchActiveScreen(playScreen.get());
	activeScreen = playScreen.get();
}
