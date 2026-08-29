#ifndef SCREEN_H
#define SCREEN_H

#include "io/Event.h"
#include "system/Cursor.h"
#include "ui/UIManager.h"


class Screen : public ICursorProvider {
public:
	~Screen() override = default;

	virtual void processEvent(const Event& event) = 0;
	virtual void update(microseconds dt) = 0;
	virtual void render() = 0;

	void resize(int screenWidth, int screenHeight, float screenDPIScale) {
		width = screenWidth;
		height = screenHeight;
		aspectRatio = (float)width / (float)height;
		dpiScale = screenDPIScale;
		doResize();
	}

	[[nodiscard]] int getWidth() const { return width; }
	[[nodiscard]] int getHeight() const { return height; }
	[[nodiscard]] float getDPIScale() const { return dpiScale; }

	[[nodiscard]] std::optional<Cursor> queryCursor() const override { return uiManager.queryCursor(); }

protected:
	int width = 0, height = 0;
	float aspectRatio{};
	float dpiScale{};

	UIManager uiManager;

private:
	virtual void doResize() {}
};

#endif // SCREEN_H
