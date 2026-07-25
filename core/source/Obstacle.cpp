#include "main.h"
#include "Colors.h"
#include "Mesh.h"
#include "Obstacle.h"

#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <array>

static glm::mat3 angleToRotation(float radians) {
	return glm::mat3_cast(glm::angleAxis(radians, OBSTACLE_ROTATION_AXIS));
}


void ObstacleKinematicState::setAngle(float radians) {
	angle = wrapAngle(radians);
	rotation = angleToRotation(radians);
}


std::string AbstractShapeSpec::serialize() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << getMinorRadius() << "," << getTypeString() << ":" << serializeData();

	return ss.str();
}
std::unique_ptr<AbstractShapeSpec> AbstractShapeSpec::deserialize(const std::string& data) {
	std::istringstream ss(data);

	std::string shapeType, shapeData;
	float minorRadius;

	char c;
	if (!((ss >> minorRadius >> c) && std::getline(ss, shapeType, ':') && std::getline(ss, shapeData)))
		throw std::invalid_argument("Invalid obstacle shape data format");

	if (SegmentSpec::getTypeStringStatic() == shapeType)
		return std::make_unique<SegmentSpec>(minorRadius, shapeData);
	if (ArcSpec::getTypeStringStatic() == shapeType)
		return std::make_unique<ArcSpec>(minorRadius, shapeData);

	throw std::invalid_argument("Unrecognised obstacle shape type");
}

std::string SegmentSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << leftLength << "," << rightLength;

	return ss.str();
}
SegmentSpec::SegmentSpec(float minorRadius, const std::string& data) :
    AbstractShapeSpec(minorRadius) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> leftLength >> c >> rightLength))
		throw std::invalid_argument("Invalid segment shape data format");

	setLeftLength(leftLength);
	setRightLength(rightLength);
}

std::string ArcSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << arcAngle << "," << arcRadius;

	return ss.str();
}
ArcSpec::ArcSpec(float minorRadius, const std::string& data) :
    AbstractShapeSpec(minorRadius) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> arcAngle >> c >> arcRadius))
		throw std::invalid_argument("Invalid arc shape data format");
}


void SegmentSpec::setLeftLength(float l) {
	leftLength = l;
	leftCap = {0.f, -l, 0.f};
}
void SegmentSpec::setRightLength(float l) {
	rightLength = l;
	rightCap = {0.f, l, 0.f};
}

void ArcSpec::setArcAngle(float radians) {
	arcAngle = radians;
	setCaps();
}
void ArcSpec::setArcRadius(float r) {
	arcRadius = r;
	setCaps();
}
void ArcSpec::setCaps() {
	float y = std::sin(getHalfArcAngle()) * getArcRadius();
	float z = std::cos(getHalfArcAngle()) * getArcRadius();
	leftCap = {0, -y, z};
	rightCap = {0, y, z};
}


void AbstractShapeSpec::generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;
	buildObstacleMesh(vs, is, color);
	obstacleMesh.setData(vs, is);
}

void SegmentSpec::buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const {
	vs.reserve(2 + 4 * (SECTORS_PER_SEMICIRCLE + 1));
	is.reserve(18 * (SECTORS_PER_SEMICIRCLE + 1));

	// Caps
	vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap().y, 0), glm::vec2(), glm::vec3(1, 0, 0), color);
	vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap().y, 0), glm::vec2(), glm::vec3(1, 0, 0), color);
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrv = y * (getMinorRadius() - getBevel());
		float zrv = z * (getMinorRadius() - getBevel());
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap().y + yrv, zrv), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getRightCap().y + yrh, zrh), glm::vec2(), glm::vec3(0, y, z), color);
		vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap().y - yrv, zrv), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getLeftCap().y - yrh, zrh), glm::vec2(), glm::vec3(0, -y, z), color);
	}
	for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 4; i += 4) {
		// Semicircles
		// Right
		is.push_back(i + 4);
		is.push_back(i + 0);
		is.push_back(0);
		// Left
		is.push_back(i + 2);
		is.push_back(i + 6);
		is.push_back(1);

		// Bevel
		// Right
		is.push_back(i + 0);
		is.push_back(i + 4);
		is.push_back(i + 5);
		is.push_back(i + 0);
		is.push_back(i + 5);
		is.push_back(i + 1);
		// Left
		is.push_back(i + 2);
		is.push_back(i + 3);
		is.push_back(i + 7);
		is.push_back(i + 2);
		is.push_back(i + 7);
		is.push_back(i + 6);
	}

	// Rectangular faces
	// Front
	is.push_back(2 + 0);
	is.push_back(2 + 2);
	is.push_back(2 + 2 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 0);
	is.push_back(2 + 2 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 0 + SECTORS_PER_SEMICIRCLE * 4);
	// Top bevel
	is.push_back(2 + 2);
	is.push_back(2 + 0);
	is.push_back(2 + 1);
	is.push_back(2 + 2);
	is.push_back(2 + 1);
	is.push_back(2 + 3);
	// Bottom bevel
	is.push_back(2 + 3 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 1 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 0 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 3 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 0 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + 2 + SECTORS_PER_SEMICIRCLE * 4);
}

void ArcSpec::buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const {
	const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() / PI);
	vs.reserve(2 + 4 * (SECTORS_PER_SEMICIRCLE + 1) + 4 * (NUM_SECTORS + 1));
	is.reserve(18 * SECTORS_PER_SEMICIRCLE + 18 * NUM_SECTORS);

	// Caps
	vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap().y, getRightCap().z), glm::vec2(), glm::vec3(1, 0, 0), color);
	vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap().y, getLeftCap().z), glm::vec2(), glm::vec3(1, 0, 0), color);
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrv = y * (getMinorRadius() - getBevel());
		float zrv = z * (getMinorRadius() - getBevel());
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap().y + yrv, getRightCap().z + zrv), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getRightCap().y + yrh, getRightCap().z + zrh), glm::vec2(), glm::vec3(0, y, z), color);
		vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap().y - yrv, getLeftCap().z + zrv), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getLeftCap().y - yrh, getLeftCap().z + zrh), glm::vec2(), glm::vec3(0, -y, z), color);
	}
	for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 4; i += 4) {
		// Semicircles
		// Right
		is.push_back(i + 4);
		is.push_back(i + 0);
		is.push_back(0);
		// Left
		is.push_back(i + 2);
		is.push_back(i + 6);
		is.push_back(1);

		// Bevel
		// Right
		is.push_back(i + 0);
		is.push_back(i + 4);
		is.push_back(i + 5);
		is.push_back(i + 0);
		is.push_back(i + 5);
		is.push_back(i + 1);
		// Left
		is.push_back(i + 2);
		is.push_back(i + 3);
		is.push_back(i + 7);
		is.push_back(i + 2);
		is.push_back(i + 7);
		is.push_back(i + 6);
	}

	// Arc
	for (int i = 0; i <= NUM_SECTORS; i++) {
		float ang = (float)i / (float)NUM_SECTORS * getArcAngle() - getHalfArcAngle();
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrov = y * (getArcRadius() + getMinorRadius() - getBevel());
		float zrov = z * (getArcRadius() + getMinorRadius() - getBevel());
		float yriv = y * (getArcRadius() - getMinorRadius() + getBevel());
		float zriv = z * (getArcRadius() - getMinorRadius() + getBevel());
		float yroh = y * (getArcRadius() + getMinorRadius());
		float zroh = z * (getArcRadius() + getMinorRadius());
		float yrih = y * (getArcRadius() - getMinorRadius());
		float zrih = z * (getArcRadius() - getMinorRadius());
		vs.emplace_back(glm::vec3(getHalfDepth(), yriv, zriv), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), yrih, zrih), glm::vec2(), glm::vec3(0, -y, -z), color);
		vs.emplace_back(glm::vec3(getHalfDepth(), yrov, zrov), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), yroh, zroh), glm::vec2(), glm::vec3(0, y, z), color);
	}
	for (int i = 2 + 4 * (SECTORS_PER_SEMICIRCLE + 1); i < 2 + 4 * (SECTORS_PER_SEMICIRCLE + 1) + NUM_SECTORS * 4; i += 4) {
		// Front
		is.push_back(i + 0);
		is.push_back(i + 4);
		is.push_back(i + 6);
		is.push_back(i + 0);
		is.push_back(i + 6);
		is.push_back(i + 2);

		// Bevel
		// Inside
		is.push_back(i + 5);
		is.push_back(i + 4);
		is.push_back(i + 0);
		is.push_back(i + 1);
		is.push_back(i + 5);
		is.push_back(i + 0);
		// Outside
		is.push_back(i + 7);
		is.push_back(i + 3);
		is.push_back(i + 2);
		is.push_back(i + 6);
		is.push_back(i + 7);
		is.push_back(i + 2);
	}
}


