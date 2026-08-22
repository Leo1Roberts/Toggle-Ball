#ifndef OBSTACLE_MOTION_H
#define OBSTACLE_MOTION_H

#include "glm/gtc/constants.hpp"
#include "opengl/Mesh.h"

#include <optional>

class AbstractShapeSpec;
class ObstacleKinematicState;
class Smoother;


inline float to_deg(float rad) { return -rad * glm::one_over_pi<float>() * 180.f; }
inline float to_rad(float deg) { return -deg / 180.f * glm::pi<float>(); }

inline float to_rpm(float radPerSec) { return -radPerSec * glm::one_over_pi<float>() * 30.f; }
inline float to_opm(float radPerSec) { return to_rpm(radPerSec); }
inline float to_radPerSec(float rpm_or_opm) { return -rpm_or_opm / 30.f * glm::pi<float>(); }


class IMotionSpec {
public:
	enum class Type : int {
		Static, TogglingPosition, TogglingAngle, Spinning, OscillatingPosition, OscillatingAngle, COUNT
	};

	static std::string getTypeName(Type type) {
		switch (type) {
		case Type::Static:
			return "Static";
		case Type::TogglingPosition:
			return "Toggling position";
		case Type::TogglingAngle:
			return "Toggling angle";
		case Type::Spinning:
			return "Spinning";
		case Type::OscillatingPosition:
			return "Oscillating position";
		case Type::OscillatingAngle:
			return "Oscillating angle";
		default:;
			return "Unknown";
		}
	}


	enum class Property : int {
		Position_X, Position_Y,
		Position1_X,Position1_Y, Position2_X,Position2_Y,

		Angle,
		InitialAngle,
		Angle1, Angle2,

		AngularSpeed,

		AngularFrequency,

		COUNT
	};
	enum class State : int { _, A, B, COUNT };

	struct PropertyDescriptor {
		Property property;
		State associatedState;

		bool operator==(const PropertyDescriptor&) const = default;
	};

	using PropertyValues = std::array<std::array<float, (int)State::COUNT> , (int)Property::COUNT>;
	using IncompletePropertyValues = std::array<std::array<std::optional<float>, (int)State::COUNT> , (int)Property::COUNT>;

	static std::string getPropertyName(Property property) {
		switch (property) {
		case Property::Position_X:       return "Position X";
		case Property::Position_Y:       return "Position Y";
		case Property::Position1_X:      return "Position 1 X";
		case Property::Position1_Y:      return "Position 1 Y";
		case Property::Position2_X:      return "Position 2 X";
		case Property::Position2_Y:      return "Position 2 Y";
		case Property::Angle:            return "Angle";
		case Property::InitialAngle:     return "Initial angle";
		case Property::Angle1:           return "Angle 1";
		case Property::Angle2:           return "Angle 2";
		case Property::AngularSpeed:     return "Revolutions/min";
		case Property::AngularFrequency: return "Oscillations/min";
		default: return "Unknown";
		}
	}

	virtual ~IMotionSpec() = default;

	IMotionSpec(const IMotionSpec&) = default;
	IMotionSpec& operator=(const IMotionSpec&) = default;
	IMotionSpec(IMotionSpec&&) = default;
	IMotionSpec& operator=(IMotionSpec&&) = default;

	[[nodiscard]] virtual std::unique_ptr<IMotionSpec> clone() const = 0;
	[[nodiscard]] static std::unique_ptr<IMotionSpec> make(Type type, const IncompletePropertyValues& values, bool toggled);

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<IMotionSpec> deserialize(const std::string& data);

	bool operator==(const IMotionSpec& other) const;

	virtual void scale(float factor) = 0;

	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const;

	// Initialises the kinematic state
	virtual void initKinematicState(ObstacleKinematicState& kinematicState) const = 0;
	// Updates the kinematic state by one physics frame. Purely incremental - kinematicState must be initialised separately.
	virtual void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const = 0;
	// Updates the (stationary) kinematic state for obstacles in the editor. Purely incremental - kinematicState must be initialised separately.
	virtual void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const = 0;


