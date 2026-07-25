#ifndef APP_MODE_H
#define APP_MODE_H

#include "Screen.h"
#include "main.h"


class Screen;

class AppMode {
public:
	virtual ~AppMode() = default;

	virtual void tick(microseconds dt) {
		if (activeScreen) { activeScreen->update(dt); activeScreen->render(); }
	}
	virtual void resize(int windowWidth, int windowHeight, float windowDPI) {
		if (activeScreen) activeScreen->resize(windowWidth, windowHeight, windowDPI);
	}
	virtual void processEvent(const Event& event) {
		if (activeScreen) activeScreen->processEvent(event);
	}

protected:
	void resizeToMatchActiveScreen(Screen* screen) const {
		if (activeScreen)
			screen->resize(activeScreen->getWidth(), activeScreen->getHeight(), activeScreen->getDPIScale());
	}

	Screen* activeScreen = nullptr;
};


#endif // APP_MODE_H
