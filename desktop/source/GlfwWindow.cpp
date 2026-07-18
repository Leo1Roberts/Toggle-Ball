#include "GlfwWindow.h"

void GlfwWindow::toggleFullscreen() {
	if (currentlyFullscreen) {
		glfwSetWindowMonitor(windowHandle, nullptr,
							 savedX, savedY, savedWidth, savedHeight,
							 GLFW_DONT_CARE);
		currentlyFullscreen = false;
	} else {
		glfwGetWindowPos(windowHandle, &savedX, &savedY);
		glfwGetWindowSize(windowHandle, &savedWidth, &savedHeight);

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(windowHandle, monitor,
							 0, 0, mode->width, mode->height,
							 mode->refreshRate);
		currentlyFullscreen = true;
	}
}