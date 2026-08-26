#ifndef LEVEL_BROWSER_SCREEN_H
#define LEVEL_BROWSER_SCREEN_H

#include "Screen.h"


class LevelBrowserScreen : public Screen {
public:
	explicit LevelBrowserScreen(const std::function<void(std::string)>& playLevelCallback);

	void processEvent(const Event& event) override { uiManager.processEvent(event); }
	void update(microseconds dt) override { uiManager.update(dt); }
	void render() override { uiManager.render(); }

private:
	void doResize() override { uiManager.resize(width, height, dpiScale); }
};


#endif // LEVEL_BROWSER_SCREEN_H
