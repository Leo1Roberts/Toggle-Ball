#include "main.h"
#include "Colors.h"
#include "Mesh.h"
#include "Obstacle.h"
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <sstream>

static void angleToRotation(float radians, mat3& rotation) {
	rotation.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, radians);
}


void KinematicState::setAngle(float radians) {
	angle = wrapAngle(radians);
	angleToRotation(radians, rotation);
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
	else if (ArcSpec::getTypeStringStatic() == shapeType)
		return std::make_unique<ArcSpec>(minorRadius, shapeData);
	else
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
	leftCap.y = -l;
}
void SegmentSpec::setRightLength(float l) {
	rightLength = l;
	rightCap.y = l;
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
	float y = sin(getHalfArcAngle()) * getArcRadius();
	float z = cos(getHalfArcAngle()) * getArcRadius();
	leftCap.set(0, -y, z);
	rightCap.set(0, y, z);
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
	vs.emplace_back(vec3(getHalfDepth(), getRightCap().y, 0), vec2(), vec3(1, 0, 0), color);
	vs.emplace_back(vec3(getHalfDepth(), getLeftCap().y, 0), vec2(), vec3(1, 0, 0), color);
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
		float y = sin(ang);
		float z = cos(ang);
		float yrv = y * (getMinorRadius() - getBevel());
		float zrv = z * (getMinorRadius() - getBevel());
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(vec3(getHalfDepth(), getRightCap().y + yrv, zrv), vec2(), vec3(1, 0, 0), color);
		vs.emplace_back(vec3(getHalfDepth() - getBevel(), getRightCap().y + yrh, zrh), vec2(), vec3(0, y, z), color);
		vs.emplace_back(vec3(getHalfDepth(), getLeftCap().y - yrv, zrv), vec2(), vec3(1, 0, 0), color);
		vs.emplace_back(vec3(getHalfDepth() - getBevel(), getLeftCap().y - yrh, zrh), vec2(), vec3(0, -y, z), color);
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
	const int NUM_SECTORS = (int)ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() / PI);
	vs.reserve(2 + 4 * (SECTORS_PER_SEMICIRCLE + 1) + 4 * (NUM_SECTORS + 1));
	is.reserve(18 * SECTORS_PER_SEMICIRCLE + 18 * NUM_SECTORS);

	// Caps
	vs.emplace_back(vec3(getHalfDepth(), getRightCap().y, getRightCap().z), vec2(), vec3(1, 0, 0), color);
	vs.emplace_back(vec3(getHalfDepth(), getLeftCap().y, getLeftCap().z), vec2(), vec3(1, 0, 0), color);
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
		float y = sin(ang);
		float z = cos(ang);
		float yrv = y * (getMinorRadius() - getBevel());
		float zrv = z * (getMinorRadius() - getBevel());
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(vec3(getHalfDepth(), getRightCap().y + yrv, getRightCap().z + zrv), vec2(), vec3(1, 0, 0), color);
		vs.emplace_back(vec3(getHalfDepth() - getBevel(), getRightCap().y + yrh, getRightCap().z + zrh), vec2(), vec3(0, y, z), color);
		vs.emplace_back(vec3(getHalfDepth(), getLeftCap().y - yrv, getLeftCap().z + zrv), vec2(), vec3(1, 0, 0), color);
		vs.emplace_back(vec3(getHalfDepth() - getBevel(), getLeftCap().y - yrh, getLeftCap().z + zrh), vec2(), vec3(0, -y, z), color);
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
		float y = sin(ang);
		float z = cos(ang);
		float yrov = y * (getArcRadius() + getMinorRadius() - getBevel());
		float zrov = z * (getArcRadius() + getMinorRadius() - getBevel());
		float yriv = y * (getArcRadius() - getMinorRadius() + getBevel());
		float zriv = z * (getArcRadius() - getMinorRadius() + getBevel());
		float yroh = y * (getArcRadius() + getMinorRadius());
		float zroh = z * (getArcRadius() + getMinorRadius());
		float yrih = y * (getArcRadius() - getMinorRadius());
		float zrih = z * (getArcRadius() - getMinorRadius());
		vs.emplace_back(vec3(getHalfDepth(), yriv, zriv), vec2(), vec3(1, 0, 0), color);
		vs.emplace_back(vec3(getHalfDepth() - getBevel(), yrih, zrih), vec2(), vec3(0, -y, -z), color);
		vs.emplace_back(vec3(getHalfDepth(), yrov, zrov), vec2(), vec3(1, 0, 0), color);
		vs.emplace_back(vec3(getHalfDepth() - getBevel(), yroh, zroh), vec2(), vec3(0, y, z), color);
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
	vs.emplace_back(vec3(0, getRightCap().y, 0), vec2(), vec3());
	vs.emplace_back(vec3(0, getLeftCap().y, 0), vec2(), vec3());
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
		float y = sin(ang);
		float z = cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(vec3(0, getRightCap().y + yrh, zrh), vec2(), vec3());
		vs.emplace_back(vec3(0, getLeftCap().y - yrh, zrh), vec2(), vec3());
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
	const int NUM_SECTORS = (int)ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() / PI);
	vs.reserve(2 + 2 * (SECTORS_PER_SEMICIRCLE + 1) + 2 * (NUM_SECTORS + 1));
	is.reserve(6 * SECTORS_PER_SEMICIRCLE + 6 * NUM_SECTORS);

	// Caps
	vs.emplace_back(vec3(0, getRightCap().y, getRightCap().z), vec2(), vec3());
	vs.emplace_back(vec3(0, getLeftCap().y, getLeftCap().z), vec2(), vec3());
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
		float y = sin(ang);
		float z = cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		vs.emplace_back(vec3(0, getRightCap().y + yrh, getRightCap().z + zrh), vec2(), vec3());
		vs.emplace_back(vec3(0, getLeftCap().y - yrh, getLeftCap().z + zrh), vec2(), vec3());
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
		float y = sin(ang);
		float z = cos(ang);
		float yroh = y * (getArcRadius() + getMinorRadius());
		float zroh = z * (getArcRadius() + getMinorRadius());
		float yrih = y * (getArcRadius() - getMinorRadius());
		float zrih = z * (getArcRadius() - getMinorRadius());
		vs.emplace_back(vec3(0, yrih, zrih), vec2(), vec3());
		vs.emplace_back(vec3(0, yroh, zroh), vec2(), vec3());
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
		float y = sin(ang);
		float z = cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		float yrv = y * getOutlineRadius();
		float zrv = z * getOutlineRadius();
		vs.emplace_back(vec3(0, getRightCap().y + yrh, getRightCap().z + zrh), vec2(), vec3());
		vs.emplace_back(vec3(0, getRightCap().y + yrv, getRightCap().z + zrv), vec2(), vec3());
		vs.emplace_back(vec3(0, getLeftCap().y - yrh, getLeftCap().z + zrh), vec2(), vec3());
		vs.emplace_back(vec3(0, getLeftCap().y - yrv, getLeftCap().z + zrv), vec2(), vec3());
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
	const int NUM_SECTORS = (int)ceil((float)SECTORS_PER_SEMICIRCLE * getArcAngle() / PI);
	vs.reserve(4 * (SECTORS_PER_SEMICIRCLE + 1) + 4 * (NUM_SECTORS + 1));
	is.reserve(12 * SECTORS_PER_SEMICIRCLE + 12 * NUM_SECTORS);

	// Caps
	for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
		float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
		float y = sin(ang);
		float z = cos(ang);
		float yrh = y * getMinorRadius();
		float zrh = z * getMinorRadius();
		float yrv = y * getOutlineRadius();
		float zrv = z * getOutlineRadius();
		vs.emplace_back(vec3(0, getRightCap().y + yrh, getRightCap().z + zrh), vec2(), vec3());
		vs.emplace_back(vec3(0, getRightCap().y + yrv, getRightCap().z + zrv), vec2(), vec3());
		vs.emplace_back(vec3(0, getLeftCap().y - yrh, getLeftCap().z + zrh), vec2(), vec3());
		vs.emplace_back(vec3(0, getLeftCap().y - yrv, getLeftCap().z + zrv), vec2(), vec3());
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
		float y = sin(ang);
		float z = cos(ang);
		float yroh = y * (getArcRadius() + getMinorRadius());
		float zroh = z * (getArcRadius() + getMinorRadius());
		float yrih = y * (getArcRadius() - getMinorRadius());
		float zrih = z * (getArcRadius() - getMinorRadius());
		float yrov = y * (getArcRadius() + getOutlineRadius());
		float zrov = z * (getArcRadius() + getOutlineRadius());
		float yriv = y * (getArcRadius() - getOutlineRadius());
		float zriv = z * (getArcRadius() - getOutlineRadius());
		vs.emplace_back(vec3(0, yrih, zrih), vec2(), vec3());
		vs.emplace_back(vec3(0, yriv, zriv), vec2(), vec3());
		vs.emplace_back(vec3(0, yroh, zroh), vec2(), vec3());
		vs.emplace_back(vec3(0, yrov, zrov), vec2(), vec3());
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


