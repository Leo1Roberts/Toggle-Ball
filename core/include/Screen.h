#ifndef SCREEN_H
#define SCREEN_H

#include "Event.h"

class Screen {
public:
	virtual ~Screen() = default;

	void processEvent(const Event& event) { doProcessEvent(event); }

	void update(microseconds dt) { doUpdate(dt); }

	void draw() { doDraw(); }

	void resize(int screenWidth, int screenHeight, float screenDPIScale) {
		if (width == screenWidth && height == screenHeight && dpiScale == screenDPIScale)
			return;

		width = screenWidth;
		height = screenHeight;
		aspectRatio = (float)width / (float)height;
		dpiScale = screenDPIScale;
		doResize(width, height, dpiScale);
	}

	[[nodiscard]] bool isActive() const { return active; }
	void activate() { active = true; }
	void deactivate() { active = false; }

protected:
	int width = 0, height = 0;
	float aspectRatio{};
	float dpiScale{};

private:
	virtual void doProcessEvent(const Event& event) = 0;
	virtual void doUpdate(microseconds dt) = 0;
	virtual void doDraw() = 0;
	virtual void doResize(int screenWidth, int screenHeight, float screenDPIScale) {}

	bool active = false;
};

#endif // SCREEN_H
