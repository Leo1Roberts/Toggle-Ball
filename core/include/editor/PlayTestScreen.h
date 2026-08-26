#ifndef PLAY_TEST_SCREEN_H
#define PLAY_TEST_SCREEN_H

#include "game/GameWorld.h"
#include "Screen.h"


class UIButton;
class PlayTestScreen : public Screen {
public:
	explicit PlayTestScreen(const LevelDescriptor& levelToPlay, const std::function<void()>& editLevelCallback);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

private:
	void doResize() override;

	GameWorld game;

	UIButton* levelCompleteDisplay = nullptr;
};


#endif // PLAY_TEST_SCREEN_H
