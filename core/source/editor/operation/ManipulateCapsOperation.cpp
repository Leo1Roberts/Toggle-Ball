#include "editor/operation/ManipulateCapsOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ManipulateCapsOperation::ManipulateCapsOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, const std::vector<CapInfo>& allCapsInfo) :
	Operation(ctx, trigger, initialPlanarPosition) {
	std::vector<EntityReference> manipulatedEntities;
	manipulatedEntities.reserve(allCapsInfo.size());
	for (auto info : allCapsInfo)
		manipulatedEntities.emplace_back(EntityType::Obstacle, info.obstacleIndex);

	manipulateCapOperations.reserve(allCapsInfo.size());

	for (auto& info : allCapsInfo) {
		const auto& obstacle = ctx.scene.obstacles[info.obstacleIndex];
		manipulateCapOperations.emplace_back(ctx, trigger, initialPlanarPosition, info.obstacleIndex, info.leftCap,
			obstacle.getKinematicState()->getAngle() + obstacle.descriptor->shape->getCapAngle(!info.leftCap),
			manipulatedEntities, true);
	}
}


void ManipulateCapsOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	for (const auto& capOperation : manipulateCapOperations)
		capOperation.addGizmos(gizmoRenderer);
}


OperationResponse ManipulateCapsOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			for (auto& capOperation : manipulateCapOperations)
				capOperation.pointerPlanarPosition = pointerPlanarPosition;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}

	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateCapsOperation::applyOperation() {
	using Line = ManipulateCapOperation::Restriction::Line;

	auto linesAreIdentical = [](Line l1, Line l2) {
	    glm::vec2 d1(std::cos(l1.angle), std::sin(l1.angle));
	    glm::vec2 d2(std::cos(l2.angle), std::sin(l2.angle));

	    if (std::abs(dot(d1, d2)) < 0.9999f)
	        return false; // Different angle

	    auto pointDiff = l1.point - l2.point;

	    if (length2(pointDiff) < 0.00000001f)
	        return true; // Same point

	    if (std::abs(dot(normalize(pointDiff), d1)) > 0.9999f)
	        return true; // Both points align at the correct angle

	    return false;
	};

	if (smoothJoin) {
	    auto rawHandlePosition = manipulateCapOperations[0].initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;
	    glm::vec2 handlePosition;

	    auto p1 = manipulateCapOperations[0].fixedCapPlanarPosition;
	    auto p2 = manipulateCapOperations[1].fixedCapPlanarPosition;
	    auto t1 = *manipulateCapOperations[0].fixedTangentAngle;
	    auto t2 = *manipulateCapOperations[1].fixedTangentAngle;

	    auto p1_to_p2 = p2 - p1;
	    float p1_to_p2_distanceSq = length2(p1_to_p2);
	    if (p1_to_p2_distanceSq < 0.00000001f)
    		return;

	    float f = dot(rawHandlePosition - p1, p1_to_p2) / p1_to_p2_distanceSq;

	    if (linesAreIdentical({p1, t1}, {p2, t2})) {
		    if (std::atan2( p1_to_p2.y,  p1_to_p2.x) - t1 < 0.0001f &&
			    std::atan2(-p1_to_p2.y, -p1_to_p2.x) - t2 < 0.0001f)
			    handlePosition = p1 + std::clamp(f, 0.f, 1.f) * p1_to_p2;
		    else return;
	    } else {
	    	f = std::clamp(f, 0.001f, 0.999f);

	        float sin_t1 = std::sin(t1);
	        float cos_t1 = std::cos(t1);
	        float sin_t2 = std::sin(t2);
	        float cos_t2 = std::cos(t2);

	        float A = p1_to_p2.x;
	        float B = p1_to_p2.y;

	        float L_sq = A*A + B*B;
	        float L = std::sqrt(L_sq);

	        float cos1 = (-A * sin_t1 + B * cos_t1) / L;
	        float sin1 = ( A * cos_t1 + B * sin_t1) / L;
	        float cos2 = (-A * sin_t2 + B * cos_t2) / L;
	        float sin2 = ( A * cos_t2 + B * sin_t2) / L;

	        float a = 2.f * (cos1*cos1 - (1.f - f) + f * cos1 * cos2 - (1.f - f) * sin1 * sin2);
	        float b = L * ((1.f - 2.f * f) * cos2 - (1.f + 2.f * f) * cos1);

	    	constexpr float eps = 0.0001f;
	    	glm::vec2 p1_to_p2_dir = p1_to_p2 / L;
	    	glm::vec2 perpDir(-p1_to_p2_dir.y, p1_to_p2_dir.x);

	        auto solveBranch = [&](float sign) -> std::optional<glm::vec2> {
        		float k1, k2;

        		if (std::abs(1.f - f) < eps) {
        			if (sign < 0.f)
        				return std::nullopt;
        			k1 = 2.f * cos1 / L;
        			k2 = 0.f;
        		} else {
        			float disc = b*b - 4.f * a * f * L_sq;
        			if (disc < -eps * L_sq)
        				return std::nullopt;
        			disc = std::max(disc, 0.f);
        			float sqrtDisc = std::sqrt(disc);

        			if (std::abs(f) > eps)
        				k1 = (-b + sign * sqrtDisc) / (2.f * f * L_sq);
        			else {
        				if (sign < 0.f || std::abs(b) < eps)
        					return std::nullopt;
        				k1 = -a / b;
        			}

        			k2 = (f * L * k1 - (cos1 + cos2)) / (L * (1.f - f));
        		}

        		if (std::abs(k1 + k2) < eps / L)
        			return std::nullopt;

        		return p1 + (f * L) * p1_to_p2_dir + ((sin1 + sin2) / (k1 + k2)) * perpDir;
	        };

	        auto solA = solveBranch( 1.f);
	        auto solB = solveBranch(-1.f);

	    	auto directionAlignment = [&](glm::vec2 J) {
	    		glm::vec2 subChord = J - p1;
	    		glm::vec2 T1(-std::cos(t1), -std::sin(t1));
	    		glm::vec2 TJ;
	    		if (length2(subChord) < 0.00000001f)
	    			TJ = T1;
	    		else {
	    			glm::vec2 u1 = normalize(subChord);
	    			TJ = 2.f * dot(T1, u1) * u1 - T1;
	    		}
	    		return dot(TJ, p1_to_p2_dir);
	    	};

	    	std::optional<glm::vec2> chosen;
	    	if (solA && solB)
	    		chosen = (directionAlignment(*solA) > directionAlignment(*solB)) ? solA : solB;
	    	else
	    		chosen = solA ? solA : solB;

	        if (!chosen)
	        	return;
	        handlePosition = *chosen;

	    	int segmentIndex;
	    	std::optional<Line> lineRestriction;
	        for (int i = 0; i < 2; i++) {
        		auto restriction = manipulateCapOperations[i].getRestriction({.value = handlePosition, .type = SnapType::Cap});
        		if (restriction.impossible)
        			return;
        		if (restriction.line) {
        			if (!lineRestriction) {
        				lineRestriction = *restriction.line;
        				segmentIndex = i;
        			} else if (!linesAreIdentical(*lineRestriction, *restriction.line))
        				return;
        		}
	        }

	        if (lineRestriction) {
        		int arcIndex = 1 - segmentIndex;

        		glm::vec2 Q = lineRestriction->point;
        		glm::vec2 D = { std::cos(lineRestriction->angle), std::sin(lineRestriction->angle) };
        		glm::vec2 NL = { -std::sin(lineRestriction->angle), std::cos(lineRestriction->angle) };

	        	auto P_arc = manipulateCapOperations[arcIndex].fixedCapPlanarPosition;
	        	auto P_seg = manipulateCapOperations[segmentIndex].fixedCapPlanarPosition;

	        	auto tArc = wrapAngle(*manipulateCapOperations[arcIndex].fixedTangentAngle + glm::pi<float>());
	        	glm::vec2 N = { -std::sin(tArc), std::cos(tArc) };

	        	glm::vec2 W = Q - P_arc;

	        	auto solveArcLine = [&](float sigma) -> std::optional<glm::vec2> {
	        		glm::vec2 V = N + sigma * NL;
	        		float det = D.x * V.y - D.y * V.x;
	        		if (std::abs(det) < 0.0001f)
	        			return std::nullopt;

	        		float V_cross_W = V.x * W.y - V.y * W.x;
	        		float t = V_cross_W / det;

	        		glm::vec2 J = Q + t * D;

	        		glm::vec2 chord = J - P_arc;
	        		float chordLenSq = length2(chord);
	        		glm::vec2 T_J;
	        		if (chordLenSq < 0.000001f) {
	        			T_J = { std::cos(tArc), std::sin(tArc) };
	        		} else {
	        			glm::vec2 chordDir = chord / std::sqrt(chordLenSq);
	        			glm::vec2 T_P = { std::cos(tArc), std::sin(tArc) };
	        			T_J = 2.f * dot(T_P, chordDir) * chordDir - T_P;
	        		}

	        		glm::vec2 V_seg = P_seg - J;
	        		if (length2(V_seg) > 0.000001f)
	        			if (dot(T_J, V_seg) < 0.f)
	        				return std::nullopt;

	        		return J;
	        	};

        		auto pPlus = solveArcLine(1.f);
        		auto pMinus = solveArcLine(-1.f);

        		if (pPlus && pMinus) {
        			float distPlus = length2(*pPlus - rawHandlePosition);
        			float distMinus = length2(*pMinus - rawHandlePosition);
        			handlePosition = (distPlus <= distMinus) ? *pPlus : *pMinus;
        		} else if (pPlus)
        			handlePosition = *pPlus;
        		else if (pMinus)
        			handlePosition = *pMinus;
        		else
        			return;

        		std::optional<Line> newLineRestriction;
        		for (auto& capOperation : manipulateCapOperations) {
        			auto restriction = capOperation.getRestriction({.value = handlePosition, .type = SnapType::Cap});
        			if (restriction.impossible)
        				return;
        			if (restriction.line) {
        				if (!newLineRestriction)
        					newLineRestriction = *restriction.line;
        				else if (!linesAreIdentical(*newLineRestriction, *restriction.line))
        					return;
        			}
        		}
	        }
	    }

	    for (auto& capOperation : manipulateCapOperations)
	        capOperation.applyOperationWithSnapResult({.value = handlePosition, .type = SnapType::Cap}, true);
    } else {
        std::vector<Line> lineRestrictions;
        auto idealHandlePosition = manipulateCapOperations[0].initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;

        for (auto& capOperation : manipulateCapOperations) {
            SnapResult idealHandle = {
                .value = idealHandlePosition,
                .type = SnapType::Cap,
                .angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
            };
            auto restriction = capOperation.getRestriction(idealHandle);
            if (restriction.impossible)
                return;
            if (restriction.line) {
                bool addLine = true;
                for (auto existingRestriction : lineRestrictions)
                    if (linesAreIdentical(existingRestriction, *restriction.line))
                        addLine = false; // Don't add duplicate lines
                if (addLine) {
                    if (lineRestrictions.size() == 2)
                        return;
                    lineRestrictions.push_back(*restriction.line);
                }
            }
        }

        if (lineRestrictions.empty())
            for (auto& capOperation : manipulateCapOperations) {
                SnapResult handle = {
                    .value = idealHandlePosition,
                    .type = SnapType::Cap,
                    .angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
                };
                capOperation.applyOperationWithSnapResult(handle, true);
            }
        else {
            glm::vec2 handlePosition;

            if (lineRestrictions.size() == 1) {
                glm::vec2 dir = {std::cos(lineRestrictions[0].angle), std::sin(lineRestrictions[0].angle)};
                handlePosition = lineRestrictions[0].point + dir * dot(dir, idealHandlePosition - lineRestrictions[0].point);
            } else {
                glm::vec2 d1(std::cos(lineRestrictions[0].angle), std::sin(lineRestrictions[0].angle));
                glm::vec2 d2(std::cos(lineRestrictions[1].angle), std::sin(lineRestrictions[1].angle));

                float det = d1.x * d2.y - d1.y * d2.x;

                if (std::abs(det) < 0.0001f)
                    return; // (nearly) parallel

                glm::vec2 dp = lineRestrictions[1].point - lineRestrictions[0].point;
                float t = (dp.x * d2.y - dp.y * d2.x) / det;

                handlePosition = lineRestrictions[0].point + t * d1;
            }

            std::vector<Line> newLineRestrictions;
            for (auto& capOperation : manipulateCapOperations) {
                SnapResult handle = {
                    .value = handlePosition,
                    .type = SnapType::Cap,
                    .angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
                };
                auto restriction = capOperation.getRestriction(handle);
                if (restriction.impossible)
                    return;
                if (restriction.line) {
                    bool addLine = true;
                    for (auto existingRestriction : newLineRestrictions)
                        if (linesAreIdentical(existingRestriction, *restriction.line))
                            addLine = false; // Don't add duplicate lines
                    if (addLine) {
                        if (newLineRestrictions.size() >= lineRestrictions.size())
                            return;
                        newLineRestrictions.push_back(*restriction.line);
                    }
                }
            }
            for (auto& capOperation : manipulateCapOperations) {
                SnapResult handle = {
                    .value = handlePosition,
                    .type = SnapType::Cap,
                    .angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
                };
                capOperation.applyOperationWithSnapResult(handle, true);
            }
        }
    }
}


void ManipulateCapsOperation::applyModifiers(byte mods) {
	preserveLinkedAngles = mods & MOD_ALT;
	smoothJoin = manipulateCapOperations.size() == 2 && !preserveLinkedAngles && ctx.quickSettings.shape.alignWithTangent && mods & MOD_CTRL;
	for (auto& capOperation : manipulateCapOperations)
		capOperation.useSnappedTangent = preserveLinkedAngles;
}