	virtual void translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) = 0;
	virtual void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool stateless, bool toggled, bool individual, const IMotionSpec* base) = 0;
	virtual void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) = 0;

	[[nodiscard]] virtual Type getType() const = 0;

	[[nodiscard]] constexpr virtual std::vector<PropertyDescriptor> getPropertyDescriptors() const = 0;
	[[nodiscard]] virtual std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const = 0;
	virtual void setProperty(float value, PropertyDescriptor desc) = 0;

	[[nodiscard]] virtual col getColor() const = 0;
	[[nodiscard]] virtual glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const = 0;

protected:
	IMotionSpec() = default;

private:
	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	[[nodiscard]] virtual bool equals(const IMotionSpec& other) const = 0;

	virtual void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const = 0;
};


class StaticSpec : public IMotionSpec {
public:
	StaticSpec(glm::vec2 position, float angle) :
	    position(position) {
		setAngle(angle);
	}
	explicit StaticSpec(const PropertyValues& allValues) {
		position.x = allValues[(int)Property::Position_X][(int)State::_];
		position.y = allValues[(int)Property::Position_Y][(int)State::_];
		setAngle(allValues[(int)Property::Angle][(int)State::_]);
	}

	explicit StaticSpec(const std::string& data);

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<StaticSpec>(*this);
	}

	~StaticSpec() override = default;

	void scale(float factor) override {
		*this = StaticSpec(position * factor, angle);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState&, const Smoother&) const override {}
	void updateEditorKinematicState(ObstacleKinematicState&, const Smoother&) const override {}

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const StaticSpec*)base)->position + vector;
	}
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool, const IMotionSpec* base) override;
	void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) override;

	void setAngle(float radians);

	[[nodiscard]] Type getType() const override { return Type::Static; }

	[[nodiscard]] constexpr std::vector<PropertyDescriptor> getPropertyDescriptors() const override {
		return {
			{ Property::Position_X, State::_ },
			{ Property::Position_Y, State::_ },
			{ Property::Angle,      State::_ },
		};
	}
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const override {
		switch (desc.property) {
		case Property::Position_X: return position.x;
		case Property::Position_Y: return position.y;
		case Property::Angle:      return convertUnits ? to_deg(angle) : angle;
		default: return std::nullopt;
		}
	}
	void setProperty(float value, PropertyDescriptor desc) override {
		switch (desc.property) {
		case Property::Position_X: position.x = value; break;
		case Property::Position_Y: position.y = value; break;
		case Property::Angle:      setAngle(to_rad(value)); break;
		default:;
		}
	}

	[[nodiscard]] col getColor() const override { return Color::White; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

private:
	glm::vec2 position{0.f}; // SSOT
	float angle{};           // SSOT
	glm::mat2 rotation{1.f};

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "static"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherStaticSpec = (const StaticSpec&)other;
		return position == otherStaticSpec.position && angle == otherStaticSpec.angle;
	}

	void buildDomainMesh(std::vector<ObjectVertex>&, std::vector<Index>&, const AbstractShapeSpec*, float) const override {}
};

class TogglingPositionSpec : public IMotionSpec {
public:
	TogglingPositionSpec(float angle, glm::vec2 positionA, glm::vec2 positionB) :
	    positionA(positionA),
	    positionB(positionB) {
		setAngle(angle);
	}
	explicit TogglingPositionSpec(const PropertyValues& allValues) {
		positionA.x = allValues[(int)Property::Position_X][(int)State::A];
		positionB.x = allValues[(int)Property::Position_X][(int)State::B];
		positionA.y = allValues[(int)Property::Position_Y][(int)State::A];
		positionB.y = allValues[(int)Property::Position_Y][(int)State::B];
		setAngle(allValues[(int)Property::Angle][(int)State::_]);
	}

