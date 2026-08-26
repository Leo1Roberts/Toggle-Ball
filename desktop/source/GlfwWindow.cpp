#include "GlfwWindow.h"

#include "system/System.h"

#include <glm/glm.hpp>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"


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


void GlfwWindow::setCursor(const Cursor& cursor) {
	// Cursor textures are double the size of the cursor itself, with the hotspot in the centre
	int size = 2 * System::getCursorSize().value_or((int)(24.f * config.dpiScale));

	if (size != cursorSize || activeCursor != cursor) {
		activeCursor = cursor;
		cursorSize = size;

		if (cursor.dynamic) {
			glfwSetInputMode(windowHandle, GLFW_CURSOR, cursor.captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_HIDDEN);
			if (!cursors[(int)Cursor::Style::Invisible]) {
				unsigned char emptyPixel[4] = {0, 0, 0, 0};
				GLFWimage emptyImage = { .width = 1, .height = 1, .pixels = emptyPixel };
				cursors[(int)Cursor::Style::Invisible] = glfwCreateCursor(&emptyImage, 0, 0);
			}
			glfwSetCursor(windowHandle, cursors[(int)Cursor::Style::Invisible]);
		} else {
			glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			if (!cursors[(int)cursor.style] || size != cursorSize) {
				GLFWcursor* glfwCursor;
#if defined(CUSTOM_CURSORS)
				auto texture = cursor.getTexture();
				std::vector<unsigned char> scaledPixels(size * size * 4);
				stbir_resize_uint8_srgb( // Assumes success
					 texture->getData(), texture->getWidth(), texture->getHeight(), 0,
					scaledPixels.data(), size               , size                , 0,
					STBIR_RGBA);

				GLFWimage image = {
					.width  = size,
					.height = size,
					.pixels = scaledPixels.data(),
				};
				glfwCursor = glfwCreateCursor(&image, image.width / 2, image.height / 2);
#else
				switch (cursor.style) {
				case Cursor::Style::Arrow:
					glfwCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
					break;
				case Cursor::Style::PointingHand:
					glfwCursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
					break;
				case Cursor::Style::Text:
					glfwCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
					break;
				case Cursor::Style::HorizontalResize:
					glfwCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
					break;
				case Cursor::Style::VerticalResize:
					glfwCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
					break;
				default:
					glfwCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
				}
#endif
				glfwDestroyCursor(cursors[(int)cursor.style]);
				cursors[(int)cursor.style] = glfwCursor;
			}
			glfwSetCursor(windowHandle, cursors[(int)cursor.style]);
		}
	}

	if (cursor.dynamic) {
		double x, y;
		glfwGetCursorPos(windowHandle, &x, &y);
		cursor.drawDynamic((float)size, translateMousePosition(x, y), {config.width, config.height});
	}
}


glm::vec2 GlfwWindow::translateMousePosition(double x, double y) const {
#if defined(PLATFORM_LINUX)
	return glm::vec2(x, y) * config.dpiScale;
#else
	return {x, y};
#endif
}