#include "editor/EditorScreen.h"

#include "editor/tool/DefaultMode.h"
#include "editor/EditorObstacle.h"
#include "Settings.h"
#include "opengl/Shader.h"
#include "editor/operation/TranslateOperation.h"
#include "ui/Theme.h"
#include "ui/UIVerticalList.h"
#include "ui/UiTextBox.h"

#include <iomanip>


const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
static glm::vec3 viewUpDirection;
const glm::vec3 sunDirection = normalize(glm::vec3(2, 2, 3));
static glm::vec3 viewSunDirection;


EditorScreen::EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit) :
	scene(
		std::move(levelToEdit),
		[this] { updateEphemeralMeshes(); }),
	context(
		&quickSettings, &scene, &camera, &gizmoRenderer, &uiToWorldScale,
		[this](std::unique_ptr<Operation> operation) {
			activeOperation = std::move(operation);
			return activeOperation.get();
		},
		[this] { activeOperation = nullptr; }),
	currentToolMode(std::make_unique<DefaultMode>(&context)) {
	camera.reset(scene.level->arenaWidth, scene.level->arenaHeight);
	viewUpDirection = camera.getWorldToViewRotationMatrix() * upDirection;
	viewSunDirection = camera.getWorldToViewRotationMatrix() * sunDirection;


	auto propertiesPanel = std::make_unique<UIPanel>(Theme::DarkPanel);
	propertiesPanel->layout = {
		.anchor = Anchor::CentreRight,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Relative,
		.width = 300.f, .height = 1.f,
	};

	obstacleMotionPropertiesList = propertiesPanel->addChild(std::make_unique<UIVerticalList>(glm::vec2(20.f), 10.f));

	uiManager.addNode(std::move(propertiesPanel));
}


void EditorScreen::processEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->id == 0)
			pointer0Position = pointer->position;
	}

	if (activeOperation && activeOperation->processEvent(event))
		return;

	if (uiManager.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			switch (*actionCode) {
			case ActionCode::Toggle:
				if (key->action == KeyAction::Down) {
					scene.toggle();
					return;
				} break;
			case ActionCode::InstantToggle:
				if (key->action == KeyAction::Down) {
					scene.toggle(false);
					return;
				} break;
			case ActionCode::SelectAll:
				if (key->action == KeyAction::Down) {
					scene.selectAll();
					scene.commitSelectionChange();
					return;
				} break;
			case ActionCode::DeselectAll:
				if (key->action == KeyAction::Down) {
					scene.deselectAll();
					scene.commitSelectionChange();
					return;
				} break;
			case ActionCode::Undo:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					if (activeOperation) {
						activeOperation->finish();
						activeOperation->commit();
					}
					scene.undo();
					return;
				} break;
			case ActionCode::Redo:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					scene.redo();
					return;
				} break;
			case ActionCode::Translate:
				if (key->action == KeyAction::Down) {
					activeOperation = std::make_unique<TranslateOperation>(&context, TriggerType::TriggerKey, pointer0Position);
					if (!activeOperation->start(key->chord.modifiers))
						activeOperation = nullptr;
					return;
				} break;
			case ActionCode::ToggleTransformBothStates:
				if (key->action == KeyAction::Down) {
					quickSettings.transformBothStates = !quickSettings.transformBothStates;
					if (activeOperation)
						activeOperation->onQuickSettingsChanged();
					return;
				} break;
			case ActionCode::ToggleTransformLocally:
				if (key->action == KeyAction::Down) {
					quickSettings.transformLocally = !quickSettings.transformLocally;
					if (activeOperation)
						activeOperation->onQuickSettingsChanged();
					return;
				} break;
			default:;
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::Down:
			if (pointer->button == PointerButton::Tertiary) {
				panning = true;
				camera.startPan(pointer->position);
				return;
			} break;
		case PointerAction::Move:
		case PointerAction::Drag:
			if (panning) {
				camera.updatePan(pointer->position);
				return;
			} break;
		case PointerAction::Up:
			if (pointer->button == PointerButton::Tertiary) {
				panning = false;
				return;
			} break;
		case PointerAction::Scroll: {
			float zoomChange = 1.f;
			if (pointer->scroll.y > 0)
				zoomChange = 1.2f;
			else if (pointer->scroll.y < 0)
				zoomChange = 1.f / 1.2f;
			camera.zoom(zoomChange, pointer->position);
			updateEphemeralMeshes();
			return;
		}
		default:;
		}
	}

	if (currentToolMode && (int)scene.getTogglePosition() == scene.isToggled())
		currentToolMode->processEvent(event);
}


