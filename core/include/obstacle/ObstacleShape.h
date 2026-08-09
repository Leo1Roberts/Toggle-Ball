#ifndef OBSTACLE_SHAPE_H
#define OBSTACLE_SHAPE_H

#include "opengl/Mesh.h"

#include "glm/gtc/constants.hpp"

class GameBall;
class ObstacleKinematicState;
struct PlaneDescriptor;
struct SelectBox;


struct BallCollisionInfo {
	bool colliding{};
	glm::vec3 normal{};
	float separation{};
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

	virtual void scale(float factor) = 0;

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh, float uiToWorldScale) const;
	virtual void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

	void scaleBy(float factor, bool affectMinorRadius, bool affectMajorRadius, const AbstractShapeSpec* base);
	virtual void scaleMajorRadiusBy(float factor, const AbstractShapeSpec* base) = 0;

	[[nodiscard]] bool isInSelectBox(const ObstacleKinematicState& s, SelectBox box) const;

	[[nodiscard]] virtual bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const = 0;
	// Only sets all members if .collision == true
	[[nodiscard]] virtual BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) = 0;

	[[nodiscard]] float getMinorRadius() const { return minorRadius; }
	[[nodiscard]] float getBoundingRadius() const { return getMajorRadius() + getMinorRadius(); }
	[[nodiscard]] glm::vec2 getLeftCap() const { return leftCap; }
	[[nodiscard]] glm::vec2 getRightCap() const { return rightCap; }
	[[nodiscard]] virtual float getLeftCapAngle() const = 0;
	[[nodiscard]] virtual float getRightCapAngle() const = 0;
	[[nodiscard]] float getHalfDepth() const { return getMinorRadius(); }

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
	float minorRadius; // SSOT

	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	[[nodiscard]] virtual bool equals(const AbstractShapeSpec& other) const = 0;

	virtual void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const = 0;
	virtual void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const = 0;

	[[nodiscard]] virtual bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const = 0;

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
		*this = SegmentSpec(getMinorRadius() * factor, leftLength * factor, rightLength * factor);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void scaleMajorRadiusBy(float factor, const AbstractShapeSpec* base) override;

	[[nodiscard]] bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const override {
		return leftPlaneDistance <= 0 && rightPlaneDistance <= 0;
	}
	[[nodiscard]] BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) override;

	void setLeftLength(float len);
	void setRightLength(float len);

	[[nodiscard]] float getLeftCapAngle() const override { return glm::pi<float>(); }
	[[nodiscard]] float getRightCapAngle() const override { return 0; }
	[[nodiscard]] float getLeftLength() const { return leftLength; }
	[[nodiscard]] float getRightLength() const { return rightLength; }

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

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const override;
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
		*this = ArcSpec(getMinorRadius() * factor, arcAngle, arcRadius * factor);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void scaleMajorRadiusBy(float factor, const AbstractShapeSpec* base) override;

	[[nodiscard]] bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const override {
		return getArcAngle() <= glm::pi<float>() && (leftPlaneDistance <= 0 && rightPlaneDistance <= 0) ||
			   getArcAngle() >  glm::pi<float>() && (leftPlaneDistance <= 0 || rightPlaneDistance <= 0);
	}
	[[nodiscard]] BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) override;

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

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const override;

	void setCaps();

	[[nodiscard]] float getMajorRadius() const override { return arcRadius; }
};


#endif // OBSTACLE_SHAPE_H