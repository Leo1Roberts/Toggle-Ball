#ifndef OBSTACLE_SHAPE_H
#define OBSTACLE_SHAPE_H

#include "opengl/Mesh.h"

#include "glm/gtc/constants.hpp"
#include "utilities/Utilities.h"

#include <algorithm>
#include <optional>

class GameBall;
class ObstacleKinematicState;
struct PlaneDescriptor;
struct SelectBox;


struct BallCollisionInfo {
	bool colliding{};
	glm::vec3 normal{};
	float separation{};
};

struct ProximityInfo {
	float distance; // Positive distance means point is outside the shape, negative for inside
	glm::vec2 direction; // Direction from target to point, not normalised
};


constexpr int SECTORS_PER_SEMICIRCLE = 64;
constexpr int SECTORS_PER_CIRCLE = SECTORS_PER_SEMICIRCLE * 2;
constexpr int SECTORS_PER_DOT = 8;
constexpr float BEVEL_AMOUNT = 0.1f;


class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

	[[nodiscard]] virtual std::unique_ptr<AbstractShapeSpec> clone() const = 0;

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<AbstractShapeSpec> deserialize(const std::string& data);

	bool operator==(const AbstractShapeSpec& other) const;

	enum class Property : int {
		MinorRadius,

		LeftLength, RightLength,

		ArcAngle, ArcRadius,
	};

	static std::string getPropertyName(Property property) {
		switch (property) {
		case Property::MinorRadius: return "Minor radius";
		case Property::LeftLength:  return "Left length";
		case Property::RightLength: return "Right length";
		case Property::ArcAngle:    return "Arc angle";
		case Property::ArcRadius:   return "Arc radius";
		default: return "Unknown";
		}
	}

	[[nodiscard]] std::vector<Property> getProperties() const;
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, Property property) const;
	void setProperty(float value, Property property);

	virtual void scale(float factor) = 0;

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh, float uiToWorldScale) const;
	virtual void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

	void scaleBy(float factor, bool affectMinorRadius, bool affectMajorRadius, const AbstractShapeSpec* base);
	virtual void scaleMajorRadiusBy(float factor, const AbstractShapeSpec* base) = 0;

	[[nodiscard]] bool isInSelectBox(const ObstacleKinematicState& s, SelectBox box) const;

	[[nodiscard]] virtual bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const = 0;
	// Only sets all members if .collision == true. TODO: use std::optional instead
	[[nodiscard]] virtual BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) = 0;
	[[nodiscard]] ProximityInfo getRimProximity(const ObstacleKinematicState& kinematicState, glm::vec2 point) const;
	[[nodiscard]] ProximityInfo getSpineProximity(const ObstacleKinematicState& kinematicState, glm::vec2 point) const;
	[[nodiscard]] virtual std::vector<glm::vec2> getPointsOnLine(const ObstacleKinematicState& kinematicState, glm::vec2 pointOnLine, float lineAngle) const = 0;
	[[nodiscard]] virtual std::vector<glm::vec2> getPointsOnCircle(const ObstacleKinematicState& kinematicState, glm::vec2 circleCentre, float circleRadius) const = 0;

	[[nodiscard]] float getBoundingRadius() const { return getMajorRadius() + minorRadius; }
	[[nodiscard]] glm::vec2 getLeftCap() const { return leftCap; }
	[[nodiscard]] glm::vec2 getRightCap() const { return rightCap; }
	[[nodiscard]] virtual float getLeftCapAngle() const = 0;
	[[nodiscard]] virtual float getRightCapAngle() const = 0;
	[[nodiscard]] float getHalfDepth() const { return minorRadius; }

	float minorRadius; // SSOT

protected:
	glm::vec2 leftCap{}, rightCap{};

	explicit AbstractShapeSpec(float minorRadius) :
	    minorRadius(minorRadius) {}

	AbstractShapeSpec(const AbstractShapeSpec&) = default;
	AbstractShapeSpec& operator=(const AbstractShapeSpec&) = default;
	AbstractShapeSpec(AbstractShapeSpec&&) = default;
	AbstractShapeSpec& operator=(AbstractShapeSpec&&) = default;

	[[nodiscard]] float getBevel() const;

private:
	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	[[nodiscard]] virtual bool equals(const AbstractShapeSpec& other) const = 0;

	[[nodiscard]] constexpr virtual std::vector<Property> getSpecificProperties() const = 0;
	[[nodiscard]] virtual std::optional<float> getSpecificProperty(bool convertUnits, Property property) const = 0;
	virtual void setSpecificProperty(float value, Property property) = 0;

	virtual void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const = 0;
	virtual void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const = 0;

	[[nodiscard]] virtual bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const = 0;

	[[nodiscard]] glm::vec2 getClosestSpineVector(const ObstacleKinematicState& kinematicState, glm::vec2 point) const;
	[[nodiscard]] virtual glm::vec2 getMidsectionSpineVector(glm::vec2 position, glm::mat2 rotation, glm::vec2 point) const = 0;

	[[nodiscard]] virtual float getMajorRadius() const = 0;
};

class SegmentSpec : public AbstractShapeSpec {
public:
	SegmentSpec(float minorRadius, float leftLength, float rightLength) :
	    AbstractShapeSpec(minorRadius) {
		setLeftLength(leftLength);
		setRightLength(rightLength);
	}
	SegmentSpec(float minorRadius, const std::string& data);

	~SegmentSpec() override = default;

	[[nodiscard]] std::unique_ptr<AbstractShapeSpec> clone() const override {
		return std::make_unique<SegmentSpec>(*this);
	}

