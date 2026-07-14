#ifndef COLORS_H
#define COLORS_H

#include "main.h"

struct col {
	byte r, g, b, a;

	constexpr col() : r(0), g(0), b(0), a(0) {}

	constexpr col(byte r, byte g, byte b) : r(r), g(g), b(b), a(255) {}

	constexpr col(byte r, byte g, byte b, byte a) : r(r), g(g), b(b), a(a) {}

	bool operator==(const col& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; };

	col operator*(float f) const { return {(byte)((float)r * f), (byte)((float)g * f), (byte)((float)b * f), a}; }

	[[nodiscard]] constexpr vec3 toVec3() const { return {(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f}; }
	[[nodiscard]] constexpr vec4 toVec4() const { return {(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f}; }

	static col fromVec(const vec4& v) { return {(byte)(v.r * 255.0f), (byte)(v.g * 255.0f), (byte)(v.b * 255.0f), (byte)(v.a * 255.0f)}; }
};

inline col lerp(const col& a, const col& b, float t) { return col::fromVec(lerp(a.toVec4(), b.toVec4(), t)); }

constexpr col INVISIBLE = col(0, 0, 0, 120);

constexpr col BLACK = col(0, 0, 0);
constexpr col WHITE = col(255, 255, 255);
constexpr col RED = col(255, 0, 0);
constexpr col GREEN = col(0, 255, 0);
constexpr col BLUE = col(0, 0, 255);
constexpr col CYAN = col(0, 255, 255);
constexpr col YELLOW = col(255, 255, 0);
constexpr col MAGENTA = col(255, 0, 255);

constexpr col DARK_GREY = col(60, 60, 60);

constexpr col SOFT_RED = col(240, 10, 10);
constexpr col SOFT_GREEN = col(10, 240, 10);
constexpr col SOFT_BLUE = col(10, 10, 240);
constexpr col SOFT_CYAN = col(10, 240, 240);
constexpr col SOFT_MAGENTA = col(240, 10, 240);
constexpr col SOFT_YELLOW = col(240, 240, 10);

constexpr col BOUNDARY = col(177, 220, 237);

constexpr col SELECTED = col(255, 127, 0);
constexpr vec4 SELECTED_VEC4 = SELECTED.toVec4();
constexpr col SELECT_BOX = col(255, 127, 0, 63);
constexpr col FOCUSED = col(255, 191, 0);
constexpr vec4 FOCUSED_VEC4 = FOCUSED.toVec4();
constexpr col WARNING = col(224, 0, 44);
constexpr vec4 WARNING_VEC4 = WARNING.toVec4();

constexpr col OB_BLOB_TERMINAL = col(0, 100, 255);
constexpr col OB_BLOB_TERMINAL_HOVER = col(0, 90, 230);
constexpr col OB_BLOB_TERMINAL_PRESSED = col(0, 70, 179);
constexpr col OB_BLOB_CENTRAL = col(100, 0, 255);
constexpr col OB_BLOB_CENTRAL_HOVER = col(90, 0, 230);
constexpr col OB_BLOB_CENTRAL_PRESSED = col(70, 0, 179);

constexpr col GREY_T = col(200, 200, 200, 180);
constexpr col GREY_T_HOVER = col(180, 180, 180, 180);
constexpr col GREY_T_PRESSED = col(160, 160, 160, 180);

constexpr col GREEN_T = col(0, 255, 0, 180);
constexpr col GREEN_T_HOVER = col(0, 230, 0, 180);
constexpr col GREEN_T_PRESSED = col(0, 205, 0, 180);

constexpr col RED_T = col(255, 0, 0, 180);
constexpr col RED_T_HOVER = col(230, 0, 0, 180);
constexpr col RED_T_PRESSED = col(205, 0, 0, 180);

constexpr col TEXT_ACTIVE = WHITE;
constexpr col TEXT_INACTIVE = col(210, 210, 210);
constexpr col TEXT_BOX = col(50, 50, 50);
constexpr col TEXT_BOX_HOVER = col(40, 40, 40);
constexpr col TEXT_BOX_ACTIVE = col(30, 30, 30);
constexpr col TEXT_HIGHLIGHT = col(33, 66, 131);

constexpr col STATE_A = col(204, 22, 22);
constexpr col STATE_B = col(22, 95, 204);
extern col STATE;
extern col STATE_HOVER;
extern col STATE_ACTIVE;
extern col STATE_INSTANT;

constexpr col TOGGLE_BLOB = col(220, 220, 220);

#endif // COLORS_H
