#include "io/TextInputBuffer.h"

#include "Settings.h"
#include "system/System.h"

bool TextInputBuffer::Float(char c, int cursor, const std::string& buffer) {
	if (std::isdigit(c) && buffer.find('-', cursor) == std::string::npos) return true;
	if (c == '-' && cursor == 0 && buffer.find('-') == std::string::npos) return true;
	if (c == '.' && buffer.find('.') == std::string::npos && buffer.find('-', cursor) == std::string::npos) return true;
	return false;
}


bool TextInputBuffer::processEvent(const Event& event) {
	if (auto* c = std::get_if<char>(&event)) {
		if (charIsValid(*c, cursorIndex, buffer)) {
			eraseSelection();
			buffer.insert(buffer.begin() + cursorIndex, *c);
			moveCursorTo(cursorIndex + 1);
		}
		return true;
	}
	if (auto key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			switch (*actionCode) {
			case ActionCode::Copy:
				if (key->action == KeyAction::Down) {
					if (selectionStartIndex == selectionEndIndex)
						System::setClipboardText(buffer);
					else
						System::setClipboardText(buffer.substr(selectionStartIndex, selectionEndIndex - selectionStartIndex));
					return true;
				}
			case ActionCode::Paste:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					eraseSelection();
					for (char c : System::getClipboardText())
						if (charIsValid(c, cursorIndex, buffer)) {
							buffer.insert(buffer.begin() + cursorIndex, c);
							moveCursorTo(cursorIndex + 1);
						}
					return true;
				}
			default:;
			}
		}
		
		if (key->chord.code == KeyCode::Backspace) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				if (selectionStartIndex != selectionEndIndex)
					eraseSelection();
				else if (cursorIndex > 0) {
					moveCursorTo(key->chord.modifiers & MOD_CTRL ? getWordJumpCursorIndex(true) : cursorIndex - 1, true);
					eraseSelection();
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
					else if (cursorIndex < buffer.size()) {
						moveCursorTo(key->chord.modifiers & MOD_CTRL ? getWordJumpCursorIndex(false) : cursorIndex + 1, true);
						eraseSelection();
					} else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Left:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (selectionStartIndex != selectionEndIndex && !(key->chord.modifiers & MOD_SHIFT))
						moveCursorTo(selectionStartIndex, key->chord.modifiers & MOD_SHIFT);
					else if (cursorIndex > 0)
						moveCursorTo(key->chord.modifiers & MOD_CTRL ? getWordJumpCursorIndex(true) : cursorIndex - 1,
							key->chord.modifiers & MOD_SHIFT);
					else if (key->action == KeyAction::Repeat)
						return false;
					return true;
				}
			case KeyCode::Right:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (selectionStartIndex != selectionEndIndex && !(key->chord.modifiers & MOD_SHIFT))
						moveCursorTo(selectionEndIndex, key->chord.modifiers & MOD_SHIFT);
					else if (cursorIndex < buffer.size())
						moveCursorTo(key->chord.modifiers & MOD_CTRL ? getWordJumpCursorIndex(false) : cursorIndex + 1,
							key->chord.modifiers & MOD_SHIFT);
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
					moveCursorTo((int)buffer.length(), key->chord.modifiers & MOD_SHIFT);
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


TextInputBuffer::CharClass TextInputBuffer::getCharClass(char c) {
	if (std::isspace(c))
		return CharClass::Whitespace;
	if (std::isalnum(c) || c == '_')
		return CharClass::Word;

	return CharClass::Punctuation;
}

int TextInputBuffer::getWordJumpCursorIndex(bool left) const {
	int n = (int)buffer.length();
	int index = cursorIndex;

	if (left) { // Jump one word to the left
		if (index == 0) return 0;

		while (index > 0 && getCharClass(buffer[index - 1]) == CharClass::Whitespace)
			index--;

		if (index > 0) {
			CharClass targetClass = getCharClass(buffer[index - 1]);
			while (index > 0 && getCharClass(buffer[index - 1]) == targetClass)
				index--;
		}
	} else { // Jump one word to the right
		if (index == n) return n;

		CharClass startClass = getCharClass(buffer[index]);

		while (index < n && getCharClass(buffer[index]) == startClass)
			index++;

		if (startClass != CharClass::Whitespace) {
			while (index < n && getCharClass(buffer[index]) == CharClass::Whitespace)
				index++;
		}
	}

	return index;
}

void TextInputBuffer::selectWord(int charIndex) {
	if (buffer.empty()) return;

	auto targetClass = getCharClass(buffer[charIndex]);

	int start = charIndex;
	while (start > 0 && getCharClass(buffer[start - 1]) == targetClass)
		start--;

	int end = charIndex;
	while (end < buffer.length() && getCharClass(buffer[end]) == targetClass)
		end++;

	selectionStartIndex = start;
	cursorIndex = selectionEndIndex = end;
}