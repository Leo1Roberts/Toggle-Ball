#include "editor/tool/TransformMode.h"

#include "editor/operation/RotateOperation.h"
#include "editor/operation/ScaleOperation.h"
#include "editor/operation/SelectOperation.h"
#include "editor/operation/TranslateOperation.h"
#include "ui/Theme.h"
#include "ui/UIList.h"
#include "ui/UISegmentedControl.h"


std::vector<BindingHint> TransformMode::getBindingHints() const {
	std::vector<BindingHint> hints;
	if (auto binding = Settings::Bindings->findBinding(ActionCode::ToggleTransformBothStates))
		hints.emplace_back(*binding, "Toggle transform state");
	if (auto binding = Settings::Bindings->findBinding(ActionCode::ToggleTransformIndividually))
		hints.emplace_back(*binding, "Toggle transform mode");

	if (activeOperation)
		hints.append_range(activeOperation->getBindingHints());

	if (!activeOperation || activeOperation->trigger == TriggerType::TriggerKey) {
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Translate))
			if (!dynamic_cast<TranslateOperation*>(activeOperation.get()))
				hints.emplace_back(*binding, "Translate");
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Rotate))
			if (!dynamic_cast<RotateOperation*>(activeOperation.get()))
				hints.emplace_back(*binding, "Rotate");
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Scale))
			if (!dynamic_cast<ScaleOperation*>(activeOperation.get()))
				hints.emplace_back(*binding, "Scale");
	}

	return hints;
}

void TransformMode::populateToolbar(UINode& toolbar) {
	auto settingsList = toolbar.addChild<UIHorizontalList>(20.f, 0.f);
	settingsList->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(4.f)
	});

	auto addEditorQuickSetting = [&settingsList](const std::string& settingName, const std::vector<std::string>& options, bool* target) {
		auto setting = settingsList->addChild<UIHorizontalList>(8.f, 0.f);
		setting->setLayout({
			.anchor = Anchor::Centre,
			.widthMode = SizingMode::Wrap,
			.heightMode = SizingMode::Wrap,
		});

		auto label = setting->addChild<UIText>(settingName, TextStyle{
			.fontSize = 16.f,
			.color = Color::LightGrey,
			.alignVertical = TextAlignVertical::Middle,
		});
		label->setLayout({
			.widthMode = SizingMode::Wrap,
		});

		auto segmentedControl = setting->addChild<UISegmentedControl>(options, false, Theme::PrimarySegmentedControl, 0.f);
		segmentedControl->setLayout({
			.anchor = Anchor::Centre,
			.widthMode = SizingMode::Wrap,
			.heightMode = SizingMode::Wrap,
		});
		segmentedControl->setOptionLayout({
			.widthMode = SizingMode::Absolute, .width = 60.f,
			.heightMode = SizingMode::Wrap,
			.padding = glm::vec2(4.f)
		});
		segmentedControl->setOptionTextLayout({
			.anchor = Anchor::Centre,
			.heightMode = SizingMode::Wrap,
		});
		segmentedControl->setOnSelectedOptionChange([target](int selected) { *target = selected; });
		segmentedControl->setValueProvider([target] { return *target; });
	};

	addEditorQuickSetting("Transform state", {"Single", "Dual"}, &settings.transformBothStates);
	addEditorQuickSetting("Transform mode", {"Group", "Individual"}, &settings.transformIndividually);
}

void TransformMode::renderGizmos(GizmoRenderer& gizmoRenderer) {
	if (activeOperation)
		activeOperation->renderGizmos(gizmoRenderer);
}


