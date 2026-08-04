#ifndef GAME_MODE_H
#define GAME_MODE_H

#include "AppMode.h"
#include "PlayScreen.h"
#include "LevelBrowserScreen.h"


class GameMode : public AppMode {
public:
	GameMode();

private:
	void browseLevels();
	void playLevel(const std::string& levelName);

	std::unique_ptr<PlayScreen> playScreen;
	std::unique_ptr<LevelBrowserScreen> levelBrowserScreen;
};


#endif // GAME_MODE_H
