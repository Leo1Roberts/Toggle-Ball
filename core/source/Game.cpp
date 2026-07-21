#include "Game.h"

#include "KeyBindings.h"
#include "MatrixUtilities.h"
#include "Settings.h"
#include "Shader.h"
#include "Theme.h"
#include "UIButton.h"

#include <glm/glm.hpp>
#include <ranges>

const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
constexpr glm::vec3 sunDirection{0.4850712500726659, 0.4850712500726659, 0.7276068751089989};


Game::Game() {
	auto rootNode = std::make_unique<UINode>();

	auto restartButton = std::make_unique<UIButton>("Restart", Theme::PrimaryButton);
	restartButton->layout = {
		.anchor = Anchor::TopRight,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 100.f, .height = 60.f,
		.offset = {-20.f, 20.f}
	};
	restartButton->setOnClick([this] { this->start(); });

	rootNode->addChild(std::move(restartButton));

	uiManager.setRootNode(std::move(rootNode));
}


void Game::play(const LevelDescriptor* levelToPlay) {
	load(levelToPlay);
	start();
}

void Game::load(const LevelDescriptor* levelToLoad) {
	level = *levelToLoad;
	level.scale();

	arenaBounds[0] = {{0, 1, 0}, {0, -level.getArenaWidth() * 0.5f, 0}}; // Left
	arenaBounds[1] = {{0, -1, 0}, {0, level.getArenaWidth() * 0.5f, 0}}; // Right
	arenaBounds[2] = {{0, 0, -1}, {0, 0, level.getArenaHeight()}};       // Top
	arenaBounds[3] = {{0, 0, 1}, {0, 0, 0}};                             // Bottom

	ball = GameBall(level.getBallDescriptor().get());
	obstacles.append_range(level.getObstacleDescriptors() | std::views::transform([](const auto& d) { return GameObstacle(d.get()); }));

	updateView();
}

void Game::start() {
	togglePosition.setPosition(0);

	ball.reset();
	for (auto& obstacle: obstacles)
		obstacle.reset();
}


void Game::toggle() {
	toggled = !toggled;
	togglePosition.setDestination(toggled, 0.f, level.getTransitionTime());
}


void Game::doProcessEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Toggle:
					toggle();
					break;
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->button == PointerButton::Primary && pointer->action == PointerAction::Down)
			toggle();
	}
}


void Game::doUpdate(microseconds dt) {
	updatePhysics(dt);
}

void Game::updatePhysics(microseconds dt) {
	accumulator += toSeconds(dt);

	while (accumulator >= PHYSICS_TIMESTEP) {
		togglePosition.update(PHYSICS_TIMESTEP);

		for (auto& obstacle: obstacles)
			obstacle.stepKinematicState(togglePosition);

		ball.addNaturalForces();

		for (auto& plane: arenaBounds)
			ball.collideWithPlane(plane);

		bool levelComplete = false; // For demonstration. Eventually will do something with this.
		for (auto& obstacle: obstacles)
			levelComplete |= ball.collideWithObstacle(obstacle);

		ball.applyForces();

		accumulator -= PHYSICS_TIMESTEP;
	}
}


constexpr glm::mat3 backgroundRotation = {
0, 0, -1,
0, 1, 0,
1, 0, 0};

void Game::doDraw() {
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	for (auto& o: obstacles)
		drawObject(o.getMesh(), Textures::white.get(),
		           o.getKinematicState()->getPosition(),
		           o.getKinematicState()->getRotation());
	glDisable(GL_DEPTH_TEST);

	uiManager.render();
}

void Game::drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 pos, const glm::mat3& rot, glm::vec3 scale) {
	worldMatrix = buildScaledWorldMatrix(rot, pos, scale);

	glm::mat4 bodyToView = viewMatrix * worldMatrix;
	glm::mat3 bodyToViewRot = glm::transpose(viewRotationMatrix) * rot;

	Shaders::object->setMat3("uBodyToViewRot", bodyToViewRot, false);
	Shaders::object->setMat4("uBodyToView", bodyToView);

	texture->bind(0);
	model->draw();
}


void Game::doResize(int width, int height, float dpiScale) {
	uiManager.resize(width, height, dpiScale);
	resizeLevel();
	updateView();
}

void Game::resizeLevel() {
	float arenaWidth = level.getArenaWidth(), arenaHeight = level.getArenaHeight();
	float levelAspectRatio = arenaWidth / arenaHeight;

	if (levelAspectRatio > aspectRatio) // Level is wider than screen
		halfHeight = arenaWidth * 0.5f / aspectRatio;
	else
		halfHeight = arenaHeight * 0.5f;
}


void Game::updateView() {
	viewOrigin = {0, 0, level.getArenaHeight() * 0.5f};

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

	projectionMatrix = glm::ortho(halfHeight * aspectRatio, -halfHeight * aspectRatio, -halfHeight, halfHeight, viewDistance - level.getArenaWidth(), viewDistance + level.getArenaWidth());
	viewRotationMatrix = buildViewRotationMatrix(-viewDirection);
	viewMatrix = buildViewMatrix(viewRotationMatrix, viewPosition);

	viewUpDirection = glm::transpose(viewRotationMatrix) * upDirection;
	viewSunDirection = glm::transpose(viewRotationMatrix) * sunDirection;
}