#ifndef UTILITIES_H
#define UTILITIES_H

#include "Colors.h"
#include "main.h"

#include <glm/glm.hpp>
#include <string_view>

[[nodiscard]] glm::vec3 colorToLinear(col srgb);

[[nodiscard]] bool iequals(std::string_view a, std::string_view b);

[[nodiscard]] long now_ms();
[[nodiscard]] microseconds now();
[[nodiscard]] inline float toSeconds(microseconds t) { return (float)t / 1000000.f; }

[[nodiscard]] float wrapAngle(float radians); // Wraps angle to range (-PI, PI]
[[nodiscard]] float angleToDisplay(float angle);
[[nodiscard]] float wrapDisplayAngle(float displayAngle);
[[nodiscard]] float displayToAngle(float displayAngle);

enum { // Order of these is important
	VP_INSIDE,
	VP_BOUNDARY,
	VP_OUTSIDE
};
float clamp(float val, float min, float max, byte* valPos);

[[nodiscard]] constexpr glm::vec3 planarToWorld(glm::vec2 planarVec) {
	return {0, planarVec.x, planarVec.y};
}
[[nodiscard]] constexpr glm::vec2 worldToPlanar(glm::vec3 worldVec) {
	return {worldVec.y, worldVec.z};
}

[[nodiscard]] glm::vec2 pixelsToXNorm(float x, float y, float width, float height);

[[nodiscard]] glm::vec2 pixelsToYNorm(float x, float y, float width, float height);

[[nodiscard]] glm::vec2 pixelsToYNorm(glm::vec2 pixels, float width, float height);

[[nodiscard]] GLuint loadShader(GLenum shaderType, const std::string& shaderSource);

#endif // UTILITIES_H
