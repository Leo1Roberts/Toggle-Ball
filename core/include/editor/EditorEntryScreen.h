#ifndef EDITOR_ENTRY_SCREEN_H
#define EDITOR_ENTRY_SCREEN_H

#include "Screen.h"


class EditorEntryScreen : public Screen {
public:
	explicit EditorEntryScreen(const std::function<void(const std::string&)>& editLevelCallback);
};


#endif // EDITOR_ENTRY_SCREEN_H