std::array<float, 2> quadraticFormula(float a, float b, float c) {
	float d = b*b - 4 * a * c;
	if (d < 0) return { NAN, NAN };
	float sqrtD = sqrt(d);
	float mul = 0.5f / a;
	return { mul * (-b + sqrtD), mul * (-b - sqrtD) };
}

bool AbstractShapeSpec::isInSelectBox(const KinematicState& s, const SelectBox& box) const {
	// Check if caps are in the box

	const vec3 leftCapPos = s.getPosition() + s.getRotation() * getLeftCap();
	const vec3 rightCapPos = s.getPosition() + s.getRotation() * getRightCap();

	for (const vec3& capPos : {leftCapPos, rightCapPos})
		if (box.left < capPos.y && capPos.y < box.right &&
		    box.bottom < capPos.z && capPos.z < box.top)
			return true; // Quick check - centre of a cap is inside the box

	float rSq = getMinorRadius() * getMinorRadius();

	for (const vec3& capPos : {leftCapPos, rightCapPos})
		for (auto [side, sideStart, sideEnd, perp, para] :
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

bool SegmentSpec::midsectionIsInSelectBox(const KinematicState& s, const SelectBox& box) const {
	const vec3 leftCapPos = s.getPosition() + s.getRotation() * getLeftCap();
	const vec3 rightCapPos = s.getPosition() + s.getRotation() * getRightCap();
	vec3 upOffset = s.getRotation() * vec3(0, 0, getMinorRadius());

	for (vec3 offset : {upOffset, -upOffset}) {
		vec3 segmentLeft = leftCapPos + upOffset;
		vec3 segmentRight = rightCapPos + upOffset;

		for (auto [side, sideStart, sideEnd, leftPerp, rightPerp, leftPara, rightPara] :
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

bool ArcSpec::midsectionIsInSelectBox(const KinematicState& s, const SelectBox& box) const {
	float centerAngle = s.getAngle() + PI * 0.5f;
	bool fullCircle = getArcAngle() >= 2 * PI;

	for (float radiusOffset : {getMinorRadius(), -getMinorRadius()}) {
		float r = getArcRadius() + radiusOffset;
		float rSq = r * r;

		for (auto [side, sideStart, sideEnd, centerPerp, centerPara, vertical] :
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
						float relativeAngle = wrapAngle(atan2(z - s.getPosition().z, y - s.getPosition().y) - centerAngle);
						if (abs(relativeAngle) < getHalfArcAngle())
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
	else if (TogglingPositionSpec::getTypeStringStatic() == motionType)
		return std::make_unique<TogglingPositionSpec>(motionData);
	else if (TogglingAngleSpec::getTypeStringStatic() == motionType)
		return std::make_unique<TogglingAngleSpec>(motionData);
	else if (SpinningSpec::getTypeStringStatic() == motionType)
		return std::make_unique<SpinningSpec>(motionData);
	else if (OscillatingPositionSpec::getTypeStringStatic() == motionType)
		return std::make_unique<OscillatingPositionSpec>(motionData);
	else if (OscillatingAngleSpec::getTypeStringStatic() == motionType)
		return std::make_unique<OscillatingAngleSpec>(motionData);
	else
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
	angleToRotation(radians, rotation);
}

void TogglingPositionSpec::setAngle(float radians) {
	angle = radians;
	angleToRotation(radians, rotation);
}

void OscillatingPositionSpec::setAngle(float radians) {
	angle = radians;
	angleToRotation(radians, rotation);
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

	vec3 diff = getPositionA() - getPositionB();
	float line1Length, line2Length;
	line1Length = line2Length = diff.length();
	vec3 diffUnit = diff / line1Length;
	float diffAngle = atan2(diff.z, diff.y);
	vec3 topPointA, bottomPointA;
	vec3 diffPerpUnit = vec3(0, -diff.z, diff.y) / line1Length;

	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vec3 diffPerp = diffPerpUnit * segment->getMinorRadius();
		if (wrapAngle(getAngle() - diffAngle) > 0) {
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
			float startAng = abs(wrapAngle(diffAngle - getAngle() + arc->getHalfArcAngle()));
			float endAng = abs(wrapAngle(diffAngle - getAngle() - arc->getHalfArcAngle()));
			if (abs(startAng - endAng) < 0.001f) { // Equal
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
			float startAng = abs(wrapAngle(diffAngle - getAngle() + arc->getHalfArcAngle() + PI));
			float endAng = abs(wrapAngle(diffAngle - getAngle() - arc->getHalfArcAngle() + PI));
			if (abs(startAng - endAng) < 0.001f) { // Equal
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

	float dotSpacing = OUTLINE_WIDTH_WORLD * 2;
	vec3 dotShift = diffUnit * dotSpacing;

	vec3 start1 = topPointA - diffPerpUnit / 2 * OUTLINE_WIDTH_WORLD + diffUnit * (line1Length - (dotSpacing * (float)(numDots1 - 1))) / 2;
	vec3 start2 = bottomPointA + diffPerpUnit / 2 * OUTLINE_WIDTH_WORLD + diffUnit * (line2Length - (dotSpacing * (float)(numDots2 - 1))) / 2;

	if (drawDots) {
		vec3 dotCentre = start1;
		for (int d = 0; d < numDots1; d++) {
			vs.emplace_back(dotCentre, vec2(), vec3());
			for (int i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
				vs.emplace_back(dotCentre + vec3(0, cos(ang), sin(ang)) * OUTLINE_WIDTH_WORLD / 2, vec2(), vec3());
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
			vs.emplace_back(dotCentre, vec2(), vec3());
			for (int i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
				vs.emplace_back(dotCentre + vec3(0, cos(ang), sin(ang)) * OUTLINE_WIDTH_WORLD / 2, vec2(), vec3());
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

	mat3 rot = getRotation();
	for (const vec3& pos : {getPositionA(), getPositionB()}) {
		Index offset = (Index)vs.size();

		std::transform(vs_shadow.begin(), vs_shadow.end(), std::back_inserter(vs), [pos, rot](const ObjectVertex& v) {
			return ObjectVertex(pos + rot * v.position, v.uv, v.normal, v.color);
		});

		std::transform(is_shadow.begin(), is_shadow.end(), std::back_inserter(is), [offset](Index i) {
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
		dotsArcAngle = std::clamp(getAngleB() - getAngleA(), -2 * PI, 2 * PI);
		dotsArc1Radius = segment->getRightCap().length() + segment->getMinorRadius() - OUTLINE_WIDTH_WORLD / 2;
		dotsArc2Radius = segment->getLeftCap().length() + segment->getMinorRadius() - OUTLINE_WIDTH_WORLD / 2;
		start1 = getAngleA();
		start2 = getAngleA() + PI;
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		float angDiff = getAngleB() - getAngleA();
		if (angDiff < 0) {
			dotsArcAngle = std::clamp(angDiff + arc->getArcAngle(), -2 * PI + arc->getArcAngle(), 0.0f);
			start1 = start2 = getAngleA() + PI / 2 - arc->getHalfArcAngle();
		} else {
			dotsArcAngle = std::clamp(angDiff - arc->getArcAngle(), 0.0f, 2 * PI - arc->getArcAngle());
			start1 = start2 = getAngleA() + PI / 2 + arc->getHalfArcAngle();
		}
		dotsArc1Radius = arc->getArcRadius() + arc->getMinorRadius() - OUTLINE_WIDTH_WORLD / 2;
		dotsArc2Radius = arc->getArcRadius() - arc->getMinorRadius() + OUTLINE_WIDTH_WORLD / 2;
	} else return;

	float absArcAngle = abs(dotsArcAngle);
	float sign = dotsArcAngle < 0 ? -1.0f : 1.0f;

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
		vec3 dotCentre = vec3(0, cos(dotAngle), sin(dotAngle)) * dotsArc1Radius;
		vs.emplace_back(dotCentre, vec2(), vec3());
		for (int i = 0; i < SECTORS_PER_DOT; i++) {
			float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
			vs.emplace_back(dotCentre + vec3(0, cos(ang), sin(ang)) * OUTLINE_WIDTH_WORLD / 2, vec2(), vec3());
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
		vec3 dotCentre = vec3(0, cos(dotAngle), sin(dotAngle)) * dotsArc2Radius;
		vs.emplace_back(dotCentre, vec2(), vec3());
		for (int i = 0; i < SECTORS_PER_DOT; i++) {
			float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
			vs.emplace_back(dotCentre + vec3(0, cos(ang), sin(ang)) * OUTLINE_WIDTH_WORLD / 2, vec2(), vec3());
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
		Index offset = (Index)vs.size();

		mat3 rot;
		angleToRotation(angle, rot);
		std::transform(vs_shadow.begin(), vs_shadow.end(), std::back_inserter(vs), [rot](const ObjectVertex& v) {
			return ObjectVertex(rot * v.position, v.uv, v.normal, v.color);
		});

		std::transform(is_shadow.begin(), is_shadow.end(), std::back_inserter(is), [offset](Index i) {
			return offset + i;
		});
	}
}

void SpinningSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vs.reserve(1 + SECTORS_PER_CIRCLE);
		is.reserve(SECTORS_PER_CIRCLE * 3);

		vs.emplace_back(vec3(), vec2(), vec3());
		for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = sin(ang);
			float z = cos(ang);
			float radius = std::max(segment->getLeftLength(), segment->getRightLength()) + segment->getMinorRadius();
			vs.emplace_back(vec3(0, y * radius, z * radius), vec2(), vec3());
			is.push_back(0);
			is.push_back(1 + (i + 1) % SECTORS_PER_CIRCLE);
			is.push_back(1 + i);
		}
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		vs.reserve(2 * SECTORS_PER_CIRCLE);
		is.reserve(SECTORS_PER_CIRCLE * 6);

		for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = sin(ang);
			float z = cos(ang);
			float innerRadius = arc->getArcRadius() - arc->getMinorRadius();
			float outerRadius = arc->getArcRadius() + arc->getMinorRadius();
			vs.emplace_back(vec3(0, y * innerRadius, z * innerRadius), vec2(), vec3());
			vs.emplace_back(vec3(0, y * outerRadius, z * outerRadius), vec2(), vec3());
			is.push_back(i*2);
			is.push_back(i*2 + 1);
			is.push_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
			is.push_back(i*2);
			is.push_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
			is.push_back((i*2 + 2) % (SECTORS_PER_CIRCLE * 2));
		}
	} else return;
}

void OscillatingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vs.reserve((SECTORS_PER_SEMICIRCLE + 1) * 4 + 2);
		is.reserve((SECTORS_PER_SEMICIRCLE + 1) * 18);

		// Caps
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = sin(ang);
			float z = cos(ang);
			float yrh = y * segment->getMinorRadius();
			float zrh = z * segment->getMinorRadius();
			vec3 rotatedRightCap = getRotation() * vec3(0, segment->getRightCap().y + yrh, segment->getRightCap().z + zrh);
			vec3 rotatedLeftCap = getRotation() * vec3(0, segment->getLeftCap().y - yrh, segment->getLeftCap().z + zrh);
			vs.emplace_back(rotatedRightCap + getPosition1(), vec2(), vec3());
			vs.emplace_back(rotatedRightCap + getPosition2(), vec2(), vec3());
			vs.emplace_back(rotatedLeftCap + getPosition1(), vec2(), vec3());
			vs.emplace_back(rotatedLeftCap + getPosition2(), vec2(), vec3());
		}
		vs.emplace_back(getRotation() * segment->getRightCap() + getPosition1(), vec2(), vec3());
		vs.emplace_back(getRotation() * segment->getLeftCap() + getPosition1(), vec2(), vec3());

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
		const int NUM_SECTORS = (int)ceil((float)SECTORS_PER_SEMICIRCLE * arc->getArcAngle() / PI);

		vs.reserve((SECTORS_PER_SEMICIRCLE + 1) * 4 + 2 + (NUM_SECTORS + 1) * 4);
		is.reserve(SECTORS_PER_SEMICIRCLE * 18 + NUM_SECTORS * 18);

		// Caps
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + arc->getHalfArcAngle();
			float y = sin(ang);
			float z = cos(ang);
			float yrh = y * arc->getMinorRadius();
			float zrh = z * arc->getMinorRadius();
			vec3 rotatedRightCap = getRotation() * vec3(0, arc->getRightCap().y + yrh, arc->getRightCap().z + zrh);
			vec3 rotatedLeftCap = getRotation() * vec3(0, arc->getLeftCap().y - yrh, arc->getLeftCap().z + zrh);
			vs.emplace_back(rotatedRightCap + getPosition1(), vec2(), vec3());
			vs.emplace_back(rotatedRightCap + getPosition2(), vec2(), vec3());
			vs.emplace_back(rotatedLeftCap + getPosition1(), vec2(), vec3());
			vs.emplace_back(rotatedLeftCap + getPosition2(), vec2(), vec3());
		}
		vs.emplace_back(getRotation() * arc->getRightCap() + getPosition1(), vec2(), vec3());
		vs.emplace_back(getRotation() * arc->getLeftCap() + getPosition1(), vec2(), vec3());

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
			float y = sin(ang);
			float z = cos(ang);
			float yroh = y * (arc->getArcRadius() + arc->getMinorRadius());
			float zroh = z * (arc->getArcRadius() + arc->getMinorRadius());
			float yrih = y * (arc->getArcRadius() - arc->getMinorRadius());
			float zrih = z * (arc->getArcRadius() - arc->getMinorRadius());
			vec3 rotatedInner = getRotation() * vec3(0, yrih, zrih);
			vec3 rotatedOuter = getRotation() * vec3(0, yroh, zroh);
			vs.emplace_back(rotatedInner + getPosition1(), vec2(), vec3());
			vs.emplace_back(rotatedInner + getPosition2(), vec2(), vec3());
			vs.emplace_back(rotatedOuter + getPosition1(), vec2(), vec3());
			vs.emplace_back(rotatedOuter + getPosition2(), vec2(), vec3());
		}
		const int START_INDEX = 4 * (SECTORS_PER_SEMICIRCLE + 1) + 2;
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
	} else return;
}

void OscillatingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		bool fullCircle = abs(getAngle2() - getAngle1()) >= 2 * PI;
		float domainStartAngle, domainEndAngle;
		if (fullCircle) {
			domainStartAngle = 0;
			domainEndAngle = 2 * PI;
		} else {
			domainStartAngle = std::min(getAngle1(), getAngle2());
			domainEndAngle = std::max(getAngle1(), getAngle2());
		}

		const int NUM_SECTORS = (int)ceil((float)SECTORS_PER_SEMICIRCLE * (domainEndAngle - domainStartAngle) / PI);

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
				mat3 currentRot;
				angleToRotation(currentAngle, currentRot);
				vec3 start = currentRot * segment->getRightCap();
				vec3 end = currentRot * segment->getLeftCap();

				vs.emplace_back(start, vec2(), vec3());
				vs.emplace_back(end, vec2(), vec3());
				for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
					float angStartCap = currentAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
					float angEndCap = currentAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
					vs.emplace_back(vec3(0, start.y - sin(angStartCap) * segment->getMinorRadius(), start.z + cos(angStartCap) * segment->getMinorRadius()), vec2(), vec3());
					vs.emplace_back(vec3(0, end.y - sin(angEndCap) * segment->getMinorRadius(), end.z + cos(angEndCap) * segment->getMinorRadius()), vec2(), vec3());
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

		vs.emplace_back(vec3(), vec2(), vec3());

		const float startRadius = segment->getRightCap().length() + segment->getMinorRadius();
		const float endRadius = segment->getLeftCap().length() + segment->getMinorRadius();
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = domainStartAngle + (float)i / (float)NUM_SECTORS * (domainEndAngle - domainStartAngle);
			vec3 dir = vec3(0, cos(ang), sin(ang));
			vs.emplace_back(dir * startRadius, vec2(), vec3());
			vs.emplace_back(dir * -endRadius, vec2(), vec3());
		}

		for (int i = START_INDEX + 1; i < START_INDEX + 1 + NUM_SECTORS * 2; i += 2) {
			is.push_back(START_INDEX);
			is.push_back(i + 0);
			is.push_back(i + 2);
			is.push_back(START_INDEX);
			is.push_back(i + 1);
			is.push_back(i + 3);
		}
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		bool fullCircle = abs(getAngle2() - getAngle1()) + arc->getArcAngle() >= 2 * PI;
		float startAngle, endAngle;
		vec3 start, end;
		if (fullCircle) {
			startAngle = 0;
			endAngle = 2 * PI;
			start = end = {0, 0, arc->getArcRadius()};
		} else {
			startAngle = std::min(getAngle1(), getAngle2()) - arc->getHalfArcAngle();
			endAngle = std::max(getAngle1(), getAngle2()) + arc->getHalfArcAngle();
			start = vec3(0, -sin(startAngle), cos(startAngle)) * arc->getArcRadius();
			end = vec3(0, -sin(endAngle), cos(endAngle)) * arc->getArcRadius();
		}

		const int NUM_SECTORS = (int)ceil((float)SECTORS_PER_SEMICIRCLE * (endAngle - startAngle) / PI);

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
			vs.emplace_back(start, vec2(), vec3());
			vs.emplace_back(end, vec2(), vec3());
			for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
				float angStartCap = startAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				float angEndCap = endAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				vs.emplace_back(vec3(0, start.y - sin(angStartCap) * arc->getMinorRadius(), start.z + cos(angStartCap) * arc->getMinorRadius()), vec2(), vec3());
				vs.emplace_back(vec3(0, end.y - sin(angEndCap) * arc->getMinorRadius(), end.z + cos(angEndCap) * arc->getMinorRadius()), vec2(), vec3());
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

		// Arc
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = startAngle + (float)i / (float)NUM_SECTORS * (endAngle - startAngle);
			vec3 dir = vec3(0, -sin(ang), cos(ang));
			vs.emplace_back(dir * (arc->getArcRadius() - arc->getMinorRadius()), vec2(), vec3());
			vs.emplace_back(dir * (arc->getArcRadius() + arc->getMinorRadius()), vec2(), vec3());
		}
		for (int i = START_INDEX; i < START_INDEX + NUM_SECTORS * 2; i += 2) {
			is.push_back(i + 0);
			is.push_back(i + 1);
			is.push_back(i + 3);
			is.push_back(i + 0);
			is.push_back(i + 3);
			is.push_back(i + 2);
		}
	} else return;
}


void TogglingPositionSpec::stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setPosition(lerp(getPositionA(), getPositionB(), smoother.getCurrentPosition()));
	kinematicState.setVelocity((getPositionB() - getPositionA()) * smoother.getCurrentVelocity());
}

void TogglingAngleSpec::stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngle(lerp(getAngleA(), getAngleB(), smoother.getCurrentPosition()));
	kinematicState.setAngularVelocity((getAngleB() - getAngleA()) * smoother.getCurrentVelocity());
}

void SpinningSpec::stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngularVelocity(lerp(getAngularVelocityA(), getAngularVelocityB(), smoother.getCurrentPosition()));
	kinematicState.setAngle(kinematicState.getAngle() + kinematicState.getAngularVelocity() * PHYSICS_TIMESTEP);
}

void OscillatingPositionSpec::stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	float angularFrequency = lerp(getAngularFrequencyA(), getAngularFrequencyB(), smoother.getCurrentPosition());
	kinematicState.setPhase(kinematicState.getPhase() + angularFrequency * PHYSICS_TIMESTEP);
	kinematicState.setPosition(lerp(getPosition1(), getPosition2(), 0.5f - 0.5f * cos(kinematicState.getPhase())));
	kinematicState.setVelocity((getPosition2() - getPosition1()) * (0.5f * sin(kinematicState.getPhase()) * angularFrequency));
}