	void scale(float factor) override {
		*this = SegmentSpec(minorRadius * factor, leftLength * factor, rightLength * factor);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void scaleMajorRadiusBy(float factor, const AbstractShapeSpec* base) override;

	[[nodiscard]] bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const override {
		return leftPlaneDistance <= 0 && rightPlaneDistance <= 0;
	}
	[[nodiscard]] BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) override;

	[[nodiscard]] std::vector<glm::vec2> getPointsOnLine(const ObstacleKinematicState& kinematicState, glm::vec2 pointOnLine, float lineAngle) const override;
	[[nodiscard]] std::vector<glm::vec2> getPointsOnCircle(const ObstacleKinematicState& kinematicState, glm::vec2 circleCentre, float circleRadius) const override;

	void setLeftLength(float len);
	void setRightLength(float len);

	[[nodiscard]] float getLeftCapAngle() const override { return glm::pi<float>(); }
	[[nodiscard]] float getRightCapAngle() const override { return 0; }
	[[nodiscard]] float getLeftLength() const { return leftLength; }
	[[nodiscard]] float getRightLength() const { return rightLength; }
	[[nodiscard]] float getLength() const { return leftLength + rightLength; }

private:
	float leftLength{};  // SSOT
	float rightLength{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "segment"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const AbstractShapeSpec& other) const override {
		auto otherSegment = (const SegmentSpec&)other;
		return leftLength == otherSegment.leftLength && rightLength == otherSegment.rightLength;
	}

	[[nodiscard]] constexpr std::vector<Property> getSpecificProperties() const override {
		return { Property::LeftLength, Property::RightLength };
	}
	[[nodiscard]] std::optional<float> getSpecificProperty(bool convertUnits, Property property) const override {
		switch (property) {
		case Property::LeftLength:  return leftLength;
		case Property::RightLength: return rightLength;
		default: return std::nullopt;
		}
	}
	void setSpecificProperty(float value, Property property) override {
		switch (property) {
		case Property::LeftLength:  setLeftLength(std::abs(value)); break;
		case Property::RightLength: setRightLength(std::abs(value)); break;
		default:;
		}
	}

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const override;
	[[nodiscard]] glm::vec2 getMidsectionSpineVector(glm::vec2 position, glm::mat2 rotation, glm::vec2 point) const override;
	[[nodiscard]] float getMajorRadius() const override { return std::max(leftLength, rightLength); }

	[[nodiscard]] PlaneDescriptor getTopPlane(const ObstacleKinematicState& kinematicState) const;
};

class ArcSpec : public AbstractShapeSpec {
public:
	ArcSpec(float minorRadius, float arcAngle, float arcRadius) :
	    AbstractShapeSpec(minorRadius),
	    arcAngle(arcAngle),
	    arcRadius(arcRadius) {
		setCaps();
	}
	ArcSpec(float minorRadius, const std::string& data);

	~ArcSpec() override = default;

	[[nodiscard]] std::unique_ptr<AbstractShapeSpec> clone() const override {
		return std::make_unique<ArcSpec>(*this);
	}

	void scale(float factor) override {
		*this = ArcSpec(minorRadius * factor, arcAngle, arcRadius * factor);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void scaleMajorRadiusBy(float factor, const AbstractShapeSpec* base) override;

	[[nodiscard]] bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const override {
		return getArcAngle() <= glm::pi<float>() && (leftPlaneDistance <= 0 && rightPlaneDistance <= 0) ||
			   getArcAngle() >  glm::pi<float>() && (leftPlaneDistance <= 0 || rightPlaneDistance <= 0);
	}
	[[nodiscard]] BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) override;

	[[nodiscard]] std::vector<glm::vec2> getPointsOnLine(const ObstacleKinematicState& kinematicState, glm::vec2 pointOnLine, float lineAngle) const override;
	[[nodiscard]] std::vector<glm::vec2> getPointsOnCircle(const ObstacleKinematicState& kinematicState, glm::vec2 circleCentre, float circleRadius) const override;

	void setArcAngle(float radians);
	void setArcRadius(float r);

	[[nodiscard]] float getLeftCapAngle() const override { return getHalfArcAngle() + glm::pi<float>(); }
	[[nodiscard]] float getRightCapAngle() const override { return -getHalfArcAngle(); }
	[[nodiscard]] float getArcAngle() const { return arcAngle; }
	[[nodiscard]] float getHalfArcAngle() const { return arcAngle / 2.f; }
	[[nodiscard]] float getArcRadius() const { return arcRadius; }

private:
	float arcAngle{};  // SSOT
	float arcRadius{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "arc"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const AbstractShapeSpec& other) const override {
		auto otherArc = (const ArcSpec&)other;
		return arcAngle == otherArc.arcAngle && arcRadius == otherArc.arcRadius;
	}

	[[nodiscard]] constexpr std::vector<Property> getSpecificProperties() const override {
		return { Property::ArcAngle, Property::ArcRadius };
	}
	[[nodiscard]] std::optional<float> getSpecificProperty(bool convertUnits, Property property) const override {
		switch (property) {
		case Property::ArcAngle:  return convertUnits ? std::abs(to_deg(arcAngle)) : arcAngle;
		case Property::ArcRadius: return arcRadius;
		default: return std::nullopt;
		}
	}
	void setSpecificProperty(float value, Property property) override {
		switch (property) {
		case Property::ArcAngle:  setArcAngle(std::clamp(std::abs(to_rad(value)), 0.f, glm::two_pi<float>())); break;
		case Property::ArcRadius: setArcRadius(std::abs(value)); break;
		default:;
		}
	}

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const override;
	[[nodiscard]] glm::vec2 getMidsectionSpineVector(glm::vec2 position, glm::mat2 rotation, glm::vec2 point) const override;

	void setCaps();

	[[nodiscard]] float getMajorRadius() const override { return arcRadius; }
};


#endif // OBSTACLE_SHAPE_H