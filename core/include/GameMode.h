#ifndef GAME_MODE_H
#define GAME_MODE_H

#include "AppMode.h"
#include "PlayScreen.h"


class GameMode : public AppMode {
public:
	GameMode();

private:
	std::unique_ptr<PlayScreen> playScreen;
};


#endif // GAME_MODE_H