	explicit TogglingPositionSpec(const std::string& data);

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingPositionSpec>(*this);
	}

	~TogglingPositionSpec() override = default;

	void scale(float factor) override {
		*this = TogglingPositionSpec(angle, positionA * factor, positionB * factor);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

	void translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) override;
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) override;
	void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) override;

	void setAngle(float radians);

	[[nodiscard]] Type getType() const override { return Type::TogglingPosition; }

	[[nodiscard]] constexpr std::vector<PropertyDescriptor> getPropertyDescriptors() const override {
		return {
			{ Property::Position_X, State::A },
			{ Property::Position_X, State::B },
			{ Property::Position_Y, State::A },
			{ Property::Position_Y, State::B },
			{ Property::Angle,      State::_ },
		};
	}
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const override {
		switch (desc.property) {
		case Property::Position_X: return desc.associatedState == State::A ? positionA.x : positionB.x;
		case Property::Position_Y: return desc.associatedState == State::A ? positionA.y : positionB.y;
		case Property::Angle:      return convertUnits ? to_deg(angle) : angle;
		default: return std::nullopt;
		}
	}
	void setProperty(float value, PropertyDescriptor desc) override {
		switch (desc.property) {
		case Property::Position_X: if (desc.associatedState == State::A) positionA.x = value; else positionB.x = value; break;
		case Property::Position_Y: if (desc.associatedState == State::A) positionA.y = value; else positionB.y = value; break;
		case Property::Angle:      setAngle(to_rad(value)); break;
		default:;
		}
	}

	[[nodiscard]] col getColor() const override { return Color::SoftBlue; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2) const override { return glm::vec2(0.f); }

private:
	float angle{};            // SSOT
	glm::mat2 rotation{1.f};
	glm::vec2 positionA{0.f}; // SSOT
	glm::vec2 positionB{0.f}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "t_position"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherTogglingPositionSpec = (const TogglingPositionSpec&)other;
		return angle == otherTogglingPositionSpec.angle && positionA == otherTogglingPositionSpec.positionA && positionB == otherTogglingPositionSpec.positionB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const override;
};

class TogglingAngleSpec : public IMotionSpec {
public:
	TogglingAngleSpec(glm::vec2 position, float angleA, float angleB) :
	    position(position),
	    angleA(angleA),
		angleB(angleB) {}
	explicit TogglingAngleSpec(const PropertyValues& allValues) {
		position.x = allValues[(int)Property::Position_X][(int)State::_];
		position.y = allValues[(int)Property::Position_Y][(int)State::_];
		angleA = allValues[(int)Property::Angle][(int)State::A];
		angleB = allValues[(int)Property::Angle][(int)State::B];
	}

	explicit TogglingAngleSpec(const std::string& data);

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingAngleSpec>(*this);
	}

	~TogglingAngleSpec() override = default;

	void scale(float factor) override {
		*this = TogglingAngleSpec(position * factor, angleA, angleB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

	[[nodiscard]] Type getType() const override { return Type::TogglingAngle; }

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const TogglingAngleSpec*)base)->position + vector;
	}
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool stateless, bool toggled, bool individual, const IMotionSpec* base) override;
	void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) override;

	[[nodiscard]] constexpr std::vector<PropertyDescriptor> getPropertyDescriptors() const override {
		return {
			{ Property::Position_X, State::_ },
			{ Property::Position_Y, State::_ },
			{ Property::Angle,      State::A },
			{ Property::Angle,      State::B },
		};
	}
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const override {
		switch (desc.property) {
		case Property::Position_X: return position.x;
		case Property::Position_Y: return position.y;
		case Property::Angle: {    float angle = desc.associatedState == State::A ? angleA : angleB;
			                       return convertUnits ? to_deg(angle) : angle;}
		default: return std::nullopt;
		}
	}
	void setProperty(float value, PropertyDescriptor desc) override {
		switch (desc.property) {
		case Property::Position_X: position.x = value; break;
		case Property::Position_Y: position.y = value; break;
		case Property::Angle:      if (desc.associatedState == State::A) angleA = to_rad(value); else angleB = to_rad(value); break;
		default:;
		}
	}

	[[nodiscard]] col getColor() const override { return Color::SoftRed; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

private:
	glm::vec2 position{0.f}; // SSOT
	float angleA{};          // SSOT
	float angleB{};          // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "t_angle"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherTogglingAngleSpec = (const TogglingAngleSpec&)other;
		return position == otherTogglingAngleSpec.position && angleA == otherTogglingAngleSpec.angleA && angleB == otherTogglingAngleSpec.angleB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const override;
};

