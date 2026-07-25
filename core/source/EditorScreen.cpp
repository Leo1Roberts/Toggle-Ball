#include "main.h"
#include "EditorScreen.h"

#include "MatrixUtilities.h"
#include "Settings.h"
#include "Shader.h"
#include "glm/ext/matrix_clip_space.hpp"

#include <ranges>


const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
const glm::vec3 sunDirection = normalize(glm::vec3(2, 2, 3));


EditorScreen::EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit) {
	level = std::move(levelToEdit);
	ball = EditorBall(level->getBallDescriptor().get());
	obstacles.append_range(level->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));

	currentNode = makeUndoNode();
}


std::shared_ptr<UndoNode> EditorScreen::makeUndoNode() const {
	return std::make_shared<UndoNode>(*level, makeSelectionUndoNode());
}

std::shared_ptr<SelectionUndoNode> EditorScreen::makeSelectionUndoNode() const {
	std::vector<EntityReference> selection;
	if (ball.isSelected())
		selection.emplace_back(EntityType::Ball);
	for (int i = 0; i < obstacles.size(); i++)
		if (obstacles[i].isSelected())
			selection.emplace_back(EntityType::Obstacle, i);

	return std::make_shared<SelectionUndoNode>(SelectionState(selectionFocus, selection));
}


void EditorScreen::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Toggle:
					toggle();
					return;
				case ActionCode::InstantToggle:
					toggle(false);
					return;
				case ActionCode::Undo:
					undo();
					return;
				case ActionCode::Redo:
					redo();
					return;
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Down) {
			switch (pointer->button) {
			default:;
			}
		}
	}
}


void EditorScreen::update(microseconds dt) {
	updateObstaclePositions(dt);
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
	Shaders::object->setMat4("uProjection", projectionMatrix);
	Shaders::object->setVec3("uUpDirection", viewUpDirection);
	Shaders::object->setVec3("uSunDirection", viewSunDirection);

	drawObject(Meshes::plane.get(), Textures::white.get(),
			   {-1.f, 0, level->getArenaHeight() / 2.f},
			   backgroundRotation,
			   {level->getArenaHeight(), level->getArenaWidth(), 1});

	Shaders::outline->use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	for (int i = 0; i < obstacles.size(); i++) {
		const auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			Shaders::outline->setVec4("uOutlineColor", col(obstacle.getDescriptor()->getColor(), Settings::Colors.domainOpacity));

			// Place domains at different depths to prevent Z-fighting. Last part keeps the ball outline on top.
			float depth = ((float)i - (float)obstacles.size()) / (float)obstacles.size();
			glm::vec3 position = {depth, obstacle.getDomainPlanarPosition()};
			worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), position);
			Shaders::outline->setMat4("uProjectionFull", projectionMatrix * viewMatrix * worldMatrix);

			obstacle.getDomainMesh()->draw();
		}
	}
	glEnable(GL_CULL_FACE);

	Shaders::object->use();
	for (const auto& obstacle: obstacles) {
		Shaders::object->setFloat("uAlpha", getObstacleOpacity(obstacle));

		drawObject(obstacle.getObstacleMesh(), Textures::white.get(),
				   obstacle.getKinematicState()->getPosition(),
				   obstacle.getKinematicState()->getRotation());
	}
	glDisable(GL_BLEND);

	glDepthFunc(GL_ALWAYS);
	drawObject(Meshes::ball.get(), ball.getTexture(),
			   level->getBallDescriptor()->getInitialPosition(),
			   glm::mat3(1));


	Shaders::outline->use();
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);

	for (int i = 0; i < obstacles.size(); i++) {
		const auto& obstacle = obstacles[i];
		if (obstacle.isSelected() && (i != selectionFocus.index || selectionFocus.type != EntityType::Obstacle)) {
			glm::vec4 outlineColor;
			// if (action != ACTION_NONE && limiting[i])
			// 	outlineColor = Color::Warning;
			// else
			outlineColor = Color::Selected;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (selectionFocus.type == EntityType::Obstacle) {
		const auto& obstacle = obstacles[selectionFocus.index];
		
		glm::vec4 outlineColor;
		// if (action != ACTION_NONE && limiting[focus])
		// 	outlineColor = Color::Warning;
		// else
		outlineColor = Color::Focused;
		outlineColor.a *= getObstacleOpacity(obstacle);
		Shaders::outline->setVec4("uOutlineColor", outlineColor);

		drawObstacleOutline(obstacle);
	}

	if (ball.isSelected()) {
		// bool intersecting = checkBallObstacleCollision(&ball, true) >= 0;

		glm::vec4 outlineColor;
		// if (intersecting || (action != ACTION_NONE && limiting[MAX_OBSTACLES]))
		// 	outlineColor = Color::Warning;
		// else
		if (selectionFocus.type == EntityType::Ball)
			outlineColor = Color::Focused;
		else
			outlineColor = Color::Selected;
		Shaders::outline->setVec4("uOutlineColor", outlineColor);

		glm::vec3 ballOutlinePosition = level->getBallDescriptor()->getInitialPosition();
		ballOutlinePosition.x -= ball.getOutlineRadius();
		worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), ballOutlinePosition, glm::vec3(ball.getOutlineRadius()));
		Shaders::outline->setMat4("uProjectionFull", projectionMatrix * viewMatrix * worldMatrix);

		Meshes::ball->draw();
	}
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}


