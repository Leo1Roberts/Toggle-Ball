#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include "system/AbstractWindow.h"
#include "system/Cursor.h"

#include <array>
#include <GLFW/glfw3.h>


class GlfwWindow : public AbstractWindow {
public:
	explicit GlfwWindow(GLFWwindow* handle) : windowHandle(handle) {
		updateWindowConfiguration();
	}

	~GlfwWindow() override {
		for (auto c : cursors)
			glfwDestroyCursor(c);
	}

	[[nodiscard]] bool isFullscreen() const override { return currentlyFullscreen; }

	void close() override { glfwSetWindowShouldClose(windowHandle, GLFW_TRUE); }

	void toggleFullscreen() override;

	void updateWindowSize() override { glfwGetFramebufferSize(windowHandle, &config.width, &config.height); }
	void updateWindowDPIScale() override {
		glfwGetWindowContentScale(windowHandle, &config.dpiScale, nullptr);
		for (auto& c : cursors) {
			glfwDestroyCursor(c);
			c = nullptr;
		}
	}

	void setCursor(const Cursor& cursor) override;

	[[nodiscard]] glm::vec2 translateMousePosition(double x, double y) const;

private:
	GLFWwindow* windowHandle;
	bool currentlyFullscreen = false;

	// Saved window state to restore when exiting fullscreen
	int savedX = 0, savedY = 0;
	int savedWidth = 800, savedHeight = 600;

	std::array<GLFWcursor*, (int)Cursor::Style::COUNT> cursors{nullptr};
};


#endif // GLFW_WINDOW_H
