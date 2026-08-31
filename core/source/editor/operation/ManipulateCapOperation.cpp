#include "editor/operation/ManipulateCapOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ManipulateCapOperation::ManipulateCapOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, bool leftCap, std::optional<float> tangentAngle) :
	Operation(ctx, trigger, initialPlanarPosition),
	obstacleIndex(obstacleIndex),
	obstacle(ctx.scene.obstacles[obstacleIndex]),
	initialDescriptor(*obstacle.descriptor),
	initialAngle(obstacle.getKinematicState()->getAngle()),
	initialPosition(worldToPlanar(obstacle.getKinematicState()->getPosition())), leftCap(leftCap), tangentAngle(tangentAngle),
	fixedCapPlanarPosition(!leftCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition()),
	initialCapPlanarPosition(leftCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition()) {}


void ManipulateCapOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	auto position = currentlyLeftCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition();

	for (auto i : ctx.getPointedObstacleIndices(pointerPlanarPosition, obstacleIndex)) {
		const auto& otherObstacle = ctx.scene.obstacles[i];
		auto addInactiveHandle = [&](glm::vec2 capPos) {
			if (glm::length2(capPos - position) > 0.00000001f) {
				PanelStyle inactiveStyle = {
					.fillColor = {Color::White, 0.3f},
					.strokeColor = {Color::Black, 0.3f},
					.cornerRadius = std::min(Settings::Sizes.obstacleCapHandleRadius, gizmoRenderer.planarToUIDistance(otherObstacle.descriptor->shape->minorRadius)),
					.strokeWidth = 2.f,
				};
				gizmoRenderer.addCircle(capPos, inactiveStyle);
			}
		};
		addInactiveHandle(otherObstacle.getLeftCapPosition());
		addInactiveHandle(otherObstacle.getRightCapPosition());
	}

	PanelStyle activeStyle = {
		.fillColor = {(snapResult.snapped && length2(snapResult.value - position) < 0.00000001f ? Color::SoftGreen : Color::White), 0.8f},
		.strokeColor = {Color::Black, 0.8f},
		.cornerRadius = std::min(Settings::Sizes.obstacleCapHandleRadius, gizmoRenderer.planarToUIDistance(obstacle.descriptor->shape->minorRadius)),
		.strokeWidth = 2.f,
	};
	gizmoRenderer.addCircle(position, activeStyle);
}


OperationResponse ManipulateCapOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateCapOperation::applyOperation() {
	snapResult = ctx.snapPoint(initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition, {EntityType::Obstacle, obstacleIndex});
	auto capPlanarPosition = snapResult.value;

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

	currentlyLeftCap = leftCap;

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
        float curveTangent = leftCap ? *tangentAngle : wrapAngle(*tangentAngle + glm::pi<float>());
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
        	if (chordTangentAngleDiff > 0.f)
        		targetAngle = chordAngle;
        	else {
        		targetAngle = chordAngle + glm::pi<float>();
        		currentlyLeftCap = !leftCap;
        	}
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