#ifndef UTILITIES_H
#define UTILITIES_H

[[nodiscard]] vec3 colorToLinear(const vec3& srgb);

[[nodiscard]] long now_ms();
[[nodiscard]] microseconds now();
[[nodiscard]] inline float toSeconds(microseconds t) { return (float)t / 1000000.f; }

[[nodiscard]] float randomFloat();

[[nodiscard]] float randomFloatBeyondValue(float val);

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

[[nodiscard]] inline vec3 planarToWorld(vec2 planarVec) {
	return {0, planarVec.x, planarVec.y};
}
[[nodiscard]] inline vec2 worldToPlanar(vec3 worldVec) {
	return {worldVec.y, worldVec.z};
}

[[nodiscard]] vec2 pixelsToXNorm(float x, float y, float width, float height);

[[nodiscard]] vec2 pixelsToYNorm(float x, float y, float width, float height);

[[nodiscard]] vec2 pixelsToYNorm(vec2 pixels, float width, float height);

[[nodiscard]] GLuint loadShader(GLenum shaderType, const std::string& shaderSource);

#endif // UTILITIES_H