class SpinningSpec : public IMotionSpec {
public:
	SpinningSpec(glm::vec2 position, float initialAngle, float angularSpeedA, float angularSpeedB) :
	    position(position),
	    angularSpeedA(angularSpeedA),
	    angularSpeedB(angularSpeedB) {
		setInitialAngle(initialAngle);
	}
	explicit SpinningSpec(const PropertyValues& allValues) {
		position.x = allValues[(int)Property::Position_X][(int)State::_];
		position.y = allValues[(int)Property::Position_Y][(int)State::_];
		angularSpeedA = allValues[(int)Property::AngularSpeed][(int)State::A];
		angularSpeedB = allValues[(int)Property::AngularSpeed][(int)State::B];
		setInitialAngle(allValues[(int)Property::Angle][(int)State::_]);
	}

	explicit SpinningSpec(const std::string& data);

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<SpinningSpec>(*this);
	}

	~SpinningSpec() override = default;

	void scale(float factor) override {
		*this = SpinningSpec(position * factor, initialAngle, angularSpeedA, angularSpeedB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother&) const override;

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const SpinningSpec*)base)->position + vector;
	}
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) override;
	void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) override;

	void setInitialAngle(float radians);

	[[nodiscard]] Type getType() const override { return Type::Spinning; }

	[[nodiscard]] constexpr std::vector<PropertyDescriptor> getPropertyDescriptors() const override {
		return {
			{ Property::Position_X,   State::_ },
			{ Property::Position_Y,   State::_ },
			{ Property::InitialAngle, State::_ },
			{ Property::AngularSpeed, State::A },
			{ Property::AngularSpeed, State::B },
		};
	}
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const override {
		switch (desc.property) {
		case Property::Position_X:   return position.x;
		case Property::Position_Y:   return position.y;
		case Property::InitialAngle: return convertUnits ? to_deg(initialAngle) : initialAngle;
		case Property::AngularSpeed:{float angularSpeed = desc.associatedState == State::A ? angularSpeedA : angularSpeedB;
			                         return convertUnits ? to_rpm(angularSpeed) : angularSpeed;}
		default: return std::nullopt;
		}
	}
	void setProperty(float value, PropertyDescriptor desc) override {
		switch (desc.property) {
		case Property::Position_X:   position.x = value; break;
		case Property::Position_Y:   position.y = value; break;
		case Property::InitialAngle: setInitialAngle(to_rad(value)); break;
		case Property::AngularSpeed: if (desc.associatedState == State::A) angularSpeedA = to_radPerSec(value); else angularSpeedB = to_radPerSec(value); break;
		default:;
		}
	}

	[[nodiscard]] col getColor() const override { return Color::SoftMagenta; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

private:
	glm::vec2 position{0.f}; // SSOT
	float initialAngle{};    // SSOT
	float angularSpeedA{};   // SSOT
	float angularSpeedB{};   // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "spinning"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherSpinningSpec = (const SpinningSpec&)other;
		return
			position == otherSpinningSpec.position && initialAngle == otherSpinningSpec.initialAngle &&
			angularSpeedA == otherSpinningSpec.angularSpeedA && angularSpeedB == otherSpinningSpec.angularSpeedB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const override;
};

