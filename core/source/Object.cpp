#include "main.h"
#include "Model.h"
#include "Object.h"
#include "Sizes.h"
#include "ObjShader.h"
#include <algorithm>

const byte SECTORS_PER_SEMICIRCLE = 64;
const byte SECTORS_PER_CIRCLE = SECTORS_PER_SEMICIRCLE * 2;
const byte SECTORS_PER_DOT = 8;
const float BEVEL_AMOUNT = 0.1f;


BallProperties Ball::getProperties(byte ballType, bool normalise) {
	BallProperties properties = {};
	float radius;

	switch (ballType) {
	case BASKETBALL:
		radius = normalise ? 1 : 0.12f;
		properties = {
		MAT_BASKETBALL,
		0.45f,
		0.65f * 0.45f * 0.12f * 0.12f,
		4000.0f,
		4.0f,
		radius,
		0.5f,
		basketballTex};
		break;
	}

	return properties;
}


bool ObstacleDefinition::operator==(const ObstacleDefinition& other) const {
	return initPos == other.initPos &&
	       initAngle == other.initAngle &&
	       isStraight == other.isStraight &&
	       start == other.start &&
	       end == other.end &&
	       minorRadius == other.minorRadius &&
	       stateType == other.stateType &&
	       stateA == other.stateA &&
	       stateB == other.stateB &&
	       isGoal == other.isGoal;
}

ObstacleDefinition ObstacleDefinition::scale(float scale) const {
	return ObstacleDefinition(
		initPos * scale,
		initAngle,
		isStraight,
		start * scale,
		end * scale,
		minorRadius * scale,
		stateType,
		vec3(stateA.x, stateA.y * (stateType == ST_ANG_OSC ? 1 : scale), stateA.z * scale),
		vec3(stateB.x, stateB.y * (stateType == ST_ANG_OSC ? 1 : scale), stateB.z * scale),
		isGoal
	);
}

void Obstacle::setAngle(float iAngle) {
	angle = iAngle;
	rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, angle);
}

