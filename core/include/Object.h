#ifndef OBJECT_H
#define OBJECT_H

#include "TextureAsset.h"
#include "Model.h"
#include "Sizes.h"

enum {
	MAT_BASKETBALL,
	MAT_CONCRETE,
	MAT_NUM
};

const float FRICTION_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
0.5f, 0.58f,
0.58f, 0.4f};

const float ROLLING_RESISTANCE_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
0.02f, 0.015f,
0.015f, 0.01f};

const float GRAVITY = -9.81f;
const float AIR_DENSITY = 1.225f;
//const float DYNAMIC_VISCOSITY = 0.000018f;

const vec3 OBSTACLE_ROTATION_AXIS = {1, 0, 0};

const float MINIMUM_ARENA_SIZE = 5;
const float MAXIMUM_ARENA_SIZE = 200;

const float MINIMUM_POS_X = -MAXIMUM_ARENA_SIZE * 0.7f;
const float MAXIMUM_POS_X = MAXIMUM_ARENA_SIZE * 0.7f;
const float MINIMUM_POS_Y = -MAXIMUM_ARENA_SIZE * 0.2f;
const float MAXIMUM_POS_Y = MAXIMUM_ARENA_SIZE * 1.2f;

const float MINIMUM_TRANSITION_TIME = 0.1f;
const float MAXIMUM_TRANSITION_TIME = 20;

const float MINIMUM_MINOR_RADIUS = 0.25f;
const float MAXIMUM_MINOR_RADIUS = 50;

const float MAXIMUM_MAJOR_RADIUS = 200;

const float MINIMUM_ANGLE = -5 * PI; // -900°
const float MAXIMUM_ANGLE = 5 * PI; // 900°

const float MINIMUM_RPM = -120;
const float MAXIMUM_RPM = 120;

const float MINIMUM_OPM = MINIMUM_RPM * 0.5f;
const float MAXIMUM_OPM = MAXIMUM_RPM * 0.5f;


struct PlaneDefinition {
	vec3 normal;
	float dotProduct;
};

struct Plane {
	TextureAsset* texture;
	col color;
	byte material;

	PlaneDefinition definition;

	vec3 scale;
	vec3 pos;
	mat3 rot;

	Plane() :
	    texture(whiteTex),
		color(WHITE),
		material(0),
		definition({{0, 0, 1}, 0}) {}

	Plane(TextureAsset* iTexture, const col& iColor, byte iMaterial, const vec3& iNormal, const vec3& iScale = {1}, const vec3& iPos = {0}) :
		texture(iTexture),
		color(iColor),
		material(iMaterial),
		definition({iNormal, dot(iNormal, iPos)}),
		scale(iScale),
		pos(iPos) {
		vec3 baseNormal = {0, 0, 1};
		if (iNormal.z == -1)
			rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, PI);
		else
			rot.R_TwoVectors(baseNormal, iNormal);
	}

	void setPos(const vec3& iPos) {
		pos = iPos;
		definition.dotProduct = dot(pos, definition.normal);
	}
};

struct BallProperties {
	byte material;
	float mass;
	float moi;
	float springK;
	float dampingK;
	float radius;
	float Cd;
	TextureAsset* texture;
};

enum {
	BASKETBALL,
	FOOTBALL,
	PING_PONG,
	MARBLE
};

struct Ball {
	TextureAsset* texture;
	byte material;

	float mass;
	float springK;
	float springKInv;
	float dampingK;
	float dampingKInv;
	float radius;
	float moi;
	float Cd;

	vec3 scale;
	vec3 pos;
	mat3 rot;
	vec3 vel;
	vec3 angVel;
	vec3 force;
	vec3 torque;

	Ball() :
		texture(whiteTex),
		material(0),
		mass(0),
		springK(0),
		springKInv(0),
		dampingK(0),
		dampingKInv(0),
		radius(0),
		moi(0),
		Cd(0),
		scale(1),
		pos(0),
		rot(I_MAT3),
		vel(0),
		angVel(0),
		force(0),
		torque(0) {
	}

	Ball(byte ballType, float normalise = false) : Ball(getProperties(ballType, normalise)) {}

	Ball(BallProperties properties) :
		texture(properties.texture),
		material(properties.material),
		mass(properties.mass),
		springK(properties.springK),
		springKInv(1.0f / properties.springK),
		dampingK(properties.dampingK),
		dampingKInv(1.0f / properties.dampingK),
		radius(properties.radius),
		moi(properties.moi),
		Cd(properties.Cd),
		scale(vec3(properties.radius)),
		pos(0),
		rot(I_MAT3),
		vel(0),
		angVel(0),
		force(0),
		torque(0) {}

private:
	static BallProperties getProperties(byte ballType, bool normalise);
};