void EditorScreen::toggle(bool transition) {
	toggled = !toggled;
	if (transition)
		togglePosition.setDestination(toggled, 0.f, level->getTransitionTime());
	else
		togglePosition.setPosition(toggled);
}


void EditorScreen::updateObstaclePositions(microseconds dt) {
	togglePosition.update(toSeconds(dt));
	for (auto& obstacle: obstacles)
		obstacle.updateKinematicState(togglePosition);
}


float EditorScreen::getObstacleOpacity(const EditorObstacle& obstacle) const {
	float opacity = 1.f;

	auto motionSpec = obstacle.getDescriptor()->getMotion();
	if (dynamic_cast<OscillatingPositionSpec*>(motionSpec) ||
	    dynamic_cast<OscillatingAngleSpec*>(motionSpec))
		// Includes a small period where the obstacle is completely invisible
		opacity = 2.5f * std::abs(0.5f - togglePosition.getCurrentPosition()) - 0.2f;

	return opacity;
}

void EditorScreen::drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale) {
	worldMatrix = buildScaledWorldMatrix(rotation, position, scale);

	glm::mat4 bodyToView = viewMatrix * worldMatrix;
	glm::mat3 bodyToViewRot = glm::transpose(viewRotationMatrix) * rotation;

	Shaders::object->setMat3("uBodyToViewRot", bodyToViewRot, false);
	Shaders::object->setMat4("uBodyToView", bodyToView);

	texture->bind(0);
	model->draw();
}

void EditorScreen::drawObstacleOutline(const EditorObstacle& obstacle) {
	worldMatrix = buildScaledWorldMatrix(obstacle.getKinematicState()->getRotation(), obstacle.getKinematicState()->getPosition());
	Shaders::outline->setMat4("uProjectionFull", projectionMatrix * viewMatrix * worldMatrix);
	obstacle.getOutlineMesh()->draw();

	glDisable(GL_DEPTH_TEST);
	worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), obstacle.getKinematicState()->getPosition(), glm::vec3(centreDotRadius));
	Shaders::outline->setMat4("uProjectionFull", projectionMatrix * viewMatrix * worldMatrix);
	Meshes::ball->draw();
	glEnable(GL_DEPTH_TEST);
}


void EditorScreen::syncLevel() {
	*level = currentNode->level;

	ball = EditorBall(level->getBallDescriptor().get());

	obstacles.assign_range(level->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));
}

void EditorScreen::syncSelection() {
	selectionFocus = currentNode->selectionNode->selectionState.focus;
	for (const auto& selectedEntity : currentNode->selectionNode->selectionState.selection)
		if (selectedEntity.type == EntityType::Obstacle)
			obstacles[selectedEntity.index].select();

}

void EditorScreen::undo() {
	if (currentNode->selectionNode->previous) { // Undo selection only
		currentNode->selectionNode = currentNode->selectionNode->previous;
		syncSelection();
	} else if (currentNode->previous) { // Undo change to level
		currentNode = currentNode->previous;
		syncLevel();
		syncSelection();
	}
}

void EditorScreen::redo() {
	if (currentNode->selectionNode->next) { // Redo selection only
		currentNode->selectionNode = currentNode->selectionNode->next;
		syncSelection();
	} else if (currentNode->next) { // Redo change to level
		currentNode = currentNode->next;
		syncLevel();
		syncSelection();
	}
}


void EditorScreen::doResize(int width, int height, float dpiScale) {
	uiManager.resize(width, height, dpiScale);

	if (level->getArenaWidth() * (float)height > (float)width * level->getArenaHeight()) { // Level is wider than screen
		halfWidth = level->getArenaWidth() * 0.5f;
		halfHeight = halfWidth * (float)height / (float)width;
	} else {
		halfHeight = level->getArenaHeight() * 0.5f;
		halfWidth = halfHeight * (float)width / (float)height;
	}

	viewOrigin = {0.f, 0.f, level->getArenaHeight() * 0.5f};

	float
	ch = std::cos(heading),
	sh = std::sin(heading),
	cp = std::cos(pitch),
	sp = std::sin(pitch);
	viewDirection.x = ch * cp;
	viewDirection.y = sh * cp;
	viewDirection.z = sp;

	viewDistance = std::max(level->getArenaWidth(), level->getArenaHeight()) * 2.f;

	viewPosition = viewOrigin + viewDirection * viewDistance;

	float uiToWorldScale = dpiScale * Settings::Sizes.uiScale * halfHeight * 2.f / height;
	centreDotRadius = uiToWorldScale * Settings::Sizes.centreDotRadius;
	ball.updateOutlineRadius(uiToWorldScale);
	for (auto& obstacle : obstacles)
		obstacle.generateEphemeralMeshes(uiToWorldScale);

	projectionMatrix = glm::ortho(halfWidth, -halfWidth, -halfHeight, halfHeight, viewDistance - level->getArenaWidth(), viewDistance + level->getArenaWidth());
	viewRotationMatrix = buildViewRotationMatrix(-viewDirection);
	viewMatrix = buildViewMatrix(viewRotationMatrix, viewPosition);

	viewUpDirection = glm::transpose(viewRotationMatrix) * upDirection;
	viewSunDirection = glm::transpose(viewRotationMatrix) * sunDirection;
}