void Obstacle::createObstacleModel() {
	obstacle.vertices.clear();
	obstacle.indices.clear();
	std::vector<Vertex>& vs = obstacle.vertices;
	std::vector<Index>& is = obstacle.indices;

	float bevel = BEVEL_AMOUNT * getMinorRadius();

	const float HALF_DEPTH = getMinorRadius();


	if (getIsStraight()) {
		// Caps

		vs.emplace_back(vec3(HALF_DEPTH, getStart().y, getStart().z), vec2(), vec3(1, 0, 0), getColor());
		vs.emplace_back(vec3(HALF_DEPTH, getEnd().y, getEnd().z), vec2(), vec3(1, 0, 0), getColor());
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = sinf(ang);
			float z = cosf(ang);
			float yrv = y * (getMinorRadius() - bevel);
			float zrv = z * (getMinorRadius() - bevel);
			float yrh = y * getMinorRadius();
			float zrh = z * getMinorRadius();
			vs.emplace_back(vec3(HALF_DEPTH, getStart().y + yrv, getStart().z + zrv), vec2(), vec3(1, 0, 0), getColor());
			vs.emplace_back(vec3(HALF_DEPTH - bevel, getStart().y + yrh, getStart().z + zrh), vec2(), vec3(0, y, z), getColor());
			vs.emplace_back(vec3(HALF_DEPTH, getEnd().y - yrv, getEnd().z + zrv), vec2(), vec3(1, 0, 0), getColor());
			vs.emplace_back(vec3(HALF_DEPTH - bevel, getEnd().y - yrh, getEnd().z + zrh), vec2(), vec3(0, -y, z), getColor());
		}
		for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 4; i += 4) {

			// Semicircles
			// Start
			is.push_back(i + 4);
			is.push_back(i + 0);
			is.push_back(0);
			// End
			is.push_back(i + 2);
			is.push_back(i + 6);
			is.push_back(1);

			// Bevel
			// Start
			is.push_back(i + 0);
			is.push_back(i + 4);
			is.push_back(i + 5);
			is.push_back(i + 0);
			is.push_back(i + 5);
			is.push_back(i + 1);
			// End
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
	} else {
		// Caps

		vs.emplace_back(vec3(HALF_DEPTH, getStart().y, getStart().z), vec2(), vec3(1, 0, 0), getColor());
		vs.emplace_back(vec3(HALF_DEPTH, getEnd().y, getEnd().z), vec2(), vec3(1, 0, 0), getColor());
		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
			float y = sinf(ang);
			float z = cosf(ang);
			float yrv = y * (getMinorRadius() - bevel);
			float zrv = z * (getMinorRadius() - bevel);
			float yrh = y * getMinorRadius();
			float zrh = z * getMinorRadius();
			vs.emplace_back(vec3(HALF_DEPTH, getStart().y + yrv, getStart().z + zrv), vec2(), vec3(1, 0, 0), getColor());
			vs.emplace_back(vec3(HALF_DEPTH - bevel, getStart().y + yrh, getStart().z + zrh), vec2(), vec3(0, y, z), getColor());
			vs.emplace_back(vec3(HALF_DEPTH, getEnd().y - yrv, getEnd().z + zrv), vec2(), vec3(1, 0, 0), getColor());
			vs.emplace_back(vec3(HALF_DEPTH - bevel, getEnd().y - yrh, getEnd().z + zrh), vec2(), vec3(0, -y, z), getColor());
		}
		for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 4; i += 4) {

			// Semicircles
			// Start
			is.push_back(i + 4);
			is.push_back(i + 0);
			is.push_back(0);
			// End
			is.push_back(i + 2);
			is.push_back(i + 6);
			is.push_back(1);

			// Bevel
			// Start
			is.push_back(i + 0);
			is.push_back(i + 4);
			is.push_back(i + 5);
			is.push_back(i + 0);
			is.push_back(i + 5);
			is.push_back(i + 1);
			// End
			is.push_back(i + 2);
			is.push_back(i + 3);
			is.push_back(i + 7);
			is.push_back(i + 2);
			is.push_back(i + 7);
			is.push_back(i + 6);
		}

		// Banana

		const byte NUM_SECTORS = (byte)ceil((float)SECTORS_PER_SEMICIRCLE * 2.0f * getHalfArcAngle() / PI);
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = (float)i / (float)NUM_SECTORS * 2.0f * getHalfArcAngle() - getHalfArcAngle();
			float y = sinf(ang);
			float z = cosf(ang);
			float yrov = y * (getMajorRadius() + getMinorRadius() - bevel);
			float zrov = z * (getMajorRadius() + getMinorRadius() - bevel);
			float yriv = y * (getMajorRadius() - getMinorRadius() + bevel);
			float zriv = z * (getMajorRadius() - getMinorRadius() + bevel);
			float yroh = y * (getMajorRadius() + getMinorRadius());
			float zroh = z * (getMajorRadius() + getMinorRadius());
			float yrih = y * (getMajorRadius() - getMinorRadius());
			float zrih = z * (getMajorRadius() - getMinorRadius());
			vs.emplace_back(vec3(HALF_DEPTH, yriv, zriv), vec2(), vec3(1, 0, 0), getColor());
			vs.emplace_back(vec3(HALF_DEPTH - bevel, yrih, zrih), vec2(), vec3(0, -y, -z), getColor());
			vs.emplace_back(vec3(HALF_DEPTH, yrov, zrov), vec2(), vec3(1, 0, 0), getColor());
			vs.emplace_back(vec3(HALF_DEPTH - bevel, yroh, zroh), vec2(), vec3(0, y, z), getColor());
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

	obstacle.sendToGpu();
	objShader->setupVertexAttribs();
}

