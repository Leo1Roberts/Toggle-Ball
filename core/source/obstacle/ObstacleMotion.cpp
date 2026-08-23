#include "obstacle/ObstacleMotion.h"

#include "obstacle/ObstacleKinematicState.h"
#include "obstacle/ObstacleShape.h"
#include "game/PhysicsConstants.h"
#include "Settings.h"
#include "utilities/Smoother.h"
#include "utilities/Utilities.h"

#include <iomanip>
#include <sstream>


std::unique_ptr<IMotionSpec> IMotionSpec::make(Type type, const IncompletePropertyValues& values, bool toggled) {
	PropertyValues allValues;
	for (int property = 0; property < values.size(); property++) {
		const auto& propertyValues = values[property];
		for (int state = 0; state < propertyValues.size(); state++) {
			auto value = propertyValues[state];
			if (!value) {
				switch ((Property)property) {
				case Property::Position_X:
				case Property::Position_Y:
				case Property::Angle:
					switch ((State)state) {
					case State::_:
						if (toggled) {
							if (auto v = propertyValues[(int)State::B])
								value = *v;
						} else {
							if (auto v = propertyValues[(int)State::A])
								value = *v;
						}
						break;
					case State::A:
					case State::B:
						if (auto v = propertyValues[(int)State::_])
							value = *v;
						break;
					default:;
					}
					break;
				case Property::Position1_X:
				case Property::Position2_X:
					if (auto v = values[(int)Property::Position_X][(int)State::_])
						value = *v;
					break;
				case Property::Position1_Y:
				case Property::Position2_Y:
					if (auto v = values[(int)Property::Position_Y][(int)State::_])
						value = *v;
					break;
				case Property::InitialAngle:
					if (toggled) {
						if (auto v = values[(int)Property::Angle][(int)State::B])
							value = *v;
					} else {
						if (auto v = values[(int)Property::Angle][(int)State::A])
							value = *v;
					}
					break;
				case Property::Angle1:
				case Property::Angle2:
					if (auto v = values[(int)Property::Angle][(int)State::_])
						value = *v;
					break;
				default:;
				}
			}
			allValues[property][state] = value ? *value : 0.f;
		}
	}

	switch (type) {
	case Type::Static:
		return std::make_unique<StaticSpec>(allValues);
	case Type::TogglingPosition:
		return std::make_unique<TogglingPositionSpec>(allValues);
	case Type::TogglingAngle:
		return std::make_unique<TogglingAngleSpec>(allValues);
	case Type::Spinning:
		return std::make_unique<SpinningSpec>(allValues);
	case Type::OscillatingPosition:
		return std::make_unique<OscillatingPositionSpec>(allValues);
	case Type::OscillatingAngle:
		return std::make_unique<OscillatingAngleSpec>(allValues);
	default:;
		return nullptr;
	}
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

	ss << position.x << "," << position.y << "," << angle;

	return ss.str();
}
StaticSpec::StaticSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.x >> c >> position.y >> c >> angle))
		throw std::invalid_argument("Invalid static motion data format");
}

std::string TogglingPositionSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << angle << "," << positionA.x << "," << positionA.y << "," << positionB.x << "," << positionB.y;

	return ss.str();
}
TogglingPositionSpec::TogglingPositionSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> angle >> c >> positionA.x >> c >> positionA.y >> c >> positionB.x >> c >> positionB.y))
		throw std::invalid_argument("Invalid toggling position motion data format");
}

std::string TogglingAngleSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.x << "," << position.y << "," << angleA << "," << angleB;

	return ss.str();
}
TogglingAngleSpec::TogglingAngleSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.x >> c >> position.y >> c >> angleA >> c >> angleB))
		throw std::invalid_argument("Invalid toggling angle motion data format");
}

std::string SpinningSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.x << "," << position.y << "," << initialAngle << "," << angularSpeedA << "," << angularSpeedB;

	return ss.str();
}
SpinningSpec::SpinningSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.x >> c >> position.y >> c >> initialAngle >> c >> angularSpeedA >> c >> angularSpeedB))
		throw std::invalid_argument("Invalid spinning motion data format");
}

