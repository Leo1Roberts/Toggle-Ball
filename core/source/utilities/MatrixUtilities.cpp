#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat3 buildViewRotationMatrix(glm::vec3 viewDirection) {
	float
		a = viewDirection.x,
		b = viewDirection.y,
		c = viewDirection.z,
		h = std::sqrt(a * a + b * b);

	float st, ct, sp, cp;

	if (h < 0.001f) { // assume looking forward along y axis
		st = 1.f;
		ct = 0.f;
		sp = 0.f;
		cp = c > 0.f ? 1.f : -1.f;
	} else {
		st = b / h;
		ct = a / h;
		sp = h;
		cp = c;
	}

	return {
		glm::vec3(-st, ct, 0.f),
		glm::vec3(-ct * cp, -st * cp, sp),
		glm::vec3(-ct * sp, -st * sp, -cp)
	};
}


glm::mat4 buildViewMatrix(const glm::mat3& viewRotationMatrix, glm::vec3 viewPosition) {
	glm::mat4 rotMat = glm::transpose(glm::mat4(viewRotationMatrix));
	glm::mat4 transMat = glm::translate(glm::mat4(1.f), -viewPosition);
	return rotMat * transMat;
}


glm::mat4 fillRotationScaled(const glm::mat3& source, glm::vec3 scale) {
	glm::mat3 scaled(source[0] * scale.x, source[1] * scale.y, source[2] * scale.z);
	return glm::mat4(scaled);
}

glm::mat4 buildScaledWorldMatrix(const glm::mat3& rotation, glm::vec3 position, glm::vec3 scale) {
	glm::mat4 worldMat = fillRotationScaled(rotation, scale);
	worldMat[3] = glm::vec4(position, 1.f);
	return worldMat;
}