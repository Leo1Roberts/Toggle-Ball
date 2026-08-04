#include "ObstacleShape.h"

#include "GameBall.h"
#include "ObstacleKinematicState.h"
#include "Plane.h"
#include "SelectBox.h"
#include "Settings.h"
#include "Utilities.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"
#include <iomanip>
#include <sstream>


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


float AbstractShapeSpec::getBevel() const { return BEVEL_AMOUNT * getMinorRadius(); }


void SegmentSpec::setLeftLength(float len) {
	leftLength = len;
	leftCap = {-len, 0.f};
}
void SegmentSpec::setRightLength(float len) {
	rightLength = len;
	rightCap = {len, 0.f};
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
	float x = std::sin(getHalfArcAngle()) * getArcRadius();
	float y = std::cos(getHalfArcAngle()) * getArcRadius();
	leftCap = {-x, y};
	rightCap = {x, y};
}


bool AbstractShapeSpec::operator==(const AbstractShapeSpec& other) const {
	if (typeid(*this) != typeid(other))
		return false;

	return minorRadius == other.minorRadius && equals(other);
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
	vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap()), glm::vec2(), glm::vec3(1, 0, 0), color);
	vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap()), glm::vec2(), glm::vec3(1, 0, 0), color);
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
		float x = std::sin(ang);
		float y = std::cos(ang);
		float xrv = x * (getMinorRadius() - getBevel());
		float yrv = y * (getMinorRadius() - getBevel());
		float xrh = x * getMinorRadius();
		float yrh = y * getMinorRadius();
		vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap() + glm::vec2(xrv, yrv)), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getRightCap() + glm::vec2(xrh, yrh)), glm::vec2(), glm::vec3(0, x, y), color);
		vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap() + glm::vec2(-xrv, yrv)), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getLeftCap() + glm::vec2(-xrh, yrh)), glm::vec2(), glm::vec3(0, -x, y), color);
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
	const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() * glm::one_over_pi<float>());
	vs.reserve(2 + 4 * (SECTORS_PER_SEMICIRCLE + 1) + 4 * (NUM_SECTORS + 1));
	is.reserve(18 * SECTORS_PER_SEMICIRCLE + 18 * NUM_SECTORS);

	// Caps
	vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap()), glm::vec2(), glm::vec3(1, 0, 0), color);
	vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap()), glm::vec2(), glm::vec3(1, 0, 0), color);
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>() + getHalfArcAngle();
		float x = std::sin(ang);
		float y = std::cos(ang);
		float xrv = x * (getMinorRadius() - getBevel());
		float yrv = y * (getMinorRadius() - getBevel());
		float xrh = x * getMinorRadius();
		float yrh = y * getMinorRadius();
		vs.emplace_back(glm::vec3(getHalfDepth(), getRightCap() + glm::vec2(xrv, yrv)), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getRightCap() + glm::vec2(xrh, yrh)), glm::vec2(), glm::vec3(0, x, y), color);
		vs.emplace_back(glm::vec3(getHalfDepth(), getLeftCap() + glm::vec2(-xrv, yrv)), glm::vec2(), glm::vec3(1, 0, 0), color);
		vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), getLeftCap() + glm::vec2(-xrh, yrh)), glm::vec2(), glm::vec3(0, -x, y), color);
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

	if (NUM_SECTORS != 0) {
		// Arc
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = (float)i / (float)NUM_SECTORS * getArcAngle() - getHalfArcAngle();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float xrov = x * (getArcRadius() + getMinorRadius() - getBevel());
			float yrov = y * (getArcRadius() + getMinorRadius() - getBevel());
			float xriv = x * (getArcRadius() - getMinorRadius() + getBevel());
			float yriv = y * (getArcRadius() - getMinorRadius() + getBevel());
			float xroh = x * (getArcRadius() + getMinorRadius());
			float yroh = y * (getArcRadius() + getMinorRadius());
			float xrih = x * (getArcRadius() - getMinorRadius());
			float yrih = y * (getArcRadius() - getMinorRadius());
			vs.emplace_back(glm::vec3(getHalfDepth(), xriv, yriv), glm::vec2(), glm::vec3(1, 0, 0), color);
			vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), xrih, yrih), glm::vec2(), glm::vec3(0, -x, -y), color);
			vs.emplace_back(glm::vec3(getHalfDepth(), xrov, yrov), glm::vec2(), glm::vec3(1, 0, 0), color);
			vs.emplace_back(glm::vec3(getHalfDepth() - getBevel(), xroh, yroh), glm::vec2(), glm::vec3(0, x, y), color);
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
}


