#ifndef UTILITIES_H
#define UTILITIES_H

#include "main.h"
#include "VectorMatrix.h"

#include <string_view>

[[nodiscard]] vec3 colorToLinear(vec3 srgb);

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

[[nodiscard]] constexpr vec3 planarToWorld(vec2 planarVec) {
	return {0, planarVec.x, planarVec.y};
}
[[nodiscard]] constexpr vec2 worldToPlanar(vec3 worldVec) {
	return {worldVec.y, worldVec.z};
}

[[nodiscard]] vec2 pixelsToXNorm(float x, float y, float width, float height);

[[nodiscard]] vec2 pixelsToYNorm(float x, float y, float width, float height);

[[nodiscard]] vec2 pixelsToYNorm(vec2 pixels, float width, float height);

[[nodiscard]] GLuint loadShader(GLenum shaderType, const std::string& shaderSource);

#endif // UTILITIES_H
