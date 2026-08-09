#include "editor/operation/ScaleOperation.h"

#include "editor/GizmoRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ScaleOperation::ScaleOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
	: TransformOperation(context, trigger, initialPointerPosition) {
	pointerPlanarPosition = initialPointerPlanarPosition;

	auto focus = context->scene->selectionFocus;
	if (focus.type == EntityType::Ball)
		pivot = context->scene->ball.descriptor->initialPosition;
	else if (focus.type == EntityType::Obstacle)
		pivot = worldToPlanar(context->scene->obstacles[focus.index].getKinematicState()->getPosition());
}


void ScaleOperation::renderGizmos() {
	if (typing) return;

	context->gizmoRenderer->addLine(pivot, pointerPlanarPosition, {
		.primaryColor = Color::PointerConnectorLine1,
		.secondaryColor = Color::PointerConnectorLine2,
		.width = Settings::Sizes.lineWidth,
		.dashLength = Settings::Sizes.lineWidth * 3.f,
	});

	context->gizmoRenderer->render();
}


bool ScaleOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
		scale = textInput.getValue<float>();
		applyOperation();
		return true;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Undo:
				case ActionCode::Redo:
				case ActionCode::ToggleTransformLocally:
				case ActionCode::ToggleTransformBothStates:
					return false;
				default:
					return true;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (!typing && trigger != TriggerType::ActionKey && pointer->id == 0 && (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)) {
			pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			scale = std::sqrt(length2(pointerPlanarPosition - pivot) / length2(initialPointerPlanarPosition - pivot));
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c)) {
			typing = true;
			if (textInput.processEvent(*c) == TextInputEventEffect::Buffer) {
				scale = textInput.getValue<float>();
				applyOperation();
			}
		}
	}
	return false;
}


void ScaleOperation::applyOperation() {
	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;

	if (ball->isSelected())
		ball->scaleBy(scale, pivot, context->scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			// TODO: let the user set these
			bool affectMinorRadius = true;
			bool affectMajorRadius = true;

			obstacle.scaleBy(scale, pivot, context->quickSettings->transformLocally,
				affectMinorRadius, affectMajorRadius,
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			if (affectMinorRadius || affectMajorRadius)
				obstacle.generateMeshes(*context->uiToWorldScale);
			else
				obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}
}