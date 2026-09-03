#include "editor/operation/ManipulateCapOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ManipulateCapOperation::ManipulateCapOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, bool leftCap, std::optional<float> fixedTangentAngle) :
	Operation(ctx, trigger, initialPlanarPosition),
	obstacleIndex(obstacleIndex),
	obstacle(ctx.scene.obstacles[obstacleIndex]),
	initialDescriptor(*obstacle.descriptor),
	initialAngle(obstacle.getKinematicState()->getAngle()),
	initialPosition(worldToPlanar(obstacle.getKinematicState()->getPosition())), leftCap(leftCap), fixedTangentAngle(fixedTangentAngle),
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
					.cornerRadius = std::min(Settings::Sizes.obstacleHandleRadius, gizmoRenderer.planarToUIDistance(otherObstacle.descriptor->shape->minorRadius)),
					.strokeWidth = 2.f,
				};
				gizmoRenderer.addCircle(capPos, inactiveStyle);
			}
		};
		addInactiveHandle(otherObstacle.getLeftCapPosition());
		addInactiveHandle(otherObstacle.getRightCapPosition());
	}

	float radius = std::min(Settings::Sizes.obstacleHandleRadius, gizmoRenderer.planarToUIDistance(obstacle.descriptor->shape->minorRadius));

	switch (snapResult.type) {
	case SnapType::None:
		gizmoRenderer.addCircle(position, {
			.fillColor = {Color::White, 0.8f},
			.strokeColor = {Color::Black, 0.8f},
			.cornerRadius = radius,
			.strokeWidth = 2.f,
		});
		break;
	case SnapType::Spine:
		if (ctx.quickSettings.shape.alignWithTangent && useSnappedTangent && snapResult.angle)
			gizmoRenderer.addSplitCircle(position, radius, *snapResult.angle, {
				.fillColor = {Color::SoftCyan, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = 0.f,
				.strokeWidth = 2.f,
			}, {
				.fillColor = {Color::SoftCyan, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
		else
			gizmoRenderer.addCircle(position, {
				.fillColor = {Color::SoftCyan, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
		break;
	default:
		if (ctx.quickSettings.shape.alignWithTangent && useSnappedTangent && snapResult.angle)
			gizmoRenderer.addSplitCircle(position, radius, *snapResult.angle, {
				.fillColor = {Color::SoftGreen, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = 0.f,
				.strokeWidth = 2.f,
			}, {
				.fillColor = {Color::SoftGreen, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
		else
			gizmoRenderer.addCircle(position, {
				.fillColor = {Color::SoftGreen, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
	}
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
    auto rawCapPlanarPosition = initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;
    snapResult = ctx.snapPoint(rawCapPlanarPosition, {EntityType::Obstacle, obstacleIndex});

    glm::vec2 capPlanarPosition, capToCap, chord;
    float capToCapDistance, chordAngle;
    float sign = leftCap ? -1.f : 1.f;
    currentlyLeftCap = leftCap;

    auto updateGeometry = [&](glm::vec2 capPos) {
        capPlanarPosition = capPos;
        capToCap = capPlanarPosition - fixedCapPlanarPosition;
        capToCapDistance = length(capToCap);
        chord = sign * capToCap;
        chordAngle = std::atan2(chord.y, chord.x);
    };

    updateGeometry(snapResult.value);

    auto segmentSpec = dynamic_cast<SegmentSpec*>(initialDescriptor.shape.get());
    auto arcSpec = dynamic_cast<ArcSpec*>(initialDescriptor.shape.get());

    glm::vec2 targetPosition;
    float targetAngle = chordAngle; // Default value
    bool applyTransformation = true;
    bool alignWithTangent = false;
    float curveTangent = 0.f;

    auto isAlmostStraight = [&] {
        float diff = wrapAngle(sign * (curveTangent - chordAngle));
        return std::abs(diff) < glm::half_pi<float>() && std::abs(capToCapDistance * std::tan(diff)) < 0.5f;
    };

    auto applySegmentShape = [&](float length, float dirAngle) {
        float minorRadius = initialDescriptor.shape->minorRadius;
        if (segmentSpec) {
            float leftLength = segmentSpec->getLeftLength();
            float rightLength = segmentSpec->getRightLength();
            float diff = length - segmentSpec->getLength();

            if (leftCap) {
                leftLength += diff;
                if (leftLength < 0.f) { rightLength += leftLength; leftLength = 0.f; }
            } else {
                rightLength += diff;
                if (rightLength < 0.f) { leftLength += rightLength; rightLength = 0.f; }
            }

            float positionOffset = leftCap ? -rightLength : leftLength;
            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, leftLength, rightLength);
            targetPosition = fixedCapPlanarPosition + glm::vec2(std::cos(dirAngle), std::sin(dirAngle)) * positionOffset;
        } else {
            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, leftCap ? length : 0.f, leftCap ? 0.f : length);
            targetPosition = fixedCapPlanarPosition;
        }
    };

    auto applyArcShape = [&](float arcAngle, float arcRadius, float positionOffset) {
        obstacle.descriptor->shape = std::make_unique<ArcSpec>(initialDescriptor.shape->minorRadius, arcAngle, arcRadius);
        glm::vec2 capsMidpoint = (capPlanarPosition + fixedCapPlanarPosition) * 0.5f;
        targetPosition = capsMidpoint + glm::vec2(std::sin(chordAngle), -std::cos(chordAngle)) * positionOffset;
    };

    if (ctx.quickSettings.shape.alignWithTangent) {
        if (useSnappedTangent && snapResult.type != SnapType::None && snapResult.angle) {
            float manipulatedTangent = leftCap ? *snapResult.angle : wrapAngle(*snapResult.angle + glm::pi<float>());
            curveTangent = wrapAngle(2.f * chordAngle - manipulatedTangent);

            if (isAlmostStraight())
                snapResult.angle = std::nullopt;
            else
                alignWithTangent = true;
        } else if (!useSnappedTangent && fixedTangentAngle) {
            alignWithTangent = true;
            curveTangent = leftCap ? *fixedTangentAngle : wrapAngle(*fixedTangentAngle + glm::pi<float>());

            if (isAlmostStraight()) {
                snapResult = {};
                updateGeometry(rawCapPlanarPosition);
            }
        }
    }

    if (alignWithTangent) {
        if (isAlmostStraight()) {
            float diff = wrapAngle(sign * (curveTangent - chordAngle));
            float projectedLength = capToCapDistance * std::cos(diff);
            glm::vec2 straightnessSnappedPosition = fixedCapPlanarPosition + sign * glm::vec2(std::cos(curveTangent), std::sin(curveTangent)) * projectedLength;

            snapResult = ctx.snapPointRestrictedToLine(straightnessSnappedPosition, {EntityType::Obstacle, obstacleIndex}, fixedCapPlanarPosition, curveTangent);

            updateGeometry(snapResult.value);

            targetAngle = curveTangent;
            applySegmentShape(capToCapDistance * std::cos(wrapAngle(sign * (curveTangent - chordAngle))), curveTangent);
        } else {
            float chordTangentAngleDiff = wrapAngle(sign * (curveTangent - chordAngle));
            float absAngleDiff = std::abs(chordTangentAngleDiff);

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