void EditorScreen::update(microseconds dt) {
	scene.update(dt);

	auto selectionState = scene.getSelectionState();
	if (cachedSelectionState != selectionState) {
		updateObstacleMotionPropertiesList();
		cachedSelectionState = selectionState;
	}

	uiManager.update(dt);
}


constexpr glm::mat3 backgroundRotation = {
	0, 0, -1,
	0, 1, 0,
	1, 0, 0
};

void EditorScreen::render() {
	Shaders::object->use();

	Shaders::object->setVec3("uGroundColor", groundColor);
	Shaders::object->setVec3("uSkyColor", skyColor);
	Shaders::object->setVec3("uSunColor", sunColor);
	Shaders::object->setMat4("uProjection", camera.getProjectionMatrix());
	Shaders::object->setVec3("uUpDirection", viewUpDirection);
	Shaders::object->setVec3("uSunDirection", viewSunDirection);

	drawObject(Meshes::plane.get(), Textures::white.get(),
			   {-1.f, 0, scene.level->arenaHeight / 2.f},
			   backgroundRotation,
			   {scene.level->arenaHeight, scene.level->arenaWidth, 1.f});

	Shaders::outline->use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	for (int i = 0; i < scene.obstacles.size(); i++) {
		const auto& obstacle = scene.obstacles[i];
		if (obstacle.isSelected()) {
			Shaders::outline->setVec4("uOutlineColor", col(obstacle.descriptor->color, Settings::Colors.domainOpacity));

			// Place domains at different depths to prevent Z-fighting. Last part keeps the ball outline on top.
			float depth = ((float)i - (float)scene.obstacles.size()) / (float)scene.obstacles.size();
			glm::vec3 position = {depth, obstacle.getDomainPosition()};
			glm::mat4 worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), position);
			Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);

			obstacle.getDomainMesh()->draw();
		}
	}
	glEnable(GL_CULL_FACE);

	Shaders::object->use();
	for (const auto& obstacle: scene.obstacles) {
		Shaders::object->setFloat("uAlpha", getObstacleOpacity(obstacle));

		drawObject(obstacle.getObstacleMesh(), Textures::white.get(),
				   obstacle.getKinematicState()->getPosition(),
				   obstacle.getKinematicState()->getRotation());
	}
	glDisable(GL_BLEND);

	glDepthFunc(GL_ALWAYS);
	drawObject(Meshes::ball.get(), scene.ball.getTexture(),
			   planarToWorld(scene.ball.descriptor->initialPosition),
			   glm::mat3(1));


	Shaders::outline->use();
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);

	for (int i = 0; i < scene.obstacles.size(); i++) {
		const auto& obstacle = scene.obstacles[i];
		if (obstacle.isSelected() && (i != scene.selectionFocus.index || scene.selectionFocus.type != EntityType::Obstacle)) {
			glm::vec4 outlineColor = Color::Selected;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (scene.selectionFocus.type == EntityType::Obstacle) {
		const auto& obstacle = scene.obstacles[scene.selectionFocus.index];
		if (obstacle.isSelected()) {
			glm::vec4 outlineColor = Color::Focused;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (scene.ball.isSelected()) {
		glm::vec4 outlineColor;
		if (scene.selectionFocus.type == EntityType::Ball)
			outlineColor = Color::Focused;
		else
			outlineColor = Color::Selected;
		Shaders::outline->setVec4("uOutlineColor", outlineColor);

		glm::mat4 worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), planarToWorld(scene.level->ballDescriptor->initialPosition), planarToWorld(glm::vec2(scene.ball.getOutlineRadius())));
		Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);

		Meshes::ball->draw();
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (activeOperation)
		activeOperation->renderGizmos();
	else if (currentToolMode)
		currentToolMode->renderGizmos();

	uiManager.render();

	glDisable(GL_BLEND);
}


float EditorScreen::getObstacleOpacity(const EditorObstacle& obstacle) const {
	float opacity = 1.f;

	auto motionSpec = obstacle.descriptor->motion.get();
	if (dynamic_cast<OscillatingPositionSpec*>(motionSpec) ||
	    dynamic_cast<OscillatingAngleSpec*>(motionSpec))
		// Includes a small period where the obstacle is completely invisible
		opacity = 2.5f * std::abs(0.5f - scene.getTogglePosition()) - 0.2f;

	return opacity;
}

void EditorScreen::drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale) const {
	glm::mat4 worldMatrix = buildScaledWorldMatrix(rotation, position, scale);

	glm::mat4 bodyToView = camera.getViewMatrix() * worldMatrix;
	glm::mat3 bodyToViewRotation = camera.getWorldToViewRotationMatrix() * rotation;

	Shaders::object->setMat3("uBodyToViewRot", bodyToViewRotation, false);
	Shaders::object->setMat4("uBodyToView", bodyToView);

	texture->bind(0);
	model->draw();
}

