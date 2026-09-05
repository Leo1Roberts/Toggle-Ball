#ifndef UTILITIES_H
#define UTILITIES_H

#include "Color.h"

#include <glm/glm.hpp>
#include "glm/gtc/constants.hpp"
#include <string>

[[nodiscard]] glm::vec3 colorToLinear(col srgb);

[[nodiscard]] bool iequals(std::string_view a, std::string_view b);

[[nodiscard]] long now_ms();
[[nodiscard]] microseconds now();
[[nodiscard]] inline float toSeconds(microseconds t) { return (float)t / 1000000.f; }
[[nodiscard]] inline float to_deg(float rad) { return -rad * glm::one_over_pi<float>() * 180.f; }
[[nodiscard]] inline float to_rad(float deg) { return -deg / 180.f * glm::pi<float>(); }

inline glm::mat2 angleToRotation2D(float radians) {
	float c = std::cos(radians);
	float s = std::sin(radians);
	return { c,  s,
			-s,  c };
}

inline float cross2D(glm::vec2 a, glm::vec2 b) {
	return a.x * b.y - a.y * b.x;
}

[[nodiscard]] float angleDifference(glm::vec2 newPos, glm::vec2 oldPos, glm::vec2 pivot);

[[nodiscard]] float wrapAngle(float radians); // Wraps angle to range (-PI, PI]

[[nodiscard]] std::string floatToString(float f, int maxPrecision, bool forceMax = false);

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

#endif // UTILITIES_H
