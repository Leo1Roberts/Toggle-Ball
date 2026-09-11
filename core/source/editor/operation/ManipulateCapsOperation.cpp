#include "editor/operation/ManipulateCapsOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <ranges>


ManipulateCapsOperation::ManipulateCapsOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, const std::vector<CapInfo>& allCapsInfo) :
	Operation(ctx, trigger, initialPlanarPosition) {
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


std::vector<BindingHint> ManipulateCapsOperation::getBindingHints() const {
	std::vector<BindingHint> hints;

	if (ctx.quickSettings.shape.alignWithTangent) {
		hints.emplace_back(KeyChord(KeyCode::Unknown, MOD_ALT), "Preserve joint angles");
		if (manipulateCapOperations.size() == 2 && !preserveLinkedAngles)
			hints.emplace_back(KeyChord(KeyCode::Unknown, MOD_CTRL), "Smooth connect");
	}

	return hints;
}


void ManipulateCapsOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	auto manipulatedCapPos = manipulateCapOperations[0].obstacle.getCapPosition(manipulateCapOperations[0].currentlyLeftCap);

	float maxMinorRadius = 0.f;
	for (const auto& op : manipulateCapOperations)
		maxMinorRadius = std::max(maxMinorRadius, op.initialDescriptor.shape->minorRadius);
	float radius = std::min(Settings::Sizes.obstacleHandleRadius, gizmoRenderer.planarToUIDistance(maxMinorRadius));

	if (smoothJoin) {
		float angle = manipulateCapOperations[0].obstacle.getKinematicState()->getAngle() +
			manipulateCapOperations[0].obstacle.descriptor->shape->getCapAngle(manipulateCapOperations[0].currentlyLeftCap);

		gizmoRenderer.addSplitCircle(manipulatedCapPos, radius, angle, {
			.fillColor = {Color::White, 0.8f},
			.strokeColor = {Color::Black, 0.8f},
			.cornerRadius = 0.f,
			.strokeWidth = 2.f,
		}, {
			.fillColor = {Color::White, 0.8f},
			.strokeColor = {Color::Black, 0.8f},
			.cornerRadius = 0.f,
			.strokeWidth = 2.f,
		});
	} else {
		for (auto i : ctx.getPointedObstacleIndices(pointerPlanarPosition,
			manipulatedEntities | std::views::transform(&EntityReference::index) | std::ranges::to<std::vector>())) {
			const auto& otherObstacle = ctx.scene.obstacles[i];
			auto addInactiveHandle = [&](glm::vec2 capPos) {
				if (length2(capPos - manipulatedCapPos) > 0.00000001f &&
					std::ranges::all_of(manipulateCapOperations, [&](const auto& op) {
						return length2(capPos - op.fixedCapPlanarPosition) > 0.00000001f;
				})) {
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

		switch (snapResult.type) {
		case SnapType::None:
			gizmoRenderer.addCircle(manipulatedCapPos, {
				.fillColor = {Color::White, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
			break;
		case SnapType::Spine:
			gizmoRenderer.addCircle(manipulatedCapPos, {
				.fillColor = {Color::SoftCyan, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
			break;
		default:
			gizmoRenderer.addCircle(manipulatedCapPos, {
				.fillColor = {Color::SoftGreen, 0.8f},
				.strokeColor = {Color::Black, 0.8f},
				.cornerRadius = radius,
				.strokeWidth = 2.f,
			});
		}
	}
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

		float f_base = dot(rawHandlePosition - p1, p1_to_p2) / p1_to_p2_distanceSq;

	    if (linesAreIdentical({p1, t1}, {p2, t2})) {
		    if (std::atan2( p1_to_p2.y,  p1_to_p2.x) - t1 < 0.0001f &&
			    std::atan2(-p1_to_p2.y, -p1_to_p2.x) - t2 < 0.0001f)
			    handlePosition = p1 + std::clamp(f_base, 0.f, 1.f) * p1_to_p2;
		    else return;
	    } else {
	    	// Note: finding 'chosen' was vibe coded

	    	float sin_t1 = std::sin(t1), cos_t1 = std::cos(t1);
			float sin_t2 = std::sin(t2), cos_t2 = std::cos(t2);

			float A = p1_to_p2.x, B = p1_to_p2.y;
			float L_sq = A*A + B*B;
			float L = std::sqrt(L_sq);

			float cos1 = (-A * sin_t1 + B * cos_t1) / L;
			float sin1 = ( A * cos_t1 + B * sin_t1) / L;
			float cos2 = (-A * sin_t2 + B * cos_t2) / L;
			float sin2 = ( A * cos_t2 + B * sin_t2) / L;

			auto p1_to_p2_dir = p1_to_p2 / L;
			glm::vec2 perpDir = {-p1_to_p2_dir.y, p1_to_p2_dir.x};

			constexpr float eps = 0.0001f;
			constexpr double maxCurvature = 1e5;

			auto evalJ = [&](float fIn, float sign) -> std::optional<glm::vec2> {
				double f = fIn;
				double dcos1 = cos1, dsin1 = sin1, dcos2 = cos2, dsin2 = sin2;
				double dL = L, dL_sq = L_sq;

				double a = 2. * (dcos1*dcos1 - (1. - f) + f*dcos1*dcos2 - (1. - f)*dsin1*dsin2);
				double b = dL * ((1. - 2.*f) * dcos2 - (1. + 2.*f) * dcos1);

				double k1, k2;
				if (std::abs(1. - f) < eps) {
					if (sign < 0.f)
						return std::nullopt;
					k1 = 2. * dcos1 / dL;
					k2 = 0.;
				} else {
					double C = f * dL_sq;
					double disc = b*b - 4. * C * a;
					if (disc < (double)-eps * dL_sq)
						return std::nullopt;
					disc = std::max(disc, 0.);
					double sqrtDisc = std::sqrt(disc);

					if (std::abs(C) > (double)eps * dL_sq) {
						double sgnB = (b >= 0.) ? 1. : -1.;
						double q = -0.5 * (b + sgnB * sqrtDisc);
						if (sign * sgnB > 0.) {
							if (std::abs(q) < 1e-12)
								return std::nullopt;
							k1 = a / q;
						} else
							k1 = q / C;
					} else {
						if (sign < 0.f || std::abs(b) < eps)
							return std::nullopt;
						k1 = -a / b;
					}
					k2 = (f * dL * k1 - (dcos1 + dcos2)) / (dL * (1. - f));
				}

				if (std::abs(k1) > maxCurvature / dL || std::abs(k2) > maxCurvature / dL)
					return std::nullopt;

				if (std::abs(k1 + k2) < (double)eps / dL)
					return std::nullopt;

				auto dchordDir = glm::dvec2(p1_to_p2_dir), dperpDir = glm::dvec2(perpDir);
				auto dp1 = glm::dvec2(p1);
				auto J = dp1 + (f * dL) * dchordDir + ((dsin1 + dsin2) / (k1 + k2)) * dperpDir;

				glm::dvec2 N1 = {(double)-sin_t1, (double)cos_t1};
				glm::dvec2 N2 = {(double)-sin_t2, (double)cos_t2};

				auto d1 = J - dp1;
				auto d2 = J - glm::dvec2(p2);

				double residual1 = k1 * dot(d1, d1) - 2. * dot(d1, N1);
				double residual2 = k2 * dot(d2, d2) - 2. * dot(d2, N2);

				double tol = 1e-3 * dL;
				if (std::abs(residual1) > tol || std::abs(residual2) > tol)
					return std::nullopt;

				return glm::vec2(J);
			};

			auto findBestOnBranch = [&](float sign) -> std::optional<glm::vec2> {
				constexpr int coarseSamples = 41;
				constexpr float coarseRange = 6.f;

				float bestF = f_base;
				std::optional<float> bestCost;
				std::optional<glm::vec2> bestJ;

				for (int i = 0; i < coarseSamples; i++) {
					float trialF = f_base + coarseRange * ((float)i / (coarseSamples - 1) - 0.5f) * 2.f;
					if (auto J = evalJ(trialF, sign)) {
						float c = length2(*J - rawHandlePosition);
						if (!bestCost || c < *bestCost) {
							bestCost = c;
							bestF = trialF;
							bestJ = J;
						}
					}
				}
				if (!bestCost)
					return std::nullopt;

				float f = bestF;
				float step = coarseRange / (coarseSamples - 1);
				for (int iter = 0; iter < 30 && step > 1e-5f; iter++) {
					bool improved = false;
					for (float trialF : { f + step, f - step }) {
						if (auto J = evalJ(trialF, sign)) {
							float c = length2(*J - rawHandlePosition);
							if (c < *bestCost) {
								bestCost = c;
								f = trialF;
								bestJ = J;
								improved = true;
							}
						}
					}
					if (!improved)
						step *= 0.5f;
				}

				return bestJ;
			};

			auto solA = findBestOnBranch( 1.f);
			auto solB = findBestOnBranch(-1.f);

			std::optional<glm::vec2> chosen;
			if (solA && solB) {
				float distA = length2(*solA - rawHandlePosition);
				float distB = length2(*solB - rawHandlePosition);
				chosen = (distA <= distB) ? solA : solB;
			} else
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
        auto rawHandlePosition = manipulateCapOperations[0].initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;

    	snapResult = ctx.snapPoint(rawHandlePosition, manipulatedEntities);
    	glm::vec2 handlePosition = snapResult.value;

    	if (snapResult.type != SnapType::None) {
    		for (auto& capOperation : manipulateCapOperations) {
    			SnapResult handle = {
    				.value = snapResult.value, .type = snapResult.type,
					.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
				};
    			auto restriction = capOperation.getRestriction(handle);
    			if (restriction.impossible || restriction.line) {
    				snapResult = {};
    				handlePosition = rawHandlePosition;
    				break;
    			}
    		}
    	}

        std::vector<Line> lineRestrictions;

    	if (snapResult.type == SnapType::None) {
    		for (auto& capOperation : manipulateCapOperations) {
    			SnapResult handle = {
    				.value = handlePosition, .type = SnapType::Cap,
					.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
				};
    			auto restriction = capOperation.getRestriction(handle);
    			if (restriction.impossible)
    				return;
    			if (restriction.line) {
    				if (!std::ranges::any_of(lineRestrictions, [&](const auto& r) {
    					return linesAreIdentical(r, *restriction.line); // Don't add duplicate lines
    				}))
    					lineRestrictions.push_back(*restriction.line);
    			}
    		}
    	}

        if (lineRestrictions.empty())
            for (auto& capOperation : manipulateCapOperations) {
                SnapResult handle = {
                    .value = handlePosition, .type = SnapType::Cap,
                    .angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
                };
                capOperation.applyOperationWithSnapResult(handle, true);
            }
        else {
            glm::vec2 finalHandlePosition;

            if (lineRestrictions.size() == 1) {
                glm::vec2 dir = {std::cos(lineRestrictions[0].angle), std::sin(lineRestrictions[0].angle)};
                auto straightnessSnappedPosition = lineRestrictions[0].point + dir * dot(dir, rawHandlePosition - lineRestrictions[0].point);
            	
            	snapResult = ctx.snapPointRestrictedToLine(straightnessSnappedPosition, manipulatedEntities, lineRestrictions[0].point, lineRestrictions[0].angle);
            	finalHandlePosition = snapResult.value;

            	if (snapResult.type != SnapType::None) {
            		for (auto& capOperation : manipulateCapOperations) {
            			SnapResult handle = {
            				.value = finalHandlePosition, .type = SnapType::Cap,
							.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
						};
            			auto restriction = capOperation.getRestriction(handle);
            			if (restriction.impossible ||
            				restriction.line && !linesAreIdentical(lineRestrictions[0], *restriction.line)) {
            				snapResult = {};
            				finalHandlePosition = straightnessSnappedPosition;
            				break;
            			}
            		}
            	}
            } else {
	            glm::vec2 dir_0 = {std::cos(lineRestrictions[0].angle), std::sin(lineRestrictions[0].angle)};
            	bool finalHandlePositionSet = false;

            	for (int i = 1; i < lineRestrictions.size(); i++) {
            		glm::vec2 dir_i = {std::cos(lineRestrictions[i].angle), std::sin(lineRestrictions[i].angle)};

            		float det = dir_0.x * dir_i.y - dir_0.y * dir_i.x;

            		if (std::abs(det) < 0.0001f)
            			return; // (nearly) parallel

            		glm::vec2 dp = lineRestrictions[i].point - lineRestrictions[0].point;
            		float t = (dp.x * dir_i.y - dp.y * dir_i.x) / det;

            		auto intersection = lineRestrictions[0].point + t * dir_0;
            		if (finalHandlePositionSet) {
            			if (length2(finalHandlePosition - intersection) > 0.00000001f)
            				return;
            		} else {
            			finalHandlePosition = intersection;
            			finalHandlePositionSet = true;
            		}
            	}
            }

            std::vector<Line> newLineRestrictions;
            for (auto& capOperation : manipulateCapOperations) {
                SnapResult handle = {
                    .value = finalHandlePosition,
                    .type = SnapType::Cap,
                    .angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
                };
                auto restriction = capOperation.getRestriction(handle);
                if (restriction.impossible)
                    return;
                if (restriction.line) {
                	if (!std::ranges::any_of(newLineRestrictions, [&](const auto& r) {
						return linesAreIdentical(r, *restriction.line); // Don't add duplicate lines
					})) {
                		if (newLineRestrictions.size() >= lineRestrictions.size())
                			return;
                		newLineRestrictions.push_back(*restriction.line);
                	}
                }
            }
            for (auto& capOperation : manipulateCapOperations) {
                SnapResult handle = {
                    .value = finalHandlePosition,
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