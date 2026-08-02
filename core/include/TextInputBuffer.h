#ifndef TEXT_INPUT_BUFFER_H
#define TEXT_INPUT_BUFFER_H

#include "Event.h"

enum class TextInputMode { Simple, Rich };


class TextInputBuffer {
public:
	using Validator = std::function<bool(char, const std::string&)>;
	static bool Float(char c, const std::string& buffer = "");

	explicit TextInputBuffer(const Validator& validator, TextInputMode mode = TextInputMode::Rich)
		: charIsValid(validator), mode(mode) {}

	bool processEvent(const Event& event);

	void clear() { buffer.clear(); }

	template <typename T>
	T getValue() const;

	[[nodiscard]] int getCursorIndex() const { return cursorIndex; }

private:
	Validator charIsValid;
	TextInputMode mode;
	std::string buffer;
	int cursorIndex = 0;
};

template <>
inline std::string TextInputBuffer::getValue<std::string>() const { return buffer; }
template <>
inline float TextInputBuffer::getValue<float>() const {
	try { return std::stof(buffer); }
	catch (...) { return 0.f; }
}


#endif // TEXT_INPUT_BUFFER_H