class OscillatingPositionSpec : public IMotionSpec {
public:
	OscillatingPositionSpec(float angle, glm::vec2 position1, glm::vec2 position2, float angularFrequencyA, float angularFrequencyB) :
	    position1(position1),
	    position2(position2),
	    angularFrequencyA(angularFrequencyA),
	    angularFrequencyB(angularFrequencyB) {
		setAngle(angle);
	}
	explicit OscillatingPositionSpec(const PropertyValues& allValues) {
		position1.x = allValues[(int)Property::Position1_X][(int)State::_];
		position1.y = allValues[(int)Property::Position1_Y][(int)State::_];
		position2.x = allValues[(int)Property::Position2_X][(int)State::_];
		position2.y = allValues[(int)Property::Position2_Y][(int)State::_];
		angularFrequencyA = allValues[(int)Property::AngularFrequency][(int)State::A];
		angularFrequencyB = allValues[(int)Property::AngularFrequency][(int)State::B];
		setAngle(allValues[(int)Property::Angle][(int)State::_]);
	}

	explicit OscillatingPositionSpec(const std::string& data);

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingPositionSpec>(*this);
	}

	~OscillatingPositionSpec() override = default;

	void scale(float factor) override {
		*this = OscillatingPositionSpec(angle, position1 * factor, position2 * factor, angularFrequencyA, angularFrequencyB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

	void translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) override;
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool, bool, bool individual, const IMotionSpec* base) override;
	void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) override;

	void setAngle(float radians);

	[[nodiscard]] Type getType() const override { return Type::OscillatingPosition; }

	[[nodiscard]] constexpr std::vector<PropertyDescriptor> getPropertyDescriptors() const override {
		return {
			{ Property::Position1_X,      State::_ },
			{ Property::Position1_Y,      State::_ },
			{ Property::Position2_X,      State::_ },
			{ Property::Position2_Y,      State::_ },
			{ Property::Angle,            State::_ },
			{ Property::AngularFrequency, State::A },
			{ Property::AngularFrequency, State::B },
		};
	}
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const override {
		switch (desc.property) {
		case Property::Position1_X:      return position1.x;
		case Property::Position1_Y:      return position1.y;
		case Property::Position2_X:      return position2.x;
		case Property::Position2_Y:      return position2.y;
		case Property::Angle:            return convertUnits ? to_deg(angle) : angle;
		case Property::AngularFrequency:{float angularFrequency = desc.associatedState == State::A ? angularFrequencyA : angularFrequencyB;
			                             return convertUnits ? to_opm(angularFrequency) : angularFrequency;}
		default: return std::nullopt;
		}
	}
	void setProperty(float value, PropertyDescriptor desc) override {
		switch (desc.property) {
		case Property::Position1_X:      position1.x = value; break;
		case Property::Position1_Y:      position1.y = value; break;
		case Property::Position2_X:      position2.x = value; break;
		case Property::Position2_Y:      position2.y = value; break;
		case Property::Angle:            setAngle(to_rad(value)); break;
		case Property::AngularFrequency: if (desc.associatedState == State::A) angularFrequencyA = to_radPerSec(value); else angularFrequencyB = to_radPerSec(value); break;
		default:;
		}
	}

	[[nodiscard]] col getColor() const override { return Color::SoftCyan; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2) const override { return glm::vec2(0.f); }

private:
	glm::vec2 position1{0.f};  // SSOT
	glm::vec2 position2{0.f};  // SSOT
	float angle{};             // SSOT
	glm::mat2 rotation{1.f};
	float angularFrequencyA{}; // SSOT
	float angularFrequencyB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "o_position"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherOscillatingPositionSpec = (const OscillatingPositionSpec&)other;
		return
			position1 == otherOscillatingPositionSpec.position1 && position2 == otherOscillatingPositionSpec.position2 &&
			angle == otherOscillatingPositionSpec.angle &&
			angularFrequencyA == otherOscillatingPositionSpec.angularFrequencyA && angularFrequencyB == otherOscillatingPositionSpec.angularFrequencyB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const override;
};

