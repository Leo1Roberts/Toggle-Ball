#ifndef COLORS_H
#define COLORS_H

#include "main.h"

struct col {
	byte r, g, b, a;

	col() : r(0), g(0), b(0), a(0) {}

	col(byte r, byte g, byte b) : r(r), g(g), b(b), a(255) {}

	col(byte r, byte g, byte b, byte a) : r(r), g(g), b(b), a(a) {}

	bool operator==(const col& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; };

	col operator*(float f) const { return col((byte)((float)r * f), (byte)((float)g * f), (byte)((float)b * f), a); }

	vec3 toVec3() const { return {(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f}; }
	vec4 toVec4() const { return {(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f}; }

	static col fromVec(const vec4& v) { return col((byte)(v.r * 255.0f), (byte)(v.g * 255.0f), (byte)(v.b * 255.0f), (byte)(v.a * 255.0f)); }
};

inline col lerp(const col& a, const col& b, float t) { return col::fromVec(lerp(a.toVec4(), b.toVec4(), t)); }

const col INVISIBLE = col(0, 0, 0, 120);

const col BLACK = col(0, 0, 0);
const col WHITE = col(255, 255, 255);
const col RED = col(255, 0, 0);
const col GREEN = col(0, 255, 0);
const col BLUE = col(0, 0, 255);
const col CYAN = col(0, 255, 255);
const col YELLOW = col(255, 255, 0);
const col MAGENTA = col(255, 0, 255);

const col DARK_GREY = col(60, 60, 60);

const col SOFT_RED = col(240, 10, 10);
const col SOFT_GREEN = col(10, 240, 10);
const col SOFT_BLUE = col(10, 10, 240);
const col SOFT_CYAN = col(10, 240, 240);
const col SOFT_MAGENTA = col(240, 10, 240);
const col SOFT_YELLOW = col(240, 240, 10);

const col BOUNDARY = col(177, 220, 237);

const col SELECTED = col(255, 127, 0);
const vec4 SELECTED_VEC4 = SELECTED.toVec4();
const col SELECT_BOX = col(255, 127, 0, 63);
const col FOCUSED = col(255, 191, 0);
const vec4 FOCUSED_VEC4 = FOCUSED.toVec4();
const col WARNING = col(224, 0, 44);
const vec4 WARNING_VEC4 = WARNING.toVec4();

const col OB_BLOB_TERMINAL = col(0, 100, 255);
const col OB_BLOB_TERMINAL_HOVER = col(0, 90, 230);
const col OB_BLOB_TERMINAL_PRESSED = col(0, 70, 179);
const col OB_BLOB_CENTRAL = col(100, 0, 255);
const col OB_BLOB_CENTRAL_HOVER = col(90, 0, 230);
const col OB_BLOB_CENTRAL_PRESSED = col(70, 0, 179);

const col GREY_T = col(200, 200, 200, 180);
const col GREY_T_HOVER = col(180, 180, 180, 180);
const col GREY_T_PRESSED = col(160, 160, 160, 180);

const col GREEN_T = col(0, 255, 0, 180);
const col GREEN_T_HOVER = col(0, 230, 0, 180);
const col GREEN_T_PRESSED = col(0, 205, 0, 180);

const col RED_T = col(255, 0, 0, 180);
const col RED_T_HOVER = col(230, 0, 0, 180);
const col RED_T_PRESSED = col(205, 0, 0, 180);

const col TEXT_ACTIVE = WHITE;
const col TEXT_INACTIVE = col(210, 210, 210);
const col TEXT_BOX = col(50, 50, 50);
const col TEXT_BOX_HOVER = col(40, 40, 40);
const col TEXT_BOX_ACTIVE = col(30, 30, 30);
const col TEXT_HIGHLIGHT = col(33, 66, 131);

const col STATE_A = col(204, 22, 22);
const col STATE_B = col(22, 95, 204);
extern col STATE;
extern col STATE_HOVER;
extern col STATE_ACTIVE;
extern col STATE_INSTANT;

const col TOGGLE_BLOB = col(220, 220, 220);

#endif // COLORS_H
