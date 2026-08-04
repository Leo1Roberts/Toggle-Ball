#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include "../../core/include/system/AbstractWindow.h"

#include <GLFW/glfw3.h>


class GlfwWindow : public AbstractWindow {
public:
	explicit GlfwWindow(GLFWwindow* handle) : windowHandle(handle) {
		updateWindowConfiguration();
	}

	[[nodiscard]] bool isFullscreen() const override { return currentlyFullscreen; }

	void close() override { glfwSetWindowShouldClose(windowHandle, GLFW_TRUE); }

	void toggleFullscreen() override;

	void updateWindowSize() override { glfwGetFramebufferSize(windowHandle, &config.width, &config.height); }
	void updateWindowDPIScale() override { glfwGetWindowContentScale(windowHandle, &config.dpiScale, nullptr); }

private:
	GLFWwindow* windowHandle;
	bool currentlyFullscreen = false;

	// Saved window state to restore when exiting fullscreen
	int savedX = 0, savedY = 0;
	int savedWidth = 800, savedHeight = 600;
};


#endif // GLFW_WINDOW_H