class OscillatingAngleSpec : public IMotionSpec {
public:
	OscillatingAngleSpec(glm::vec2 position, float angle1, float angle2, float angularFrequencyA, float angularFrequencyB) :
	    position(position),
	    angle1(angle1),
	    angle2(angle2),
	    angularFrequencyA(angularFrequencyA),
		angularFrequencyB(angularFrequencyB) {}
	explicit OscillatingAngleSpec(const PropertyValues& allValues) {
		position.x = allValues[(int)Property::Position_X][(int)State::_];
		position.y = allValues[(int)Property::Position_Y][(int)State::_];
		angle1 = allValues[(int)Property::Angle1][(int)State::_];
		angle2 = allValues[(int)Property::Angle2][(int)State::_];
		angularFrequencyA = allValues[(int)Property::AngularFrequency][(int)State::A];
		angularFrequencyB = allValues[(int)Property::AngularFrequency][(int)State::B];
	}

	OscillatingAngleSpec(const std::string& data);

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingAngleSpec>(*this);
	}

	~OscillatingAngleSpec() override = default;

	void scale(float factor) override {
		*this = OscillatingAngleSpec(position * factor, angle1, angle2, angularFrequencyA, angularFrequencyB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const OscillatingAngleSpec*)base)->position + vector;
	}
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool stateless, bool toggled, bool individual, const IMotionSpec* base) override;
	void scaleBy(float factor, glm::vec2 pivot, bool individual, const IMotionSpec* base) override;

	[[nodiscard]] Type getType() const override { return Type::OscillatingAngle; }

	[[nodiscard]] constexpr std::vector<PropertyDescriptor> getPropertyDescriptors() const override {
		return {
			{ Property::Position_X,       State::_ },
			{ Property::Position_Y,       State::_ },
			{ Property::Angle1,           State::_ },
			{ Property::Angle2,           State::_ },
			{ Property::AngularFrequency, State::A },
			{ Property::AngularFrequency, State::B },
		};
	}
	[[nodiscard]] std::optional<float> getProperty(bool convertUnits, PropertyDescriptor desc) const override {
		switch (desc.property) {
		case Property::Position_X:       return position.x;
		case Property::Position_Y:       return position.y;
		case Property::Angle1:           return convertUnits ? to_deg(angle1) : angle1;
		case Property::Angle2:           return convertUnits ? to_deg(angle2) : angle2;
		case Property::AngularFrequency:{float angularFrequency = desc.associatedState == State::A ? angularFrequencyA : angularFrequencyB;
			                             return convertUnits ? to_opm(angularFrequency) : angularFrequency;}
		default: return std::nullopt;
		}
	}
	void setProperty(float value, PropertyDescriptor desc) override {
		switch (desc.property) {
		case Property::Position_X:       position.x = value; break;
		case Property::Position_Y:       position.y = value; break;
		case Property::Angle1:           angle1 = to_rad(value); break;
		case Property::Angle2:           angle2 = to_rad(value); break;
		case Property::AngularFrequency: if (desc.associatedState == State::A) angularFrequencyA = to_radPerSec(value); else angularFrequencyB = to_radPerSec(value); break;
		default:;
		}
	}

	[[nodiscard]] col getColor() const override { return Color::SoftYellow; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

private:
	glm::vec2 position{0.f};   // SSOT
	float angle1{};            // SSOT
	float angle2{};            // SSOT
	float angularFrequencyA{}; // SSOT
	float angularFrequencyB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "o_angle"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherOscillatingAngleSpec = (const OscillatingAngleSpec&)other;
		return
			position == otherOscillatingAngleSpec.position &&
			angle1 == otherOscillatingAngleSpec.angle1 && angle2 == otherOscillatingAngleSpec.angle2 &&
			angularFrequencyA == otherOscillatingAngleSpec.angularFrequencyA && angularFrequencyB == otherOscillatingAngleSpec.angularFrequencyB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const override;
};


#endif // OBSTACLE_MOTION_H