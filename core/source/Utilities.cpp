#include "main.h"
#include <stdexcept>

void colorToLinear(vec3* srgb) {
	srgb->x = (float) pow(srgb->x, 2.2);
	srgb->y = (float) pow(srgb->y, 2.2);
	srgb->z = (float) pow(srgb->z, 2.2);
}

long now_ms() {
#ifdef WINDOWS_VERSION
	return timeGetTime();
#else
	struct timespec res;
	clock_gettime(CLOCK_MONOTONIC, &res);
	return (long) (1000.0 * (double) res.tv_sec + (double) res.tv_nsec / 1e6);
#endif
}

float randomFloat() { // Returns random number between -1 and 1
	return ((float) rand() / (float) RAND_MAX) * 2.0f - 1.0f;
}

float randomFloatBeyondValue(float val) {
	float result;
	do result = randomFloat();
	while (abs(result) < val);
	return result;
}

float wrapAngle(float angle) {
	angle = fmodf(angle, PI * 2);
	if (angle <= -PI) angle += PI * 2;
	else if (angle > PI) angle -= PI * 2;
	return angle;
}

float angleToDisplay(float angle) {
	angle *= -180 / PI;
	if (angle == 0) angle = 0; // Remove -0
	return round(angle * 100) * 0.01f;
}

float wrapDisplayAngle(float displayAngle) {
	displayAngle = fmodf(displayAngle, 360.0f);
	if (displayAngle <= -180) displayAngle += 360;
	else if (displayAngle > 180) displayAngle -= 360;
	return displayAngle;
}

float displayToAngle(float displayAngle) {
	return displayAngle * PI / -180;
}

float clamp(float val, float min, float max, byte* valPos) {
	if (max < min)
		throw std::invalid_argument("Max is smaller than min");

	if (val == min || val == max)
		*valPos = VP_BOUNDARY;
	else if (val < min) {
		val = min;
		*valPos = VP_OUTSIDE;
	} else if (val > max) {
		val = max;
		*valPos = VP_OUTSIDE;
	} else
		*valPos = VP_INSIDE;

	return val;
}


vec2 pixelsToXNorm(float x, float y, float width, float height) {
	float widthInv = 1.0f / width;
	return {
			x * 2.0f * widthInv - 1.0f,
			(-y * 2.0f + height) * widthInv
	};
}

vec2 pixelsToYNorm(float x, float y, float width, float height) {
	float heightInv = 1.0f / height;
	return {
			(x * 2.0f - width) * heightInv,
			-y * 2.0f * heightInv + 1.0f
	};
}

vec2 pixelsToYNorm(vec2 pixels, float width, float height) {
	return pixelsToYNorm(pixels.x,
	                     pixels.y,
	                     width, height);
}

#ifdef WINDOWS_VERSION
std::string importShader(const std::string& path) {
	FILE* file;
	fopen_s(&file, path.c_str(), "rt");
	if (file == nullptr) {
		return "";
	}

	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	std::string result(length, 0);

	fseek(file, 0, SEEK_SET);

	fread_s(&result[0], length, 1, length, file);

	fclose(file);

	return result;
}
#else

std::string importShader(AAssetManager* assetManager, const std::string& path) {
	AAsset* file = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
	if (file == nullptr) {
		return "";
	}

	const char* buffer = (const char*) AAsset_getBuffer(file);

	int length = AAsset_getLength(file);
	std::string result(length, 0);

	int current = 0;
	for (int i = 0; i < length; i++) {
		if (buffer[i] != '\r') {
			result[current++] = buffer[i];
		}
	}

	AAsset_close(file);

	return result;
}

#endif

GLuint loadShader(GLenum shaderType, const std::string& shaderSource) {
	GLuint shader = glCreateShader(shaderType);
	if (shader) {
		auto* shaderRawString = (GLchar*) shaderSource.c_str();
		GLint shaderLength = (GLint) shaderSource.length();
		glShaderSource(shader, 1, &shaderRawString, &shaderLength);
		glCompileShader(shader);

		GLint shaderCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);

		// If the shader doesn't compile, log the result to the terminal for debugging
		if (!shaderCompiled) {
			GLint infoLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

			if (infoLength) {
				auto* infoLog = new GLchar[infoLength];
				glGetShaderInfoLog(shader, infoLength, nullptr, infoLog);
				delete[] infoLog;
			}

			glDeleteShader(shader);
			shader = 0;
		}
	}
	return shader;
}
