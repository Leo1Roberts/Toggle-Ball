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

	void close() override { GameActivity_finish(app->activity); }

	void toggleFullscreen() override;

    void updateWindowSize() override {
        config.width = ANativeWindow_getWidth(app->window);
        config.height = ANativeWindow_getHeight(app->window);
    }
    void updateWindowDPIScale() override {
        int densityDpi = AConfiguration_getDensity(app->config);
        if (densityDpi != 0 && densityDpi != ACONFIGURATION_DENSITY_NONE)
            config.dpiScale = (float)densityDpi / (float)ACONFIGURATION_DENSITY_MEDIUM;
    }

private:
	struct android_app* app;
	bool currentlyFullscreen = true;
};


#endif // ANDROID_WINDOW_H
