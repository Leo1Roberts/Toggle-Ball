#ifndef COLORS_H
#define COLORS_H

#include <glm/glm.hpp>

struct col {
	glm::uint8 r = 0, g = 0, b = 0, a = 0;

	constexpr col() = default;
	constexpr col(glm::uint8 r, glm::uint8 g, glm::uint8 b, glm::uint8 a = 255)
		: r(r), g(g), b(b), a(a) {}
	constexpr col(const glm::vec3& v)
		: r((glm::uint8)(v.r * 255.f)), g((glm::uint8)(v.g * 255.f)), b((glm::uint8)(v.b * 255.f)), a(255) {}
	constexpr col(const glm::vec4& v)
		: r((glm::uint8)(v.r * 255.f)), g((glm::uint8)(v.g * 255.f)), b((glm::uint8)(v.b * 255.f)), a((glm::uint8)(v.a * 255.f)) {}

	operator glm::vec3() const { return glm::vec3(r, g, b) / 255.f; }
	operator glm::vec4() const { return glm::vec4(r, g, b, a) / 255.f; }

	bool operator==(const col& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
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