void SegmentSpec::buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	vs.reserve(2 + 2 * (SECTORS_PER_SEMICIRCLE + 1));
	is.reserve(6 * (SECTORS_PER_SEMICIRCLE + 1));

	// Caps
	vs.emplace_back(planarToWorld(getRightCap()), glm::vec2(), glm::vec3());
	vs.emplace_back(planarToWorld(getLeftCap()), glm::vec2(), glm::vec3());
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
		float x = std::sin(ang);
		float y = std::cos(ang);
		float xrh = x * getMinorRadius();
		float yrh = y * getMinorRadius();
		vs.emplace_back(planarToWorld(getRightCap() + glm::vec2(xrh, yrh)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getLeftCap() + glm::vec2(-xrh, yrh)), glm::vec2(), glm::vec3());
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
	const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() * glm::one_over_pi<float>());
	vs.reserve(2 + 2 * (SECTORS_PER_SEMICIRCLE + 1) + 2 * (NUM_SECTORS + 1));
	is.reserve(6 * SECTORS_PER_SEMICIRCLE + 6 * NUM_SECTORS);

	// Caps
	vs.emplace_back(planarToWorld(getRightCap()), glm::vec2(), glm::vec3());
	vs.emplace_back(planarToWorld(getLeftCap()), glm::vec2(), glm::vec3());
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>() + getHalfArcAngle();
		float x = std::sin(ang);
		float y = std::cos(ang);
		float xrh = x * getMinorRadius();
		float yrh = y * getMinorRadius();
		vs.emplace_back(planarToWorld(getRightCap() + glm::vec2(xrh, yrh)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getLeftCap() + glm::vec2(-xrh, yrh)), glm::vec2(), glm::vec3());
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

	if (NUM_SECTORS != 0) {
		// Arc
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = (float)i / (float)NUM_SECTORS * getArcAngle() - getHalfArcAngle();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float xroh = x * (getArcRadius() + getMinorRadius());
			float yroh = y * (getArcRadius() + getMinorRadius());
			float xrih = x * (getArcRadius() - getMinorRadius());
			float yrih = y * (getArcRadius() - getMinorRadius());
			vs.emplace_back(planarToWorld({xrih, yrih}), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld({xroh, yroh}), glm::vec2(), glm::vec3());
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
}


void AbstractShapeSpec::generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh, float uiToWorldScale) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;
	buildOutlineMesh(vs, is, uiToWorldScale);
	outlineMesh.setData(vs, is);
}