std::string OscillatingPositionSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position1.x << "," << position1.y << "," << position2.x << "," << position2.y << "," << angle << "," << angularFrequencyA << "," << angularFrequencyB;

	return ss.str();
}
OscillatingPositionSpec::OscillatingPositionSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position1.x >> c >> position1.y >> c >> position2.x >> c >> position2.y >> c >> angle >> c >> angularFrequencyA >> c >> angularFrequencyB))
		throw std::invalid_argument("Invalid oscillating position motion data format");
}

std::string OscillatingAngleSpec::serializeData() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << position.x << "," << position.y << "," << angle1 << "," << angle2 << "," << angularFrequencyA << "," << angularFrequencyB;

	return ss.str();
}
OscillatingAngleSpec::OscillatingAngleSpec(const std::string& data) {
	std::istringstream ss(data);

	char c;
	if (!(ss >> position.x >> c >> position.y >> c >> angle1 >> c >> angle2 >> c >> angularFrequencyA >> c >> angularFrequencyB))
		throw std::invalid_argument("Invalid oscillating angle motion data format");
}


bool IMotionSpec::operator==(const IMotionSpec& other) const {
	if (typeid(*this) != typeid(other))
		return false;

	return equals(other);
}


void StaticSpec::setAngle(float radians) {
	angle = wrapAngle(radians);
	rotation = angleToRotation2D(radians);
}
void TogglingPositionSpec::setAngle(float radians) {
	angle = wrapAngle(radians);
	rotation = angleToRotation2D(radians);
}
void OscillatingPositionSpec::setAngle(float radians) {
	angle = wrapAngle(radians);
	rotation = angleToRotation2D(radians);
}

void SpinningSpec::setInitialAngle(float radians) {
	initialAngle = wrapAngle(radians);
}


void IMotionSpec::generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;
	buildDomainMesh(vs, is, shapeSpec, uiToWorldScale);
	domainMesh.setData(vs, is);
}

void TogglingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const {
	std::vector<ObjectVertex> vs_shadow;
	std::vector<Index> is_shadow;
	shapeSpec->buildShadowMesh(vs_shadow, is_shadow);

	glm::vec2 diff = positionB - positionA;
	float line1Length, line2Length;
	line1Length = line2Length = length(diff);
	if (line1Length > 0.001f) {
		glm::vec2 diffUnit = diff / line1Length;
		float diffAngle = std::atan2(diff.y, diff.x);
		glm::vec2 topPointA, bottomPointA;
		glm::vec2 diffPerpUnit = glm::vec2(-diff.y, diff.x) / line1Length;

		if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
			glm::vec2 diffPerp = diffPerpUnit * segment->getMinorRadius();
			if (wrapAngle(angle - diffAngle) > 0.f) {
				topPointA = positionA + rotation * segment->getRightCap() + diffPerp;
				bottomPointA = positionA + rotation * segment->getLeftCap() - diffPerp;
			} else {
				topPointA = positionA + rotation * segment->getLeftCap() + diffPerp;
				bottomPointA = positionA + rotation * segment->getRightCap() - diffPerp;
			}
		} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
			float ang = wrapAngle(diffAngle - angle);
			if (ang > -arc->getHalfArcAngle() && ang < arc->getHalfArcAngle()) {
				topPointA = positionA + diffPerpUnit * (arc->getArcRadius() + arc->getMinorRadius());
			} else {
				float startAng = std::abs(wrapAngle(diffAngle - angle + arc->getHalfArcAngle()));
				float endAng = std::abs(wrapAngle(diffAngle - angle - arc->getHalfArcAngle()));
				if (std::abs(startAng - endAng) < 0.001f) { // Equal
					topPointA = positionA + rotation * arc->getRightCap() + diffPerpUnit * arc->getMinorRadius();
					line1Length += arc->getRightCap().x - arc->getLeftCap().x;
				} else if (startAng < endAng)
					topPointA = positionA + rotation * arc->getRightCap() + diffPerpUnit * arc->getMinorRadius();
				else
					topPointA = positionA + rotation * arc->getLeftCap() + diffPerpUnit * arc->getMinorRadius();
			}

			ang = wrapAngle(diffAngle - angle + glm::pi<float>());
			if (ang > -arc->getHalfArcAngle() && ang < arc->getHalfArcAngle()) {
				bottomPointA = positionA - diffPerpUnit * (arc->getArcRadius() + arc->getMinorRadius());
			} else {
				float startAng = std::abs(wrapAngle(diffAngle - angle + arc->getHalfArcAngle() + glm::pi<float>()));
				float endAng = std::abs(wrapAngle(diffAngle - angle - arc->getHalfArcAngle() + glm::pi<float>()));
				if (std::abs(startAng - endAng) < 0.001f) { // Equal
					bottomPointA = positionA + rotation * arc->getLeftCap() - diffPerpUnit * arc->getMinorRadius();
					line2Length += arc->getRightCap().x- arc->getLeftCap().x;
				} else if (startAng < endAng)
					bottomPointA = positionA + rotation * arc->getRightCap() - diffPerpUnit * arc->getMinorRadius();
				else
					bottomPointA = positionA + rotation * arc->getLeftCap() - diffPerpUnit * arc->getMinorRadius();
			}
		} else return;

		const float dotDiameter = uiToWorldScale * Settings::Sizes.outlineWidth;

		int numDots1 = 2 * (((int)(line1Length / (dotDiameter * 2.f)) + 1) / 2);
		int numDots2 = 2 * (((int)(line2Length / (dotDiameter * 2.f)) + 1) / 2);
		bool drawDots = numDots1 < 1000 && numDots2 < 1000; // Don't draw dots if there are too many

		size_t vsSize = 2 * vs_shadow.size();
		size_t isSize = 2 * is_shadow.size();
		if (drawDots) {
			vsSize += (numDots1 + numDots2) * (SECTORS_PER_DOT + 1);
			isSize += (numDots1 + numDots2) * SECTORS_PER_DOT * 3;
		}
		vs.reserve(vsSize);
		is.reserve(isSize);

		float dotSpacing = dotDiameter * 2.f;
		glm::vec3 dotShift = planarToWorld(diffUnit * dotSpacing);

		glm::vec3 start1 = planarToWorld(topPointA - diffPerpUnit / 2.f * dotDiameter + diffUnit * (line1Length - (dotSpacing * (float)(numDots1 - 1))) / 2.f);
		glm::vec3 start2 = planarToWorld(bottomPointA + diffPerpUnit / 2.f * dotDiameter + diffUnit * (line2Length - (dotSpacing * (float)(numDots2 - 1))) / 2.f);

		if (drawDots) {
			glm::vec3 dotCentre = start1;
			for (int d = 0; d < numDots1; d++) {
				vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
				for (int i = 0; i < SECTORS_PER_DOT; i++) {
					float ang = (float)i / (float)SECTORS_PER_DOT * glm::two_pi<float>();
					vs.emplace_back(dotCentre + planarToWorld({std::cos(ang), std::sin(ang)}) * dotDiameter / 2.f, glm::vec2(), glm::vec3());
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
					float ang = (float)i / (float)SECTORS_PER_DOT * glm::two_pi<float>();
					vs.emplace_back(dotCentre + planarToWorld({std::cos(ang), std::sin(ang)}) * dotDiameter / 2.f, glm::vec2(), glm::vec3());
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
	}

	glm::mat3 rot = angleToRotation3D(angle);
	for (glm::vec3 pos : {planarToWorld(positionA), planarToWorld(positionB)}) {
		auto offset = (Index)vs.size();

		std::ranges::transform(vs_shadow, std::back_inserter(vs), [pos, rot](const ObjectVertex& v) {
			return ObjectVertex(pos + rot * v.position, v.uv, v.normal, v.color);
		});

		std::ranges::transform(is_shadow, std::back_inserter(is), [offset](Index i) {
			return offset + i;
		});
	}
}

void TogglingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const {
	std::vector<ObjectVertex> vs_shadow;
	std::vector<Index> is_shadow;
	shapeSpec->buildShadowMesh(vs_shadow, is_shadow);

	float dotsArcAngle;
	float dotsArc1Radius, dotsArc2Radius;
	float start1, start2;

	const float dotDiameter = uiToWorldScale * Settings::Sizes.outlineWidth;

	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		dotsArcAngle = std::clamp(angleB - angleA, -glm::two_pi<float>(), glm::two_pi<float>());
		dotsArc1Radius = length(segment->getRightCap()) + segment->getMinorRadius() - dotDiameter / 2.f;
		dotsArc2Radius = length(segment->getLeftCap()) + segment->getMinorRadius() - dotDiameter / 2.f;
		start1 = angleA;
		start2 = angleA + glm::pi<float>();
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		float angDiff = angleB - angleA;
		if (angDiff < 0) {
			dotsArcAngle = std::clamp(angDiff + arc->getArcAngle(), -glm::two_pi<float>() + arc->getArcAngle(), 0.f);
			start1 = start2 = angleA + glm::half_pi<float>() - arc->getHalfArcAngle();
		} else {
			dotsArcAngle = std::clamp(angDiff - arc->getArcAngle(), 0.f, glm::two_pi<float>() - arc->getArcAngle());
			start1 = start2 = angleA + glm::half_pi<float>() + arc->getHalfArcAngle();
		}
		dotsArc1Radius = arc->getArcRadius() + arc->getMinorRadius() - dotDiameter / 2.f;
		dotsArc2Radius = arc->getArcRadius() - arc->getMinorRadius() + dotDiameter / 2.f;
	} else return;

	float absArcAngle = std::abs(dotsArcAngle);
	float sign = dotsArcAngle < 0.f ? -1.f : 1.f;

	float arc1Length = dotsArc1Radius * absArcAngle;
	float arc2Length = dotsArc2Radius * absArcAngle;

	int numDots1 = 2 * (((int)(arc1Length / (dotDiameter * 2.f)) + 1) / 2);
	int numDots2 = 2 * (((int)(arc2Length / (dotDiameter * 2.f)) + 1) / 2);
	bool drawDots = numDots1 < 1000 && numDots2 < 1000; // Don't draw dots if there are too many

	size_t vsSize = 2 * vs_shadow.size();
	size_t isSize = 2 * is_shadow.size();
	if (drawDots) {
		vsSize += (numDots1 + numDots2) * (SECTORS_PER_DOT + 1);
		isSize += (numDots1 + numDots2) * SECTORS_PER_DOT * 3;
	}
	vs.reserve(vsSize);
	is.reserve(isSize);

	float dotSpacing = dotDiameter * 2.f;
	float dotShift1 = sign * dotSpacing / dotsArc1Radius;
	float dotShift2 = sign * dotSpacing / dotsArc2Radius;
	start1 += sign * (arc1Length - (dotSpacing * (float)(numDots1 - 1))) / (dotsArc1Radius * 2);
	start2 += sign * (arc2Length - (dotSpacing * (float)(numDots2 - 1))) / (dotsArc2Radius * 2);

	float dotAngle = start1;
	for (int d = 0; d < numDots1; d++) {
		glm::vec3 dotCentre = planarToWorld({std::cos(dotAngle), std::sin(dotAngle)}) * dotsArc1Radius;
		vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
		for (int i = 0; i < SECTORS_PER_DOT; i++) {
			float ang = (float)i / (float)SECTORS_PER_DOT * glm::two_pi<float>();
			vs.emplace_back(dotCentre + planarToWorld({std::cos(ang), std::sin(ang)}) * dotDiameter / 2.f, glm::vec2(), glm::vec3());
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
		glm::vec3 dotCentre = planarToWorld({std::cos(dotAngle), std::sin(dotAngle)}) * dotsArc2Radius;
		vs.emplace_back(dotCentre, glm::vec2(), glm::vec3());
		for (int i = 0; i < SECTORS_PER_DOT; i++) {
			float ang = (float)i / (float)SECTORS_PER_DOT * glm::two_pi<float>();
			vs.emplace_back(dotCentre + planarToWorld({std::cos(ang), std::sin(ang)}) * dotDiameter / 2.f, glm::vec2(), glm::vec3());
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

	for (float angle : {angleA, angleB}) {
		auto offset = (Index)vs.size();

		glm::mat3 rot = angleToRotation3D(angle);
		std::ranges::transform(vs_shadow, std::back_inserter(vs), [rot](const ObjectVertex& v) {
			return ObjectVertex(rot * v.position, v.uv, v.normal, v.color);
		});

		std::ranges::transform(is_shadow, std::back_inserter(is), [offset](Index i) {
			return offset + i;
		});
	}
}

void SpinningSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vs.reserve(1 + SECTORS_PER_CIRCLE);
		is.reserve(SECTORS_PER_CIRCLE * 3);

		vs.emplace_back(glm::vec3(), glm::vec2(), glm::vec3());
		for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float radius = std::max(segment->getLeftLength(), segment->getRightLength()) + segment->getMinorRadius();
			vs.emplace_back(planarToWorld({x * radius, y * radius}), glm::vec2(), glm::vec3());
			is.push_back(0);
			is.push_back(1 + (i + 1) % SECTORS_PER_CIRCLE);
			is.push_back(1 + i);
		}
	} else if (auto* arc = dynamic_cast<const ArcSpec*>(shapeSpec)) {
		vs.reserve(2 * SECTORS_PER_CIRCLE);
		is.reserve(SECTORS_PER_CIRCLE * 6);

		for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float innerRadius = arc->getArcRadius() - arc->getMinorRadius();
			float outerRadius = arc->getArcRadius() + arc->getMinorRadius();
			vs.emplace_back(planarToWorld({x * innerRadius, y * innerRadius}), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld({x * outerRadius, y * outerRadius}), glm::vec2(), glm::vec3());
			is.push_back(i*2);
			is.push_back(i*2 + 1);
			is.push_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
			is.push_back(i*2);
			is.push_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
			is.push_back((i*2 + 2) % (SECTORS_PER_CIRCLE * 2));
		}
	}
}

void OscillatingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		vs.reserve((SECTORS_PER_SEMICIRCLE + 1) * 4 + 2);
		is.reserve((SECTORS_PER_SEMICIRCLE + 1) * 18);

		// Caps
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float xrh = x * segment->getMinorRadius();
			float yrh = y * segment->getMinorRadius();
			glm::vec2 rotatedRightCap = rotation * (segment->getRightCap() + glm::vec2(xrh, yrh));
			glm::vec2 rotatedLeftCap = rotation * (segment->getLeftCap() + glm::vec2(-xrh, yrh));
			vs.emplace_back(planarToWorld(rotatedRightCap + position1), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(rotatedRightCap + position2), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(rotatedLeftCap + position1), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(rotatedLeftCap + position2), glm::vec2(), glm::vec3());
		}
		vs.emplace_back(planarToWorld(rotation * segment->getRightCap() + position1), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(rotation * segment->getLeftCap() + position1), glm::vec2(), glm::vec3());

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
		const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * arc->getArcAngle() * glm::one_over_pi<float>());

		vs.reserve((SECTORS_PER_SEMICIRCLE + 1) * 4 + 2 + (NUM_SECTORS + 1) * 4);
		is.reserve(SECTORS_PER_SEMICIRCLE * 18 + NUM_SECTORS * 18);

		// Caps
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>() + arc->getHalfArcAngle();
			float x = std::sin(ang);
			float y = std::cos(ang);
			float xrh = x * arc->getMinorRadius();
			float yrh = y * arc->getMinorRadius();
			glm::vec2 rotatedRightCap = rotation * (arc->getRightCap() + glm::vec2(xrh, yrh));
			glm::vec2 rotatedLeftCap = rotation * (arc->getLeftCap() + glm::vec2(-xrh, yrh));
			vs.emplace_back(planarToWorld(rotatedRightCap + position1), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(rotatedRightCap + position2), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(rotatedLeftCap + position1), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(rotatedLeftCap + position2), glm::vec2(), glm::vec3());
		}
		vs.emplace_back(planarToWorld(rotation * arc->getRightCap() + position1), glm::vec2(), glm::vec3());
		vs.emplace_back(planarToWorld(rotation * arc->getLeftCap() + position1), glm::vec2(), glm::vec3());

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

		if (NUM_SECTORS != 0) {
			// Arc
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = (float)i / (float)NUM_SECTORS * arc->getArcAngle() - arc->getHalfArcAngle();
				float x = std::sin(ang);
				float y = std::cos(ang);
				float xroh = x * (arc->getArcRadius() + arc->getMinorRadius());
				float yroh = y * (arc->getArcRadius() + arc->getMinorRadius());
				float xrih = x * (arc->getArcRadius() - arc->getMinorRadius());
				float yrih = y * (arc->getArcRadius() - arc->getMinorRadius());
				glm::vec2 rotatedInner = rotation * glm::vec2(xrih, yrih);
				glm::vec2 rotatedOuter = rotation * glm::vec2(xroh, yroh);
				vs.emplace_back(planarToWorld(rotatedInner + position1), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(rotatedInner + position2), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(rotatedOuter + position1), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(rotatedOuter + position2), glm::vec2(), glm::vec3());
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
}

void OscillatingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const {
	if (auto* segment = dynamic_cast<const SegmentSpec*>(shapeSpec)) {
		bool fullCircle = std::abs(angle2 - angle1) >= glm::two_pi<float>();
		float domainStartAngle, domainEndAngle;
		if (fullCircle) {
			domainStartAngle = 0.f;
			domainEndAngle = glm::two_pi<float>();
		} else {
			domainStartAngle = std::min(angle1, angle2);
			domainEndAngle = std::max(angle1, angle2);
		}

		const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * (domainEndAngle - domainStartAngle) * glm::one_over_pi<float>());

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
				glm::mat2 currentRot = angleToRotation2D(currentAngle);
				glm::vec2 start = currentRot * segment->getRightCap();
				glm::vec2 end = currentRot * segment->getLeftCap();

				vs.emplace_back(planarToWorld(start), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(end), glm::vec2(), glm::vec3());
				for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
					float angStartCap = currentAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
					float angEndCap = currentAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
					vs.emplace_back(planarToWorld(start + glm::vec2(-std::sin(angStartCap) * segment->getMinorRadius(), std::cos(angStartCap) * segment->getMinorRadius())), glm::vec2(), glm::vec3());
					vs.emplace_back(planarToWorld(end + glm::vec2(-std::sin(angEndCap) * segment->getMinorRadius(), std::cos(angEndCap) * segment->getMinorRadius())), glm::vec2(), glm::vec3());
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

			const float startRadius = length(segment->getRightCap()) + segment->getMinorRadius();
			const float endRadius = length(segment->getLeftCap()) + segment->getMinorRadius();
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = domainStartAngle + (float)i / (float)NUM_SECTORS * (domainEndAngle - domainStartAngle);
				glm::vec2 dir = {std::cos(ang), std::sin(ang)};
				vs.emplace_back(planarToWorld(dir * startRadius), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(dir * -endRadius), glm::vec2(), glm::vec3());
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
		bool fullCircle = std::abs(angle2 - angle1) + arc->getArcAngle() >= glm::two_pi<float>();
		float startAngle, endAngle;
		glm::vec2 start, end;
		if (fullCircle) {
			startAngle = 0.f;
			endAngle = glm::two_pi<float>();
		} else {
			startAngle = std::min(angle1, angle2) - arc->getHalfArcAngle();
			endAngle = std::max(angle1, angle2) + arc->getHalfArcAngle();
			start = glm::vec2(-std::sin(startAngle), std::cos(startAngle)) * arc->getArcRadius();
			end = glm::vec2(-std::sin(endAngle), std::cos(endAngle)) * arc->getArcRadius();
		}

		const int NUM_SECTORS = (int)std::ceil((float)SECTORS_PER_SEMICIRCLE * (endAngle - startAngle) * glm::one_over_pi<float>());

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
			vs.emplace_back(planarToWorld(start), glm::vec2(), glm::vec3());
			vs.emplace_back(planarToWorld(end), glm::vec2(), glm::vec3());
			for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
				float angStartCap = startAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
				float angEndCap = endAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * glm::pi<float>();
				vs.emplace_back(planarToWorld(start + glm::vec2(-std::sin(angStartCap) * arc->getMinorRadius(), std::cos(angStartCap) * arc->getMinorRadius())), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(end + glm::vec2(-std::sin(angEndCap) * arc->getMinorRadius(), std::cos(angEndCap) * arc->getMinorRadius())), glm::vec2(), glm::vec3());
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
				glm::vec2 dir = glm::vec2(-std::sin(ang), std::cos(ang));
				vs.emplace_back(planarToWorld(dir * (arc->getArcRadius() - arc->getMinorRadius())), glm::vec2(), glm::vec3());
				vs.emplace_back(planarToWorld(dir * (arc->getArcRadius() + arc->getMinorRadius())), glm::vec2(), glm::vec3());
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


void StaticSpec::initKinematicState(ObstacleKinematicState& kinematicState, bool) const {
	kinematicState.setPosition(planarToWorld(position));
	kinematicState.setAngle(angle);
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularSpeed(0.f);
}

void TogglingPositionSpec::initKinematicState(ObstacleKinematicState& kinematicState, bool) const {
	kinematicState.setPosition(planarToWorld(positionA));
	kinematicState.setAngle(angle);
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularSpeed(0.f);
}

void TogglingAngleSpec::initKinematicState(ObstacleKinematicState& kinematicState, bool) const {
	kinematicState.setPosition(planarToWorld(position));
	kinematicState.setAngle(angleA);
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularSpeed(0.f);
}

void SpinningSpec::initKinematicState(ObstacleKinematicState& kinematicState, bool keepPhase) const {
	kinematicState.setPosition(planarToWorld(position));
	if (!keepPhase)
		kinematicState.setAngle(initialAngle);
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularSpeed(angularSpeedA);
}

void OscillatingPositionSpec::initKinematicState(ObstacleKinematicState& kinematicState, bool keepPhase) const {
	kinematicState.setPosition(planarToWorld(position1));
	kinematicState.setAngle(angle);
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularSpeed(0.f);
	if (!keepPhase)
		kinematicState.setPhase(0.f);
}

void OscillatingAngleSpec::initKinematicState(ObstacleKinematicState& kinematicState, bool keepPhase) const {
	kinematicState.setPosition(planarToWorld(position));
	kinematicState.setAngle(angle1);
	kinematicState.setVelocity(glm::vec3(0.f));
	kinematicState.setAngularSpeed(0.f);
	if (!keepPhase)
		kinematicState.setPhase(0.f);
}


void TogglingPositionSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setPosition(planarToWorld(glm::mix(positionA, positionB, smoother.getCurrentPosition())));
	kinematicState.setVelocity(planarToWorld((positionB - positionA) * smoother.getCurrentVelocity()));
}

void TogglingAngleSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngle(glm::mix(angleA, angleB, smoother.getCurrentPosition()));
	kinematicState.setAngularSpeed((angleB - angleA) * smoother.getCurrentVelocity());
}

void SpinningSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngularSpeed(glm::mix(angularSpeedA, angularSpeedB, smoother.getCurrentPosition()));
	kinematicState.setAngle(kinematicState.getAngle() + kinematicState.getAngularSpeed() * PHYSICS_TIMESTEP);
}

void OscillatingPositionSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	float angularFrequency = glm::mix(angularFrequencyA, angularFrequencyB, smoother.getCurrentPosition());
	kinematicState.setPhase(kinematicState.getPhase() + angularFrequency * PHYSICS_TIMESTEP);
	kinematicState.setPosition(planarToWorld(glm::mix(position1, position2, 0.5f - 0.5f * std::cos(kinematicState.getPhase()))));
	kinematicState.setVelocity(planarToWorld((position2 - position1) * (0.5f * std::sin(kinematicState.getPhase()) * angularFrequency)));
}

void OscillatingAngleSpec::stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	float angularFrequency = glm::mix(angularFrequencyA, angularFrequencyB, smoother.getCurrentPosition());
	kinematicState.setPhase(kinematicState.getPhase() + angularFrequency * PHYSICS_TIMESTEP);
	kinematicState.setAngle(glm::mix(angle1, angle2, 0.5f - 0.5f * std::cos(kinematicState.getPhase())));
	kinematicState.setAngularSpeed((angle2 - angle1) * (0.5f * std::sin(kinematicState.getPhase()) * angularFrequency));
}


void TogglingPositionSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setPosition(planarToWorld(glm::mix(positionA, positionB, smoother.getCurrentPosition())));
}

void TogglingAngleSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	kinematicState.setAngle(glm::mix(angleA, angleB, smoother.getCurrentPosition()));
}

void SpinningSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother&) const {
	kinematicState.setAngle(initialAngle);
}

void OscillatingPositionSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	bool toggled = smoother.getCurrentPosition() > 0.5f;
	kinematicState.setPhase(toggled ? glm::pi<float>() : 0.f);
	kinematicState.setPosition(planarToWorld(toggled ? position2 : position1));
}

void OscillatingAngleSpec::updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const {
	bool toggled = smoother.getCurrentPosition() > 0.5f;
	kinematicState.setPhase(toggled ? glm::pi<float>() : 0.f);
	kinematicState.setAngle(toggled ? angle2 : angle1);
}


void TogglingPositionSpec::translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) {
	auto baseSpec = (const TogglingPositionSpec*)base;
	positionA = baseSpec->positionA;
	positionB = baseSpec->positionB;
	if (stateless || !toggled)
		positionA += vector;
	if (stateless || toggled)
		positionB += vector;
}
void OscillatingPositionSpec::translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) {
	auto baseSpec = (const OscillatingPositionSpec*)base;
	position1 = baseSpec->position1;
	position2 = baseSpec->position2;
	if (stateless || !toggled)
		position1 += vector;
	if (stateless || toggled)
		position2 += vector;
}

