#ifndef UTILITIES_H
#define UTILITIES_H

void colorToLinear(vec3* srgb);

long now_ms();

float randomFloat();

float randomFloatBeyondValue(float val);

float wrapAngle(float radians); // Wraps angle to range (-PI, PI]
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

GLuint loadShader(GLenum shaderType, const std::string& shaderSource);

#endif // UTILITIES_H
