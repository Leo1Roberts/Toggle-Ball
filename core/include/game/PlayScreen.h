#ifndef GAME_H
#define GAME_H

#include "GameWorld.h"
#include "Screen.h"
#include "ui/UIManager.h"


class UIButton;
class PlayScreen : public Screen {
public:
	explicit PlayScreen(const LevelDescriptor& levelToPlay, const std::function<void()>& browseLevelsCallback);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

private:
	void doResize() override;

	GameWorld game;
	UIManager uiManager{};

	UIButton* levelCompleteDisplay = nullptr;
};


#endif // GAME_H