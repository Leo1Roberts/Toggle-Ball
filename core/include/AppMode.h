#ifndef APP_MODE_H
#define APP_MODE_H

#include "Screen.h"


class AppMode : public ICursorProvider {
public:
	~AppMode() override = default;

	virtual void tick(microseconds dt) {
		if (activeScreen) { activeScreen->update(dt); activeScreen->render(); }
	}
	virtual void resize(int windowWidth, int windowHeight, float windowDPI) {
		if (activeScreen) activeScreen->resize(windowWidth, windowHeight, windowDPI);
	}
	virtual void processEvent(const Event& event) {
		if (activeScreen) activeScreen->processEvent(event);
	}

	[[nodiscard]] std::optional<Cursor> queryCursor() const override {
		if (activeScreen)
			return activeScreen->queryCursor();
		return std::nullopt;
	}

protected:
	void resizeToMatchActiveScreen(Screen* screen) const {
		if (activeScreen)
			screen->resize(activeScreen->getWidth(), activeScreen->getHeight(), activeScreen->getDPIScale());
	}

	Screen* activeScreen = nullptr;
};


#endif // APP_MODE_H
