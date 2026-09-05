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


std::vector<BindingHint> ManipulateCapOperation::getBindingHints() const {
	std::vector<BindingHint> hints =
		{{KeyChord(KeyCode::Unknown, MOD_SHIFT), "Preserve shape"}};

	if (preserveShape)
		hints.emplace_back(KeyChord(KeyCode::Unknown, MOD_CTRL),  "Symmetrical");
	else if (ctx.quickSettings.shape.alignWithTangent)
		hints.emplace_back(KeyChord(KeyCode::Unknown, MOD_ALT), "Use snapped tangent");

	return hints;
}


void ManipulateCapOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	auto leftCapPos = obstacle.getLeftCapPosition();
	auto rightCapPos = obstacle.getRightCapPosition();

	for (auto i : ctx.getPointedObstacleIndices(pointerPlanarPosition, obstacleIndex)) {
		const auto& otherObstacle = ctx.scene.obstacles[i];
		auto addInactiveHandle = [&](glm::vec2 capPos) {
			if (length2(capPos - leftCapPos) > 0.00000001f &&
				length2(capPos - rightCapPos) > 0.00000001f) {
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

	auto addActiveHandle = [&](glm::vec2 capPos, SnapType snapType = SnapType::None) {
		float radius = std::min(Settings::Sizes.obstacleHandleRadius, gizmoRenderer.planarToUIDistance(obstacle.descriptor->shape->minorRadius));

		switch (snapType) {
		case SnapType::None:
			gizmoRenderer.addCircle(capPos, {
				.fillColor = {Color::White, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
			break;
		case SnapType::Spine:
			if (ctx.quickSettings.shape.alignWithTangent && useSnappedTangent && snapResult.angle)
				gizmoRenderer.addSplitCircle(capPos, radius, *snapResult.angle, {
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
				gizmoRenderer.addCircle(capPos, {
					.fillColor = {Color::SoftCyan, 0.8f},
					.strokeColor = {Color::Black, 0.8f},
					.cornerRadius = radius,
					.strokeWidth = 2.f,
				});
			break;
		default:
			if (ctx.quickSettings.shape.alignWithTangent && useSnappedTangent && snapResult.angle)
				gizmoRenderer.addSplitCircle(capPos, radius, *snapResult.angle, {
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
				gizmoRenderer.addCircle(capPos, {
					.fillColor = {Color::SoftGreen, 0.8f},
					.strokeColor = {Color::Black, 0.8f},
					.cornerRadius = radius,
					.strokeWidth = 2.f,
				});
		}
	};

	if (symmetrical) {
		if (snapResult.type == SnapType::None) {
			addActiveHandle(leftCapPos);
			addActiveHandle(rightCapPos);
		} else {
			if (length2(snapResult.value - leftCapPos) > length2(snapResult.value - rightCapPos))
				addActiveHandle(leftCapPos);
			else
				addActiveHandle(rightCapPos);
			addActiveHandle(snapResult.value, snapResult.type);
		}
	} else
		addActiveHandle(currentlyLeftCap ? leftCapPos : rightCapPos, snapResult.type);
}


OperationResponse ManipulateCapOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			auto newPointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			rotationAngle += angleDifference(newPointerPlanarPosition, pointerPlanarPosition, initialPosition);
			pointerPlanarPosition = newPointerPlanarPosition;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateCapOperation::applyOperation() {
    auto rawCapPlanarPosition = initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;
	glm::vec2 capPlanarPosition;

	auto segmentSpec = dynamic_cast<SegmentSpec*>(initialDescriptor.shape.get());
	auto arcSpec = dynamic_cast<ArcSpec*>(initialDescriptor.shape.get());

	glm::vec2 targetPosition = initialPosition;
	float targetAngle = initialAngle;
	bool applyTransformation = true;

	auto applySegmentShape = [&](float length, float dirAngle) {
		float minorRadius = initialDescriptor.shape->minorRadius;
		if (segmentSpec) {
			float leftLength = segmentSpec->getLeftLength();
			float rightLength = segmentSpec->getRightLength();
			float diff = length - segmentSpec->getLength();

			if (symmetrical) {
				float halfDiff = diff / 2.f;
				leftLength += halfDiff;
				rightLength += halfDiff;
			} else
				if (leftCap)
					leftLength += diff;
				else
					rightLength += diff;

			float positionOffset = 0.f;
			if (leftLength < 0.f) {
				rightLength += leftLength;
				positionOffset = leftLength;
				leftLength = 0.f;
			} else if (rightLength < 0.f) {
				leftLength += rightLength;
				positionOffset = rightLength;
				rightLength = 0.f;
			}

			obstacle.descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, leftLength, rightLength);

			auto dirVec = glm::vec2(std::cos(dirAngle), std::sin(dirAngle));
			if (symmetrical) {
				targetPosition = initialPosition + dirVec * positionOffset;
			} else
				targetPosition = fixedCapPlanarPosition + dirVec * (leftCap ? -rightLength : leftLength);
		} else {
			if (symmetrical) {
				float halfLength = length / 2.f;
				obstacle.descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, halfLength, halfLength);
				targetPosition = (fixedCapPlanarPosition + capPlanarPosition) / 2.f;
			} else {
				obstacle.descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, leftCap ? length : 0.f, leftCap ? 0.f : length);
				targetPosition = fixedCapPlanarPosition;
			}
		}
	};

	if (preserveShape) {
		currentlyLeftCap = leftCap;

		if (segmentSpec) {
			glm::vec2 lineDir = { std::cos(initialAngle), std::sin(initialAngle) };
			float lengthDiff = dot(pointerPlanarPosition - initialPlanarPosition, lineDir);

			auto manipulatedCap = initialCapPlanarPosition + lineDir * lengthDiff;
			auto manipulatedSnapResult = ctx.snapPointRestrictedToLine(manipulatedCap, {EntityType::Obstacle, obstacleIndex}, initialPosition, initialAngle);

			if (symmetrical) {
				float manipulatedDiffSq = manipulatedSnapResult.type != SnapType::None ? length2(manipulatedSnapResult.value - manipulatedCap) : std::numeric_limits<float>::max();

				auto fixedCap = fixedCapPlanarPosition - lineDir * lengthDiff;
				auto fixedSnapResult = ctx.snapPointRestrictedToLine(fixedCap, {EntityType::Obstacle, obstacleIndex}, initialPosition, initialAngle);
				float fixedDiffSq = fixedSnapResult.type != SnapType::None ? length2(fixedSnapResult.value - fixedCap) : std::numeric_limits<float>::max();

				if (manipulatedDiffSq < fixedDiffSq) {
					if (manipulatedSnapResult.type == SnapType::None)
						snapResult = {};
					else {
						snapResult = manipulatedSnapResult;
						lengthDiff = dot(manipulatedSnapResult.value - initialCapPlanarPosition, lineDir);
					}
				} else {
					if (fixedSnapResult.type == SnapType::None)
						snapResult = {};
					else {
						snapResult = fixedSnapResult;
						lengthDiff = -dot(fixedSnapResult.value - fixedCapPlanarPosition, lineDir);
					}
				}
			} else {
				snapResult = manipulatedSnapResult;
				if (snapResult.type != SnapType::None)
					lengthDiff = dot(snapResult.value - initialCapPlanarPosition, lineDir);
			}

			float leftLength = segmentSpec->getLeftLength();
			float rightLength = segmentSpec->getRightLength();
			if (leftCap)
				leftLength -= lengthDiff;
			else
				rightLength += lengthDiff;
			if (symmetrical) {
				if (leftCap)
					rightLength -= lengthDiff;
				else
					leftLength += lengthDiff;
			}

			float length = leftLength + rightLength;
			if (length < 0.f) {
				snapResult = {};
				length = 0.f;
			}

			applySegmentShape(length, initialAngle);
		} else if (arcSpec) {
			float rawArcAngleDiff = (leftCap ? rotationAngle : -rotationAngle) * (symmetrical ? 2.f : 1.f);
			float rawArcAngle = arcSpec->getArcAngle() + rawArcAngleDiff;
			float limitedArcAngle = std::clamp(rawArcAngle, 0.f, glm::two_pi<float>());

			float arcAngle;
			if (symmetrical) {
                float halfRawDiff = rawArcAngleDiff / 2.f;
                float rawLeftCapAngle = initialAngle + arcSpec->getLeftCapAngle() - glm::half_pi<float>() + halfRawDiff;
                float rawRightCapAngle = initialAngle + arcSpec->getRightCapAngle() + glm::half_pi<float>() - halfRawDiff;

                auto rawLeftCapPos = initialPosition + glm::vec2(std::cos(rawLeftCapAngle), std::sin(rawLeftCapAngle)) * arcSpec->getArcRadius();
                auto rawRightCapPos = initialPosition + glm::vec2(std::cos(rawRightCapAngle), std::sin(rawRightCapAngle)) * arcSpec->getArcRadius();

                auto leftSnapResult = ctx.snapPointRestrictedToCircle(rawLeftCapPos, {EntityType::Obstacle, obstacleIndex}, initialPosition, arcSpec->getArcRadius());
                float leftDiffSq = leftSnapResult.type != SnapType::None ? length2(leftSnapResult.value - rawLeftCapPos) : std::numeric_limits<float>::max();

                auto rightSnapResult = ctx.snapPointRestrictedToCircle(rawRightCapPos, {EntityType::Obstacle, obstacleIndex}, initialPosition, arcSpec->getArcRadius());
                float rightDiffSq = rightSnapResult.type != SnapType::None ? length2(rightSnapResult.value - rawRightCapPos) : std::numeric_limits<float>::max();

                float capAngleDiff = 0.f;
                float sign = 1.f;

                if (leftDiffSq < rightDiffSq && leftSnapResult.type != SnapType::None) {
                    snapResult = leftSnapResult;
                    capAngleDiff = angleDifference(leftSnapResult.value, rawLeftCapPos, initialPosition);
                    sign = 1.f;
                } else if (rightSnapResult.type != SnapType::None) {
                    snapResult = rightSnapResult;
                    capAngleDiff = angleDifference(rightSnapResult.value, rawRightCapPos, initialPosition);
                    sign = -1.f;
                } else
                    snapResult = {};

                float snappedArcAngle = rawArcAngle + sign * capAngleDiff * 2.f;
                float limitedSnappedArcAngle = std::clamp(snappedArcAngle, 0.f, glm::two_pi<float>());

                if (std::abs(limitedSnappedArcAngle - snappedArcAngle) > 0.000001f) {
                    snapResult = {};
                    arcAngle = limitedArcAngle;
                } else
                    arcAngle = limitedSnappedArcAngle;
			} else {
				float rawCapAngle = initialAngle + (leftCap
					? arcSpec->getLeftCapAngle() - glm::half_pi<float>() + rawArcAngleDiff
					: arcSpec->getRightCapAngle() + glm::half_pi<float>() - rawArcAngleDiff);

				auto rawCapPos = initialPosition + glm::vec2(std::cos(rawCapAngle), std::sin(rawCapAngle)) * arcSpec->getArcRadius();

				snapResult = ctx.snapPointRestrictedToCircle(rawCapPos, {EntityType::Obstacle, obstacleIndex}, initialPosition, arcSpec->getArcRadius());

				float capAngleDiff = angleDifference(snapResult.value, rawCapPos, initialPosition);
				float sign = leftCap ? 1.f : -1.f;

				float snappedArcAngle = rawArcAngle + sign * capAngleDiff;
				float limitedSnappedArcAngle = std::clamp(snappedArcAngle, 0.f, glm::two_pi<float>());

				if (std::abs(limitedSnappedArcAngle - snappedArcAngle) > 0.000001f) {
					snapResult = {};
					arcAngle = limitedArcAngle;
				} else
					arcAngle = limitedSnappedArcAngle;

				targetAngle = initialAngle + sign * ((arcAngle - arcSpec->getArcAngle()) / 2.f);
			}

			obstacle.descriptor->shape = std::make_unique<ArcSpec>(
				initialDescriptor.shape->minorRadius, arcAngle, arcSpec->getArcRadius());
		}
	} else {
	    snapResult = ctx.snapPoint(rawCapPlanarPosition, {EntityType::Obstacle, obstacleIndex});

	    glm::vec2 capToCap, chord;
	    float capToCapDistance, chordAngle;
	    float sign = leftCap ? -1.f : 1.f;

	    auto updateGeometry = [&](glm::vec2 capPos) {
	        capPlanarPosition = capPos;
	        capToCap = capPlanarPosition - fixedCapPlanarPosition;
	        capToCapDistance = length(capToCap);
	        chord = sign * capToCap;
	        chordAngle = std::atan2(chord.y, chord.x);
	    };

	    updateGeometry(snapResult.value);

		targetAngle = chordAngle; // Default value
	    bool alignWithTangent = false;
	    float curveTangent = 0.f;

	    auto isAlmostStraight = [&] {
	        if (capToCapDistance < 0.0001f) return true;
	        glm::vec2 tangent = {std::cos(curveTangent), std::sin(curveTangent)};
	        return dot(normalize(chord), tangent) > std::cos(0.05f);
	    };

	    auto isArcTooLarge = [&] {
	        if (capToCapDistance < 0.0001f) return false;
	        glm::vec2 tangent = {std::cos(curveTangent), std::sin(curveTangent)};
	        return dot(normalize(chord), tangent) < std::cos((glm::two_pi<float>() - 0.2f) / 2.f);
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

				if (isAlmostStraight() || isArcTooLarge())
					snapResult.angle = std::nullopt;
				else
					alignWithTangent = true;
			} else if (!useSnappedTangent && fixedTangentAngle) {
				alignWithTangent = true;
				curveTangent = leftCap ? *fixedTangentAngle : wrapAngle(*fixedTangentAngle + glm::pi<float>());

				if (isAlmostStraight() || isArcTooLarge()) {
					snapResult = {};
					updateGeometry(rawCapPlanarPosition);
				}
			}
		}

		if (alignWithTangent && isArcTooLarge())
			return;

		currentlyLeftCap = leftCap;

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

					if (arcAngle > glm::two_pi<float>() - 0.2f)
						return;

					applyArcShape(arcAngle, arcRadius, positionOffset);
				} else
					applyTransformation = false;
			}
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