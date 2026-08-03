#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>


namespace System {
	void setClipboardText(const std::string& text);
	std::string getClipboardText();
}

#endif // SYSTEM_H
