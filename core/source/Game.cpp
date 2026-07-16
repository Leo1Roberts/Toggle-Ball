#include "Game.h"

#include "MatrixUtilities.h"
#include "Shader.h"

const vec3 groundColor = colorToLinear({0.3f, 0.3f, 0.3f});
const vec3 skyColor = colorToLinear({85.0f / 255.0f, 110.0f / 255.0f, 128.0f / 255.0f});
const vec3 sunColor = colorToLinear({1.0f, 1.0f, 0.9f});

constexpr vec3 upDirection{0, 0, 1};
constexpr vec3 sunDirection{0.4850712500726659, 0.4850712500726659, 0.7276068751089989};


void Game::play(const LevelDescriptor* levelToPlay) {
	load(levelToPlay);
	start();
}

void Game::load(const LevelDescriptor* levelToLoad) {
	level = *levelToLoad;
	level.scale();

	arenaBounds[0] = {{0, 1, 0}, {0, -level.getArenaWidth() * 0.5f, 0}}; // Left
	arenaBounds[1] = {{0, -1, 0}, {0, level.getArenaWidth() * 0.5f, 0}}; // Right
	arenaBounds[2] = {{0, 0, -1}, {0, 0, level.getArenaHeight()}}; // Top
	arenaBounds[3] = {{0, 0, 1}, {0, 0, 0}}; // Bottom

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


bool Game::doProcessEvent(const Event&) {
	return false;
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

		for (const auto& plane : arenaBounds)
			ball.collideWithPlane(plane);

		for (auto& obstacle: obstacles)
			ball.collideWithObstacle(obstacle);

		ball.applyForces();

		accumulator -= PHYSICS_TIMESTEP;
	}
}


constexpr mat3 backgroundRotation = {
	 0, 0, 1,
	 0, 1, 0,
	-1, 0, 0
};

void Game::doDraw() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shaders::object->use();

	Shaders::object->setVec3("uGroundColor", groundColor);
	Shaders::object->setVec3("uSkyColor", skyColor);
	Shaders::object->setVec3("uSunColor", sunColor);
	Shaders::object->setMat4("uProjection", projectionMatrix);
	Shaders::object->setVec3("uUpDirection", viewUpDirection);
	Shaders::object->setVec3("uSunDirection", viewSunDirection);

	drawObject(Meshes::plane.get(), Textures::white.get(),
		{-level.getArenaWidth() / 2.f, 0, level.getArenaHeight() / 2.f},
		backgroundRotation,
		{1, level.getArenaWidth(), level.getArenaHeight()});

	drawObject(Meshes::ball.get(), ball.getTexture(),
		ball.getKinematicState()->position,
		ball.getKinematicState()->rotation,
		vec3(ball.getProperties()->radius));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	for (auto& o: obstacles)
		drawObject(o.getMesh(), Textures::white.get(),
			o.getKinematicState()->getPosition(),
			o.getKinematicState()->getRotation());
	glDisable(GL_DEPTH_TEST);
}

void Game::drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, vec3 pos, const mat3& rot, vec3 scale) {
	buildScaledWorldMatrix(&worldMatrix, rot, pos, scale);

	mat4 bodyToView = worldMatrix * viewMatrix;
	mat3 bodyToViewRot = viewRotationMatrix * rot;

	Shaders::object->setMat3("uBodyToViewRot", bodyToViewRot, true);
	Shaders::object->setMat4("uBodyToView", bodyToView);

	texture->bind(0);
	model->draw();
}


void Game::doResize(int, int) {
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
	viewOrigin.set(0, 0, level.getArenaHeight() * 0.5f);

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

	// For perspective use: buildProjectionMatrix(&projMat, 2.f * std::tan(FOV * 0.5f), 0.2f, 1000.f, { 0, 0 }, getScreenWidth(), getScreenHeight());
	buildOrthographicMatrix(&projectionMatrix, halfHeight, aspectRatio, level.getArenaWidth() + viewDistance * 2.f, -level.getArenaWidth() - viewDistance * 2.f);
	buildViewRotationMatrix(&viewRotationMatrix, viewDirection);
	buildViewMatrix(&viewMatrix, viewRotationMatrix, viewPosition);

	viewUpDirection = viewRotationMatrix * upDirection;
	viewSunDirection = viewRotationMatrix * sunDirection;
}