void SegmentSpec::buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	vs.reserve(2 + 2 * (SECTORS_PER_SEMICIRCLE + 1));
	is.reserve(6 * (SECTORS_PER_SEMICIRCLE + 1));

	// Caps
	vs.emplace_back(glm::vec3(0, getRightCap().y, 0), glm::vec2(), glm::vec3());
	vs.emplace_back(glm::vec3(0, getLeftCap().y, 0), glm::vec2(), glm::vec3());
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(glm::vec3(0, getRightCap().y + yrh, zrh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getLeftCap().y - yrh, zrh), glm::vec2(), glm::vec3());
	}
	for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 2; i += 2) {
		// Semicircles
		// Right
		is.push_back(i + 2);
		is.push_back(i + 0);
		is.push_back(0);
		// Left
		is.push_back(i + 1);
		is.push_back(i + 3);
		is.push_back(1);
	}

	// Rectangular faces
	// Front
	is.push_back(2 + 0);
	is.push_back(2 + 1);
	is.push_back(2 + 1 + SECTORS_PER_SEMICIRCLE * 2);
	is.push_back(2 + 0);
	is.push_back(2 + 1 + SECTORS_PER_SEMICIRCLE * 2);
	is.push_back(2 + 0 + SECTORS_PER_SEMICIRCLE * 2);
}

void ArcSpec::buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() / PI);
	vs.reserve(2 + 2 * (SECTORS_PER_SEMICIRCLE + 1) + 2 * (NUM_SECTORS + 1));
	is.reserve(6 * SECTORS_PER_SEMICIRCLE + 6 * NUM_SECTORS);

	// Caps
	vs.emplace_back(glm::vec3(0, getRightCap().y, getRightCap().z), glm::vec2(), glm::vec3());
	vs.emplace_back(glm::vec3(0, getLeftCap().y, getLeftCap().z), glm::vec2(), glm::vec3());
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(glm::vec3(0, getRightCap().y + yrh, getRightCap().z + zrh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getLeftCap().y - yrh, getLeftCap().z + zrh), glm::vec2(), glm::vec3());
	}
	for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 2; i += 2) {
		// Semicircles
		// Right
		is.push_back(i + 2);
		is.push_back(i + 0);
		is.push_back(0);
		// Left
		is.push_back(i + 1);
		is.push_back(i + 3);
		is.push_back(1);
	}

	// Arc
	for (int i = 0; i <= NUM_SECTORS; i++) {
		float ang = (float)i / (float)NUM_SECTORS * getArcAngle() - getHalfArcAngle();
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yroh = y * (getArcRadius() + getMinorRadius());
		float zroh = z * (getArcRadius() + getMinorRadius());
		float yrih = y * (getArcRadius() - getMinorRadius());
		float zrih = z * (getArcRadius() - getMinorRadius());
		vs.emplace_back(glm::vec3(0, yrih, zrih), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, yroh, zroh), glm::vec2(), glm::vec3());
	}
	for (int i = 2 + 2 * (SECTORS_PER_SEMICIRCLE + 1); i < 2 + 2 * (SECTORS_PER_SEMICIRCLE + 1) + NUM_SECTORS * 2; i += 2) {
		// Front
		is.push_back(i + 0);
		is.push_back(i + 2);
		is.push_back(i + 3);
		is.push_back(i + 0);
		is.push_back(i + 3);
		is.push_back(i + 1);
	}
}


void AbstractShapeSpec::generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;
	buildOutlineMesh(vs, is);
	outlineMesh.setData(vs, is);
}

void SegmentSpec::buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	vs.reserve(4 * (SECTORS_PER_SEMICIRCLE + 1));
	is.reserve(12 * SECTORS_PER_SEMICIRCLE + 12);

	// Caps
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		float yrv = y * getOutlineRadius();
		float zrv = z * getOutlineRadius();
		vs.emplace_back(glm::vec3(0, getRightCap().y + yrh, getRightCap().z + zrh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getRightCap().y + yrv, getRightCap().z + zrv), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getLeftCap().y - yrh, getLeftCap().z + zrh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getLeftCap().y - yrv, getLeftCap().z + zrv), glm::vec2(), glm::vec3());
	}
	for (int i = 0; i < SECTORS_PER_SEMICIRCLE * 4; i += 4) {
		is.push_back(i + 0);
		is.push_back(i + 5);
		is.push_back(i + 1);
		is.push_back(i + 0);
		is.push_back(i + 4);
		is.push_back(i + 5);

		is.push_back(i + 2);
		is.push_back(i + 3);
		is.push_back(i + 7);
		is.push_back(i + 2);
		is.push_back(i + 7);
		is.push_back(i + 6);
	}

	// Segment

	is.push_back(0);
	is.push_back(3);
	is.push_back(2);
	is.push_back(0);
	is.push_back(1);
	is.push_back(3);

	is.push_back(0 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(2 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(3 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(0 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(3 + SECTORS_PER_SEMICIRCLE * 4);
	is.push_back(1 + SECTORS_PER_SEMICIRCLE * 4);
}

void ArcSpec::buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() / PI);
	vs.reserve(4 * (SECTORS_PER_SEMICIRCLE + 1) + 4 * (NUM_SECTORS + 1));
	is.reserve(12 * SECTORS_PER_SEMICIRCLE + 12 * NUM_SECTORS);

	// Caps
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		float yrv = y * getOutlineRadius();
		float zrv = z * getOutlineRadius();
		vs.emplace_back(glm::vec3(0, getRightCap().y + yrh, getRightCap().z + zrh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getRightCap().y + yrv, getRightCap().z + zrv), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getLeftCap().y - yrh, getLeftCap().z + zrh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, getLeftCap().y - yrv, getLeftCap().z + zrv), glm::vec2(), glm::vec3());
	}
	for (int i = 0; i < SECTORS_PER_SEMICIRCLE * 4; i += 4) {
		is.push_back(i + 0);
		is.push_back(i + 5);
		is.push_back(i + 1);
		is.push_back(i + 0);
		is.push_back(i + 4);
		is.push_back(i + 5);

		is.push_back(i + 2);
		is.push_back(i + 3);
		is.push_back(i + 7);
		is.push_back(i + 2);
		is.push_back(i + 7);
		is.push_back(i + 6);
	}

	// Arc
	for (int i = 0; i <= NUM_SECTORS; i++) {
		float ang = (float)i / (float)NUM_SECTORS * getArcAngle() - getHalfArcAngle();
		float y = std::sin(ang);
		float z = std::cos(ang);
		float yroh = y * (getArcRadius() + getMinorRadius());
		float zroh = z * (getArcRadius() + getMinorRadius());
		float yrih = y * (getArcRadius() - getMinorRadius());
		float zrih = z * (getArcRadius() - getMinorRadius());
		float yrov = y * (getArcRadius() + getOutlineRadius());
		float zrov = z * (getArcRadius() + getOutlineRadius());
		float yriv = y * (getArcRadius() - getOutlineRadius());
		float zriv = z * (getArcRadius() - getOutlineRadius());
		vs.emplace_back(glm::vec3(0, yrih, zrih), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, yriv, zriv), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, yroh, zroh), glm::vec2(), glm::vec3());
		vs.emplace_back(glm::vec3(0, yrov, zrov), glm::vec2(), glm::vec3());
	}
	for (int i = 4 * (SECTORS_PER_SEMICIRCLE + 1); i < 4 * (SECTORS_PER_SEMICIRCLE + 1) + NUM_SECTORS * 4; i += 4) {
		is.push_back(i + 0);
		is.push_back(i + 1);
		is.push_back(i + 5);
		is.push_back(i + 0);
		is.push_back(i + 5);
		is.push_back(i + 4);

		is.push_back(i + 2);
		is.push_back(i + 7);
		is.push_back(i + 3);
		is.push_back(i + 2);
		is.push_back(i + 6);
		is.push_back(i + 7);
	}
}


