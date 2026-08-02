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
		if (key->chord.code == KeyCode::Backspace) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				if (cursorIndex > 0) {
					buffer.erase(buffer.begin() + (cursorIndex - 1), buffer.begin() + cursorIndex);
					cursorIndex--;
				} else if (key->action == KeyAction::Repeat)
					return false;
				return true;
			}
		}

		if (mode == TextInputMode::Rich) {
			switch (key->chord.code) {
			case KeyCode::Delete:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex < buffer.size())
						buffer.erase(buffer.begin() + cursorIndex, buffer.begin() + (cursorIndex + 1));
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Left:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex > 0)
						cursorIndex--;
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Right:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex < buffer.size())
						cursorIndex++;
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Up:
			case KeyCode::Home:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex > 0)
						cursorIndex = 0;
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Down:
			case KeyCode::End:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex < buffer.size())
						cursorIndex = buffer.size();
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			default:;
			}
		}
	}
	return false;
}