enum {
	ST_STATIC,
	ST_POS,
	ST_POS_OSC,
	ST_ANG,
	ST_ANG_VEL,
	ST_ANG_OSC,
	ST_NUM
};

struct ObstacleDefinition {
	vec3 initPos;
	float initAngle;
	bool isStraight;
	vec3 start; // Shape will be an arc from start to end, anticlockwise OR a straight line from start to end, right to left
	vec3 end;
	float majorRadius; // Derived from start and end
	float minorRadius;
	float halfArcAngle; // Derived from start
	byte stateType;
	vec3 stateA; // Different interpretations depending on stateType:
	// ST_STATIC : { 0				, 0		  , 0		   } (unused)
	// ST_POS	: { 0				, positionA.y, positionA.z } (positionA == initPos)
	// ST_POS_OSC: { angularFrequencyA, position1.y, position1.z } (position1 == initPos)
	// ST_ANG	: { angleA		   , 0		  , 0		   } (angleA == initAngle)
	// ST_ANG_VEL: { angularVelocityA , 0		  , 0		   }
	// ST_ANG_OSC: { angularFrequencyA, angle1		  , 0		   } (angle1 == initAngle)
	vec3 stateB; // Different interpretations depending on stateType:
	// ST_STATIC : { 0				, 0		  , 0		   } (unused)
	// ST_POS	: { 0				, positionB.y, positionB.z }
	// ST_POS_OSC: { angularFrequencyB, position2.y, position2.z }
	// ST_ANG	: { angleB		   , 0		  , 0		   }
	// ST_ANG_VEL: { angularVelocityB , 0		  , 0		   }
	// ST_ANG_OSC: { angularFrequencyB, angle2		  , 0		   }
	bool isGoal;
	col color; // Derived from stateType

	ObstacleDefinition() :
		initPos(0),
		initAngle(0),
		isStraight(false),
		start(0),
		end(0),
		majorRadius(0),
		minorRadius(0),
		halfArcAngle(0),
		stateType(0),
		stateA(0),
		stateB(0),
		isGoal(false),
		color(WHITE) {}

	ObstacleDefinition(const vec3& iInitPos, float iInitAngle, bool iIsStraight, const vec3& iStart, const vec3& iEnd, float iMinorRadius, byte iStateType, const vec3& iStateA, const vec3& iStateB, bool iIsGoal) :
		initPos(iInitPos),
		initAngle(iInitAngle),
		isStraight(iIsStraight),
		start(iStart),
		end(iEnd),
		minorRadius(iMinorRadius),
		halfArcAngle(iIsStraight ? 0 : atan2f(iStart.y, iStart.z)),
		stateType(iStateType),
		stateA(iStateA),
		stateB(iStateB),
		isGoal(iIsGoal) {
		majorRadius = fmax(start.length(), end.length());
		if (isGoal)
			color = SOFT_GREEN;
		else
			switch (stateType) {
			case ST_STATIC:
				color = WHITE;
				break;
			case ST_POS:
				color = SOFT_BLUE;
				break;
			case ST_POS_OSC:
				color = SOFT_CYAN;
				break;
			case ST_ANG:
				color = SOFT_RED;
				break;
			case ST_ANG_VEL:
				color = SOFT_MAGENTA;
				break;
			case ST_ANG_OSC:
				color = SOFT_YELLOW;
				break;
			}
	}

	bool operator==(const ObstacleDefinition& other) const;

	ObstacleDefinition scale(float scale) const;
};

struct Obstacle {
	TextureAsset* texture;
	Model obstacle;
	Model outline;
	Model domain;

	Obstacle(ObstacleDefinition* definition, float scale) :
	    pDef(definition),
	    def(definition->scale(scale)),
		texture(whiteTex),
		color(definition->color),
		material(MAT_CONCRETE),
		scale(1),
		vel(0),
		angVel(0),
		extra(0),
		obstacle(),
		outline(),
		domain() {
		setPos(def.initPos);
		setAngle(def.initAngle);
	}

	Obstacle(const Obstacle&) = delete;
	Obstacle& operator=(const Obstacle&) = delete;

	Obstacle(Obstacle&&) noexcept = default;
	Obstacle& operator=(Obstacle&&) noexcept = default;

