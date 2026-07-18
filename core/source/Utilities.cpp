#include "main.h"
#include "Utilities.h"

#include <chrono>
#include <cmath>
#include <algorithm>

vec3 colorToLinear(vec3 srgb) {
	return {
		std::pow(srgb.x, 2.2f),
		std::pow(srgb.y, 2.2f),
		std::pow(srgb.z, 2.2f)
	};
}

bool iequals(std::string_view a, std::string_view b) {
	if (a.length() != b.length())
		return false;

	return std::ranges::equal(a, b,
		[](char charA, char charB) {
			return std::tolower((byte)charA) == std::tolower((byte)charB);
		}
	);
}

long now_ms() {
	auto now = std::chrono::steady_clock::now();
	auto duration = now.time_since_epoch();
	return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}
microseconds now() {
	auto now = std::chrono::steady_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

float wrapAngle(float radians) {
	radians = fmodf(radians, PI * 2);
	if (radians <= -PI)
		radians += PI * 2;
	else if (radians > PI)
		radians -= PI * 2;
	return radians;
}

float angleToDisplay(float angle) {
	angle *= -180 / PI;
	if (angle == 0) angle = 0; // Remove -0
	return std::round(angle * 100) * 0.01f;
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

GLuint loadShader(GLenum shaderType, const std::string& shaderSource) {
	GLuint shader = glCreateShader(shaderType);
	if (shader) {
		auto* shaderRawString = const_cast<GLchar*>(shaderSource.c_str());
		auto shaderLength = static_cast<GLint>(shaderSource.length());
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