void Obstacle::createOutlineModel() {
	outline.vertices.clear();
	outline.indices.clear();
	std::vector<Vertex>& vs = outline.vertices;
	std::vector<Index>& is = outline.indices;

	const float outlineRadius = getMinorRadius() + OUTLINE_WIDTH_WORLD;

	if (getIsStraight()) {
		// Caps

		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
			float y = sinf(ang);
			float z = cosf(ang);
			float yrh = y * getMinorRadius();
			float zrh = z * getMinorRadius();
			float yrv = y * outlineRadius;
			float zrv = z * outlineRadius;
			vs.emplace_back(vec3(0, getStart().y + yrh, getStart().z + zrh), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, getStart().y + yrv, getStart().z + zrv), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, getEnd().y - yrh, getEnd().z + zrh), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, getEnd().y - yrv, getEnd().z + zrv), vec2(), vec3(), WHITE);
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

		// Straight section

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
	} else {
		// Caps

		for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
			float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
			float y = sinf(ang);
			float z = cosf(ang);
			float yrh = y * getMinorRadius();
			float zrh = z * getMinorRadius();
			float yrv = y * outlineRadius;
			float zrv = z * outlineRadius;
			vs.emplace_back(vec3(0, getStart().y + yrh, getStart().z + zrh), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, getStart().y + yrv, getStart().z + zrv), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, getEnd().y - yrh, getEnd().z + zrh), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, getEnd().y - yrv, getEnd().z + zrv), vec2(), vec3(), WHITE);
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

		// Banana

		const byte NUM_SECTORS = (byte)ceil((float)SECTORS_PER_SEMICIRCLE * 2.0f * getHalfArcAngle() / PI);
		for (int i = 0; i <= NUM_SECTORS; i++) {
			float ang = (float)i / (float)NUM_SECTORS * 2.0f * getHalfArcAngle() - getHalfArcAngle();
			float y = sinf(ang);
			float z = cosf(ang);
			float yroh = y * (getMajorRadius() + getMinorRadius());
			float zroh = z * (getMajorRadius() + getMinorRadius());
			float yrih = y * (getMajorRadius() - getMinorRadius());
			float zrih = z * (getMajorRadius() - getMinorRadius());
			float yrov = y * (getMajorRadius() + outlineRadius);
			float zrov = z * (getMajorRadius() + outlineRadius);
			float yriv = y * (getMajorRadius() - outlineRadius);
			float zriv = z * (getMajorRadius() - outlineRadius);
			vs.emplace_back(vec3(0, yrih, zrih), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, yriv, zriv), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, yroh, zroh), vec2(), vec3(), WHITE);
			vs.emplace_back(vec3(0, yrov, zrov), vec2(), vec3(), WHITE);
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

	outline.sendToGpu();
	outlineShader->setupVertexAttribs();
}