void StaticSpec::rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const StaticSpec*)base;
	setAngle(baseSpec->angle + radians);

	if (individual)
		position = baseSpec->position;
	else
		position = pivot + rotationMatrix * (baseSpec->position - pivot);
}
void TogglingPositionSpec::rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const TogglingPositionSpec*)base;
	setAngle(baseSpec->angle + radians);
	if (individual) {
		positionA = baseSpec->positionA;
		positionB = baseSpec->positionB;
	} else {
		positionA = pivot + rotationMatrix * (baseSpec->positionA - pivot);
		positionB = pivot + rotationMatrix * (baseSpec->positionB - pivot);
	}
}
void TogglingAngleSpec::rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool stateless, bool toggled, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const TogglingAngleSpec*)base;
	angleA = baseSpec->angleA;
	angleB = baseSpec->angleB;
	if (stateless || !toggled)
		angleA += radians;
	if (stateless || toggled)
		angleB += radians;
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + rotationMatrix * (baseSpec->position - pivot);
}
void SpinningSpec::rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const SpinningSpec*)base;
	setInitialAngle(baseSpec->initialAngle + radians);
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + rotationMatrix * (baseSpec->position - pivot);
}
void OscillatingPositionSpec::rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const OscillatingPositionSpec*)base;
	setAngle(baseSpec->angle + radians);
	if (individual) {
		position1 = baseSpec->position1;
		position2 = baseSpec->position2;
	} else {
		position1 = pivot + rotationMatrix * (baseSpec->position1 - pivot);
		position2 = pivot + rotationMatrix * (baseSpec->position2 - pivot);
	}
}
void OscillatingAngleSpec::rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool stateless, bool toggled, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const OscillatingAngleSpec*)base;
	angle1 = baseSpec->angle1;
	angle2 = baseSpec->angle2;
	if (stateless || !toggled)
		angle1 += radians;
	if (stateless || toggled)
		angle2 += radians;
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + rotationMatrix * (baseSpec->position - pivot);
}

void StaticSpec::scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const StaticSpec*)base;
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + factor * (baseSpec->position - pivot);
}
void TogglingPositionSpec::scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const TogglingPositionSpec*)base;
	if (individual) {
		positionA = baseSpec->positionA;
		positionB = baseSpec->positionB;
	} else {
		positionA = pivot + factor * (baseSpec->positionA - pivot);
		positionB = pivot + factor * (baseSpec->positionB - pivot);
	}
}
void TogglingAngleSpec::scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const TogglingAngleSpec*)base;
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + factor * (baseSpec->position - pivot);
}
void SpinningSpec::scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const SpinningSpec*)base;
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + factor * (baseSpec->position - pivot);
}
void OscillatingPositionSpec::scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const OscillatingPositionSpec*)base;
	if (individual) {
		position1 = baseSpec->position1;
		position2 = baseSpec->position2;
	} else {
		position1 = pivot + factor * (baseSpec->position1 - pivot);
		position2 = pivot + factor * (baseSpec->position2 - pivot);
	}
}
void OscillatingAngleSpec::scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) {
	auto baseSpec = (const OscillatingAngleSpec*)base;
	if (individual)
		position = baseSpec->position;
	else
		position = pivot + factor * (baseSpec->position - pivot);
}