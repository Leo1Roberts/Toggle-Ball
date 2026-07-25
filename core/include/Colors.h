#ifndef COLORS_H
#define COLORS_H

#include "main.h"

#include <glm/glm.hpp>

struct col {
	byte r = 0, g = 0, b = 0, a = 0;

	constexpr col() = default;
	constexpr col(byte r, byte g, byte b, byte a = 255)
		: r(r), g(g), b(b), a(a) {}
	constexpr col(const glm::vec3& v)
		: r((byte)(v.r * 255.f)), g((byte)(v.g * 255.f)), b((byte)(v.b * 255.f)), a(255) {}
	constexpr col(const glm::vec4& v)
		: r((byte)(v.r * 255.f)), g((byte)(v.g * 255.f)), b((byte)(v.b * 255.f)), a((byte)(v.a * 255.f)) {}

	operator glm::vec3() const { return glm::vec3(r, g, b) / 255.f; }
	operator glm::vec4() const { return glm::vec4(r, g, b, a) / 255.f; }

	bool operator==(const col&) const = default;
};

namespace Color {
	constexpr col Black = col(0, 0, 0);
	constexpr col White = col(255, 255, 255);

	constexpr col SoftRed = col(240, 10, 10);
	constexpr col SoftGreen = col(10, 240, 10);
	constexpr col SoftBlue = col(10, 10, 240);
	constexpr col SoftCyan = col(10, 240, 240);
	constexpr col SoftMagenta = col(240, 10, 240);
	constexpr col SoftYellow = col(240, 240, 10);

	constexpr col Boundary = col(177, 220, 237);

	constexpr col Selected = col(255, 127, 0);
	const glm::vec4 SelectedVec4 = Selected;
	constexpr col SelectBox = col(255, 127, 0, 63);
	constexpr col Focused = col(255, 191, 0);
	const glm::vec4 FocusedVec4 = Focused;
	constexpr col Warning = col(224, 0, 44);
	const glm::vec4 WarningVec4 = Warning;

	constexpr col ObBlobTerminal = col(0, 100, 255);
	constexpr col ObBlobTerminalHover = col(0, 90, 230);
	constexpr col ObBlobTerminalPressed = col(0, 70, 179);
	constexpr col ObBlobCentral = col(100, 0, 255);
	constexpr col ObBlobCentralHover = col(90, 0, 230);
	constexpr col ObBlobCentralPressed = col(70, 0, 179);

	constexpr col StateA = col(204, 22, 22);
	constexpr col StateB = col(22, 95, 204);
	extern col State;
	extern col StateHover;
	extern col StateActive;
	extern col StateInstant;

	constexpr col ToggleBlob = col(220, 220, 220);
}

#endif // COLORS_H