void SegmentSpec::buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const {
	vs.reserve(4 * (SECTORS_PER_SEMICIRCLE + 1));
	is.reserve(12 * SECTORS_PER_SEMICIRCLE + 12);

	const float outlineRadius = getMinorRadius() + uiToWorldScale * Settings::Sizes.outlineWidth;

	// Caps
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
		float x = std::sin(ang);
		float y = std::cos(ang);
		float xrh = x * getMinorRadius();
		float yrh = y * getMinorRadius();
		float xrv = x * outlineRadius;
		float yrv = y * outlineRadius;
		vs.emplace_back(planarToWorld(getRightCap() + glm::vec2(xrh, yrh)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getRightCap() + glm::vec2(xrv, yrv)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getLeftCap() + glm::vec2(-xrh, yrh)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getLeftCap() + glm::vec2(-xrv, yrv)), glm::vec2(), glm::vec3());
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

void ArcSpec::buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const {
	const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() * glm::one_over_pi<float>());
	vs.reserve(4 * (SECTORS_PER_SEMICIRCLE + 1) + 4 * (NUM_SECTORS + 1));
	is.reserve(12 * SECTORS_PER_SEMICIRCLE + 12 * NUM_SECTORS);

	const float outlineRadius = getMinorRadius() + uiToWorldScale * Settings::Sizes.outlineWidth;

	// Caps
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>() + getHalfArcAngle();
		float x = std::sin(ang);
		float y = std::cos(ang);
		float xrh = x * getMinorRadius();
		float yrh = y * getMinorRadius();
		float xrv = x * outlineRadius;
		float yrv = y * outlineRadius;
		vs.emplace_back(planarToWorld(getRightCap() + glm::vec2(xrh, yrh)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getRightCap() + glm::vec2(xrv, yrv)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getLeftCap() + glm::vec2(-xrh, yrh)), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(getLeftCap() + glm::vec2(-xrv, yrv)), glm::vec2(), glm::vec3());
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

	if (NUM_SECTORS != 0) {
		// Arc
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = (float)i / (float)NUM_SECTORS * getArcAngle() - getHalfArcAngle();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float xroh = x * (getArcRadius() + getMinorRadius());
			float yroh = y * (getArcRadius() + getMinorRadius());
			float xrih = x * (getArcRadius() - getMinorRadius());
			float yrih = y * (getArcRadius() - getMinorRadius());
			float xrov = x * (getArcRadius() + outlineRadius);
			float yrov = y * (getArcRadius() + outlineRadius);
			float xriv = x * (getArcRadius() - outlineRadius);
			float yriv = y * (getArcRadius() - outlineRadius);
			vs.emplace_back(planarToWorld({xrih, yrih}), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld({xriv, yriv}), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld({xroh, yroh}), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld({xrov, yrov}), glm::vec2(), glm::vec3());
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
}


bool AbstractShapeSpec::isInSelectBox(const ObstacleKinematicState& s, SelectBox box) const {
	// Check if caps are in the box
	const glm::vec2 leftCapPlanarPosition = worldToPlanar(s.getPosition() + s.getRotation() * planarToWorld(getLeftCap()));
	const glm::vec2 rightCapPlanarPosition = worldToPlanar(s.getPosition() + s.getRotation() * planarToWorld(getRightCap()));
	if (box.touchesCircle(leftCapPlanarPosition, getMinorRadius()) || box.touchesCircle(rightCapPlanarPosition, getMinorRadius()))
		return true;

	// Check if the cap-connecting midsection is in the box
	return midsectionIsInSelectBox(s, box);
}

// Helper: Checks if a 2D line segment P0->P1 intersects an AABB (SelectBox) using Liang-Barsky clipping. [AI GENERATED]
static bool segmentIntersectsBox(glm::vec2 p0, glm::vec2 p1, SelectBox box) {
    float tmin = 0.f;
    float tmax = 1.f;

    // Check X axis (box.left to box.right)
    float dx = p1.x - p0.x;
    if (std::abs(dx) == 0.f) {
        if (p0.x < box.left || p0.x > box.right) return false;
    } else {
        float invD = 1.f / dx;
        float t1 = (box.left - p0.x) * invD;
        float t2 = (box.right - p0.x) * invD;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }

    // Check Y axis (box.bottom to box.top)
    float dy = p1.y - p0.y;
    if (std::abs(dy) == 0.f) {
        if (p0.y < box.bottom || p0.y > box.top) return false;
    } else {
        float invD = 1.f / dy;
        float t1 = (box.bottom - p0.y) * invD;
        float t2 = (box.top - p0.y) * invD;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }

    return tmin <= tmax;
}

// Helper: Checks if an angle difference falls within the arc's angular range [-halfArc, halfArc]. [AI GENERATED]
static bool angleIsInArc(glm::vec2 diff, float centreAngle, float halfArcAngle, bool fullCircle) {
    if (fullCircle) return true;
    float angle = std::atan2(diff.y, diff.x);
    float delta = std::atan2(std::sin(angle - centreAngle), std::cos(angle - centreAngle));
    return std::abs(delta) <= halfArcAngle;
}

// Helper: Checks if a circular arc intersects an AABB (SelectBox). [AI GENERATED]
static bool arcIntersectsBox(glm::vec2 centre, float r, float centreAngle, float halfArcAngle, bool fullCircle, SelectBox box) {
    auto isPointInBox = [](glm::vec2 p, SelectBox b) {
        return p.x >= b.left && p.x <= b.right && p.y >= b.bottom && p.y <= b.top;
    };

    // Check arc endpoints (or a sample point for a full circle)
    if (fullCircle) {
        if (isPointInBox(centre + glm::vec2(r, 0.f), box)) return true;
    } else {
        glm::vec2 e1 = centre + r * glm::vec2(std::cos(centreAngle - halfArcAngle), std::sin(centreAngle - halfArcAngle));
        glm::vec2 e2 = centre + r * glm::vec2(std::cos(centreAngle + halfArcAngle), std::sin(centreAngle + halfArcAngle));
        if (isPointInBox(e1, box) || isPointInBox(e2, box)) return true;
    }

    // Check vertical box sides (X = side, Y in [bottom, top])
    float rSq = r * r;
    for (float xSide : {box.left, box.right}) {
        float dx = xSide - centre.x;
        float distSq = dx * dx;
        if (distSq <= rSq) {
            float dy = std::sqrt(rSq - distSq);
            for (float y : {centre.y + dy, centre.y - dy})
                if (y >= box.bottom && y <= box.top &&
                	angleIsInArc(glm::vec2(xSide, y) - centre, centreAngle, halfArcAngle, fullCircle))
                    return true;
        }
    }

    // Check horizontal box sides (Y = side, X in [left, right])
    for (float ySide : {box.bottom, box.top}) {
        float dy = ySide - centre.y;
        float distSq = dy * dy;
        if (distSq <= rSq) {
            float dx = std::sqrt(rSq - distSq);
            for (float x : {centre.x + dx, centre.x - dx})
                if (x >= box.left && x <= box.right &&
                	angleIsInArc(glm::vec2(x, ySide) - centre, centreAngle, halfArcAngle, fullCircle))
                    return true;
        }
    }

    return false;
}


bool SegmentSpec::midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const {
    glm::vec2 leftCapPos = worldToPlanar(s.getPosition() + s.getRotation() * planarToWorld(getLeftCap()));
	glm::vec2 rightCapPos = worldToPlanar(s.getPosition() + s.getRotation() * planarToWorld(getRightCap()));

	glm::vec2 leftToRight = rightCapPos - leftCapPos;
	float lengthSq = length2(leftToRight);
	if (lengthSq == 0.f) return false;

    glm::vec2 upOffset = worldToPlanar(s.getRotation() * planarToWorld({0.f, getMinorRadius()}));

    if (segmentIntersectsBox(leftCapPos + upOffset, rightCapPos + upOffset, box) ||
        segmentIntersectsBox(leftCapPos - upOffset, rightCapPos - upOffset, box))
        return true;

    // Check if the box is fully inside the midsection
    glm::vec2 boxCentre = {(box.left + box.right) * 0.5f, (box.bottom + box.top) * 0.5f};
    glm::vec2 leftCapToBox = boxCentre - leftCapPos;

    float x = dot(leftCapToBox, leftToRight) / lengthSq; // Projection along length
    float y = dot(leftCapToBox, upOffset) / length2(upOffset); // Projection along width

    return x >= 0.f && x <= 1.f && std::abs(y) <= 1.f;
}

bool ArcSpec::midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const {
    glm::vec2 centre = worldToPlanar(s.getPosition());
    float centreAngle = s.getAngle() + glm::half_pi<float>();
    bool fullCircle = getArcAngle() >= glm::two_pi<float>();

    float rInner = getArcRadius() - getMinorRadius();
    float rOuter = getArcRadius() + getMinorRadius();

    if (arcIntersectsBox(centre, rOuter, centreAngle, getHalfArcAngle(), fullCircle, box) ||
        arcIntersectsBox(centre, rInner, centreAngle, getHalfArcAngle(), fullCircle, box))
        return true;

    // Check if the box is fully inside the midsection
    glm::vec2 boxCentre = {(box.left + box.right) * 0.5f, (box.bottom + box.top) * 0.5f};
    glm::vec2 centreToBox = boxCentre - centre;
    float centreToBoxDistance = length(centreToBox);

    return centreToBoxDistance >= rInner && centreToBoxDistance <= rOuter &&
    	   angleIsInArc(centreToBox, centreAngle, getHalfArcAngle(), fullCircle);
}


PlaneDescriptor SegmentSpec::getTopPlane(const ObstacleKinematicState& kinematicState) const {
	glm::vec3 normal = planarToWorld({-std::sin(kinematicState.getAngle()), std::cos(kinematicState.getAngle())});
	return { normal, kinematicState.getPosition() + kinematicState.getRotation() * planarToWorld({0.f, getMinorRadius()}) };
}


BallCollisionInfo SegmentSpec::getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) {
	PlaneDescriptor plane = getTopPlane(kinematicState);
	float separation = dot(plane.normal, ball.getKinematicState()->position) - plane.dotProduct;
	if (separation < -getMinorRadius()) {
		plane.normal.y = -plane.normal.y;
		plane.normal.z = -plane.normal.z;
		plane.dotProduct = getMinorRadius() * 2.f - plane.dotProduct;
		separation = dot(plane.normal, ball.getKinematicState()->position) - plane.dotProduct;
	}

	if (separation > 0.f && separation < ball.getProperties()->radius)
		return { true, plane.normal, separation };

	return {.colliding = false};
}

BallCollisionInfo ArcSpec::getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) {
	glm::vec3 centreToBall = ball.getKinematicState()->position - kinematicState.getPosition();
	float
	distanceToBallSq = length2(centreToBall),
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
			glm::vec3 normal = distanceToBallSq > 0.000001f ? centreToBall / -distanceToBall : kinematicState.getRotation() * planarToWorld({0.f, -1.f});
			return { true, normal, getMajorRadius() - distanceToBall - getMinorRadius() };
		}
	}

	return {.colliding = false};
}
