#include "TextInputBuffer.h"

#include "Settings.h"

bool TextInputBuffer::Float(char c, const std::string& buffer) {
	if (std::isdigit(c)) return true;
	if (c == '-' && buffer.empty()) return true; // Only allow minus at the start
	if (c == '.' && buffer.find('.') == std::string::npos) return true; // Only one decimal point
	return false;
}


bool TextInputBuffer::processEvent(const Event& event) {
	if (auto* c = std::get_if<char>(&event)) {
		if (charIsValid(*c, buffer)) {
			eraseSelection();
			buffer.insert(buffer.begin() + cursorIndex, *c);
			moveCursorTo(cursorIndex + 1);
		}
		return true;
	}
	if (auto key = std::get_if<KeyEvent>(&event)) {
		if (key->chord.code == KeyCode::Backspace) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				if (selectionStartIndex != selectionEndIndex)
					eraseSelection();
				else if (cursorIndex > 0) {
					buffer.erase(buffer.begin() + (cursorIndex - 1), buffer.begin() + cursorIndex);
					moveCursorTo(cursorIndex - 1);
				} else if (key->action == KeyAction::Repeat)
					return false;
				return true;
			}
		}

		if (mode == TextInputMode::Rich) {
			if (auto actionCode = Settings::Bindings->translate(key->chord)) {
				switch (*actionCode) {
				case ActionCode::SelectAll:
					if (key->action == KeyAction::Down) {
						selectAll();
						return true;
					} return false;
				case ActionCode::DeselectAll:
					if (key->action == KeyAction::Down) {
						deselectAll();
						return true;
					} return false;
				default:
					return false;
				}
			}
			switch (key->chord.code) {
			case KeyCode::Delete:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (selectionStartIndex != selectionEndIndex)
						eraseSelection();
					else if (cursorIndex < buffer.size())
						buffer.erase(buffer.begin() + cursorIndex, buffer.begin() + (cursorIndex + 1));
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Left:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (selectionStartIndex != selectionEndIndex && !(key->chord.modifiers & MOD_SHIFT))
						moveCursorTo(selectionStartIndex, key->chord.modifiers & MOD_SHIFT);
					else if (cursorIndex > 0)
						moveCursorTo(cursorIndex - 1, key->chord.modifiers & MOD_SHIFT);
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Right:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (selectionStartIndex != selectionEndIndex && !(key->chord.modifiers & MOD_SHIFT))
						moveCursorTo(selectionEndIndex, key->chord.modifiers & MOD_SHIFT);
					else if (cursorIndex < buffer.size())
						moveCursorTo(cursorIndex + 1, key->chord.modifiers & MOD_SHIFT);
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Up:
			case KeyCode::Home:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex == 0 && key->action == KeyAction::Repeat)
						return false;
					moveCursorTo(0, key->chord.modifiers & MOD_SHIFT);
					return true;
				}
			case KeyCode::Down:
			case KeyCode::End:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (cursorIndex == buffer.length() && key->action == KeyAction::Repeat)
						return false;
					moveCursorTo(buffer.length(), key->chord.modifiers & MOD_SHIFT);
					return true;
				}
			default:;
			}
		}
	}
	return false;
}


void TextInputBuffer::moveCursorTo(int index, bool highlight) {
	if (highlight) {
		int anchor;
		if (cursorIndex == selectionStartIndex)
			anchor = selectionEndIndex;
		else if (cursorIndex == selectionEndIndex)
			anchor = selectionStartIndex;
		else
			anchor = selectionStartIndex = selectionEndIndex = cursorIndex;
		selectionStartIndex = std::min(anchor, index);
		selectionEndIndex = std::max(anchor, index);
	} else {
		selectionStartIndex = index;
		selectionEndIndex = index;
	}

	cursorIndex = index;
}