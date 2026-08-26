#include "editor/tool/ToolMode.h"

#include "editor/EditorScene.h"
#include "editor/SelectBox.h"
#include "editor/operation/RotateOperation.h"
#include "editor/operation/ScaleOperation.h"
#include "editor/operation/SelectOperation.h"
#include "editor/operation/TranslateOperation.h"
#include "ui/Theme.h"
#include "ui/UIList.h"
#include "ui/UISegmentedControl.h"


ToolModeResponse ToolMode::processEvent(const Event& event) {
	bool operationChanged = false;

	if (activeOperation) {
		auto response = activeOperation->processEvent(event);
		if (response.status != OperationStatus::Running) {
			activeOperation.reset();
			operationChanged = true;
		}
		if (response.consumedEvent)
			return {.consumedEvent = true, .operationChanged = operationChanged};
	}

	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->id == 0)
			pointer0Position = pointer->position;

		switch (pointer->action) {
		case PointerAction::Down:
			pointerDownEvent = *pointer;
			if (pointer->button == PointerButton::Primary)
				pointerPrimaryDown = true;
			else if (pointer->button == PointerButton::Secondary)
				pointerSecondaryDown = true;
			break;
		case PointerAction::StartDrag:
			dragging = true;
			if (pointer->button == PointerButton::Primary && pointerPrimaryDown ||
				pointer->button == PointerButton::Secondary && pointerSecondaryDown) {
				activeOperation = startDrag(*pointer);
				return {.consumedEvent = true, .operationChanged = activeOperation != nullptr};
			}
			return {.consumedEvent = true, .operationChanged = false};
		case PointerAction::Up:
			if (dragging) {
				if (pointer->button == PointerButton::Primary)
					pointerPrimaryDown = false;
				else if (pointer->button == PointerButton::Secondary)
					pointerSecondaryDown = false;
				if (!pointerPrimaryDown && !pointerSecondaryDown)
					dragging = false;
			} else {
				if (pointer->button == PointerButton::Primary) {
					bool doPrimaryAction = pointerPrimaryDown && !pointerSecondaryDown;
					pointerPrimaryDown = false;
					if (doPrimaryAction) {
						performPrimaryAction(*pointer);
						return {.consumedEvent = true, .operationChanged = false};
					}
				} else if (pointer->button == PointerButton::Secondary) {
					bool doSecondaryAction = pointerSecondaryDown && !pointerPrimaryDown;
					pointerSecondaryDown = false;
					if (doSecondaryAction) {
						performSecondaryAction(*pointer);
						return {.consumedEvent = true, .operationChanged = false};
					}
				}
			}
			break;
		default:;
		}
	} else if (auto key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (auto actionCode = Settings::Bindings->translate(key->chord)) {
				auto result = processObstacleExistenceAction(*actionCode, key->chord.modifiers);
				if (result.consumedEvent)
					return result;

				switch (*actionCode) {
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
					activeOperation = TransformOperation::make(*actionCode, scene, camera, settings, TriggerType::TriggerKey, camera.screenToPlanarPosition(pointer0Position));
					if (!activeOperation->start(key->chord.modifiers))
						activeOperation.reset();
					return {.consumedEvent = true, .operationChanged = true};
				case ActionCode::ToggleTransformBothStates:
					settings.transformBothStates = !settings.transformBothStates;
					return {.consumedEvent = true, .operationChanged = false};
				case ActionCode::ToggleTransformIndividually:
					settings.transformIndividually = !settings.transformIndividually;
					return {.consumedEvent = true, .operationChanged = false};
				default:;
				}
			}

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

	auto response = doProcessEvent(event);
	response.operationChanged |= operationChanged;
	return response;
}

ToolModeResponse ToolMode::processObstacleExistenceAction(ActionCode actionCode, byte modifiers) {
	switch (actionCode) {
	case ActionCode::Copy:
		scene.copySelection();
		return {.consumedEvent = true, .operationChanged = false};
	case ActionCode::Delete: {
		bool operationChanged = false;
		if (activeOperation) {
			auto ss = scene.getSelectionState();
			activeOperation->cancel();
			activeOperation.reset();
			operationChanged = true;
			scene.applySelectionState(ss);
		}
		scene.deleteSelection();
		return {.consumedEvent = true, .operationChanged = operationChanged};
	}
	case ActionCode::Cut: {
		scene.copySelection();
		bool operationChanged = false;
		if (activeOperation) {
			auto ss = scene.getSelectionState();
			activeOperation->cancel();
			activeOperation.reset();
			operationChanged = true;
			scene.applySelectionState(ss);
		}
		scene.deleteSelection();
		return {.consumedEvent = true, .operationChanged = operationChanged};
	}
	case ActionCode::Paste:
		if (!activeOperation && scene.paste()) {
			auto meanCentre = glm::vec2(0.f);
			int selectedCount = 0;
			for (const auto& obstacle : scene.obstacles)
				if (obstacle.isSelected()) {
					meanCentre += worldToPlanar(obstacle.getKinematicState()->getPosition());
					selectedCount++;
				}
			meanCentre /= selectedCount;

			activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::TriggerKey, meanCentre);
			if (!activeOperation->start(modifiers) ||
				activeOperation->processEvent(PointerEvent(0, pointer0Position, PointerAction::Move)).status != OperationStatus::Running)
				activeOperation.reset();
			return {.consumedEvent = true, .operationChanged = true};
		}
		break;
	case ActionCode::Duplicate:
		if (!activeOperation && scene.duplicateSelection()) {
			activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::TriggerKey, camera.screenToPlanarPosition(pointer0Position));
			if (!activeOperation->start(modifiers))
				activeOperation.reset();
			return {.consumedEvent = true, .operationChanged = true};
		}
		break;
	default:;
	}

	return {.consumedEvent = false, .operationChanged = false};
}


std::vector<BindingHint> ToolMode::getBindingHints() const {
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

void ToolMode::populateToolbar(UINode& toolbar) {
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


void ToolMode::createOperationUI(UINode& container) const {
	if (activeOperation) {
		activeOperation->createUI(container);
		activeOperation->updateUI();
	}
}


void ToolMode::cancelActiveOperation() {
	if (activeOperation)
		activeOperation->cancel();
	activeOperation.reset();
}
void ToolMode::commitActiveOperation() {
	if (activeOperation) {
		activeOperation->finish();
		activeOperation->commit();
	}
	activeOperation.reset();
}


std::optional<Cursor> ToolMode::queryCursor() const {
	if (activeOperation)
		return activeOperation->queryCursor();
	return std::nullopt;
}


bool ToolMode::pointedAtBall(glm::vec2 pointerPosition) const {
	auto hitTestBox = SelectBox(camera.screenToPlanarPosition(pointerPosition));
	return scene.ball.isInSelectBox(hitTestBox);
}
bool ToolMode::pointedAtObstacle(glm::vec2 pointerPosition) const {
	auto hitTestBox = SelectBox(camera.screenToPlanarPosition(pointerPosition));
	return std::ranges::any_of(scene.obstacles,
		[&hitTestBox](const auto& obstacle) {
			return obstacle.isInSelectBox(hitTestBox);
		});
}


void ToolMode::performPrimaryAction(const PointerEvent& upEvent) {
	auto selectOperation = SelectOperation(
		scene, camera, TriggerType::Pointer,
		camera.screenToPlanarPosition(pointerDownEvent.position), true);
	if (selectOperation.start(pointerDownEvent.modifiers)) {
		selectOperation.finish();
		selectOperation.commit();
	}
}