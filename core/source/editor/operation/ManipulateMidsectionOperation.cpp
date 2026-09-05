#include "editor/operation/ManipulateMidsectionOperation.h"

#include "editor/EditorContext.h"
#include "editor/EditorScene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ManipulateMidsectionOperation::ManipulateMidsectionOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, glm::vec2 initialHandlePosition) :
	Operation(ctx, trigger, initialPlanarPosition),
	obstacleIndex(obstacleIndex),
	obstacle(ctx.scene.obstacles[obstacleIndex]),
	initialDescriptor(*obstacle.descriptor),
	initialAngle(obstacle.getKinematicState()->getAngle()),
	initialPosition(worldToPlanar(obstacle.getKinematicState()->getPosition())),
	initialHandlePosition(initialHandlePosition),
	cap1(obstacle.getLeftCapPosition()),
	cap2(obstacle.getRightCapPosition()) {
	if (auto arcSpec = dynamic_cast<ArcSpec*>(initialDescriptor.shape.get()))
		initialHandleMinusArcRadius = arcSpec->getArcRadius() - length(initialHandlePosition - initialPosition);
}


std::optional<Cursor> ManipulateMidsectionOperation::queryCursor() const {
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


OperationResponse ManipulateMidsectionOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateMidsectionOperation::applyOperation() {
    auto handle = initialHandlePosition + pointerPlanarPosition - initialPlanarPosition;

	if (changeArcRadius && initialHandleMinusArcRadius) {
		auto arcSpec = dynamic_cast<ArcSpec*>(obstacle.descriptor->shape.get());
		arcSpec->setArcRadius(*initialHandleMinusArcRadius + length(handle - initialPosition));
	} else {
		auto capToCap = cap2 - cap1;
		float capToCapDistance = glm::length(capToCap);

		glm::vec2 targetPosition;
		float targetAngle;
		float minorRadius = initialDescriptor.shape->minorRadius;

		if (capToCapDistance < 0.0001f) { // Shape is a full circle
			auto m = handle - cap1;
			float hDist = glm::length(m);

			if (hDist < 0.0001f) {
				if (auto segmentSpec = dynamic_cast<SegmentSpec*>(initialDescriptor.shape.get())) {
					obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
						minorRadius, segmentSpec->getLeftLength(), segmentSpec->getRightLength()
					);
					targetPosition = initialPosition;
					targetAngle = initialAngle;
				} else {
					obstacle.descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, 0.f, 0.f);
					targetPosition = cap1;
					targetAngle = initialAngle;
				}
			} else {
				auto initialD = initialPosition - cap1;
				float absInitialD = glm::length(initialD);

				glm::vec2 D = (absInitialD < 0.0001f) ? (m / hDist) : (initialD / absInitialD);

				float denom = 2.f * dot(m, D);

				if (std::abs(denom) < 0.0001f)
					return;

				float r = dot(m, m) / denom;
				float radius = std::abs(r);

				obstacle.descriptor->shape = std::make_unique<ArcSpec>(minorRadius, glm::two_pi<float>(), radius);
				targetPosition = cap1 + D * r;

				targetAngle = initialAngle;
			}
		} else {
			float chordAngle = std::atan2(capToCap.y, capToCap.x);
			auto capsMidpoint = (cap1 + cap2) * 0.5f;
			glm::vec2 normal = {std::sin(chordAngle), -std::cos(chordAngle)};

			float chordOffset = dot(handle - capsMidpoint, normal);
			float t = dot(handle - cap1, capToCap) / (capToCapDistance * capToCapDistance);
			bool isBetween = t >= 0.f && t <= 1.f;

			auto isAlmostStraight = [&] {
				if (!isBetween) return false;

				float h = std::abs(chordOffset);
				float L = capToCapDistance;
				float denom = L * L * t * (1.f - t) - h * h;

				return denom > 0.f && (L * h < denom * std::tan(0.05f)); // Same 0.05rad threshold as in ManipulateCapOperation
			};

			if (isAlmostStraight()) {
				if (auto segmentSpec = dynamic_cast<SegmentSpec*>(initialDescriptor.shape.get())) {
					obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
						minorRadius, segmentSpec->getLeftLength(), segmentSpec->getRightLength()
					);
					targetPosition = initialPosition;
					targetAngle = initialAngle;
				} else {
					obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
						minorRadius, capToCapDistance * 0.5f, capToCapDistance * 0.5f
					);
					targetPosition = capsMidpoint;
					targetAngle = chordAngle;
				}
			} else {
				if (!isBetween && std::abs(chordOffset) < 0.01f)
					return;

				auto m = handle - cap1;
				float D = 2.f * (m.x * capToCap.y - m.y * capToCap.x);

				if (std::abs(D) < 0.0001f) {
					obstacle.descriptor->shape = std::make_unique<SegmentSpec>(
						minorRadius, capToCapDistance * 0.5f, capToCapDistance * 0.5f
					);
					targetPosition = capsMidpoint;
					targetAngle = chordAngle;
				} else {
					float mSq = dot(m, m);
					float c2Sq = dot(capToCap, capToCap);
					glm::vec2 centreRelative1 = {
						(capToCap.y * mSq - m.y * c2Sq) / D,
						(m.x * c2Sq - capToCap.x * mSq) / D
					};

					auto arcCentre = cap1 + centreRelative1;
					float arcRadius = glm::length(centreRelative1);

					auto v1 = normalize(cap1 - arcCentre);
					auto v2 = normalize(cap2 - arcCentre);
					auto vM = normalize(handle - arcCentre);

					float a1 = std::atan2(v1.y, v1.x);
					float a2 = std::atan2(v2.y, v2.x);
					float aM = std::atan2(vM.y, vM.x);

					float sweep2 = wrapAngle(a2 - a1);
					if (sweep2 < 0.f) sweep2 += glm::two_pi<float>();

					float sweepM = wrapAngle(aM - a1);
					if (sweepM < 0.f) sweepM += glm::two_pi<float>();

					float arcAngle = (sweepM <= sweep2) ? sweep2 : glm::two_pi<float>() - sweep2;
					float positionOffset = dot(arcCentre - capsMidpoint, normal);

					obstacle.descriptor->shape = std::make_unique<ArcSpec>(minorRadius, arcAngle, arcRadius);
					targetPosition = capsMidpoint + normal * positionOffset;
					targetAngle = chordAngle + (chordOffset < 0.f ? 0.f : glm::pi<float>());
				}
			}
		}

		auto translatedInitialDescriptor = initialDescriptor;
		translatedInitialDescriptor.motion->translateBy(targetPosition - initialPosition, true, false, initialDescriptor.motion.get());

		float angleDiff = wrapAngle(targetAngle - initialAngle);
		obstacle.rotateBy(angleDiff, angleToRotation2D(angleDiff), glm::vec2(0.f), true, false, true, &translatedInitialDescriptor);

		obstacle.initKinematicState();
	}

    obstacle.invalidateAllMeshes();
}