static std::array<float, 2> quadraticFormula(float a, float b, float c) {
	float d = b*b - 4 * a * c;
	if (d < 0) return { NAN, NAN };
	float sqrtD = std::sqrt(d);
	float mul = 0.5f / a;
	return { mul * (-b + sqrtD), mul * (-b - sqrtD) };
}

bool AbstractShapeSpec::isInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const {
	// Check if caps are in the box

	const glm::vec3 leftCapPos = s.getPosition() + s.getRotation() * getLeftCap();
	const glm::vec3 rightCapPos = s.getPosition() + s.getRotation() * getRightCap();

	for (glm::vec3 capPos : {leftCapPos, rightCapPos})
		if (box.left < capPos.y && capPos.y < box.right &&
		    box.bottom < capPos.z && capPos.z < box.top)
			return true; // Quick check - centre of a cap is inside the box

	float rSq = getMinorRadius() * getMinorRadius();

	for (glm::vec3 capPos : {leftCapPos, rightCapPos})
		for (auto& [side, sideStart, sideEnd, perp, para] :
		     {std::tuple{box.left, box.bottom, box.top, capPos.y, capPos.z},
		      std::tuple{box.right, box.bottom, box.top, capPos.y, capPos.z},
		      std::tuple{box.bottom, box.left, box.right, capPos.z, capPos.y},
		      std::tuple{box.top, box.left, box.right, capPos.z, capPos.y}}) {

			float dist = perp - side;
			float distSq = dist * dist;
			if (rSq - distSq > 0) { // Cap intersects with the line on which the side lies
				auto roots = quadraticFormula(1, -2 * para, para * para - rSq + distSq);
				for (float root : roots)
					if (sideStart < root && root < sideEnd)
						return true; // Cap intersects with the side of the box
			}
		}

	// Check if the cap-connecting midsection is in the box
	return midsectionIsInSelectBox(s, box);
}

bool SegmentSpec::midsectionIsInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const {
	const glm::vec3 leftCapPos = s.getPosition() + s.getRotation() * getLeftCap();
	const glm::vec3 rightCapPos = s.getPosition() + s.getRotation() * getRightCap();
	glm::vec3 upOffset = s.getRotation() * glm::vec3(0, 0, getMinorRadius());

	for (glm::vec3 offset : {upOffset, -upOffset}) {
		glm::vec3 segmentLeft = leftCapPos + offset;
		glm::vec3 segmentRight = rightCapPos + offset;

		for (auto& [side, sideStart, sideEnd, leftPerp, rightPerp, leftPara, rightPara] :
		     {std::tuple{box.left, box.bottom, box.top, segmentLeft.y, segmentRight.y, segmentLeft.z, segmentRight.z},
		      std::tuple{box.right, box.bottom, box.top, segmentLeft.y, segmentRight.y, segmentLeft.z, segmentRight.z},
		      std::tuple{box.bottom, box.left, box.right, segmentLeft.z, segmentRight.z, segmentLeft.y, segmentRight.y},
		      std::tuple{box.top, box.left, box.right, segmentLeft.z, segmentRight.z, segmentLeft.y, segmentRight.y}}) {

			float perpendicularDiff = leftPerp - rightPerp;
			if (perpendicularDiff == 0) continue;
			float perpendicularDiffInv = 1 / perpendicularDiff;
			if (std::min(leftPerp, rightPerp) < side && side < std::max(leftPerp, rightPerp)) {
				float root = perpendicularDiffInv * ((leftPara - rightPara) * side + leftPerp * rightPara - rightPerp * leftPara);
				if (sideStart < root && root < sideEnd)
					return true;
			}
		}
	}

	return false;
}

bool ArcSpec::midsectionIsInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const {
	float centerAngle = s.getAngle() + PI * 0.5f;
	bool fullCircle = getArcAngle() >= 2 * PI;

	for (float radiusOffset : {getMinorRadius(), -getMinorRadius()}) {
		float r = getArcRadius() + radiusOffset;
		float rSq = r * r;

		for (auto& [side, sideStart, sideEnd, centerPerp, centerPara, vertical] :
		     {std::tuple{box.left, box.bottom, box.top, s.getPosition().y, s.getPosition().z, true},
		      std::tuple{box.right, box.bottom, box.top, s.getPosition().y, s.getPosition().z, true},
		      std::tuple{box.bottom, box.left, box.right, s.getPosition().z, s.getPosition().y, false},
		      std::tuple{box.top, box.left, box.right, s.getPosition().z, s.getPosition().y, false}}) {

			float dist = centerPerp - side;
			float distSq = dist * dist;
			if (rSq - distSq > 0) {
				auto roots = quadraticFormula(1, -2 * centerPara, centerPara * centerPara - rSq + distSq);
				for (float root : roots)
					if (sideStart < root && root < sideEnd) {
						if (fullCircle)
							return true;
						float y = vertical ? side : root;
						float z = vertical ? root : side;
						float relativeAngle = wrapAngle(std::atan2(z - s.getPosition().z, y - s.getPosition().y) - centerAngle);
						if (std::abs(relativeAngle) < getHalfArcAngle())
							return true;
					}
			}
		}
	}

	return false;
}



std::string IMotionSpec::serialize() const {
	std::ostringstream ss;

	ss << getTypeString() << ":" << serializeData();

	return ss.str();
}
std::unique_ptr<IMotionSpec> IMotionSpec::deserialize(const std::string& data) {
	std::istringstream ss(data);

	std::string motionType, motionData;

	if (!(std::getline(ss, motionType, ':') && std::getline(ss, motionData)))
		throw std::invalid_argument("Invalid obstacle motion data format");

	if (StaticSpec::getTypeStringStatic() == motionType)
		return std::make_unique<StaticSpec>(motionData);
	if (TogglingPositionSpec::getTypeStringStatic() == motionType)
		return std::make_unique<TogglingPositionSpec>(motionData);
	if (TogglingAngleSpec::getTypeStringStatic() == motionType)
		return std::make_unique<TogglingAngleSpec>(motionData);
	if (SpinningSpec::getTypeStringStatic() == motionType)
		return std::make_unique<SpinningSpec>(motionData);
	if (OscillatingPositionSpec::getTypeStringStatic() == motionType)
		return std::make_unique<OscillatingPositionSpec>(motionData);
	if (OscillatingAngleSpec::getTypeStringStatic() == motionType)
		return std::make_unique<OscillatingAngleSpec>(motionData);

	throw std::invalid_argument("Unrecognised obstacle motion type");
}

std::string StaticSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.y << "," << position.z << "," << angle;

	return ss.str();
}
StaticSpec::StaticSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.y >> c >> position.z >> c >> angle))
		throw std::invalid_argument("Invalid static motion data format");
}

std::string TogglingPositionSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << angle << "," << positionA.y << "," << positionA.z << "," << positionB.y << "," << positionB.z;

	return ss.str();
}
TogglingPositionSpec::TogglingPositionSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> angle >> c >> positionA.y >> c >> positionA.z >> c >> positionB.y >> c >> positionB.z))
		throw std::invalid_argument("Invalid toggling position motion data format");
}

std::string TogglingAngleSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.y << "," << position.z << "," << angleA << "," << angleB;

	return ss.str();
}
TogglingAngleSpec::TogglingAngleSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.y >> c >> position.z >> c >> angleA >> c >> angleB))
		throw std::invalid_argument("Invalid toggling angle motion data format");
}

