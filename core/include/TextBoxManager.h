#ifndef TEXT_BOX_MANAGER_H
#define TEXT_BOX_MANAGER_H


enum { // Text restrictions
	TR_NONE,
	TR_FLOAT,
	TR_FILE_NAME,
	TR_NUM
};

enum { // Property being modified
	FLOAT_PROPERTY_START,

	PROPERTY_ARENA_WIDTH,
	PROPERTY_ARENA_HEIGHT,
	PROPERTY_TRANSITION_TIME,

	PROPERTY_POS_X,
	PROPERTY_POS_Y,
	PROPERTY_ANGLE,
	PROPERTY_MINOR_RADIUS,

	PROPERTY_OPM,
	PROPERTY_RPM,

	FLOAT_PROPERTY_END,



	FILE_NAME_PROPERTY_START,

	PROPERTY_LEVEL_NAME,

	FILE_NAME_PROPERTY_END,

	PROPERTY_NUM
};

struct TextBox {
	std::string value;
	byte restriction;
	byte property;
	void (* onFocus)();
	void (* onTyped)(byte property);
};

struct TextBoxManager {
	static int focusedBoxIndex;
	static bool anythingChanged;
	static std::string text;
	static short cursorPos;
	static long start_ms;
	static bool selected;
	static bool updateCursorPos;
	static float pointerPosX;

	static void clearTextBoxes() { // MUST be called each frame before adding text boxes
		numTextBoxes = 0;
	}

	static void addTextBox(float l, float r, float t, float b,
	                       const std::string&,
						   byte restriction,
						   byte property,
						   void (* onFocus)(),
						   void (* onTyped)(byte property),
	                       bool stateful = false);

	static void focusTextBox(int index);
	static void focusTextBoxByProperty(byte property);

	static void typed(char c);
	static void typed(const std::string&);

	inline static byte getCurrentProperty() {
		if (focusedBoxIndex == -1) return PROPERTY_NUM;
		return textBoxes[focusedBoxIndex].property;
	}
	inline static std::string getCurrentValue() { return textBoxes[focusedBoxIndex].value; }

private:
	static TextBox textBoxes[];
	static int numTextBoxes;
};


#endif // TEXT_BOX_MANAGER_H