void Obstacle::createDomainModel() {
	domain.vertices.clear();
	domain.indices.clear();
	std::vector<Vertex>& vs = domain.vertices;
	std::vector<Index>& is = domain.indices;

	switch (getStateType()) {
	case ST_POS: {
		vec3 diff = getStateB() - getStateA();
		float line1Length, line2Length;
		line1Length = line2Length = diff.length();
		vec3 diffUnit = diff / line1Length;
		float diffAngle = atan2f(diff.z, diff.y);
		vec3 topPointA, bottomPointA;
		vec3 diffPerpUnit = vec3(0, -diff.z, diff.y) / line1Length;

		if (getIsStraight()) {
			vec3 diffPerp = diffPerpUnit * getMinorRadius();
			if (wrapAngle(getAngle() - diffAngle) > 0) {
				topPointA = getStateA() + getRot() * getStart() + diffPerp;
				bottomPointA = getStateA() + getRot() * getEnd() - diffPerp;
			} else {
				topPointA = getStateA() + getRot() * getEnd() + diffPerp;
				bottomPointA = getStateA() + getRot() * getStart() - diffPerp;
			}
		} else {
			float ang = wrapAngle(diffAngle - getAngle());
			if (ang > -getHalfArcAngle() && ang < getHalfArcAngle()) {
				topPointA = getStateA() + diffPerpUnit * (getMajorRadius() + getMinorRadius());
			} else {
				float startAng = fabsf(wrapAngle(diffAngle - getAngle() + getHalfArcAngle()));
				float endAng = fabsf(wrapAngle(diffAngle - getAngle() - getHalfArcAngle()));
				if (fabsf(startAng - endAng) < 0.001f) { // Equal
					topPointA = getStateA() + getRot() * getStart() + diffPerpUnit * getMinorRadius();
					line1Length += getStart().y - getEnd().y;
				} else if (startAng < endAng)
					topPointA = getStateA() + getRot() * getStart() + diffPerpUnit * getMinorRadius();
				else
					topPointA = getStateA() + getRot() * getEnd() + diffPerpUnit * getMinorRadius();
			}

			ang = wrapAngle(diffAngle - getAngle() + PI);
			if (ang > -getHalfArcAngle() && ang < getHalfArcAngle()) {
				bottomPointA = getStateA() - diffPerpUnit * (getMajorRadius() + getMinorRadius());
			} else {
				float startAng = fabsf(wrapAngle(diffAngle - getAngle() + getHalfArcAngle() + PI));
				float endAng = fabsf(wrapAngle(diffAngle - getAngle() - getHalfArcAngle() + PI));
				if (fabsf(startAng - endAng) < 0.001f) { // Equal
					bottomPointA = getStateA() + getRot() * getEnd() - diffPerpUnit * getMinorRadius();
					line2Length += getStart().y - getEnd().y;
				} else if (startAng < endAng)
					bottomPointA = getStateA() + getRot() * getStart() - diffPerpUnit * getMinorRadius();
				else
					bottomPointA = getStateA() + getRot() * getEnd() - diffPerpUnit * getMinorRadius();
			}
		}

		float dotSpacing = OUTLINE_WIDTH_WORLD * 2;
		vec3 dotShift = diffUnit * dotSpacing;

		int numDots1 = 2 * (((int)(line1Length / OUTLINE_WIDTH_WORLD * 0.5f) + 1) / 2);
		vec3 start1 = topPointA - diffPerpUnit * 0.5f * OUTLINE_WIDTH_WORLD + diffUnit * (line1Length - (dotSpacing * (float)(numDots1 - 1))) * 0.5f;
		int numDots2 = 2 * (((int)(line2Length / OUTLINE_WIDTH_WORLD * 0.5f) + 1) / 2);
		vec3 start2 = bottomPointA + diffPerpUnit * 0.5f * OUTLINE_WIDTH_WORLD + diffUnit * (line2Length - (dotSpacing * (float)(numDots2 - 1))) * 0.5f;

		if (numDots1 > 1000 || numDots2 > 1000)
			break; // Too many dots, skip drawing

		vec3 dotCentre = start1;
		for (int d = 0; d < numDots1; d++) {
			vs.emplace_back(dotCentre, vec2(), vec3(), WHITE);
			for (byte i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
				vs.emplace_back(dotCentre + vec3(0, cosf(ang), sinf(ang)) * OUTLINE_WIDTH_WORLD * 0.5f, vec2(), vec3(), WHITE);
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
			vs.emplace_back(dotCentre, vec2(), vec3(), WHITE);
			for (byte i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
				vs.emplace_back(dotCentre + vec3(0, cosf(ang), sinf(ang)) * OUTLINE_WIDTH_WORLD * 0.5f, vec2(), vec3(), WHITE);
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
	}   break;
	case ST_POS_OSC: {
		const vec3 pos1 = vec3(0, getStateA().y, getStateA().z);
		const vec3 pos2 = vec3(0, getStateB().y, getStateB().z);

		if (getIsStraight()) {
			// Caps

			for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
				float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				float y = sinf(ang);
				float z = cosf(ang);
				float yrh = y * getMinorRadius();
				float zrh = z * getMinorRadius();
				vec3 rotatedStart = getRot() * vec3(0, getStart().y + yrh, getStart().z + zrh);
				vec3 rotatedEnd = getRot() * vec3(0, getEnd().y - yrh, getEnd().z + zrh);
				vs.emplace_back(rotatedStart + pos1, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedStart + pos2, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedEnd + pos1, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedEnd + pos2, vec2(), vec3(), WHITE);
			}
			vs.emplace_back(getRot() * getStart() + pos1, vec2(), vec3(), WHITE);
			vs.emplace_back(getRot() * getEnd() + pos1, vec2(), vec3(), WHITE);

			for (int i = 0; i < SECTORS_PER_SEMICIRCLE * 4; i += 4) {
				// Join caps
				// Start
				is.push_back(i + 0);
				is.push_back(i + 1);
				is.push_back(i + 4);
				is.push_back(i + 4);
				is.push_back(i + 1);
				is.push_back(i + 5);
				// End
				is.push_back(i + 2);
				is.push_back(i + 3);
				is.push_back(i + 6);
				is.push_back(i + 6);
				is.push_back(i + 3);
				is.push_back(i + 7);

				// Semicircles
				// Start
				is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4);
				is.push_back(i + 4);
				is.push_back(i + 0);
				// End
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
		} else {
			// Caps

			for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
				float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI + getHalfArcAngle();
				float y = sinf(ang);
				float z = cosf(ang);
				float yrh = y * getMinorRadius();
				float zrh = z * getMinorRadius();
				vec3 rotatedStart = getRot() * vec3(0, getStart().y + yrh, getStart().z + zrh);
				vec3 rotatedEnd = getRot() * vec3(0, getEnd().y - yrh, getEnd().z + zrh);
				vs.emplace_back(rotatedStart + pos1, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedStart + pos2, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedEnd + pos1, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedEnd + pos2, vec2(), vec3(), WHITE);
			}
			vs.emplace_back(getRot() * getStart() + pos1, vec2(), vec3(), WHITE);
			vs.emplace_back(getRot() * getEnd() + pos1, vec2(), vec3(), WHITE);

			for (int i = 0; i < SECTORS_PER_SEMICIRCLE * 4; i += 4) {
				// Join caps
				// Start
				is.push_back(i + 0);
				is.push_back(i + 1);
				is.push_back(i + 4);
				is.push_back(i + 4);
				is.push_back(i + 1);
				is.push_back(i + 5);
				// End
				is.push_back(i + 2);
				is.push_back(i + 3);
				is.push_back(i + 6);
				is.push_back(i + 6);
				is.push_back(i + 3);
				is.push_back(i + 7);

				// Semicircles
				// Start
				is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4);
				is.push_back(i + 4);
				is.push_back(i + 0);
				// End
				is.push_back((SECTORS_PER_SEMICIRCLE + 1) * 4 + 1);
				is.push_back(i + 2);
				is.push_back(i + 6);
			}

			// Banana

			const byte NUM_SECTORS = (byte)ceil((float)SECTORS_PER_SEMICIRCLE * 2.0f * getHalfArcAngle() / PI);
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = (float)i / (float)NUM_SECTORS * 2.0f * getHalfArcAngle() - getHalfArcAngle();
				float y = sinf(ang);
				float z = cosf(ang);
				float yroh = y * (getMajorRadius() + getMinorRadius());
				float zroh = z * (getMajorRadius() + getMinorRadius());
				float yrih = y * (getMajorRadius() - getMinorRadius());
				float zrih = z * (getMajorRadius() - getMinorRadius());
				vec3 rotatedInner = getRot() * vec3(0, yrih, zrih);
				vec3 rotatedOuter = getRot() * vec3(0, yroh, zroh);
				vs.emplace_back(rotatedInner + pos1, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedInner + pos2, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedOuter + pos1, vec2(), vec3(), WHITE);
				vs.emplace_back(rotatedOuter + pos2, vec2(), vec3(), WHITE);
			}
			const int START_INDEX = 4 * (SECTORS_PER_SEMICIRCLE + 1) + 2;
			for (int i = START_INDEX; i < START_INDEX + NUM_SECTORS * 4; i += 4) {
				// Join bananas
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
	}   break;
	case ST_ANG: {
		float arcAngle;
		float arc1Radius, arc2Radius;
		float start1, start2;

        if (getIsStraight()) {
            arcAngle = std::clamp(getStateB().x - getStateA().x, -2 * PI, 2 * PI);
			arc1Radius = getStart().length() + getMinorRadius() - 0.5f * OUTLINE_WIDTH_WORLD;
			arc2Radius = getEnd().length() + getMinorRadius() - 0.5f * OUTLINE_WIDTH_WORLD;
			start1 = getStateA().x;
			start2 = getStateA().x + PI;
		} else {
			float angDiff = getStateB().x - getStateA().x;
			if (angDiff < 0) {
                arcAngle = std::clamp(angDiff + 2 * getHalfArcAngle(), -2 * PI + 2 * getHalfArcAngle(), 0.0f);
				start1 = start2 = getStateA().x + PI * 0.5f - getHalfArcAngle();
			} else {
                arcAngle = std::clamp(angDiff - 2 * getHalfArcAngle(), 0.0f, 2 * PI - 2 * getHalfArcAngle());
				start1 = start2 = getStateA().x + PI * 0.5f + getHalfArcAngle();
			}
			arc1Radius = getMajorRadius() + getMinorRadius() - 0.5f * OUTLINE_WIDTH_WORLD;
			arc2Radius = getMajorRadius() - getMinorRadius() + 0.5f * OUTLINE_WIDTH_WORLD;
		}

		float absArcAngle = fabsf(arcAngle);
		float sign = arcAngle < 0 ? -1.0f : 1.0f;

		float arc1Length = arc1Radius * absArcAngle;
		float arc2Length = arc2Radius * absArcAngle;

		int numDots1 = 2 * (((int)(arc1Length / OUTLINE_WIDTH_WORLD * 0.5f) + 1) / 2);
		int numDots2 = 2 * (((int)(arc2Length / OUTLINE_WIDTH_WORLD * 0.5f) + 1) / 2);

		if (numDots1 > 1000 || numDots2 > 1000)
			break; // Too many dots, skip drawing

		float dotSpacing = OUTLINE_WIDTH_WORLD * 2;
		float dotShift1 = sign * dotSpacing / arc1Radius;
		float dotShift2 = sign * dotSpacing / arc2Radius;
		start1 += sign * (arc1Length - (dotSpacing * (float)(numDots1 - 1))) / arc1Radius * 0.5f;
		start2 += sign * (arc2Length - (dotSpacing * (float)(numDots2 - 1))) / arc2Radius * 0.5f;

		float dotAngle = start1;
		for (int d = 0; d < numDots1; d++) {
			vec3 dotCentre = vec3(0, cosf(dotAngle), sinf(dotAngle)) * arc1Radius;
			vs.emplace_back(dotCentre, vec2(), vec3(), WHITE);
			for (byte i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
				vs.emplace_back(dotCentre + vec3(0, cosf(ang), sinf(ang)) * OUTLINE_WIDTH_WORLD * 0.5f, vec2(), vec3(), WHITE);
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
			vec3 dotCentre = vec3(0, cosf(dotAngle), sinf(dotAngle)) * arc2Radius;
			vs.emplace_back(dotCentre, vec2(), vec3(), WHITE);
			for (byte i = 0; i < SECTORS_PER_DOT; i++) {
				float ang = (float)i / (float)SECTORS_PER_DOT * 2 * PI;
				vs.emplace_back(dotCentre + vec3(0, cosf(ang), sinf(ang)) * OUTLINE_WIDTH_WORLD * 0.5f, vec2(), vec3(), WHITE);
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
	}   break;
	case ST_ANG_VEL:
		if (getIsStraight()) {
			vs.emplace_back(vec3(), vec2(), vec3(), WHITE);
			for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
				float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				float y = sinf(ang);
				float z = cosf(ang);
				float radius = getMajorRadius() + getMinorRadius();
				vs.emplace_back(vec3(0, y * radius, z * radius), vec2(), vec3(), WHITE);
				is.emplace_back(0);
				is.emplace_back(1 + (i + 1) % SECTORS_PER_CIRCLE);
				is.emplace_back(1 + i);
			}
		} else {
			for (int i = 0; i < SECTORS_PER_CIRCLE; i++) {
				float ang = (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
				float y = sinf(ang);
				float z = cosf(ang);
				float innerRadius = getMajorRadius() - getMinorRadius();
				float outerRadius = getMajorRadius() + getMinorRadius();
				vs.emplace_back(vec3(0, y * innerRadius, z * innerRadius), vec2(), vec3(), WHITE);
				vs.emplace_back(vec3(0, y * outerRadius, z * outerRadius), vec2(), vec3(), WHITE);
				is.emplace_back(i*2);
				is.emplace_back(i*2 + 1);
				is.emplace_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
				is.emplace_back(i*2);
				is.emplace_back((i*2 + 3) % (SECTORS_PER_CIRCLE * 2));
				is.emplace_back((i*2 + 2) % (SECTORS_PER_CIRCLE * 2));
			}
		}
		break;
	case ST_ANG_OSC:
		if (getIsStraight()) {
			float domainStartAngle, domainEndAngle;
			short START_INDEX = 0;

			if (fabsf(getStateB().y - getStateA().y) >= 2 * PI) {
				domainStartAngle = 0;
				domainEndAngle = 2 * PI;
			} else {
				if (getStateB().y > getStateA().y) {
					domainStartAngle = getStateA().y;
					domainEndAngle = getStateB().y;
				} else {
					domainStartAngle = getStateB().y;
					domainEndAngle = getStateA().y;
				}

				for (byte j = 0; j < 2; j++) {
					float currentAngle = j ? domainEndAngle : domainStartAngle;
					mat3 currentRot;
					currentRot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, currentAngle);
					vec3 start = currentRot * getStart();
					vec3 end = currentRot * getEnd();

					vs.emplace_back(start, vec2(), vec3(), getColor());
					vs.emplace_back(end, vec2(), vec3(), getColor());
					for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
						float angStartCap = currentAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
						float angEndCap = currentAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
						vs.emplace_back(vec3(0, start.y - sinf(angStartCap) * getMinorRadius(), start.z + cosf(angStartCap) * getMinorRadius()), vec2(), vec3(), WHITE);
						vs.emplace_back(vec3(0, end.y - sinf(angEndCap) * getMinorRadius(), end.z + cosf(angEndCap) * getMinorRadius()), vec2(), vec3(), WHITE);
					}
					for (int i = START_INDEX + 2; i < START_INDEX + 2 + SECTORS_PER_SEMICIRCLE * 2; i += 2) {
						// Caps
						// Start
						is.push_back(START_INDEX + 0);
						is.push_back(i + 0);
						is.push_back(i + 2);
						// End
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

			vs.emplace_back(vec3(), vec2(), vec3(), getColor());

			const float startRadius = getStart().length() + getMinorRadius();
			const float endRadius = getEnd().length() + getMinorRadius();
			const byte NUM_SECTORS = (byte)ceil((float)SECTORS_PER_SEMICIRCLE * (domainEndAngle - domainStartAngle) / PI);
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = domainStartAngle + (float)i / (float)NUM_SECTORS * (domainEndAngle - domainStartAngle);
				vec3 dir = vec3(0, cosf(ang), sinf(ang));
				vs.emplace_back(dir * startRadius, vec2(), vec3(), WHITE);
				vs.emplace_back(dir * -endRadius, vec2(), vec3(), WHITE);
			}

			for (int i = START_INDEX + 1; i < START_INDEX + 1 + NUM_SECTORS * 2; i += 2) {
				is.push_back(START_INDEX);
				is.push_back(i + 0);
				is.push_back(i + 2);
				is.push_back(START_INDEX);
				is.push_back(i + 1);
				is.push_back(i + 3);
			}
		} else {
			float startAngle, endAngle;
			vec3 start, end;
			short START_INDEX;

			if (fabsf(getStateB().y - getStateA().y) + 2 * getHalfArcAngle() >= 2 * PI) {
				startAngle = 0;
				endAngle = 2 * PI;
				start = end = {0, 0, getMajorRadius()};
				START_INDEX = 0;
			} else {
				if (getStateB().y > getStateA().y) {
					startAngle = getStateA().y - getHalfArcAngle();
					endAngle = getStateB().y + getHalfArcAngle();
				} else {
					startAngle = getStateB().y - getHalfArcAngle();
					endAngle = getStateA().y + getHalfArcAngle();
				}
				start = vec3(0, -sinf(startAngle), cosf(startAngle)) * getMajorRadius();
				end = vec3(0, -sinf(endAngle), cosf(endAngle)) * getMajorRadius();

				vs.emplace_back(start, vec2(), vec3(), getColor());
				vs.emplace_back(end, vec2(), vec3(), getColor());
				for (int i = 0; i <= SECTORS_PER_SEMICIRCLE; i++) {
					float angStartCap = startAngle - (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
					float angEndCap = endAngle + (float)i / (float)SECTORS_PER_SEMICIRCLE * PI;
					vs.emplace_back(vec3(0, start.y - sinf(angStartCap) * getMinorRadius(), start.z + cosf(angStartCap) * getMinorRadius()), vec2(), vec3(), WHITE);
					vs.emplace_back(vec3(0, end.y - sinf(angEndCap) * getMinorRadius(), end.z + cosf(angEndCap) * getMinorRadius()), vec2(), vec3(), WHITE);
				}
				for (int i = 2; i < 2 + SECTORS_PER_SEMICIRCLE * 2; i += 2) {
					// Caps
					// Start
					is.push_back(0);
					is.push_back(i + 0);
					is.push_back(i + 2);
					// End
					is.push_back(1);
					is.push_back(i + 1);
					is.push_back(i + 3);
				}

				START_INDEX = 2 + (SECTORS_PER_SEMICIRCLE + 1) * 2;
			}

			// Banana

			const byte NUM_SECTORS = (byte)ceil((float)SECTORS_PER_SEMICIRCLE * (endAngle - startAngle) / PI);
			for (int i = 0; i <= NUM_SECTORS; i++) {
				float ang = startAngle + (float)i / (float)NUM_SECTORS * (endAngle - startAngle);
				vec3 dir = vec3(0, -sinf(ang), cosf(ang));
				vs.emplace_back(dir * (getMajorRadius() - getMinorRadius()), vec2(), vec3(), WHITE);
				vs.emplace_back(dir * (getMajorRadius() + getMinorRadius()), vec2(), vec3(), WHITE);
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
		break;
	default:
		break;
	}

	domain.sendToGpu();
	outlineShader->setupVertexAttribs();
}

void Obstacle::createAllModels(bool inEditor) {
	createObstacleModel();
	if (inEditor) {
		createOutlineModel();
		if (getStateType() != ST_STATIC)
			createDomainModel();
	}
}


PlaneDefinition Obstacle::getDividingPlane(bool startCap) const {
	float ang = startCap ? getAngle() - getHalfArcAngle() + PI : getAngle() + getHalfArcAngle();
	float y = cosf(ang);
	float z = sinf(ang);
	vec3 normal = {0, y, z};
	return {normal, dot(normal, startCap ? getPos() + getRot() * getStart() : getPos() + getRot() * getEnd())};
}

PlaneDefinition Obstacle::getTopPlane() const {
	float y = -sinf(getAngle());
	float z = cosf(getAngle());
	vec3 normal = {0, y, z};
	return {normal, dot(normal, getPos() + getRot() * (getStart() + vec3(0, 0, getMinorRadius())))};
}

void Obstacle::restart() {
	setPos(getInitPos());
	setVel(0);
	setAngle(getInitAngle());
	setAngVel(getStateType() == ST_ANG_VEL ? getStateA().x : 0);
	setExtra(0);
}