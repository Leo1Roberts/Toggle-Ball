#ifndef LEVEL_BROWSER_SCREEN_H
#define LEVEL_BROWSER_SCREEN_H

#include "Screen.h"
#include "UIManager.h"


class LevelBrowserScreen : public Screen {
public:
	explicit LevelBrowserScreen(const std::function<void(std::string)>& playLevelCallback);

	void processEvent(const Event& event) override;
	void render() override;

private:
	void doResize(int screenWidth, int screenHeight, float screenDPIScale) override;

	UIManager uiManager{};
};


#endif // LEVEL_BROWSER_SCREEN_H
