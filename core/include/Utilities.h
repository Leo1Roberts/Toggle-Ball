#ifndef UTILITIES_H
#define UTILITIES_H

void colorToLinear(vec3* srgb);

long now_ms();

float randomFloat();

float randomFloatBeyondValue(float val);

constexpr inline float lerp(float a, float b, float t) noexcept {
	return a * (1-t) + b * t;
}

float wrapAngle(float radians);
float angleToDisplay(float angle);
float wrapDisplayAngle(float displayAngle);
float displayToAngle(float displayAngle);

enum { // Order of these is important
	VP_INSIDE,
	VP_BOUNDARY,
	VP_OUTSIDE
};
float clamp(float val, float min, float max, byte* valPos);

vec2 pixelsToXNorm(float x, float y, float width, float height);

vec2 pixelsToYNorm(float x, float y, float width, float height);

vec2 pixelsToYNorm(vec2 pixels, float width, float height);

#ifdef WINDOWS_VERSION
std::string importTextFile(const std::string& path);
#else
std::string importTextFile(AAssetManager* assetManager, const std::string& path);
#endif

GLuint loadShader(GLenum shaderType, const std::string& shaderSource);

#endif // UTILITIES_H