ToolModeResponse TransformMode::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->id == 0)
			pointer0Position = pointer->position;
	} else if (auto key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Copy:
					scene->copySelection();
					return {.consumedEvent = true, .operationChanged = false};
				case ActionCode::Delete: {
					bool operationChanged = false;
					if (activeOperation) {
						auto ss = scene->getSelectionState();
						activeOperation->cancel();
						activeOperation.reset();
						operationChanged = true;
						scene->applySelectionState(ss);
					}
					scene->deleteSelection();
					return {.consumedEvent = true, .operationChanged = operationChanged};
				}
				case ActionCode::Cut: {
					scene->copySelection();
					bool operationChanged = false;
					if (activeOperation) {
						auto ss = scene->getSelectionState();
						activeOperation->cancel();
						activeOperation.reset();
						operationChanged = true;
						scene->applySelectionState(ss);
					}
					scene->deleteSelection();
					return {.consumedEvent = true, .operationChanged = operationChanged};
				}
				case ActionCode::Paste:
					if (!activeOperation && scene->paste()) {
						auto meanCentre = glm::vec2(0.f);
						int selectedCount = 0;
						for (const auto& obstacle : scene->obstacles)
							if (obstacle.isSelected()) {
								meanCentre += worldToPlanar(obstacle.getKinematicState()->getPosition());
								selectedCount++;
							}
						meanCentre /= selectedCount;

						activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::TriggerKey, meanCentre);
						if (!activeOperation->start(key->chord.modifiers) ||
							activeOperation->processEvent(PointerEvent(0, pointer0Position, PointerAction::Move)).status != OperationStatus::Running)
							activeOperation.reset();
						return {.consumedEvent = true, .operationChanged = true};
					}
					break;
				case ActionCode::Duplicate:
					if (!activeOperation && scene->duplicateSelection()) {
						activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::TriggerKey, camera->screenToPlanarPosition(pointer0Position));
						if (!activeOperation->start(key->chord.modifiers))
							activeOperation.reset();
						return {.consumedEvent = true, .operationChanged = true};
					}
					break;
				case ActionCode::Translate:
				case ActionCode::Rotate:
				case ActionCode::Scale:
					if (activeOperation) {
						if (activeOperation->trigger == TriggerType::ActionKey) {
							activeOperation->finish();
							activeOperation->commit();
							activeOperation.reset();
						} else if (activeOperation->trigger == TriggerType::TriggerKey) {
							if (auto transform = dynamic_cast<TransformOperation*>(activeOperation.get())) {
								transform->cancel();
								activeOperation = TransformOperation::makeFromExisting(*actionCode, transform);
								if (!activeOperation->start(key->chord.modifiers) ||
									activeOperation->processEvent(PointerEvent(0, pointer0Position, PointerAction::Move)).status != OperationStatus::Running)
									activeOperation.reset();

								return {.consumedEvent = true, .operationChanged = true};
							}
							return {.consumedEvent = true, .operationChanged = false};
						} else
							return {.consumedEvent = true, .operationChanged = false};
					}
					activeOperation = TransformOperation::make(*actionCode, scene, camera, settings, TriggerType::TriggerKey, camera->screenToPlanarPosition(pointer0Position));
					if (!activeOperation->start(key->chord.modifiers))
						activeOperation.reset();
					return {.consumedEvent = true, .operationChanged = true};
				case ActionCode::ToggleTransformBothStates:
					settings.transformBothStates = !settings.transformBothStates;
					return {.consumedEvent = true, .operationChanged = false};
				case ActionCode::ToggleTransformIndividually:
					settings.transformIndividually = !settings.transformIndividually;
					return {.consumedEvent = true, .operationChanged = false};
				default:
					return {.consumedEvent = false, .operationChanged = false};
				}
			}
		}

		if (key->action == KeyAction::Down) {
			if (TranslateOperation::keyToTranslationVector(key->chord.code)) {
				activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::ActionKey);
				bool success =
					activeOperation->start(key->chord.modifiers) &&
					activeOperation->processEvent(event).status == OperationStatus::Running;
				if (!success)
					activeOperation.reset();
				return {.consumedEvent = true, .operationChanged = success};
			}
		}
	}

	return {.consumedEvent = false, .operationChanged = false};
}


void TransformMode::performPrimaryAction(const PointerEvent& upEvent) {
	auto selectOperation = SelectOperation(
		scene, camera, TriggerType::Pointer,
		camera->screenToPlanarPosition(pointerDownEvent.position), true);
	if (selectOperation.start(pointerDownEvent.modifiers)) {
		selectOperation.finish();
		selectOperation.commit();
	}
}

std::unique_ptr<Operation> TransformMode::startDrag(const PointerEvent& dragStartEvent) {
	bool hit = pointedAtEntity(pointerDownEvent.position);

	if (hit) {
		auto selectOperation = SelectOperation(scene, camera, TriggerType::Pointer, camera->screenToPlanarPosition(pointerDownEvent.position), true);
		if (selectOperation.start(pointerDownEvent.modifiers))
			selectOperation.finish();
		else return nullptr;

		if (selectOperation.getMode() == SelectionMode::Subtract) {
			selectOperation.cancel();
			return nullptr;
		}
	}

	if (dragStartEvent.button == PointerButton::Primary) {
		if (hit) {
			auto translateOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::Pointer, camera->screenToPlanarPosition(pointerDownEvent.position));
			if (translateOperation->start(pointerDownEvent.modifiers))
				return translateOperation;
			return nullptr;
		}

		auto selectOperation = std::make_unique<SelectOperation>(scene, camera, TriggerType::Pointer, camera->screenToPlanarPosition(pointerDownEvent.position));
		if (selectOperation->start(pointerDownEvent.modifiers))
			return selectOperation;
		return nullptr;
	}

	if (dragStartEvent.button == PointerButton::Secondary) {
		if (hit) {
			auto rotateOperation = std::make_unique<RotateOperation>(scene, camera, settings, TriggerType::Pointer, camera->screenToPlanarPosition(pointerDownEvent.position));
			if (rotateOperation->start(pointerDownEvent.modifiers))
				return rotateOperation;
			return nullptr;
		}
	}

	return nullptr;
}