	inline vec3 getInitPos() const { return def.initPos; }
	inline float getInitAngle() const { return def.initAngle; }
	inline bool getIsStraight() const { return def.isStraight; }
	inline vec3 getStart() const { return def.start; }
	inline vec3 getEnd() const { return def.end; }
	inline float getMajorRadius() const { return def.majorRadius; }
	inline float getMinorRadius() const { return def.minorRadius; }
	inline float getHalfArcAngle() const { return def.halfArcAngle; }
	inline byte getStateType() const { return def.stateType; }
	inline vec3 getStateA() const { return def.stateA; }
	inline vec3 getStateB() const { return def.stateB; }
	inline bool getIsGoal() const { return def.isGoal; }
	inline col getColor() const { return def.color; }

	inline vec3 getPos() const { return pos; }
	inline float getAngle() const { return angle; }
	inline mat3 getRot() const { return rot; }
	inline vec3 getVel() const { return vel; }
	inline vec3 getAngVel() const { return angVel; }
	inline float getExtra() const { return extra; }
	inline vec3 getScale() const { return scale; }
	inline byte getMaterial() const { return material; }


	inline void setInitPos(const vec3& iPos) { pDef->initPos = def.initPos = iPos; }
	inline void setInitAngle(float iAngle) { pDef->initAngle = def.initAngle = iAngle; }
	inline void setIsStraight(bool iIsStraight) { pDef->isStraight = def.isStraight = iIsStraight; }
	inline void setStart(const vec3& iStart) { pDef->start = def.start = iStart; }
	inline void setStartY(float iStartY) { pDef->start.y = def.start.y = iStartY; }
	inline void setStartZ(float iStartZ) { pDef->start.z = def.start.z = iStartZ; }
	inline void setEnd(const vec3& iEnd) { pDef->end = def.end = iEnd; }
	inline void setEndY(float iEndY) { pDef->end.y = def.end.y = iEndY; }
	inline void setEndZ(float iEndZ) { pDef->end.z = def.end.z = iEndZ; }
	inline void setMajorRadius(float iMajorRadius) { pDef->majorRadius = def.majorRadius = iMajorRadius; }
	inline void setMinorRadius(float iMinorRadius) { pDef->minorRadius = def.minorRadius = iMinorRadius; }
	inline void setHalfArcAngle(float iHalfArcAngle) { pDef->halfArcAngle = def.halfArcAngle = iHalfArcAngle; }
	//inline void setStateType(byte iStateType) { pDef->stateType = def.stateType = iStateType; }
	//inline void setIsGoal(bool iIsGoal) { pDef->isGoal = def.isGoal = iIsGoal; }
	inline void setStateA(const vec3& iStateA) { pDef->stateA = def.stateA = iStateA; }
	inline void setStateAX(float iStateAX) { pDef->stateA.x = def.stateA.x = iStateAX; }
	inline void setStateAY(float iStateAY) { pDef->stateA.y = def.stateA.y = iStateAY; }
	inline void setStateB(const vec3& iStateB) { pDef->stateB = def.stateB = iStateB; }
	inline void setStateBX(float iStateBX) { pDef->stateB.x = def.stateB.x = iStateBX; }
	inline void setStateBY(float iStateBY) { pDef->stateB.y = def.stateB.y = iStateBY; }

	void setPos(const vec3& iPos) { pos.set(0, iPos.y, iPos.z); }
	void setAngle(float iAngle);
	inline void setVel(const vec3& iVel) { vel.set(0, iVel.y, iVel.z); }
	inline void setAngVel(float iAngVel) { angVel = vec3(iAngVel, 0, 0); }
	inline void setExtra(float iExtra) { extra = iExtra; }


	void createObstacleModel();
	void createOutlineModel();
	void createDomainModel();
	void createAllModels(bool inEditor = true);


	void restart();

	[[nodiscard]] PlaneDefinition getDividingPlane(bool startCap) const;

	[[nodiscard]] PlaneDefinition getTopPlane() const;

private:
	ObstacleDefinition* pDef;
	ObstacleDefinition def;

	col color;
	byte material;

	vec3 scale;
	vec3 pos;
	float angle;
	mat3 rot;
	vec3 vel;
	vec3 angVel;
	float extra; // If isGoal, the number of seconds in contact with the ball. If stateType == ST_POS_OSC, the stage of the oscillation (from 0 to 2*PI). Otherwise 0 (unused).
};

#endif // OBJECT_H
