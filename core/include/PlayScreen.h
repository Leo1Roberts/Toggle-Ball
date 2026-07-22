#ifndef GAME_H
#define GAME_H

#include "GameWorld.h"
#include "Screen.h"
#include "UIManager.h"


class PlayScreen : public Screen {
public:
	PlayScreen() = default;
	explicit PlayScreen(const LevelDescriptor* levelToPlay);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

private:
	void doResize(int width, int height, float dpiScale) override;

	GameWorld game;
	UIManager uiManager{};
};


#endif // GAME_H