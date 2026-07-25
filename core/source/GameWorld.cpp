#include "GameWorld.h"

#include "MatrixUtilities.h"
#include "Settings.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <ranges>


const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
const glm::vec3 sunDirection = normalize(glm::vec3(2, 2, 3));


GameWorld::GameWorld(const LevelDescriptor& levelDescriptor) {
	level = levelDescriptor;
	level.scale();

	arenaBounds[0] = {{0, 1, 0}, {0, -level.getArenaWidth() * 0.5f, 0}}; // Left
	arenaBounds[1] = {{0, -1, 0}, {0, level.getArenaWidth() * 0.5f, 0}}; // Right
	arenaBounds[2] = {{0, 0, -1}, {0, 0, level.getArenaHeight()}};       // Top
	arenaBounds[3] = {{0, 0, 1}, {0, 0, 0}};                             // Bottom

	ball = GameBall(level.getBallDescriptor().get());
	obstacles.append_range(level.getObstacleDescriptors() | std::views::transform([](const auto& d) { return GameObstacle(d.get()); }));
}


void GameWorld::start() {
	togglePosition.setPosition(0);
	levelComplete = false;

	ball.reset();
	for (auto& obstacle: obstacles)
		obstacle.reset();
}


bool GameWorld::processEvent(const Event& event) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Toggle:
					toggle();
					return true;
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->button == PointerButton::Primary && pointer->action == PointerAction::Down) {
			toggle();
			return true;
		}
	}

	return false;
}


void GameWorld::update(microseconds dt) {
	updatePhysics(dt);
}


constexpr glm::mat3 backgroundRotation = {
	0, 0, -1,
	0, 1, 0,
	1, 0, 0
};

void GameWorld::render() {
	Shaders::object->use();

	Shaders::object->setVec3("uGroundColor", groundColor);
	Shaders::object->setVec3("uSkyColor", skyColor);
	Shaders::object->setVec3("uSunColor", sunColor);
	Shaders::object->setMat4("uProjection", projectionMatrix);
	Shaders::object->setVec3("uUpDirection", viewUpDirection);
	Shaders::object->setVec3("uSunDirection", viewSunDirection);

	drawObject(Meshes::plane.get(), Textures::white.get(),
			   {-1.f, 0, level.getArenaHeight() / 2.f},
			   backgroundRotation,
			   {level.getArenaHeight(), level.getArenaWidth(), 1});

	drawObject(Meshes::ball.get(), ball.getTexture(),
			   ball.getKinematicState()->position,
			   ball.getKinematicState()->rotation,
			   glm::vec3(ball.getProperties()->radius));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	for (const auto& obstacle: obstacles)
		drawObject(obstacle.getMesh(), Textures::white.get(),
				   obstacle.getKinematicState()->getPosition(),
				   obstacle.getKinematicState()->getRotation());
	glDisable(GL_DEPTH_TEST);
}


void GameWorld::resize(int screenWidth, int screenHeight) {
	if (level.getArenaWidth() * (float)screenHeight > (float)screenWidth * level.getArenaHeight()) { // Level is wider than screen
		halfWidth = level.getArenaWidth() * 0.5f;
		halfHeight = halfWidth * (float)screenHeight / (float)screenWidth;
	} else {
		halfHeight = level.getArenaHeight() * 0.5f;
		halfWidth = halfHeight * (float)screenWidth / (float)screenHeight;
	}

	viewOrigin = {0.f, 0.f, level.getArenaHeight() * 0.5f};

	float
	ch = std::cos(heading),
	sh = std::sin(heading),
	cp = std::cos(pitch),
	sp = std::sin(pitch);
	viewDirection.x = ch * cp;
	viewDirection.y = sh * cp;
	viewDirection.z = sp;

	viewDistance = std::max(level.getArenaWidth(), level.getArenaHeight()) * 2.f;

	viewPosition = viewOrigin + viewDirection * viewDistance;

	projectionMatrix = glm::ortho(halfWidth, -halfWidth, -halfHeight, halfHeight, viewDistance - level.getArenaWidth(), viewDistance + level.getArenaWidth());
	viewRotationMatrix = buildViewRotationMatrix(-viewDirection);
	viewMatrix = buildViewMatrix(viewRotationMatrix, viewPosition);

	viewUpDirection = glm::transpose(viewRotationMatrix) * upDirection;
	viewSunDirection = glm::transpose(viewRotationMatrix) * sunDirection;
}



void GameWorld::toggle() {
	toggled = !toggled;
	togglePosition.setDestination(toggled, 0.f, level.getTransitionTime());
}


void GameWorld::updatePhysics(microseconds dt) {
	accumulator += toSeconds(dt);

	while (accumulator >= PHYSICS_TIMESTEP) {
		togglePosition.update(PHYSICS_TIMESTEP);

		for (auto& obstacle: obstacles)
			obstacle.stepKinematicState(togglePosition);

		ball.addNaturalForces();

		for (auto& plane: arenaBounds)
			ball.collideWithPlane(plane);

		for (auto& obstacle: obstacles)
			levelComplete |= ball.collideWithObstacle(obstacle) && obstacle.getDescriptor()->isGoal();

		ball.applyForces();

		accumulator -= PHYSICS_TIMESTEP;
	}
}


void GameWorld::drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale) {
	worldMatrix = buildScaledWorldMatrix(rotation, position, scale);

	glm::mat4 bodyToView = viewMatrix * worldMatrix;
	glm::mat3 bodyToViewRot = glm::transpose(viewRotationMatrix) * rotation;

	Shaders::object->setMat3("uBodyToViewRot", bodyToViewRot, false);
	Shaders::object->setMat4("uBodyToView", bodyToView);

	texture->bind(0);
	model->draw();
}