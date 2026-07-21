#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include "IWindow.h"

#include <GLFW/glfw3.h>


class GlfwWindow : public IWindow {
public:
	explicit GlfwWindow(GLFWwindow* handle) : windowHandle(handle) {}

	[[nodiscard]] bool isFullscreen() const override {
		return currentlyFullscreen;
	}

	void close() override {
		glfwSetWindowShouldClose(windowHandle, GLFW_TRUE);
	}

	void toggleFullscreen() override;

private:
	GLFWwindow* windowHandle;
	bool currentlyFullscreen = false;

	// Saved window state to restore when exiting fullscreen
	int savedX = 0, savedY = 0;
	int savedWidth = 800, savedHeight = 600;
};


#endif // GLFW_WINDOW_H
