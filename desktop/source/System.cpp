#include "../../core/include/system/System.h"

#include <GLFW/glfw3.h>

namespace System {
	void setClipboardText(const std::string& text) {
		glfwSetClipboardString(nullptr, text.c_str());
	}

	std::string getClipboardText() {
		return glfwGetClipboardString(nullptr);
	}
}