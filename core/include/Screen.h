#ifndef SCREEN_H
#define SCREEN_H

#include "Event.h"
#include "Framebuffer.h"

class Screen {
public:
	Screen() : framebuffer() {}
	virtual ~Screen() = default;

	void processEvent(const Event& event) { doProcessEvent(event); }

	void update(microseconds dt) { doUpdate(dt); }

	void draw() {
		framebuffer.bind();
		glScissor(0, 0, getScreenWidth(), getScreenHeight());
		doDraw();
	}

	void resize(int width, int height, float dpiScale) {
		if (framebuffer.resize(width, height)) {
			aspectRatio = (float) width / (float) height;
			doResize(width, height, dpiScale);
		}
	}

	[[nodiscard]] int getScreenWidth() const { return framebuffer.getWidth(); }
	[[nodiscard]] int getScreenHeight() const { return framebuffer.getHeight(); }
	[[nodiscard]] const GLTexture& getTexture() const { return framebuffer.getTexture(); }

	Framebuffer framebuffer;
	float aspectRatio{};

private:
	virtual void doProcessEvent(const Event& event) = 0;
	virtual void doUpdate(microseconds dt) = 0;
	virtual void doDraw() = 0;
	virtual void doResize(int width, int height, float dpiScale) {}
};

#endif // SCREEN_H
