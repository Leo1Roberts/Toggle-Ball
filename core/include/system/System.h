#ifndef SYSTEM_H
#define SYSTEM_H

#include <optional>
#include <string>


namespace System {
	void setClipboardText(const std::string& text);
	[[nodiscard]] std::string getClipboardText();

	[[nodiscard]] std::optional<int> getCursorSize();
}

#endif // SYSTEM_H
