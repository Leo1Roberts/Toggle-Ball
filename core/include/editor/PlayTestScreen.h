#ifndef PLAY_TEST_SCREEN_H
#define PLAY_TEST_SCREEN_H

#include "game/GameWorld.h"
#include "Screen.h"
#include "ui/UIManager.h"


class UIButton;
class PlayTestScreen : public Screen {
public:
	explicit PlayTestScreen(const LevelDescriptor& levelToPlay);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

private:
	void doResize() override;

	GameWorld game;
	UIManager uiManager{};

	UIButton* levelCompleteDisplay = nullptr;
};


#endif // PLAY_TEST_SCREEN_H