void EditorScreen::drawObstacleOutline(const EditorObstacle& obstacle) const {
	glm::mat4 worldMatrix = buildScaledWorldMatrix(obstacle.getKinematicState()->getRotation(), obstacle.getKinematicState()->getPosition());
	Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);
	obstacle.getOutlineMesh()->draw();

	glDisable(GL_DEPTH_TEST);
	worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), obstacle.getKinematicState()->getPosition(), glm::vec3(uiToWorldScale * Settings::Sizes.centreDotRadius));
	Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);
	Meshes::ball->draw();
	glEnable(GL_DEPTH_TEST);
}


void EditorScreen::updateEphemeralMeshes() {
	uiToWorldScale = uiManager.getScale() * camera.getHalfHeight() * 2.f / (float)height;
	scene.ball.updateOutlineRadius(uiToWorldScale);
	for (auto& obstacle : scene.obstacles)
		obstacle.generateEphemeralMeshes(uiToWorldScale);
}

void EditorScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	camera.update((float)width, (float)height, scene.level->arenaWidth, scene.level->arenaHeight);
	updateEphemeralMeshes();
}


void EditorScreen::updateObstacleMotionPropertiesList() {
	uiManager.removeAllChildrenOfNode(obstacleMotionPropertiesList);

	std::vector<MotionSpecPropertyDescriptor> commonProperties;
	for (const auto& obstacle : scene.obstacles)
		if (obstacle.isSelected()) {
			const auto& properties = obstacle.descriptor->motion->getPropertyDescriptors();

			if (commonProperties.empty())
				commonProperties = properties;
			else {
				std::erase_if(commonProperties, [&](const auto& property) {
					return std::ranges::find(properties, property) == properties.end(); });

				if (commonProperties.empty()) break;
			}
		}

	for (const auto& propertyDescriptor : commonProperties) {
		auto makeListItem = [this, propertyDescriptor](col labelColor, bool toggled = false) {
			auto item =  std::make_unique<UINode>();
			item->layout = {
				.heightMode = SizingMode::Absolute, .height = 50.f
			};
			auto label = item->addChild(std::make_unique<UIText>(getMotionSpecPropertyName(propertyDescriptor.property) + ":", TextStyle{
				.color = labelColor, .alignVertical = TextAlignVertical::Middle}));
			label->layout = {
				.anchor = Anchor::CentreLeft,
				.width = 0.5f
			};
			auto textField = item->addChild(std::make_unique<UITextBox>(TextInputBuffer::Float, Theme::PrimaryTextBox, "-"));
			textField->layout = {
				.anchor = Anchor::CentreRight,
				.width = 0.5f
			};
			textField->setOnTextChange([this, propertyDescriptor, toggled](const UITextBox& tb) {
				if (tb.isEmpty())
					scene.cancelLevelChange();
				else
					for (auto& obstacle : scene.obstacles)
						if (obstacle.isSelected()) {
							obstacle.setMotionProperty(
								tb.getValue<float>(), propertyDescriptor.property, toggled);
							obstacle.initKinematicState();
							obstacle.generateDomainMesh(uiToWorldScale);
						}
			});
			textField->setOnConfirm([this](const UITextBox& tb) {
				if (tb.isEmpty())
					scene.cancelLevelChange();
				else
					scene.commitLevelChange();
			});
			textField->setOnCancel([this](const UITextBox&) { scene.cancelLevelChange(); });
			textField->setValueProvider([this, propertyDescriptor, toggled] -> std::string {
				float value = NAN;
				for (const auto& obstacle : scene.obstacles)
					if (obstacle.isSelected()) {
						float v = obstacle.getMotionProperty(propertyDescriptor.property, toggled);
						if (std::isnan(value))
							value = v;
						else if (v != value) {
							value = NAN;
							break;
						}
					}

				if (std::isnan(value)) return "";

				std::stringstream ss;
				ss << std::fixed << std::setprecision(3) << value;
				std::string result = ss.str();
				if (result.find('.') != std::string::npos) {
					result.erase(result.find_last_not_of('0') + 1, std::string::npos);
					if (result.back() == '.')
						result.pop_back();
				}

				if (result == "-0") return "0";
				return result;
			});

			return item;
		};

		if (propertyDescriptor.stateful) {
			obstacleMotionPropertiesList->addChild(makeListItem(Color::StateA, false));
			obstacleMotionPropertiesList->addChild(makeListItem(Color::StateB, true));
		} else
			obstacleMotionPropertiesList->addChild(makeListItem({200, 200, 200}));
	}

	obstacleMotionPropertiesList->updateBounds(obstacleMotionPropertiesList->getParent()->getAbsoluteBounds());
}