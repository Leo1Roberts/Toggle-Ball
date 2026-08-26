#ifndef ABSTRACT_WINDOW_H
#define ABSTRACT_WINDOW_H

#include "Cursor.h"


struct WindowConfiguration {
	int width = 0.f;
	int height = 0.f;
	float dpiScale = 1.f;
};

class AbstractWindow {
public:
	virtual ~AbstractWindow() = default;

	virtual void toggleFullscreen() = 0;
	[[nodiscard]] virtual bool isFullscreen() const = 0;
	virtual void close() = 0;

	virtual void updateWindowSize() = 0;
	virtual void updateWindowDPIScale() = 0;
	void updateWindowConfiguration() {
		updateWindowSize();
		updateWindowDPIScale();
	}

	virtual void setCursor(const Cursor& cursor = {}) {}

	WindowConfiguration config{};
	Cursor activeCursor{.style = Cursor::Style::COUNT}; // Make sure default cursor is generated
	int cursorSize;
};


#endif // ABSTRACT_WINDOW_H
