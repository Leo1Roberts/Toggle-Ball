#include "editor/operation/CurvatureOperation.h"

#include "editor/EditorContext.h"
#include "editor/EditorScene.h"


CurvatureOperation::CurvatureOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, glm::vec2 initialHandlePosition) :
	Operation(ctx, trigger, initialPlanarPosition),
	obstacleIndex(obstacleIndex),
	obstacle(ctx.scene.obstacles[obstacleIndex]),
	initialDescriptor(*obstacle.descriptor),
	initialAngle(obstacle.getKinematicState()->getAngle()),
	initialPosition(worldToPlanar(obstacle.getKinematicState()->getPosition())),
	initialHandlePosition(initialHandlePosition),
	cap1(obstacle.getLeftCapPosition()),
	cap2(obstacle.getRightCapPosition()) {}


std::optional<Cursor> CurvatureOperation::queryCursor() const {
	float angle = 0.f;
	if (dynamic_cast<SegmentSpec*>(obstacle.descriptor->shape.get()))
		angle = -obstacle.getKinematicState()->getAngle();
	else if (dynamic_cast<ArcSpec*>(obstacle.descriptor->shape.get())) {
		auto diff = Camera::planarToScreenDirection(
			pointerPlanarPosition - worldToPlanar(obstacle.getKinematicState()->getPosition()));
		angle = std::atan2(diff.y, diff.x) + glm::half_pi<float>();
	}

	return Cursor{
		.style = Cursor::Style::DynamicResize,
		.dynamic = true,
		.angle = angle,
	};
}


OperationResponse CurvatureOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void CurvatureOperation::applyOperation() {
    auto handle = initialHandlePosition + pointerPlanarPosition - initialPlanarPosition;

    auto capToCap = cap2 - cap1;
    float capToCapDistance = glm::length(capToCap);

    if (capToCapDistance < 0.0001f)
        return;

    float chordAngle = std::atan2(capToCap.y, capToCap.x);
    auto capsMidpoint = (cap1 + cap2) * 0.5f;

    glm::vec2 normal = {std::sin(chordAngle), -std::cos(chordAngle)};
    float chordOffset = glm::dot(handle - capsMidpoint, normal);

    float vSq = capToCapDistance * capToCapDistance;
    float t = glm::dot(handle - cap1, capToCap) / vSq;
    bool isBetween = (t >= 0.f && t <= 1.f);

    glm::vec2 targetPosition;
    float targetAngle;

    if (isBetween && std::abs(chordOffset) < 0.5f) { // TODO: use same threshold calculation as in ManipulateCapOperation
		if (auto segmentSpec = dynamic_cast<SegmentSpec*>(initialDescriptor.shape.get())) {
            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
                initialDescriptor.shape->minorRadius,
                segmentSpec->getLeftLength(),
                segmentSpec->getRightLength()
            );
            targetPosition = initialPosition;
            targetAngle = initialAngle;
        } else {
            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
                initialDescriptor.shape->minorRadius,
                capToCapDistance * 0.5f,
                capToCapDistance * 0.5f
            );
            targetPosition = capsMidpoint;
            targetAngle = chordAngle;
        }
    } else {
        auto effectiveM = handle;
        if (!isBetween && std::abs(chordOffset) < 0.01f)
        	return;

        auto m = effectiveM - cap1;
        auto c2 = cap2 - cap1;

        float D = 2.f * (m.x * c2.y - m.y * c2.x);

        if (std::abs(D) < 0.0001f) {
            obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
                initialDescriptor.shape->minorRadius,
                capToCapDistance * 0.5f,
                capToCapDistance * 0.5f
            );
            targetPosition = capsMidpoint;
            targetAngle = chordAngle;
        } else {
            float mSq = glm::dot(m, m);
            float c2Sq = glm::dot(c2, c2);

            glm::vec2 centreRelative1 = {
	            (c2.y * mSq - m.y * c2Sq) / D,
				(m.x * c2Sq - c2.x * mSq) / D
            };

            auto arcCentre = cap1 + centreRelative1;
            float arcRadius = glm::length(centreRelative1);

            auto v1 = glm::normalize(cap1 - arcCentre);
            auto v2 = glm::normalize(cap2 - arcCentre);
            auto vM = glm::normalize(effectiveM - arcCentre);

            float a1 = std::atan2(v1.y, v1.x);
            float a2 = std::atan2(v2.y, v2.x);
            float aM = std::atan2(vM.y, vM.x);

            float sweep2 = wrapAngle(a2 - a1);
            if (sweep2 < 0.f) sweep2 += glm::two_pi<float>();

            float sweepM = wrapAngle(aM - a1);
            if (sweepM < 0.f) sweepM += glm::two_pi<float>();

            float arcAngle;
            if (sweepM <= sweep2)
                arcAngle = sweep2;
            else
                arcAngle = glm::two_pi<float>() - sweep2;

            float positionOffset = glm::dot(arcCentre - capsMidpoint, normal);

            obstacle.descriptor->shape = std::make_unique<ArcSpec>(
                initialDescriptor.shape->minorRadius,
                arcAngle,
                arcRadius
            );

            targetPosition = capsMidpoint + normal * positionOffset;

            if (chordOffset < 0.f)
                targetAngle = chordAngle;
            else
                targetAngle = chordAngle + glm::pi<float>();
        }
    }

    auto translatedInitialDescriptor = initialDescriptor;
    translatedInitialDescriptor.motion->translateBy(targetPosition - initialPosition, true, false, initialDescriptor.motion.get());

    float angleDiff = wrapAngle(targetAngle - initialAngle);
    obstacle.rotateBy(angleDiff, angleToRotation2D(angleDiff), glm::vec2(0.f), true, false, true, &translatedInitialDescriptor);

    obstacle.initKinematicState();
    obstacle.invalidateAllMeshes();
}