void OscillatingAngleSpec::stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	float angularFrequency = lerp(getAngularFrequencyA(), getAngularFrequencyB(), smoother.getCurrentPosition());
	kinematicState.setPhase(kinematicState.getPhase() + angularFrequency * PHYSICS_TIMESTEP);
	kinematicState.setAngle(lerp(getAngle1(), getAngle2(), 0.5f - 0.5f * cos(kinematicState.getPhase())));
	kinematicState.setAngularVelocity((getAngle2() - getAngle1()) * (0.5f * sin(kinematicState.getPhase()) * angularFrequency));
}


void TogglingPositionSpec::updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setPosition(lerp(getPositionA(), getPositionB(), smoother.getCurrentPosition()));
}

void TogglingAngleSpec::updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngle(lerp(getAngleA(), getAngleB(), smoother.getCurrentPosition()));
}

void SpinningSpec::updateEditorKinematicState(KinematicState& kinematicState, const Smoother&) const {
	kinematicState.setAngle(getInitialAngle());
}

void OscillatingPositionSpec::updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	bool toggled = smoother.getCurrentPosition() > 0.5f;
	kinematicState.setPhase(toggled ? PI : 0);
	kinematicState.setPosition(toggled ? getPosition2() : getPosition1());
}

void OscillatingAngleSpec::updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const {
	bool toggled = smoother.getCurrentPosition() > 0.5f;
	kinematicState.setPhase(toggled ? PI : 0);
	kinematicState.setAngle(toggled ? getAngle2() : getAngle1());
}


void EditorObstacle::updateKinematicState(const Smoother& smoother, int numSteps) {
	if (numSteps >= 0) // Demonstrate motion
		while (numSteps--)
			descriptor->getMotion()->stepKinematicState(kinematicState, smoother);
	else // Just set 'stationary' attributes
		descriptor->getMotion()->updateEditorKinematicState(kinematicState, smoother);
}


ObstacleDescriptor::ObstacleDescriptor(const std::string& data) {
	std::istringstream ss(data);

	std::string shapeString, motionString;

	if (!(std::getline(ss, shapeString, '|') && std::getline(ss, motionString, '|') && ss >> goal))
		throw std::invalid_argument("Invalid obstacle data format");

	shape = std::move(AbstractShapeSpec::deserialize(shapeString));
	motion = std::move(IMotionSpec::deserialize(motionString));
}


std::string ObstacleDescriptor::serialize() const {
	std::ostringstream ss;

	ss << shape->serialize() << "|" << motion->serialize() << "|" << isGoal();

	return ss.str();
}
