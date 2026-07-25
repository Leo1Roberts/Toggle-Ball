#ifndef ANDROID_WINDOW_H
#define ANDROID_WINDOW_H

#include "AbstractWindow.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>


class AndroidWindow : public AbstractWindow {
public:
	AndroidWindow(struct android_app* androidApp) : app(androidApp) {
        AndroidWindow::updateWindowConfiguration();
	}

	[[nodiscard]] bool isFullscreen() const override {
		return currentlyFullscreen;
	}

	void close() override {
		GameActivity_finish(app->activity);
	}

	void toggleFullscreen() override;

	void updateWindowConfiguration() override;

private:
	struct android_app* app;
	bool currentlyFullscreen = true;
};


#endif // ANDROID_WINDOW_H
