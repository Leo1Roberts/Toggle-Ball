#ifndef LEVEL_BROWSER_SCREEN_H
#define LEVEL_BROWSER_SCREEN_H

#include "Screen.h"


class LevelBrowserScreen : public Screen {
public:
	explicit LevelBrowserScreen(const std::function<void(std::string)>& playLevelCallback);
};


#endif // LEVEL_BROWSER_SCREEN_H
