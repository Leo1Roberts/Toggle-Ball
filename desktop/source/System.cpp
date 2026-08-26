#include "system/System.h"

#include <GLFW/glfw3.h>
#if defined(PLATFORM_WINDOWS)
  #include <windows.h>
#endif

namespace System {
	void setClipboardText(const std::string& text) {
		glfwSetClipboardString(nullptr, text.c_str());
	}
	std::string getClipboardText() {
		return glfwGetClipboardString(nullptr);
	}


	std::optional<int> getCursorSize() {
#if defined(PLATFORM_WINDOWS)
		return GetSystemMetrics(SM_CXCURSOR);
#elif defined(PLATFORM_LINUX)
		const char* env_size = std::getenv("XCURSOR_SIZE");
		if (env_size && std::atoi(env_size) > 0)
			return std::atoi(env_size);
#endif
		return std::nullopt;
	}
}