std::string SpinningSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.y << "," << position.z << "," << initialAngle << "," << angularVelocityA << "," << angularVelocityB;

	return ss.str();
}
SpinningSpec::SpinningSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.y >> c >> position.z >> c >> initialAngle >> c >> angularVelocityA >> c >> angularVelocityB))
		throw std::invalid_argument("Invalid spinning motion data format");
}

std::string OscillatingPositionSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position1.y << "," << position1.z << "," << position2.y << "," << position2.z << "," << angle << "," << angularFrequencyA << "," << angularFrequencyB;

	return ss.str();
}
OscillatingPositionSpec::OscillatingPositionSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position1.y >> c >> position1.z >> c >> position2.y >> c >> position2.z >> c >> angle >> c >> angularFrequencyA >> c >> angularFrequencyB))
		throw std::invalid_argument("Invalid oscillating position motion data format");
}

std::string OscillatingAngleSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.y << "," << position.z << "," << angle1 << "," << angle2 << "," << angularFrequencyA << "," << angularFrequencyB;

	return ss.str();
}
OscillatingAngleSpec::OscillatingAngleSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.y >> c >> position.z >> c >> angle1 >> c >> angle2 >> c >> angularFrequencyA >> c >> angularFrequencyB))
		throw std::invalid_argument("Invalid oscillating angle motion data format");
}


void StaticSpec::setAngle(float radians) {
	angle = radians;
	rotation = angleToRotation(radians);
}

void TogglingPositionSpec::setAngle(float radians) {
	angle = radians;
	rotation = angleToRotation(radians);
}

void OscillatingPositionSpec::setAngle(float radians) {
	angle = radians;
	rotation = angleToRotation(radians);
}


void IMotionSpec::generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;
	buildDomainMesh(vs, is, shapeSpec);
	domainMesh.setData(vs, is);
}

void TogglingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	std::vector<ObjectVertex> vs_shadow;
	std::vector<Index> is_shadow;
	shapeSpec->buildShadowMesh(vs_shadow, is_shadow);

	glm::vec3 diff = getPositionA() - getPositionB();
	float line1Length, line2Length;
	line1Length = line2Length = glm::length(diff);
	glm::vec3 diffUnit = diff / line1Length;
	float diffAngle = std::atan2(diff.z, diff.y);
	glm::vec3 topPointA, bottomPointA;
	glm::vec3 diffPerpUnit = glm::vec3(0.f, -diff.z, diff.y) / line1Length;

	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		glm::vec3 diffPerp = diffPerpUnit * segment->getMinorRadius();
		if (wrapAngle(getAngle() - diffAngle) > 0.f) {
			topPointA = getPositionA() + getRotation() * segment->getRightCap() + diffPerp;
			bottomPointA = getPositionA() + getRotation() * segment->getLeftCap() - diffPerp;
		} else {
			topPointA = getPositionA() + getRotation() * segment->getLeftCap() + diffPerp;
			bottomPointA = getPositionA() + getRotation() * segment->getRightCap() - diffPerp;
		}
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		float ang = wrapAngle(diffAngle - getAngle());
		if (ang > -arc->getHalfArcAngle() && ang < arc->getHalfArcAngle()) {
			topPointA = getPositionA() + diffPerpUnit * (arc->getArcRadius() + arc->getMinorRadius());
		} else {
			float startAng = std::abs(wrapAngle(diffAngle - getAngle() + arc->getHalfArcAngle()));
			float endAng = std::abs(wrapAngle(diffAngle - getAngle() - arc->getHalfArcAngle()));
			if (std::abs(startAng - endAng) < 0.001f) { // Equal
				topPointA = getPositionA() + getRotation() * arc->getRightCap() + diffPerpUnit * arc->getMinorRadius();
				line1Length += arc->getRightCap().y - arc->getLeftCap().y;
			} else if (startAng < endAng)
				topPointA = getPositionA() + getRotation() * arc->getRightCap() + diffPerpUnit * arc->getMinorRadius();
			else
				topPointA = getPositionA() + getRotation() * arc->getLeftCap() + diffPerpUnit * arc->getMinorRadius();
		}
		
		ang = wrapAngle(diffAngle - getAngle() + PI);
		if (ang > -arc->getHalfArcAngle() && ang < arc->getHalfArcAngle()) {
			bottomPointA = getPositionA() - diffPerpUnit * (arc->getArcRadius() + arc->getMinorRadius());
		} else {
			float startAng = std::abs(wrapAngle(diffAngle - getAngle() + arc->getHalfArcAngle() + PI));
			float endAng = std::abs(wrapAngle(diffAngle - getAngle() - arc->getHalfArcAngle() + PI));
			if (std::abs(startAng - endAng) < 0.001f) { // Equal
				bottomPointA = getPositionA() + getRotation() * arc->getLeftCap() - diffPerpUnit * arc->getMinorRadius();
				line2Length += arc->getRightCap().y - arc->getLeftCap().y;
			} else if (startAng < endAng)
				bottomPointA = getPositionA() + getRotation() * arc->getRightCap() - diffPerpUnit * arc->getMinorRadius();
			else
				bottomPointA = getPositionA() + getRotation() * arc->getLeftCap() - diffPerpUnit * arc->getMinorRadius();
		}
	} else return;

	int numDots1 = 2 * (((int)(line1Length / OUTLINE_WIDTH_WORLD / 2) + 1) / 2);
	int numDots2 = 2 * (((int)(line2Length / OUTLINE_WIDTH_WORLD / 2) + 1) / 2);
	bool drawDots = numDots1 < 1000 && numDots2 < 1000; // Don't draw dots if there are too many

	size_t vsSize = 2 * vs_shadow.size();
	size_t isSize = 2 * is_shadow.size();
	if (drawDots) {
		vsSize += (numDots1 + numDots2) * (SECTORS_PER_DOT + 1);
		isSize += (numDots1 + numDots2) * SECTORS_PER_DOT * 3;
	}
	vs.reserve(vsSize);
	is.reserve(isSize);

	float dotSpacing = OUTLINE_WIDTH_WORLD * 2.f;
	glm::vec3 dotShift = diffUnit * dotSpacing;

	glm::vec3 start1 = topPointA - diffPerpUnit / 2.f * OUTLINE_WIDTH_WORLD + diffUnit * (line1Length - (dotSpacing * (float)(numDots1 - 1))) / 2.f;
	glm::vec3 start2 = bottomPointA + diffPerpUnit / 2.f * OUTLINE_WIDTH_WORLD + diffUnit * (line2Length - (dotSpacing * (float)(numDots2 - 1))) / 2.f;

	if (drawDots) {
		glm::vec3 dotCentre = start1;
		for (int d = 0; d < numDots1; d++) {
			vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
			for (int i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2.f * PI;
				vs.emplace_back(dotCentre + glm::vec3(0.f, std::cos(ang), std::sin(ang)) * OUTLINE_WIDTH_WORLD / 2.f, glm::vec2(), glm::vec3());
			}

			int CENTRE_INDEX = d * (SECTORS_PER_DOT + 1);
			for (int i = CENTRE_INDEX + 1; i < CENTRE_INDEX + SECTORS_PER_DOT; i++) {
				is.push_back(CENTRE_INDEX);
				is.push_back(i);
				is.push_back(i + 1);
			}
			is.push_back(CENTRE_INDEX);
			is.push_back(CENTRE_INDEX + SECTORS_PER_DOT);
			is.push_back(CENTRE_INDEX + 1);

			dotCentre += dotShift;
		}

		dotCentre = start2;
		for (int d = 0; d < numDots2; d++) {
			vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
			for (int i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2.f * PI;
				vs.emplace_back(dotCentre + glm::vec3(0.f, std::cos(ang), std::sin(ang)) * OUTLINE_WIDTH_WORLD / 2.f, glm::vec2(), glm::vec3());
			}

			int CENTRE_INDEX = numDots1 * (SECTORS_PER_DOT + 1) + d * (SECTORS_PER_DOT + 1);
			for (int i = CENTRE_INDEX + 1; i < CENTRE_INDEX + SECTORS_PER_DOT; i++) {
				is.push_back(CENTRE_INDEX);
				is.push_back(i);
				is.push_back(i + 1);
			}
			is.push_back(CENTRE_INDEX);
			is.push_back(CENTRE_INDEX + SECTORS_PER_DOT);
			is.push_back(CENTRE_INDEX + 1);

			dotCentre += dotShift;
		}
	}

	glm::mat3 rot = getRotation();
	for (glm::vec3 pos : {getPositionA(), getPositionB()}) {
		auto offset = (Index)vs.size();

		std::ranges::transform(vs_shadow, std::back_inserter(vs), [pos, rot](const ObjectVertex& v) {
			return ObjectVertex(pos + rot * v.position, v.uv, v.normal, v.color);
		});

		std::ranges::transform(is_shadow, std::back_inserter(is), [offset](Index i) {
			return offset + i;
		});
	}
}

void TogglingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	std::vector<ObjectVertex> vs_shadow;
	std::vector<Index> is_shadow;
	shapeSpec->buildShadowMesh(vs_shadow, is_shadow);

	float dotsArcAngle;
	float dotsArc1Radius, dotsArc2Radius;
	float start1, start2;

	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		dotsArcAngle = std::clamp(getAngleB() - getAngleA(), -2.f * PI, 2.f * PI);
		dotsArc1Radius = glm::length(segment->getRightCap()) + segment->getMinorRadius() - OUTLINE_WIDTH_WORLD / 2.f;
		dotsArc2Radius = glm::length(segment->getLeftCap()) + segment->getMinorRadius() - OUTLINE_WIDTH_WORLD / 2.f;
		start1 = getAngleA();
		start2 = getAngleA() + PI;
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		float angDiff = getAngleB() - getAngleA();
		if (angDiff < 0) {
			dotsArcAngle = std::clamp(angDiff + arc->getArcAngle(), -2.f * PI + arc->getArcAngle(), 0.f);
			start1 = start2 = getAngleA() + PI / 2.f - arc->getHalfArcAngle();
		} else {
			dotsArcAngle = std::clamp(angDiff - arc->getArcAngle(), 0.f, 2.f * PI - arc->getArcAngle());
			start1 = start2 = getAngleA() + PI / 2.f + arc->getHalfArcAngle();
		}
		dotsArc1Radius = arc->getArcRadius() + arc->getMinorRadius() - OUTLINE_WIDTH_WORLD / 2.f;
		dotsArc2Radius = arc->getArcRadius() - arc->getMinorRadius() + OUTLINE_WIDTH_WORLD / 2.f;
	} else return;

	float absArcAngle = std::abs(dotsArcAngle);
	float sign = dotsArcAngle < 0.f ? -1.f : 1.f;

	float arc1Length = dotsArc1Radius * absArcAngle;
	float arc2Length = dotsArc2Radius * absArcAngle;

	int numDots1 = 2 * (((int)(arc1Length / OUTLINE_WIDTH_WORLD / 2) + 1) / 2);
	int numDots2 = 2 * (((int)(arc2Length / OUTLINE_WIDTH_WORLD / 2) + 1) / 2);
	bool drawDots = numDots1 < 1000 && numDots2 < 1000; // Don't draw dots if there are too many

	size_t vsSize = 2 * vs_shadow.size();
	size_t isSize = 2 * is_shadow.size();
	if (drawDots) {
		vsSize += (numDots1 + numDots2) * (SECTORS_PER_DOT + 1);
		isSize += (numDots1 + numDots2) * SECTORS_PER_DOT * 3;
	}
	vs.reserve(vsSize);
	is.reserve(isSize);

	float dotSpacing = OUTLINE_WIDTH_WORLD * 2;
	float dotShift1 = sign * dotSpacing / dotsArc1Radius;
	float dotShift2 = sign * dotSpacing / dotsArc2Radius;
	start1 += sign * (arc1Length - (dotSpacing * (float)(numDots1 - 1))) / dotsArc1Radius / 2;
	start2 += sign * (arc2Length - (dotSpacing * (float)(numDots2 - 1))) / dotsArc2Radius / 2;

	float dotAngle = start1;
	for (int d = 0; d < numDots1; d++) {
		glm::vec3 dotCentre = glm::vec3(0.f, std::cos(dotAngle), std::sin(dotAngle)) * dotsArc1Radius;
		vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
		for (int i = 0; i < SECTORS_PER_DOT; i++) {
			float ang = (float)i / (float)SECTORS_PER_DOT * 2.f * PI;
			vs.emplace_back(dotCentre + glm::vec3(0.f, std::cos(ang), std::sin(ang)) * OUTLINE_WIDTH_WORLD / 2.f, glm::vec2(), glm::vec3());
		}

		int CENTRE_INDEX = d * (SECTORS_PER_DOT + 1);
		for (int i = CENTRE_INDEX + 1; i < CENTRE_INDEX + SECTORS_PER_DOT; i++) {
			is.push_back(CENTRE_INDEX);
			is.push_back(i);
			is.push_back(i + 1);
		}
		is.push_back(CENTRE_INDEX);
		is.push_back(CENTRE_INDEX + SECTORS_PER_DOT);
		is.push_back(CENTRE_INDEX + 1);

		dotAngle += dotShift1;
	}

	dotAngle = start2;
	for (int d = 0; d < numDots2; d++) {
		glm::vec3 dotCentre = glm::vec3(0.f, std::cos(dotAngle), std::sin(dotAngle)) * dotsArc2Radius;
		vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
		for (int i = 0; i < SECTORS_PER_DOT; i++) {
			float ang = (float)i / (float)SECTORS_PER_DOT * 2.f * PI;
			vs.emplace_back(dotCentre + glm::vec3(0.f, std::cos(ang), std::sin(ang)) * OUTLINE_WIDTH_WORLD / 2.f, glm::vec2(), glm::vec3());
		}

		int CENTRE_INDEX = numDots1 * (SECTORS_PER_DOT + 1) + d * (SECTORS_PER_DOT + 1);
		for (int i = CENTRE_INDEX + 1; i < CENTRE_INDEX + SECTORS_PER_DOT; i++) {
			is.push_back(CENTRE_INDEX);
			is.push_back(i);
			is.push_back(i + 1);
		}
		is.push_back(CENTRE_INDEX);
		is.push_back(CENTRE_INDEX + SECTORS_PER_DOT);
		is.push_back(CENTRE_INDEX + 1);

		dotAngle += dotShift2;
	}

	for (float angle : {getAngleA(), getAngleB()}) {
		auto offset = (Index)vs.size();

		glm::mat3 rot = angleToRotation(angle);
		std::ranges::transform(vs_shadow, std::back_inserter(vs), [rot](const ObjectVertex& v) {
			return ObjectVertex(rot * v.position, v.uv, v.normal, v.color);
		});

		std::ranges::transform(is_shadow, std::back_inserter(is), [offset](Index i) {
			return offset + i;
		});
	}
}

void SpinningSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vs.reserve(1 + SECTORS_PER_CIRCLE);
		is.reserve(SECTORS_PER_CIRCLE * 3);

		vs.emplace_back(glm::vec3(), glm::vec2(), glm::vec3());
		for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = std::sin(ang);
			float z = std::cos(ang);
			float radius = std::max(segment->getLeftLength(), segment->getRightLength()) + segment->getMinorRadius();
			vs.emplace_back(glm::vec3(0.f, y * radius, z * radius), glm::vec2(), glm::vec3());
			is.push_back(0);
			is.push_back(1 + (i + 1) % SECTORS_PER_CIRCLE);
			is.push_back(1 + i);
		}
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		vs.reserve(2 * SECTORS_PER_CIRCLE);
		is.reserve(SECTORS_PER_CIRCLE * 6);

		for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = std::sin(ang);
			float z = std::cos(ang);
			float innerRadius = arc->getArcRadius() - arc->getMinorRadius();
			float outerRadius = arc->getArcRadius() + arc->getMinorRadius();
			vs.emplace_back(glm::vec3(0.f, y * innerRadius, z * innerRadius), glm::vec2(), glm::vec3());
			vs.emplace_back(glm::vec3(0.f, y * outerRadius, z * outerRadius), glm::vec2(), glm::vec3());
			is.push_back(i*2);
			is.push_back(i*2 + 1);
			is.push_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
			is.push_back(i*2);
			is.push_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
			is.push_back((i*2 + 2) % (SECTORS_PER_CIRCLE * 2));
		}
	}
}

void OscillatingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vs.reserve((SECTORS_PER_SEMICIRCLE + 1) * 4 + 2);
		is.reserve((SECTORS_PER_SEMICIRCLE + 1) * 18);

		// Caps
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = std::sin(ang);
			float z = std::cos(ang);
			float yrh = y * segment->getMinorRadius();
			float zrh = z * segment->getMinorRadius();
			glm::vec3 rotatedRightCap = getRotation() * glm::vec3(0.f, segment->getRightCap().y + yrh, segment->getRightCap().z + zrh);
			glm::vec3 rotatedLeftCap = getRotation() * glm::vec3(0.f, segment->getLeftCap().y - yrh, segment->getLeftCap().z + zrh);
			vs.emplace_back(rotatedRightCap + getPosition1(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedRightCap + getPosition2(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedLeftCap + getPosition1(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedLeftCap + getPosition2(), glm::vec2(), glm::vec3());
		}
		vs.emplace_back(getRotation() * segment->getRightCap() + getPosition1(), glm::vec2(), glm::vec3());
		vs.emplace_back(getRotation() * segment->getLeftCap() + getPosition1(), glm::vec2(), glm::vec3());

		for (int i = 0; i < SECTORS_PER_SEMICIRCLE * 4; i += 4) {
			// Join caps
			// Right
			is.push_back(i + 0);
			is.push_back(i + 1);
			is.push_back(i + 4);
			is.push_back(i + 4);
			is.push_back(i + 1);
			is.push_back(i + 5);
			// Left
			is.push_back(i + 2);
			is.push_back(i + 3);
			is.push_back(i + 6);
			is.push_back(i + 6);
			is.push_back(i + 3);
			is.push_back(i + 7);

			// Semicircles
			// Right
			is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4);
			is.push_back(i + 4);
			is.push_back(i + 0);
			// Left
			is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4 + 1);
			is.push_back(i + 2);
			is.push_back(i + 6);
		}

		// Join straight edges
		// Top
		is.push_back(0);
		is.push_back(1);
		is.push_back(3);
		is.push_back(0);
		is.push_back(3);
		is.push_back(2);
		// Bottom
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 0);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 1);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 3);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 0);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 3);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 2);
		// Front
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 0);
		is.push_back(0);
		is.push_back(2);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 0);
		is.push_back(2);
		is.push_back(SECTORS_PER_SEMICIRCLE * 4 + 2);
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * arc->getArcAngle() / PI);

		vs.reserve((SECTORS_PER_SEMICIRCLE + 1) * 4 + 2 + (NUM_SECTORS + 1) * 4);
		is.reserve(SECTORS_PER_SEMICIRCLE * 18 + NUM_SECTORS * 18);

		// Caps
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + arc->getHalfArcAngle();
			float y = std::sin(ang);
			float z = std::cos(ang);
			float yrh = y * arc->getMinorRadius();
			float zrh = z * arc->getMinorRadius();
			glm::vec3 rotatedRightCap = getRotation() * glm::vec3(0.f, arc->getRightCap().y + yrh, arc->getRightCap().z + zrh);
			glm::vec3 rotatedLeftCap = getRotation() * glm::vec3(0.f, arc->getLeftCap().y - yrh, arc->getLeftCap().z + zrh);
			vs.emplace_back(rotatedRightCap + getPosition1(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedRightCap + getPosition2(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedLeftCap + getPosition1(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedLeftCap + getPosition2(), glm::vec2(), glm::vec3());
		}
		vs.emplace_back(getRotation() * arc->getRightCap() + getPosition1(), glm::vec2(), glm::vec3());
		vs.emplace_back(getRotation() * arc->getLeftCap() + getPosition1(), glm::vec2(), glm::vec3());

		for (int i = 0; i < SECTORS_PER_SEMICIRCLE * 4; i += 4) {
			// Join caps
			// Right
			is.push_back(i + 0);
			is.push_back(i + 1);
			is.push_back(i + 4);
			is.push_back(i + 4);
			is.push_back(i + 1);
			is.push_back(i + 5);
			// Left
			is.push_back(i + 2);
			is.push_back(i + 3);
			is.push_back(i + 6);
			is.push_back(i + 6);
			is.push_back(i + 3);
			is.push_back(i + 7);

			// Semicircles
			// Right
			is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4);
			is.push_back(i + 4);
			is.push_back(i + 0);
			// Left
			is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4 + 1);
			is.push_back(i + 2);
			is.push_back(i + 6);
		}

		// Arc
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = (float)i / (float)NUM_SECTORS * arc->getArcAngle() - arc->getHalfArcAngle();
			float y = std::sin(ang);
			float z = std::cos(ang);
			float yroh = y * (arc->getArcRadius() + arc->getMinorRadius());
			float zroh = z * (arc->getArcRadius() + arc->getMinorRadius());
			float yrih = y * (arc->getArcRadius() - arc->getMinorRadius());
			float zrih = z * (arc->getArcRadius() - arc->getMinorRadius());
			glm::vec3 rotatedInner = getRotation() * glm::vec3(0.f, yrih, zrih);
			glm::vec3 rotatedOuter = getRotation() * glm::vec3(0.f, yroh, zroh);
			vs.emplace_back(rotatedInner + getPosition1(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedInner + getPosition2(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedOuter + getPosition1(), glm::vec2(), glm::vec3());
			vs.emplace_back(rotatedOuter + getPosition2(), glm::vec2(), glm::vec3());
		}
		constexpr int START_INDEX = 4 * (SECTORS_PER_SEMICIRCLE + 1) + 2;
		for (int i = START_INDEX; i < START_INDEX + NUM_SECTORS * 4; i += 4) {
			// Join Arcs
			// Inside
			is.push_back(i + 0);
			is.push_back(i + 1);
			is.push_back(i + 4);
			is.push_back(i + 4);
			is.push_back(i + 1);
			is.push_back(i + 5);
			// Outside
			is.push_back(i + 2);
			is.push_back(i + 3);
			is.push_back(i + 6);
			is.push_back(i + 6);
			is.push_back(i + 3);
			is.push_back(i + 7);
			// Front
			is.push_back(i + 0);
			is.push_back(i + 4);
			is.push_back(i + 6);
			is.push_back(i + 0);
			is.push_back(i + 6);
			is.push_back(i + 2);
		}
	}
}

void OscillatingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		bool fullCircle = std::abs(getAngle2() - getAngle1()) >= 2 * PI;
		float domainStartAngle, domainEndAngle;
		if (fullCircle) {
			domainStartAngle = 0.f;
			domainEndAngle = 2.f * PI;
		} else {
			domainStartAngle = std::min(getAngle1(), getAngle2());
			domainEndAngle = std::max(getAngle1(), getAngle2());
		}

		const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * (domainEndAngle - domainStartAngle) / PI);

		int vsSize = 1 + 2 * (NUM_SECTORS + 1);
		int isSize = 6 * NUM_SECTORS;
		if (!fullCircle) {
			vsSize += 2 * (2 + 2 * (SECTORS_PER_SEMICIRCLE + 1));
			isSize += 2 * (6 * SECTORS_PER_SEMICIRCLE + 6);
		}
		vs.reserve(vsSize);
		is.reserve(isSize);

		int START_INDEX = 0;
		if (!fullCircle) {
			for (float currentAngle : {domainStartAngle, domainEndAngle}) {
				glm::mat3 currentRot = angleToRotation(currentAngle);
				glm::vec3 start = currentRot * segment->getRightCap();
				glm::vec3 end = currentRot * segment->getLeftCap();

				vs.emplace_back(start, glm::vec2(), glm::vec3());
				vs.emplace_back(end, glm::vec2(), glm::vec3());
				for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
					float angStartCap = currentAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
					float angEndCap = currentAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
					vs.emplace_back(glm::vec3(0.f, start.y - std::sin(angStartCap) * segment->getMinorRadius(), start.z + std::cos(angStartCap) * segment->getMinorRadius()), glm::vec2(), glm::vec3());
					vs.emplace_back(glm::vec3(0.f, end.y - std::sin(angEndCap) * segment->getMinorRadius(), end.z + std::cos(angEndCap) * segment->getMinorRadius()), glm::vec2(), glm::vec3());
				}
				for (int i = START_INDEX + 2; i < START_INDEX + 2 + SECTORS_PER_SEMICIRCLE * 2; i += 2) {
					// Caps
					// Right
					is.push_back(START_INDEX + 0);
					is.push_back(i + 0);
					is.push_back(i + 2);
					// Left
					is.push_back(START_INDEX + 1);
					is.push_back(i + 1);
					is.push_back(i + 3);
				}

				START_INDEX += 2;

				is.push_back(START_INDEX + 0);
				is.push_back(START_INDEX + 1);
				is.push_back(START_INDEX + 1 + SECTORS_PER_SEMICIRCLE * 2);
				is.push_back(START_INDEX + 0);
				is.push_back(START_INDEX + 1 + SECTORS_PER_SEMICIRCLE * 2);
				is.push_back(START_INDEX + 0 + SECTORS_PER_SEMICIRCLE * 2);

				START_INDEX += (SECTORS_PER_SEMICIRCLE + 1) * 2;
			}
		}

		if (NUM_SECTORS != 0) {
			vs.emplace_back(glm::vec3(), glm::vec2(), glm::vec3());

			const float startRadius = glm::length(segment->getRightCap()) + segment->getMinorRadius();
			const float endRadius = glm::length(segment->getLeftCap()) + segment->getMinorRadius();
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = domainStartAngle + (float)i / (float)NUM_SECTORS * (domainEndAngle - domainStartAngle);
				glm::vec3 dir = glm::vec3(0.f, std::cos(ang), std::sin(ang));
				vs.emplace_back(dir * startRadius, glm::vec2(), glm::vec3());
				vs.emplace_back(dir * -endRadius, glm::vec2(), glm::vec3());
			}

			for (int i = START_INDEX + 1; i < START_INDEX + 1 + NUM_SECTORS * 2; i += 2) {
				is.push_back(START_INDEX);
				is.push_back(i + 0);
				is.push_back(i + 2);
				is.push_back(START_INDEX);
				is.push_back(i + 1);
				is.push_back(i + 3);
			}
		}
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		bool fullCircle = std::abs(getAngle2() - getAngle1()) + arc->getArcAngle() >= 2 * PI;
		float startAngle, endAngle;
		glm::vec3 start, end;
		if (fullCircle) {
			startAngle = 0.f;
			endAngle = 2.f * PI;
			start = end = {0.f, 0.f, arc->getArcRadius()};
		} else {
			startAngle = std::min(getAngle1(), getAngle2()) - arc->getHalfArcAngle();
			endAngle = std::max(getAngle1(), getAngle2()) + arc->getHalfArcAngle();
			start = glm::vec3(0.f, -std::sin(startAngle), std::cos(startAngle)) * arc->getArcRadius();
			end = glm::vec3(0.f, -std::sin(endAngle), std::cos(endAngle)) * arc->getArcRadius();
		}

		const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * (endAngle - startAngle) / PI);

		int vsSize = (NUM_SECTORS + 1) * 2;
		int isSize = NUM_SECTORS * 6;
		if (!fullCircle) {
			vsSize += 2 + (SECTORS_PER_SEMICIRCLE + 1) * 2;
			isSize += SECTORS_PER_SEMICIRCLE * 6;
		}
		vs.reserve(vsSize);
		is.reserve(isSize);

		short START_INDEX = 0;
		if (!fullCircle) {
			vs.emplace_back(start, glm::vec2(), glm::vec3());
			vs.emplace_back(end, glm::vec2(), glm::vec3());
			for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
				float angStartCap = startAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				float angEndCap = endAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				vs.emplace_back(glm::vec3(0.f, start.y - std::sin(angStartCap) * arc->getMinorRadius(), start.z + std::cos(angStartCap) * arc->getMinorRadius()), glm::vec2(), glm::vec3());
				vs.emplace_back(glm::vec3(0.f, end.y - std::sin(angEndCap) * arc->getMinorRadius(), end.z + std::cos(angEndCap) * arc->getMinorRadius()), glm::vec2(), glm::vec3());
			}
			for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 2; i += 2) {
				// Caps
				// Right
				is.push_back(0);
				is.push_back(i + 0);
				is.push_back(i + 2);
				// Left
				is.push_back(1);
				is.push_back(i + 1);
				is.push_back(i + 3);
			}

			START_INDEX = 2 + (SECTORS_PER_SEMICIRCLE + 1) * 2;
		}

		if (NUM_SECTORS != 0) {
			// Arc
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = startAngle + (float)i / (float)NUM_SECTORS * (endAngle - startAngle);
				glm::vec3 dir = glm::vec3(0.f, -std::sin(ang), std::cos(ang));
				vs.emplace_back(dir * (arc->getArcRadius() - arc->getMinorRadius()), glm::vec2(), glm::vec3());
				vs.emplace_back(dir * (arc->getArcRadius() + arc->getMinorRadius()), glm::vec2(), glm::vec3());
			}
			for (int i = START_INDEX; i < START_INDEX + NUM_SECTORS * 2; i += 2) {
				is.push_back(i + 0);
				is.push_back(i + 1);
				is.push_back(i + 3);
				is.push_back(i + 0);
				is.push_back(i + 3);
				is.push_back(i + 2);
			}
		}
	}
}


void StaticSpec::initKinematicState(ObstacleKinematicState& kinematicState) const {
	kinematicState.setPosition(getPosition());
	kinematicState.setAngle(getAngle());
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularVelocity(0.f);
}

void TogglingPositionSpec::initKinematicState(ObstacleKinematicState& kinematicState) const {
	kinematicState.setPosition(getPositionA());
	kinematicState.setAngle(getAngle());
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularVelocity(0.f);
}

void TogglingAngleSpec::initKinematicState(ObstacleKinematicState& kinematicState) const {
	kinematicState.setPosition(getPosition());
	kinematicState.setAngle(getAngleA());
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularVelocity(0.f);
}

void SpinningSpec::initKinematicState(ObstacleKinematicState& kinematicState) const {
	kinematicState.setPosition(getPosition());
	kinematicState.setAngle(getInitialAngle());
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularVelocity(getAngularVelocityA());
}

void OscillatingPositionSpec::initKinematicState(ObstacleKinematicState& kinematicState) const {
	kinematicState.setPosition(getPosition1());
	kinematicState.setAngle(getAngle());
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularVelocity(0.f);
	kinematicState.setPhase(0.f);
}

void OscillatingAngleSpec::initKinematicState(ObstacleKinematicState& kinematicState) const {
	kinematicState.setPosition(getPosition());
	kinematicState.setAngle(getAngle1());
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularVelocity(0.f);
	kinematicState.setPhase(0.f);
}


void TogglingPositionSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setPosition(glm::mix(getPositionA(), getPositionB(), smoother.getCurrentPosition()));
	kinematicState.setVelocity((getPositionB() - getPositionA()) * smoother.getCurrentVelocity());
}

void TogglingAngleSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngle(glm::mix(getAngleA(), getAngleB(), smoother.getCurrentPosition()));
	kinematicState.setAngularVelocity((getAngleB() - getAngleA()) * smoother.getCurrentVelocity());
}

void SpinningSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngularVelocity(glm::mix(getAngularVelocityA(), getAngularVelocityB(), smoother.getCurrentPosition()));
	kinematicState.setAngle(kinematicState.getAngle() + kinematicState.getAngularVelocity() * PHYSICS_TIMESTEP);
}

void OscillatingPositionSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	float angularFrequency = glm::mix(getAngularFrequencyA(), getAngularFrequencyB(), smoother.getCurrentPosition());
	kinematicState.setPhase(kinematicState.getPhase() + angularFrequency * PHYSICS_TIMESTEP);
	kinematicState.setPosition(glm::mix(getPosition1(), getPosition2(), 0.5f - 0.5f * std::cos(kinematicState.getPhase())));
	kinematicState.setVelocity((getPosition2() - getPosition1()) * (0.5f * std::sin(kinematicState.getPhase()) * angularFrequency));
}

void OscillatingAngleSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	float angularFrequency = glm::mix(getAngularFrequencyA(), getAngularFrequencyB(), smoother.getCurrentPosition());
	kinematicState.setPhase(kinematicState.getPhase() + angularFrequency * PHYSICS_TIMESTEP);
	kinematicState.setAngle(glm::mix(getAngle1(), getAngle2(), 0.5f - 0.5f * std::cos(kinematicState.getPhase())));
	kinematicState.setAngularVelocity((getAngle2() - getAngle1()) * (0.5f * std::sin(kinematicState.getPhase()) * angularFrequency));
}


void TogglingPositionSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setPosition(glm::mix(getPositionA(), getPositionB(), smoother.getCurrentPosition()));
}

void TogglingAngleSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngle(glm::mix(getAngleA(), getAngleB(), smoother.getCurrentPosition()));
}

void SpinningSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother&) const {
	kinematicState.setAngle(getInitialAngle());
}

void OscillatingPositionSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	bool toggled = smoother.getCurrentPosition() > 0.5f;
	kinematicState.setPhase(toggled ? PI : 0.f);
	kinematicState.setPosition(toggled ? getPosition2() : getPosition1());
}

void OscillatingAngleSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	bool toggled = smoother.getCurrentPosition() > 0.5f;
	kinematicState.setPhase(toggled ? PI : 0.f);
	kinematicState.setAngle(toggled ? getAngle2() : getAngle1());
}



ObstacleDescriptor::ObstacleDescriptor(const ObstacleDescriptor& other) :
	goal(other.goal), material(other.material) {
	if (other.shape)
		shape = other.shape->clone();
	if (other.motion)
		motion = other.motion->clone();
}

ObstacleDescriptor& ObstacleDescriptor::operator=(const ObstacleDescriptor& other) {
	if (this != &other) {
		if (other.shape)
			shape = other.shape->clone();
		if (other.motion)
			motion = other.motion->clone();
		goal = other.goal;
		material = other.material;
	}
	return *this;
}


ObstacleDescriptor::ObstacleDescriptor(const std::string& data) {
	std::istringstream ss(data);

	std::string shapeString, motionString;

	if (!(std::getline(ss, shapeString, '|') && std::getline(ss, motionString, '|') && ss >> goal))
		throw std::invalid_argument("Invalid obstacle data format");

	shape = std::move(AbstractShapeSpec::deserialize(shapeString));
	motion = std::move(IMotionSpec::deserialize(motionString));
	material = MAT_CONCRETE;
}


std::string ObstacleDescriptor::serialize() const {
	std::ostringstream ss;

	ss << shape->serialize() << "|" << motion->serialize() << "|" << isGoal();

	return ss.str();
}


void ObstacleDescriptor::scale(float factor) {
	shape->scale(factor);
	motion->scale(factor);
}


PlaneDescriptor SegmentSpec::getTopPlane(const ObstacleKinematicState& kinematicState) const {
	glm::vec3 normal = { 0, -std::sin(kinematicState.getAngle()), std::cos(kinematicState.getAngle()) };
	return { normal, kinematicState.getPosition() + kinematicState.getRotation() * glm::vec3(0.f, 0.f, getMinorRadius()) };
}


BallCollisionInfo SegmentSpec::getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) {
	PlaneDescriptor plane = getTopPlane(kinematicState);
	float separation = glm::dot(plane.normal, ball.getKinematicState()->position) - plane.dotProduct;
	if (separation < -getMinorRadius()) {
		plane.normal.y = -plane.normal.y;
		plane.normal.z = -plane.normal.z;
		plane.dotProduct = getMinorRadius() * 2.f - plane.dotProduct;
		separation = glm::dot(plane.normal, ball.getKinematicState()->position) - plane.dotProduct;
	}

	if (separation > 0.f && separation < ball.getProperties()->radius)
		return { true, plane.normal, separation };
	
	return {.colliding = false};
}

BallCollisionInfo ArcSpec::getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) {
	glm::vec3 centreToBall = ball.getKinematicState()->position - kinematicState.getPosition();
	float
	distanceToBallSq = glm::length2(centreToBall),
	innerRadius = getMajorRadius() - getMinorRadius(),
	outerRadius = getMajorRadius() + getMinorRadius(),
	innerRadii = innerRadius - ball.getProperties()->radius,
	outerRadii = outerRadius + ball.getProperties()->radius;

	if (distanceToBallSq < outerRadii * outerRadii && distanceToBallSq > innerRadii * innerRadii) { // In contact with banana
		if (distanceToBallSq > outerRadius * outerRadius) { // Beyond banana
			float distanceToBall = std::sqrt(distanceToBallSq);
			return { true, centreToBall / distanceToBall, distanceToBall - getMajorRadius() - getMinorRadius() };
		} if (distanceToBallSq < innerRadius * innerRadius) { // Within banana
			float distanceToBall = std::sqrt(distanceToBallSq);
			return { true, centreToBall / -distanceToBall, getMajorRadius() - distanceToBall - getMinorRadius() };
		}
	}

	return {.colliding = false};
}



void GameObstacle::stepKinematicState(const Smoother& smoother) {
	descriptor->getMotion()->stepKinematicState(kinematicState, smoother);
}


bool GameObstacle::collideWithCap(GameBall& ball, glm::vec3 cap) const {
	glm::vec3 capPosition = kinematicState.getPosition() + kinematicState.getRotation() * cap;
	glm::vec3 capToBall = ball.getKinematicState()->position - capPosition;
	float distanceToBallSq = glm::length2(capToBall);

	float radii = ball.getProperties()->radius + descriptor->getShape()->getMinorRadius();
	if (distanceToBallSq < radii * radii && distanceToBallSq > 0.000001f) {
		float distanceToBall = std::sqrt(distanceToBallSq);
		ball.collideWithPointOnObstacle(*this, capToBall / distanceToBall, distanceToBall - descriptor->getShape()->getMinorRadius());
		return true;
	}
	return false;
}

bool GameObstacle::collideWithMidsection(GameBall& ball) const {
	BallCollisionInfo collision = descriptor->getShape()->getMidsectionCollision(kinematicState, ball);
	if (collision.colliding)
		ball.collideWithPointOnObstacle(*this, collision.normal, collision.separation);
	return collision.colliding;
}

constexpr float SUCCEED_TIME = 0.5f;
bool GameObstacle::notifyOfContactWithBall(const GameBall& ball) {
	goalContactTimer += PHYSICS_TIMESTEP;
	return goalContactTimer >= SUCCEED_TIME && glm::length2(ball.getKinematicState()->velocity) < 0.000001f;
}


PlaneDescriptor GameObstacle::getCapDividingPlane(glm::vec3 cap, float capAngle) const {
	float angle = kinematicState.getAngle() + capAngle;
	glm::vec3 normal = { 0.f, std::cos(angle), std::sin(angle) };
	return { normal, kinematicState.getPosition() + kinematicState.getRotation() * cap };
}



void EditorObstacle::updateKinematicState(const Smoother& smoother, int numSteps) {
	if (numSteps < 0) // Just set 'stationary' attributes
		descriptor->getMotion()->updateEditorKinematicState(kinematicState, smoother);
	else // Demonstrate motion
		while (numSteps--)
			descriptor->getMotion()->stepKinematicState(kinematicState, smoother);
}