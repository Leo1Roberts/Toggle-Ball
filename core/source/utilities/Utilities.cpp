#include "TypeAlias.h"
#include "utilities/Utilities.h"

#include "glm/gtc/constants.hpp"

#include <chrono>
#include <cmath>
#include <algorithm>
#include <iomanip>

glm::vec3 colorToLinear(col srgb) {
	glm::vec3 colorVec = srgb;
	return {
		std::pow(colorVec.r, 2.2f),
		std::pow(colorVec.g, 2.2f),
		std::pow(colorVec.b, 2.2f)
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
	radians = fmodf(radians, glm::two_pi<float>());
	if (radians <= -glm::pi<float>())
		radians += glm::two_pi<float>();
	else if (radians > glm::pi<float>())
		radians -= glm::two_pi<float>();
	return radians;
}


std::string floatToString(float f, int maxPrecision, bool forceMax) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(maxPrecision) << f;
	std::string result = ss.str();
	if (!forceMax && result.find('.') != std::string::npos) {
		result.erase(result.find_last_not_of('0') + 1, std::string::npos);
		if (result.back() == '.')
			result.pop_back();
	}

	if (!result.empty() && result[0] == '-' && result.find_first_not_of("-0.") == std::string::npos)
		result.erase(0, 1);
	return result;
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