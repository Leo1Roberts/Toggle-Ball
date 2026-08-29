#include "editor/operation/ManipulateCapOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ManipulateCapOperation::ManipulateCapOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, bool leftCap, float tangentAngle) :
	Operation(scene, camera, trigger, initialPlanarPosition),
	obstacle(obstacle),
	initialDescriptor(*obstacle.descriptor),
	initialAngle(obstacle.getKinematicState()->getAngle()),
	initialPosition(worldToPlanar(obstacle.getKinematicState()->getPosition())), leftCap(leftCap), tangentAngle(tangentAngle),
	fixedCapPlanarPosition(!leftCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition()),
	initialCapPlanarPosition(leftCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition()),
	capPlanarPosition(initialCapPlanarPosition) {}


OperationResponse ManipulateCapOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = camera.screenToPlanarPosition(pointer->position);
			capPlanarPosition = initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateCapOperation::applyOperation() {
    auto capToCap = capPlanarPosition - fixedCapPlanarPosition;
    auto capToCapDistance = length(capToCap);

    auto segmentSpec = dynamic_cast<SegmentSpec*>(initialDescriptor.shape.get());
    auto arcSpec = dynamic_cast<ArcSpec*>(initialDescriptor.shape.get());

    float sign = leftCap ? -1.f : 1.f;
    auto chord = sign * capToCap;
    float chordAngle = std::atan2(chord.y, chord.x);

    glm::vec2 targetPosition;
    float targetAngle = chordAngle; // Default value
    bool applyTransformation = true;

    auto applySegmentShape = [&](float length, float dirAngle) {
        if (segmentSpec) {
            float leftLength = segmentSpec->getLeftLength();
            float rightLength = segmentSpec->getRightLength();
            float positionOffset;

            if (leftCap) {
                leftLength += length - segmentSpec->getLength();
                if (leftLength < 0.f) {
	                rightLength += leftLength;
                	leftLength = 0.f;
                }
                positionOffset = -rightLength;
            } else {
                rightLength += length - segmentSpec->getLength();
                if (rightLength < 0.f) {
	                leftLength += rightLength;
                	rightLength = 0.f;
                }
                positionOffset = leftLength;
            }

            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(initialDescriptor.shape->minorRadius, leftLength, rightLength);
            targetPosition = fixedCapPlanarPosition + glm::vec2(std::cos(dirAngle), std::sin(dirAngle)) * positionOffset;
        } else {
            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
            	initialDescriptor.shape->minorRadius,
            	leftCap ? length : 0.f,
            	leftCap ? 0.f : length);
            targetPosition = fixedCapPlanarPosition;
        }
    };

    auto applyArcShape = [&](float arcAngle, float arcRadius, float positionOffset) {
        obstacle.descriptor->shape = std::make_unique<ArcSpec>(initialDescriptor.shape->minorRadius, arcAngle, arcRadius);
        glm::vec2 capsMidpoint = (capPlanarPosition + fixedCapPlanarPosition) * 0.5f;
        targetPosition = capsMidpoint + glm::vec2(std::sin(chordAngle), -std::cos(chordAngle)) * positionOffset;
    };

    if (alignWithTangent) {
        float curveTangent = leftCap ? tangentAngle : wrapAngle(tangentAngle + glm::pi<float>());
        float chordTangentAngleDiff = wrapAngle(sign * (curveTangent - chordAngle));
        float absAngleDiff = std::abs(chordTangentAngleDiff);

        if (capToCapDistance * absAngleDiff < 0.5f) { // Arbitrary threshold for snapping to being straight
            targetAngle = curveTangent;
            applySegmentShape(capToCapDistance * std::cos(chordTangentAngleDiff), curveTangent);
        } else {
            float arcAngle = 2.f * absAngleDiff;
            float arcRadius = (capToCapDistance * 0.5f) / std::sin(absAngleDiff);
            float positionOffset = (capToCapDistance * 0.5f) / std::tan(chordTangentAngleDiff);

            applyArcShape(arcAngle, arcRadius, positionOffset);
            targetAngle = chordTangentAngleDiff > 0.f ? chordAngle : chordAngle + glm::pi<float>();
        }
    } else {
        if (segmentSpec)
            applySegmentShape(capToCapDistance, chordAngle);
        else if (arcSpec) {
            float sagitta = arcSpec->getArcRadius() * (1.f - std::cos(arcSpec->getHalfArcAngle()));

            if (sagitta > 0.0001f && capToCapDistance > 0.0001f) {
                float arcRadius = (capToCapDistance * capToCapDistance + 4.f * sagitta * sagitta) / (8.f * sagitta);
                float positionOffset = arcRadius - sagitta;
                float arcAngle = 2.f * std::atan2(capToCapDistance * 0.5f, positionOffset);

                applyArcShape(arcAngle, arcRadius, positionOffset);
            } else
                applyTransformation = false;
        }
    }

    if (applyTransformation) {
        auto translatedInitialDescriptor = initialDescriptor;
        translatedInitialDescriptor.motion->translateBy(targetPosition - initialPosition, true, false, initialDescriptor.motion.get());

        float angleDiff = wrapAngle(targetAngle - initialAngle);
        obstacle.rotateBy(angleDiff, angleToRotation2D(angleDiff), glm::vec2(0.f), true, false, true, &translatedInitialDescriptor);
    }

    obstacle.initKinematicState();
    obstacle.invalidateAllMeshes();
}