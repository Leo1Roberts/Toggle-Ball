#include "main.h"
#include "EditorScreen.h"

#include "DefaultMode.h"
#include "EditorContext.h"
#include "Settings.h"
#include "Shader.h"

#include <ranges>


const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
static glm::vec3 viewUpDirection;
const glm::vec3 sunDirection = normalize(glm::vec3(2, 2, 3));
static glm::vec3 viewSunDirection;


EditorScreen::EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit) :
	scene(std::move(levelToEdit)),
	context(
		&scene, &camera, &gizmoRenderer,
		[this](std::unique_ptr<Operation> operation) {
			activeOperation = std::move(operation);
			return activeOperation.get();
		},
		[this] { activeOperation = nullptr; }),
	currentToolMode(std::make_unique<DefaultMode>(context)) {
	camera.reset(scene.getLevel()->getArenaWidth(), scene.getLevel()->getArenaHeight());
	viewUpDirection = camera.getWorldToViewRotationMatrix() * upDirection;
	viewSunDirection = camera.getWorldToViewRotationMatrix() * sunDirection;
}


void EditorScreen::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (activeOperation) {
		if (activeOperation->processEvent(event))
			return;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Toggle:
					scene.toggle();
					return;
				case ActionCode::InstantToggle:
					scene.toggle(false);
					return;
				case ActionCode::Undo:
					scene.undo();
					return;
				case ActionCode::Redo:
					scene.redo();
					return;
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::Down:
			if (pointer->button == PointerButton::Tertiary) {
				panning = true;
				camera.startPan(pointer->position);
				return;
			}
			break;
		case PointerAction::Move:
			if (panning) {
				camera.updatePan(pointer->position);
				return;
			}
			break;
		case PointerAction::Up:
			if (pointer->button == PointerButton::Tertiary) {
				panning = false;
				return;
			}
			break;
		case PointerAction::Scroll: {
			float zoomChange = 1.f;
			if (pointer->scroll.y > 0)
				zoomChange = 1.2f;
			else if (pointer->scroll.y < 0)
				zoomChange = 1.f / 1.2f;
			camera.zoom(zoomChange, pointer->position);
			updateView();
			return;
		}
		default:;
		}
	}

	if (currentToolMode)
		currentToolMode->processEvent(event);
}


void EditorScreen::update(microseconds dt) {
	scene.update(dt);
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
			   {-1.f, 0, scene.getLevel()->getArenaHeight() / 2.f},
			   backgroundRotation,
			   {scene.getLevel()->getArenaHeight(), scene.getLevel()->getArenaWidth(), 1});

	Shaders::outline->use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	for (int i = 0; i < scene.getObstacles().size(); i++) {
		const auto& obstacle = scene.getObstacles()[i];
		if (obstacle.isSelected()) {
			Shaders::outline->setVec4("uOutlineColor", col(obstacle.getDescriptor()->getColor(), Settings::Colors.domainOpacity));

			// Place domains at different depths to prevent Z-fighting. Last part keeps the ball outline on top.
			float depth = ((float)i - (float)scene.getObstacles().size()) / (float)scene.getObstacles().size();
			glm::vec3 position = {depth, obstacle.getDomainPlanarPosition()};
			glm::mat4 worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), position);
			Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);

			obstacle.getDomainMesh()->draw();
		}
	}
	glEnable(GL_CULL_FACE);

	Shaders::object->use();
	for (const auto& obstacle: scene.getObstacles()) {
		Shaders::object->setFloat("uAlpha", getObstacleOpacity(obstacle));

		drawObject(obstacle.getObstacleMesh(), Textures::white.get(),
				   obstacle.getKinematicState()->getPosition(),
				   obstacle.getKinematicState()->getRotation());
	}
	glDisable(GL_BLEND);

	glDepthFunc(GL_ALWAYS);
	drawObject(Meshes::ball.get(), scene.getBall()->getTexture(),
			   scene.getLevel()->getBallDescriptor()->getInitialPosition(),
			   glm::mat3(1));


	Shaders::outline->use();
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);

	for (int i = 0; i < scene.getObstacles().size(); i++) {
		const auto& obstacle = scene.getObstacles()[i];
		if (obstacle.isSelected() && (i != scene.getSelectionFocus()->index || scene.getSelectionFocus()->type != EntityType::Obstacle)) {
			glm::vec4 outlineColor = Color::Selected;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (scene.getSelectionFocus()->type == EntityType::Obstacle) {
		const auto& obstacle = scene.getObstacles()[scene.getSelectionFocus()->index];
		if (obstacle.isSelected()) {
			glm::vec4 outlineColor = Color::Focused;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (scene.getBall()->isSelected()) {
		glm::vec4 outlineColor;
		if (scene.getSelectionFocus()->type == EntityType::Ball)
			outlineColor = Color::Focused;
		else
			outlineColor = Color::Selected;
		Shaders::outline->setVec4("uOutlineColor", outlineColor);

		glm::vec3 ballOutlinePosition = scene.getLevel()->getBallDescriptor()->getInitialPosition();
		ballOutlinePosition.x -= scene.getBall()->getOutlineRadius();
		glm::mat4 worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), ballOutlinePosition, glm::vec3(scene.getBall()->getOutlineRadius()));
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

	auto motionSpec = obstacle.getDescriptor()->getMotion();
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
	worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), obstacle.getKinematicState()->getPosition(), glm::vec3(centreDotRadius));
	Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);
	Meshes::ball->draw();
	glEnable(GL_DEPTH_TEST);
}


void EditorScreen::updateView() {
	uiToWorldScale = uiManager.getScale() * camera.getHalfHeight() * 2.f / (float)height;
	centreDotRadius = uiToWorldScale * Settings::Sizes.centreDotRadius;
	scene.getBall()->updateOutlineRadius(uiToWorldScale);
	for (auto& obstacle : scene.getObstacles())
		obstacle.generateEphemeralMeshes(uiToWorldScale);
}

void EditorScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	camera.update((float)width, (float)height, scene.getLevel()->getArenaWidth(), scene.getLevel()->getArenaHeight());
	updateView();
}