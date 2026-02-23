#ifndef UTILITIES_H
#define UTILITIES_H

void colorToLinear(vec3* srgb);

long now_ms();

float randomFloat();

float randomFloatBeyondValue(float val);

inline float lerp(float a, float b, float t) {
	return a * (1-t) + b * t;
}

float wrapAngle(float angle);
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
std::string importShader(const std::string& path);
#else

std::string importShader(AAssetManager* assetManager, const std::string& path);

#endif

/*!
 * Helper function to load a shader of a given type
 * @param shaderType The OpenGL shader type. Should either be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 * @param shaderSource The full source of the shader
 * @return the id of the shader, as returned by glCreateShader, or 0 in the case of an error
 */
GLuint loadShader(GLenum shaderType, const std::string& shaderSource);

#endif // UTILITIES_H
