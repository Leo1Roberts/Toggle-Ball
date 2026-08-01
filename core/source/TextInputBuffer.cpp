#include "TextInputBuffer.h"

bool TextInputBuffer::Float(char c, const std::string& buffer) {
	if (std::isdigit(c)) return true;
	if (c == '-' && buffer.empty()) return true; // Only allow minus at the start
	if (c == '.' && buffer.find('.') == std::string::npos) return true; // Only one decimal point
	return false;
}


bool TextInputBuffer::processEvent(const Event& event) {
	if (auto* c = std::get_if<char>(&event)) {
		if (charIsValid(*c, buffer)) {
			buffer.insert(buffer.begin() + cursorIndex, *c);
			cursorIndex++;
		}
		return true;
	}
	if (auto key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
			if (key->chord.code == KeyCode::Backspace) {
				if (cursorIndex > 0) {
					buffer.erase(buffer.begin() + (cursorIndex - 1), buffer.begin() + cursorIndex);
					cursorIndex--;
				}
				return true;
			}
		}
	}
	return false;
}