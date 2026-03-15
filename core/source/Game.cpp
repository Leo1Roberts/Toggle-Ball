#include "main.h"
#include "Colors.h"
#include "Sizes.h"
#include "MatrixUtilities.h"
#include "TextureAsset.h"
#include "LoadOBJ.h"
#include "ButtonManager.h"
#include "Fonts.h"
#include "Smoother.h"
#include "Line.h"
#include "Level.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "Cursor.h"
#include "TextBoxManager.h"
#include "Game.h"

#ifndef WINDOWS_VERSION

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <sys/param.h>

#endif

void CheckGLError() {
#ifdef WINDOWS_VERSION
	GLenum err = glGetError();

	if (err == GL_NO_ERROR)
		return;

	const char* string = "unknown";
	switch (err) {
	case GL_INVALID_ENUM:
		string = "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		string = "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		string = "GL_INVALID_OPERATION";
		break;
	case GL_OUT_OF_MEMORY:
		string = "GL_OUT_OF_MEMORY";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		string = "GL_INVALID_FRAMEBUFFER_OPERATION";
		break;
	case 0x0507: //GL_CONTEXT_LOST:
		string = "GL_CONTEXT_LOST";
		break;
	case 0x8031: //GL_TABLE_TOO_LARGE1:
		string = "GL_TABLE_TOO_LARGE1";
		break;
	}

	char buf[1024];
	sprintf_s(buf, "Error %x: %s\n", err, string);
	printf(buf);
#endif
}

bool Game::inEditor = false;
bool Game::toggled = false;
bool Game::levelComplete = false;
bool Game::viewLocked = true;
bool slowMotion = false;

Smoother Game::smoother;

Level Game::level;
std::vector<Obstacle> Game::obstacles;
float Game::arenaWidth, Game::arenaHeight;
Plane Game::arenaBounds[4];
Plane Game::arenaBackground;
Ball Game::ball;
Ball Game::ballOutline;

void Game::setBallPos(const vec3& pos) {
	ball.pos = pos;
	ballOutline.pos = {-ballOutline.radius, pos.y, pos.z};
}

// Uniform locations
GLint uProjectionLoc;
GLint uUpDirectionLoc;
GLint uSunDirectionLoc;
GLint uBodyToViewRotLoc;
GLint uBodyToViewLoc;
GLint uAlphaLoc;

GLint uProjectionFullLoc;
// Matrices
mat4 worldMat, viewMat, projMat;
mat3 viewRotMat;

void Game::setupObjShaders() {
	std::string vsh, fsh;

#ifdef WINDOWS_VERSION
	vsh = importShader(ASSETS_PATH + "shaders/object.vert");
	fsh = importShader(ASSETS_PATH + "shaders/object.frag");
#else
	vsh = importShader(androidApp->activity->assetManager, "shaders/object.vert");
	fsh = importShader(androidApp->activity->assetManager, "shaders/object.frag");
#endif

	objShader = ObjShader::loadObjShader(vsh, fsh, "inPos", "inUV", "inNormal", "inColor");

	objShader->activate();

	// set the lighting uniforms
	vec3 groundColor = {0.3f, 0.3f, 0.3f};
	colorToLinear(&groundColor);
	vec3 skyColor = {85.0f / 255.0f, 110.0f / 255.0f, 128.0f / 255.0f};
	colorToLinear(&skyColor);
	vec3 sunColor = {1.0f, 1.0f, 0.9f};
	colorToLinear(&sunColor);

	glUniform3fv(objShader->getUniformLocation("uGroundColor"), 1, (float*)&groundColor);
	CHECK_ERROR();
	glUniform3fv(objShader->getUniformLocation("uSkyColor"), 1, (float*)&skyColor);
	CHECK_ERROR();
	glUniform3fv(objShader->getUniformLocation("uSunColor"), 1, (float*)&sunColor);
	CHECK_ERROR();
	uProjectionLoc = objShader->getUniformLocation("uProjection");
	CHECK_ERROR();
	uUpDirectionLoc = objShader->getUniformLocation("uUpDirection");
	CHECK_ERROR();
	uSunDirectionLoc = objShader->getUniformLocation("uSunDirection");
	CHECK_ERROR();
	uBodyToViewRotLoc = objShader->getUniformLocation("uBodyToViewRot");
	CHECK_ERROR();
	uBodyToViewLoc = objShader->getUniformLocation("uBodyToView");
	CHECK_ERROR();
	uAlphaLoc = objShader->getUniformLocation("uAlpha");

#ifdef WINDOWS_VERSION
	vsh = importShader(ASSETS_PATH + "shaders/objectOutline.vert");
	fsh = importShader(ASSETS_PATH + "shaders/objectOutline.frag");
#else
	vsh = importShader(androidApp->activity->assetManager, "shaders/objectOutline.vert");
	fsh = importShader(androidApp->activity->assetManager, "shaders/objectOutline.frag");
#endif

	outlineShader = ObjShader::loadObjShader(vsh, fsh, "inPos", "inUV", "inNormal", "inColor");

	outlineShader->activate();

	uProjectionFullLoc = outlineShader->getUniformLocation("uProjectionFull");
	CHECK_ERROR();

	glUseProgram(0);
}

void Game::reloadShaders() {
	ObjShader::deleteShaders();
	setupObjShaders();
}


// Camera positioning

vec3 viewPos;
float viewDist;
vec3 viewOrigin;
float heading, pitch;
vec3 viewDir;

void Game::rotateView(const vec2& dxy) { rotateView(dxy.x, dxy.y); }

void Game::rotateView(float dx, float dy) {
	if (viewLocked) return;

	float radPerPix = 6.0f / WINDOW_WIDTH;
	heading -= dx * radPerPix; // += for on-spot rotation
	if (heading > PI) heading -= 2.0f * PI;
	else if (heading < -PI)
		heading += 2.0f * PI;
	pitch += dy * radPerPix; // -= for on-spot rotation
	if (pitch > PI * 0.5f - 0.01f) pitch = PI * 0.5f - 0.01f;
	else if (pitch < -PI * 0.5f + 0.01f)
		pitch = -PI * 0.5f + 0.01f;
}

void Game::moveViewOrigin(float dx, float dy) {
	float metresPerPix = HALF_HEIGHT * 2.f / WINDOW_HEIGHT; // For perspective use: viewDist / WINDOW_WIDTH
	float horizontalDist = dx * metresPerPix;
	float verticalDist = dy * metresPerPix;

	float
	ch = cosf(heading),
	sh = sinf(heading),
	cp = cosf(pitch),
	sp = sinf(pitch);

	viewOrigin.x += sh * horizontalDist - ch * sp * verticalDist;
	viewOrigin.y += -ch * horizontalDist - sh * sp * verticalDist;
	viewOrigin.z += cp * verticalDist;
}

static void updateView() {
	float
	ch = cosf(heading),
	sh = sinf(heading),
	cp = cosf(pitch),
	sp = sinf(pitch);
	viewDir.x = ch * cp;
	viewDir.y = sh * cp;
	viewDir.z = sp;

	viewPos = viewOrigin + viewDir * viewDist;
}


// Frame rate tracking

long frameTime;
long then;
const int NUM_FRAME_TIMES = 15;
int frameTimes[NUM_FRAME_TIMES];
int nextFrameTime;
int frameTimesTotal;
int frameTimesFilled;

static float fps() {
	nextFrameTime %= NUM_FRAME_TIMES;
	frameTimesTotal -= frameTimes[nextFrameTime];
	frameTimes[nextFrameTime++] = frameTime;
	frameTimesTotal += frameTime;

	if (frameTimesFilled < NUM_FRAME_TIMES) frameTimesFilled++;

	return ((float)frameTimesFilled * 1000.0f / (float)frameTimesTotal);
}


// Window management

struct WindowInfo {
	int posX, posY, width, height, refreshRate;
};

float baseViewDist = 0;
const float FOV = 0.3f * PI;

#ifdef WINDOWS_VERSION
GLFWwindow* Game::window;

bool forceExit;
void Game::deleteGame() {
	glfwSetWindowShouldClose(window, GLFW_FALSE);
	if (!forceExit && !changesAreSaved()) {
		dialogue = DLG_CONFIRM_EXIT;
		return;
	} else
		confirmExit();

	delete Cursor::arrowTex;
	delete Cursor::handTex;
	delete Cursor::resizeTex;
#else
android_app* Game::androidApp;
EGLDisplay Game::eglDisplay;
EGLSurface Game::eglSurface;
EGLContext Game::eglContext;

void Game::deleteGame() {
	if (eglDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (eglContext != EGL_NO_CONTEXT) {
			eglDestroyContext(eglDisplay, eglContext);
			eglContext = EGL_NO_CONTEXT;
		}
		if (eglSurface != EGL_NO_SURFACE) {
			eglDestroySurface(eglDisplay, eglSurface);
			eglSurface = EGL_NO_SURFACE;
		}
		eglTerminate(eglDisplay);
		eglDisplay = EGL_NO_DISPLAY;
	}
#endif

	obstacles.clear();

	ObjShader::deleteShaders();

	delete whiteTex;
	delete basketballTex;

	delete ballModel;
	delete planeModel;

	Text::deleteFonts();
#ifdef WINDOWS_VERSION
	glfwTerminate();    // Destroys all remaining windows and cursors, restores modified gamma ramps, and frees resources.
	exit(EXIT_SUCCESS); // Function call: exit() is a C/C++ function that performs various tasks to help clean up resources.
#endif
}

void Game::updateHalfHeight() {
	float levelAspect = arenaWidth / arenaHeight;
	if (RATIO < levelAspect) // Game is wider than window
		HALF_HEIGHT = arenaWidth * 0.5f / RATIO;
	else
		HALF_HEIGHT = arenaHeight * 0.5f;
	HALF_HEIGHT *= viewDist / baseViewDist;

	OUTLINE_WIDTH_WORLD = OUTLINE_WIDTH * HALF_HEIGHT * 2;

	if (inEditor) {
		updateAllOutlines();
		for (Obstacle& o: obstacles)
			if (o.getStateType() == ST_POS || o.getStateType() == ST_ANG)
				o.createDomainModel();
	}

	pointerBallWide.radius = 10.0f * HALF_HEIGHT / WINDOW_HEIGHT;
}

void Game::updateRenderArea() {
#ifdef WINDOWS_VERSION
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
#else
	EGLint width;
	eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);

	EGLint height;
	eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);
#endif

	if (width != (int)WINDOW_WIDTH || height != (int)WINDOW_HEIGHT) {
		updateSizes((float)width, (float)height);
		glViewport(0, 0, width, height);
		CHECK_ERROR();

		updateHalfHeight();

		Cursor::size = 100.0f / WINDOW_HEIGHT;
	}
}


// General game logic

void Game::initGame() {
#ifndef WINDOWS_VERSION
	// Choose your render attributes
	constexpr EGLint attribs[] = {
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_BLUE_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_RED_SIZE, 8,
	EGL_DEPTH_SIZE, 24,
	EGL_NONE};

	// The default display is probably what you want on Android
	auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, nullptr, nullptr);

	// figure out how many configs there are
	EGLint numConfigs;
	eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

	// get the list of configurations
	std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
	eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

	// Find a config we like.
	// Could likely just grab the first if we don't care about anything else in the config.
	// Otherwise hook in your own heuristic
	auto config = *std::find_if(
	supportedConfigs.get(),
	supportedConfigs.get() + numConfigs,
	[&display](const EGLConfig& config) {
		EGLint red, green, blue, depth;
		if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red) && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green) && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue) && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

			return red == 8 && green == 8 && blue == 8 && depth == 24;
		}
		return false;
	});

	EGLint samples;
	eglGetConfigAttrib(display, supportedConfigs.get(), EGL_SAMPLES, &samples);
	eglGetConfigAttrib(display, supportedConfigs.get() + 1, EGL_SAMPLES, &samples);
	eglGetConfigAttrib(display, supportedConfigs.get() + 2, EGL_SAMPLES, &samples);

	// create the proper window surface
	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	EGLSurface surface = eglCreateWindowSurface(display, config, androidApp->window, nullptr);

	// Create a GLES 3 context
	EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
	EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

	// get some window metrics
	eglMakeCurrent(display, surface, surface, context);

	eglDisplay = display;
	eglSurface = surface;
	eglContext = context;
#endif
	// make width and height invalid so it gets updated the first frame in @a updateRenderArea()
	WINDOW_WIDTH = -1;
	WINDOW_HEIGHT = -1;

	// prepare the shader
	setupObjShaders();

	// get some models into memory
	createObjects();

	selectLevel("Dev test");

	glClearColor(0, 0, 0, 1);
	CHECK_ERROR();
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_ERROR();

	std::string vsh, fsh;

#ifdef WINDOWS_VERSION
	vsh = importShader(ASSETS_PATH + "shaders/text.vert");
	fsh = importShader(ASSETS_PATH + "shaders/text.frag");
#else
	vsh = importShader(androidApp->activity->assetManager, "shaders/text.vert");
	fsh = importShader(androidApp->activity->assetManager, "shaders/text.frag");
#endif
	Text::initTextShader(vsh, fsh, "inPos", "inUV", "inCol", "uProjection2D");

#ifdef WINDOWS_VERSION
	vsh = importShader(ASSETS_PATH + "shaders/button.vert");
	fsh = importShader(ASSETS_PATH + "shaders/button.frag");
#else
	vsh = importShader(androidApp->activity->assetManager, "shaders/button.vert");
	fsh = importShader(androidApp->activity->assetManager, "shaders/button.frag");
#endif
	ButtonManager::initButtonShader(vsh, fsh, "inPos", "inUV", "inCol", "inOutlineCol", "inOutlineRad", "uProjection2D");

#ifdef WINDOWS_VERSION
	vsh = importShader(ASSETS_PATH + "shaders/cursor.vert");
	fsh = importShader(ASSETS_PATH + "shaders/cursor.frag");
	Cursor::initShader(vsh, fsh, "inPos", "inUV", "uProjection2D");
#endif

	initFonts();

	Line::initLineShader();

	then = now_ms();
}

void Game::initFonts() {
#ifdef WINDOWS_VERSION
	byte BAHNSCHRIFT_FACE = Text::loadFace(ASSETS_PATH + "fonts/", "Bahnschrift");
	byte COURIER_NEW_FACE = Text::loadFace(ASSETS_PATH + "fonts/", "Courier New");
#else
	byte BAHNSCHRIFT_FACE = Text::loadFace(androidApp->activity->assetManager, "fonts/", "Bahnschrift");
	byte COURIER_NEW_FACE = Text::loadFace(androidApp->activity->assetManager, "fonts/", "Courier New");
#endif
	BAHNSCHRIFT = {
	BAHNSCHRIFT_FACE,
	false,
	0.008f,
	0.02f};
	COURIER_NEW = {
	COURIER_NEW_FACE,
	true,
	-0.004f,
	0.06f};
}

void Game::resetBall() {
	setBallPos(level.ballPos * ball.radius);
	ball.vel = {0};
	ball.rot.identity();
	ball.angVel = {0};
}

void Game::restartLevel(int _) {
	if (inEditor) return;

	resetBall();

	for (Obstacle& g: obstacles)
		g.restart();

	toggled = false;
	levelComplete = false;
	smoother.Reset();
}


// Dialogues

int Game::dialogue;
void Game::requestDialogue(int d) {
	switch (d) {
	case DLG_NONE:
		dialogue = DLG_NONE;
		break;
	case DLG_CHOOSE_LEVEL:
		dialogue = DLG_CHOOSE_LEVEL;
		break;
	case DLG_CONFIRM_SWITCH:
		if (changesAreSaved())
			selectPendingLevel();
		else
			dialogue = DLG_CONFIRM_SWITCH;
		break;
	case DLG_SAVE_AS:
		if (dialogue != DLG_SAVE_AS) {
			dialogue = DLG_SAVE_AS;
			justOpenedSaveAsDialogue = true;
			saveAsName = level.name;
		}
		break;
	case DLG_CONFIRM_OVERWRITE:
		if (level.name != saveAsName) {
			bool alreadyExists = false;
			for (const std::string& lvlName: getLevelList())
				if (lvlName == saveAsName) {
					dialogue = DLG_CONFIRM_OVERWRITE;
					alreadyExists = true;
					break;
				}
			if (alreadyExists)
				break;
		}
		saveAs();
		break;
	}
}

#ifdef WINDOWS_VERSION
void Game::confirmExit(int _) {
	forceExit = true;
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}
#endif

bool Game::justOpenedSaveAsDialogue = false;
std::string Game::saveAsName;
void Game::saveAs(int _) {
	if (saveAsName == level.name) {
		saveLevel();
	} else {
		level.name = saveAsName;
		saveLevel();
		selectLevel(level.name);
	}

	requestDialogue(DLG_NONE);
}


void Game::createObjects() {
	std::vector<Vertex> ballVertices;
	std::vector<Index> ballIndices;
	std::vector<Vertex> planeVertices;
	std::vector<Index> planeIndices;

#ifdef WINDOWS_VERSION
	whiteTex = TextureAsset::loadAsset(ASSETS_PATH + "textures/white.png", false);
	basketballTex = TextureAsset::loadAsset(ASSETS_PATH + "textures/Ball.png", false);
	loadOBJ(ASSETS_PATH + "models/Ball.obj", &ballVertices, &ballIndices);
	loadOBJ(ASSETS_PATH + "models/Ground.obj", &planeVertices, &planeIndices, BOUNDARY);
#else
	whiteTex = TextureAsset::loadAsset(androidApp->activity->assetManager, "textures/white.png", false);
	basketballTex = TextureAsset::loadAsset(androidApp->activity->assetManager, "textures/Ball.png", false);
	loadOBJ(androidApp->activity->assetManager, "models/Ball.obj", &ballVertices, &ballIndices);
	loadOBJ(androidApp->activity->assetManager, "models/Ground.obj", &planeVertices, &planeIndices, BOUNDARY);
#endif
	objShader->activate();

	planeModel = new Model(planeVertices, planeIndices);
	objShader->setupVertexAttribs();

	ballModel = new Model(ballVertices, ballIndices);
	objShader->setupVertexAttribs();

	arenaBounds[0] = Plane(whiteTex, BOUNDARY, MAT_CONCRETE, {0, 0, 1});
	arenaBounds[1] = Plane(whiteTex, BOUNDARY, MAT_CONCRETE, {0, 0, -1});
	arenaBounds[2] = Plane(whiteTex, BOUNDARY, MAT_CONCRETE, {0, 1, 0});
	arenaBounds[3] = Plane(whiteTex, BOUNDARY, MAT_CONCRETE, {0, -1, 0});

	arenaBackground = Plane(whiteTex, BOUNDARY, MAT_CONCRETE, {1, 0, 0});

	ballOutline = Ball();

	glUseProgram(0);
}

void Game::update() {
	long now = now_ms();
	frameTime = now - then;
	if (slowMotion) frameTime = 1;
	else
		frameTime = now - then;
	then = now;

	if (inEditor)
		moveEditorObstacles();
	else
		physicsUpdate();

	render();
}

void Game::toggle(int _) {
	toggled = !toggled;
	if (ctrlDown && inEditor)
		smoother.SetPosition(toggled ? 1.0f : 0.0f, 0.0f);
	else
		smoother.SetDestination(toggled ? 1.0f : 0.0f, 0.0f, inEditor && shiftDown ? 0 : level.transitionTime);
}

void Game::resetEditorView(int _) {
	viewDist = baseViewDist;
	viewOrigin = {0, 0, arenaHeight * 0.5f};
	updateHalfHeight();
}


// Physics

const float PHYSICS_TIMESTEP = 0.001f;

const float FORCE_LINE_MULTIPLIER = 0.02f;

static float ballSpringForce(float compression, float radius, float springK) {
	const float power = 4;            // Quartic curve...
	const float forceMultiplier = 10; // ...which ends up with a 10x higher force at c = r
	return compression * springK + powf(compression, power) * (forceMultiplier - 1) * springK * powf(radius, 1 - power);
}

void Game::ballObstacleCollision(Ball* pBall, Obstacle* g, const vec3& normal, float separation) {
	float springFactor = ballSpringForce(pBall->radius - separation, pBall->radius, pBall->springK); //(pBall->radius - separation) * pBall->springK;

	vec3 ballToPoint = normal * -separation;
	vec3 obToPoint = pBall->pos - g->getPos() + ballToPoint;
	vec3 ballPointVel = pBall->vel + cross(pBall->angVel, ballToPoint);
	vec3 obPointVel = g->getVel() + cross(g->getAngVel(), obToPoint);
	vec3 pointVelDiff = ballPointVel - obPointVel;
	float normalVelDiffFactor = dot(normal, pointVelDiff);

	float dampingFactor = pBall->dampingK * normalVelDiffFactor;

	float totalForce = springFactor - dampingFactor;
	vec3 force = normal * totalForce;
	pBall->force += force;
	//Line::addLine(pBall->pos + ballToPoint, pBall->pos + ballToPoint + force * FORCE_LINE_MULTIPLIER, YELLOW);

	vec3 parallelPointVel = pointVelDiff - normal * normalVelDiffFactor;
	float parallelPointVelLengthSq = parallelPointVel.lengthSq();
	if (parallelPointVelLengthSq > 0.00000001f) {
		vec3 friction = parallelPointVel / -sqrt(parallelPointVelLengthSq) * FRICTION_COEFFICIENTS[pBall->material][g->getMaterial()] * totalForce;
		vec3 maxFriction = parallelPointVel * -pBall->mass * pBall->moi / (PHYSICS_TIMESTEP * (pBall->moi + pBall->mass * separation * separation));
		if (friction.lengthSq() > maxFriction.lengthSq()) friction = maxFriction;
		//Line::addLine(pBall->pos + ballToPoint, pBall->pos + ballToPoint + friction * FORCE_LINE_MULTIPLIER, CYAN);
		pBall->force += friction;
		pBall->torque += cross(ballToPoint, friction);
	}
	if (g->getIsStraight() && parallelPointVelLengthSq <= 0.00000001f || parallelPointVelLengthSq <= 0.0001f) {
		vec3 ballVelDiff = pBall->vel - obPointVel;
		vec3 parallelBallVel = ballVelDiff - normal * dot(normal, ballVelDiff);
		float parallelBallVelLengthSq = parallelBallVel.lengthSq();
		if (parallelBallVelLengthSq > 0.00000001f) {
			vec3 rollingResistance = parallelBallVel / -sqrt(parallelBallVelLengthSq) * force.length() * ROLLING_RESISTANCE_COEFFICIENTS[pBall->material][g->getMaterial()];
			vec3 maxRollingResistance = parallelBallVel * -pBall->mass / PHYSICS_TIMESTEP;
			if (rollingResistance.lengthSq() > maxRollingResistance.lengthSq()) rollingResistance = maxRollingResistance;
			//Line::addLine(pBall->pos + ballToPoint, pBall->pos + ballToPoint + rollingResistance * FORCE_LINE_MULTIPLIER, BLUE);
			pBall->force += rollingResistance;
		}
	}
}

const float SUCCEED_TIME = 0.5f;

short Game::checkBallObstacleCollision(Ball* pBall, bool getIndex, bool onlyRim, bool onlySection) {
	short obIndex = -1;
	float obMinorRadius = 0;

	for (short i = 0; i < (short)obstacles.size(); i++) {
		Obstacle* g = &obstacles[i];

		vec3 outerDiff = pBall->pos - g->getPos();
		float outerSepSq = outerDiff.lengthSq();
		float outerRadii = pBall->radius + g->getMajorRadius() + g->getMinorRadius();
		if (outerSepSq < outerRadii * outerRadii || onlySection) { // Within bounding circle
			PlaneDefinition ps = g->getDividingPlane(true);
			PlaneDefinition pe = g->getDividingPlane(false);
			float psp = dot(ps.normal, pBall->pos) - ps.dotProduct;
			float pep = dot(pe.normal, pBall->pos) - pe.dotProduct;
			if (psp >= 0 && pep >= 0 && g->getHalfArcAngle() * 2 <= PI || (!g->getIsStraight() && g->getHalfArcAngle() * 2 > PI && (psp >= 0 || pep >= 0))) { // Between caps
				if (onlySection && i == focus) return 0;
				if (g->getIsStraight()) {
					PlaneDefinition plane = g->getTopPlane();
					float separation = dot(plane.normal, pBall->pos) - plane.dotProduct;
					if (separation < -g->getMinorRadius()) {
						plane.normal.y = -plane.normal.y;
						plane.normal.z = -plane.normal.z;
						plane.dotProduct = g->getMinorRadius() * 2 - plane.dotProduct;
						separation = dot(plane.normal, pBall->pos) - plane.dotProduct;
					}

					if (separation < 0 && !getIndex) continue; // Ball is inside obstacle

					if (separation < pBall->radius) {
						if (getIndex) {
							if ((!onlyRim || onlyRim && fabsf(separation) < pBall->radius) && g->getMinorRadius() >= obMinorRadius) {
								obMinorRadius = g->getMinorRadius();
								obIndex = i;
							}
							continue;
						}

						ballObstacleCollision(pBall, g, plane.normal, separation);

						if (g->getIsGoal()) {
							g->setExtra(g->getExtra() + PHYSICS_TIMESTEP);
							if (g->getExtra() >= SUCCEED_TIME && pBall->vel.lengthSq() < 0.000001f) {
								g->setExtra(0);
								levelComplete = true;
							}
						}
					} else if (g->getIsGoal())
						g->setExtra(0);
				} else {
					vec3 diff = pBall->pos - g->getPos();
					float sepSq = diff.lengthSq();
					float innerRadii = g->getMajorRadius() - g->getMinorRadius() - pBall->radius;
					if (sepSq < outerRadii * outerRadii && sepSq > innerRadii * innerRadii) { // In contact with banana
						float outerRadius = g->getMajorRadius() + g->getMinorRadius();
						float innerRadius = g->getMajorRadius() - g->getMinorRadius();
						float sep = sqrtf(sepSq);
						float separation;
						vec3 diffUnit;
						if (sepSq > outerRadius * outerRadius) { // Beyond banana
							separation = sep - g->getMajorRadius() - g->getMinorRadius();
							//if (separation < 0.001f) continue;
							diffUnit = diff / sep;
						} else if (sepSq < innerRadius * innerRadius) { // Within banana
							separation = g->getMajorRadius() - sep - g->getMinorRadius();
							//if (separation < 0.001f) continue;
							diffUnit = diff / -sep;
						} else if (!getIndex || (onlyRim && (sep - g->getMajorRadius() - g->getMinorRadius() < -pBall->radius && g->getMajorRadius() - sep - g->getMinorRadius() < -pBall->radius)))
							continue; // Ball is inside obstacle

						if (getIndex) {
							if (g->getMinorRadius() >= obMinorRadius) {
								obMinorRadius = g->getMinorRadius();
								obIndex = i;
							}
							continue;
						}
						ballObstacleCollision(pBall, g, diffUnit, separation);

						if (g->getIsGoal()) {
							g->setExtra(g->getExtra() + PHYSICS_TIMESTEP);
							if (g->getExtra() >= SUCCEED_TIME && pBall->vel.lengthSq() < 0.000001f) {
								g->setExtra(0);
								levelComplete = true;
							}
						}
					} else if (g->getIsGoal())
						g->setExtra(0);
				}
			} else {
				bool inStartSection = false;
				bool inEndSection = false;
				float sectionDistSq = 0;

				if (psp < 0) {
					vec3 absStart = g->getPos() + g->getRot() * g->getStart();
					vec3 diff = pBall->pos - absStart;
					float sepSq = diff.lengthSq();
					if (onlySection && i == focus) {
						inStartSection = true;
						sectionDistSq = sepSq;
					} else {
						float radii = pBall->radius + g->getMinorRadius();
						if (sepSq < radii * radii && sepSq > 0.000001f) {
							float separation = sqrtf(sepSq);
							vec3 diffUnit = diff / separation;
							if (getIndex) {
								if (!onlyRim && g->getMinorRadius() >= obMinorRadius) {
									obMinorRadius = g->getMinorRadius();
									obIndex = i;
									continue;
								} else if (onlyRim && separation > g->getMinorRadius() - pBall->radius)
									return (short)(i + obstacles.size());
							} else {
								ballObstacleCollision(pBall, g, diffUnit, separation - g->getMinorRadius());

								if (g->getIsGoal()) {
									g->setExtra(g->getExtra() + PHYSICS_TIMESTEP);
									if (g->getExtra() >= SUCCEED_TIME && pBall->vel.lengthSq() < 0.000001f) {
										g->setExtra(0);
										levelComplete = true;
									}
								}
							}
						}
					}
				}
				if (pep < 0) {
					vec3 absEnd = g->getPos() + g->getRot() * g->getEnd();
					vec3 diff = pBall->pos - absEnd;
					float sepSq = diff.lengthSq();
					if (onlySection && i == focus) {
						inEndSection = true;
						if (sepSq < sectionDistSq) sectionDistSq = 2;
						else
							sectionDistSq = 1;
					} else {
						float radii = pBall->radius + g->getMinorRadius();
						if (sepSq < radii * radii && sepSq > 0.000001f) {
							float separation = sqrtf(sepSq);
							vec3 diffUnit = diff / separation;
							if (getIndex) {
								if (!onlyRim && g->getMinorRadius() >= obMinorRadius) {
									obMinorRadius = g->getMinorRadius();
									obIndex = i;
									continue;
								} else if (onlyRim && separation > g->getMinorRadius() - pBall->radius)
									return (short)(i + obstacles.size() * 2);
							} else {
								ballObstacleCollision(pBall, g, diffUnit, separation - g->getMinorRadius());

								if (g->getIsGoal()) {
									g->setExtra(g->getExtra() + PHYSICS_TIMESTEP);
									if (g->getExtra() >= SUCCEED_TIME && pBall->vel.lengthSq() < 0.000001f) {
										g->setExtra(0);
										levelComplete = true;
									}
								}
							}
						}
					}
				}
				if (onlySection && i == focus) {
					if (inStartSection && inEndSection) {
						return (short)sectionDistSq;
					} else if (inStartSection)
						return 1;
					else if (inEndSection)
						return 2;
				}
			}
		}
	}
	return obIndex;
}

void Game::physicsUpdate() {
	for (int i = 0; i < (int)round((float)frameTime * 0.001f / PHYSICS_TIMESTEP); i++) {
		for (Obstacle& g: obstacles) {
			switch (g.getStateType()) {
			case ST_POS:
				g.setPos(lerp(g.getStateA(), g.getStateB(), smoother.X));
				g.setVel((g.getStateB() - g.getStateA()) * smoother.V);
				break;
			case ST_POS_OSC: {
				g.setPos(g.getStateA() + (g.getStateB() - g.getStateA()) * (0.5f - 0.5f * cosf(lerp(g.getStateA().x, g.getStateB().x, smoother.X) * PHYSICS_TIMESTEP + g.getExtra())));
				g.setExtra(wrapAngle(g.getExtra() + lerp(g.getStateA().x, g.getStateB().x, smoother.X) * PHYSICS_TIMESTEP));
				vec3 vel = (g.getStateB() - g.getStateA()) * sinf(g.getExtra()) * lerp(g.getStateA().x, g.getStateB().x, smoother.X) * 0.5f;
				g.setVel(vel);
			} break;
			case ST_ANG:
				g.setAngle(lerp(g.getStateA().x, g.getStateB().x, smoother.X));
				g.setAngVel((g.getStateB().x - g.getStateA().x) * smoother.V);
				break;
			case ST_ANG_VEL:
				g.setAngVel(lerp(g.getStateA().x, g.getStateB().x, smoother.X));
				g.setAngle(wrapAngle(g.getAngle() + g.getAngVel().x * PHYSICS_TIMESTEP));
				break;
			case ST_ANG_OSC: {
				g.setAngle(lerp(g.getStateA().y, g.getStateB().y, 0.5f - 0.5f * cosf(lerp(g.getStateA().x, g.getStateB().x, smoother.X) * PHYSICS_TIMESTEP + g.getExtra())));
				g.setExtra(wrapAngle(g.getExtra() + lerp(g.getStateA().x, g.getStateB().x, smoother.X) * PHYSICS_TIMESTEP));
				float angVel = (g.getStateB().y - g.getStateA().y) * sinf(g.getExtra()) * lerp(g.getStateA().x, g.getStateB().x, smoother.X) * 0.5f;
				g.setAngVel(angVel);
			} break;
			}
		}

		smoother.Update(PHYSICS_TIMESTEP);

		ball.force.set(0, 0, GRAVITY * ball.mass); // Reset forces
		ball.torque.set(0);                        // Reset torques

		float ballVelLength = ball.vel.length();
		ball.force -= ball.vel * ballVelLength * 0.5f * ball.Cd * AIR_DENSITY * PI * ball.radius * ball.radius;
		//ball->torque -= ball->angVel * 8.0f * PI * ball->radius * ball->radius * ball->radius * DYNAMIC_VISCOSITY; // Viscous drag torque

		/*for (int k = j + 1; k < freeObjects.size(); k++) {
			if (freeObjects[k]->type == OT_BALL) {
				Ball* other = (Ball*)freeObjects[k];
				vec3 diff = ball->pos - other->pos;
				float separationSq = diff.lengthSq();
				float radii = ball->radius + other->radius;
				if (separationSq < radii * radii && separationSq > 0.000001f) {
					float separation = sqrtf(separationSq);
					vec3 diffUnit = diff / separation;
					float combinedK = 1.0f / (ball->springKInv + other->springKInv);
					float springFactor = ball->radius + other->radius - separation;
					float collisionVel = dot(diffUnit, ball->vel - other->vel);
					float dampingFactor = (ball->dampingK * ball->springKInv + other->dampingK * other->springKInv) * collisionVel;
					float totalForce = combinedK * (springFactor - dampingFactor);
					vec3 force = diffUnit * totalForce;
					ball->force += force;
					other->force -= force;

					vec3 ballToPoint = diffUnit * -(ball->radius - (combinedK * ball->springKInv) * springFactor);
					vec3 otherToPoint = diff + ballToPoint;
					vec3 ballPointVel = ball->vel + cross(ball->angVel, ballToPoint);
					vec3 otherPointVel = other->vel + cross(other->angVel, otherToPoint);
					ballPointVel -= diffUnit * dot(diffUnit, ballPointVel);
					otherPointVel -= diffUnit * dot(diffUnit, otherPointVel);
					vec3 velDiff = ballPointVel - otherPointVel;
					float velDiffLengthSq = velDiff.lengthSq();
					if (velDiffLengthSq > 0.00000001f) {
						vec3 friction = velDiff / -sqrt(velDiffLengthSq) * FRICTION_COEFFICIENTS[ball->material][other->material] * totalForce;
						//vec3 maxFriction = velDiff * (-ball->mass * ball->moi / (PHYSICS_TIMESTEP * (ball->moi + ball->mass * separation * separation)) - other->mass * other->moi / (PHYSICS_TIMESTEP * (other->moi + other->mass * separation * separation)) * 0.5f);
						//if (friction.lengthSq() > maxFriction.lengthSq()) friction = maxFriction;
						//Line::addLine(ball->pos, ball->pos + friction * FORCE_LINE_MULTIPLIER, CYAN);
						ball->force += friction;
						other->force -= friction;
						ball->torque += cross(ballToPoint, friction);
						other->torque -= cross(otherToPoint, friction);
					}
				}
			}
		}*/

		for (Plane& p: arenaBounds) {
			PlaneDefinition& d = p.definition;
			float separation = dot(d.normal, ball.pos) - d.dotProduct;
			if (separation < ball.radius) {
				float springFactor = ballSpringForce(ball.radius - separation, ball.radius, ball.springK);
				float dampingFactor = ball.dampingK * dot(d.normal, ball.vel);
				float totalForce = springFactor - dampingFactor;
				vec3 force = d.normal * totalForce;
				ball.force += force;

				vec3 centreToPoint = d.normal * -separation;
				vec3 pointVel = ball.vel + cross(ball.angVel, centreToPoint);
				pointVel -= d.normal * dot(d.normal, pointVel);
				float pointVelLengthSq = pointVel.lengthSq();
				if (pointVelLengthSq > 0.00000001f) {
					vec3 friction = pointVel / -sqrt(pointVelLengthSq) * FRICTION_COEFFICIENTS[ball.material][p.material] * totalForce;
					vec3 maxFriction = pointVel * -ball.mass * ball.moi / (PHYSICS_TIMESTEP * (ball.moi + ball.mass * separation * separation));
					if (friction.lengthSq() > maxFriction.lengthSq()) friction = maxFriction;
					//Line::addLine(ball->pos + centreToPoint, ball->pos + centreToPoint + friction * FORCE_LINE_MULTIPLIER, CYAN);
					ball.force += friction;
					ball.torque += cross(centreToPoint, friction);
				} else {
					// No spin in 2D
					/*vec3 spin = d.normal * dot(d.normal, ball->angVel);
					spin.unit();
					ball->torque -= spin * FRICTION_COEFFICIENTS[ball->material][p.material] * totalForce * cosf(asinf(separation / ball->radius)) * ball->radius * 0.5f; // Spin friction*/
					if (ballVelLength > 0.0001f) {
						vec3 rollingResistance = ball.vel / -ballVelLength * force.length() * ROLLING_RESISTANCE_COEFFICIENTS[ball.material][p.material];
						vec3 maxRollingResistance = ball.vel * -ball.mass / PHYSICS_TIMESTEP;
						if (rollingResistance.lengthSq() > maxRollingResistance.lengthSq()) rollingResistance = maxRollingResistance;
						//Line::addLine(ball->pos + centreToPoint, ball->pos + centreToPoint + rollingResistance * FORCE_LINE_MULTIPLIER, BLUE);
						ball.force += rollingResistance;
					}
				}
			}
		}

		checkBallObstacleCollision(&ball, false);

		if (ball.force.lengthSq() > 0.00000001f) {
			vec3 acc = ball.force / ball.mass;
			ball.vel += acc * PHYSICS_TIMESTEP;
			//if (ball->vel.lengthSq() < 0.000001f)
			//	ball->vel.set(0, 0, 0);
		}

		if (ball.torque.lengthSq() > 0.00000001f) {
			vec3 angAcc = ball.torque / ball.moi;
			ball.angVel += angAcc * PHYSICS_TIMESTEP;
		}

		ball.pos += ball.vel * PHYSICS_TIMESTEP;
		float t = ball.angVel.length();
		if (t > 0) {
			mat3 deltaRot;
			ball.angVel /= t;
			deltaRot.R_VecAndAngle(ball.angVel, t * PHYSICS_TIMESTEP);
			ball.angVel *= t;
			ball.rot = deltaRot * ball.rot;
		}
	}
}

void Game::moveEditorObstacles() {
	const float step = (float)frameTime * 0.001f;

	for (short i = 0; i < (short)obstacles.size(); i++) {
		Obstacle& g = obstacles[i];

		switch (g.getStateType()) {
		case ST_POS:
			g.setPos(lerp(g.getStateA(), g.getStateB(), smoother.X));
			break;
		case ST_POS_OSC:
			if (selection[i] && TextBoxManager::getCurrentProperty() == PROPERTY_OPM) {
				g.setPos(lerp(g.getStateA(), g.getStateB(), 0.5f - 0.5f * cosf(lerp(g.getStateA().x, g.getStateB().x, smoother.X) * step + g.getExtra())));
				g.setExtra(wrapAngle(g.getExtra() + lerp(g.getStateA().x, g.getStateB().x, smoother.X) * step));
			} else {
				g.setPos(smoother.X < 0.5f ? g.getStateA() : g.getStateB());
				g.setExtra(toggled ? PI : 0);
			}
			break;
		case ST_ANG:
			g.setAngle(lerp(g.getStateA().x, g.getStateB().x, smoother.X));
			break;
		case ST_ANG_VEL:
			if (selection[i] && TextBoxManager::getCurrentProperty() == PROPERTY_RPM) {
				g.setAngVel(lerp(g.getStateA().x, g.getStateB().x, smoother.X));
				g.setAngle(wrapAngle(g.getAngle() + g.getAngVel().x * step));
			} else
				g.setAngle(g.getInitAngle());
			break;
		case ST_ANG_OSC:
			if (selection[i] && TextBoxManager::getCurrentProperty() == PROPERTY_OPM) {
				g.setAngle(g.getStateA().y + (g.getStateB().y - g.getStateA().y) * (0.5f - 0.5f * cosf(lerp(g.getStateA().x, g.getStateB().x, smoother.X) * step + g.getExtra())));
				g.setExtra(wrapAngle(g.getExtra() + lerp(g.getStateA().x, g.getStateB().x, smoother.X) * step));
			} else {
				g.setAngle(smoother.X < 0.5f ? g.getStateA().y : g.getStateB().y);
				g.setExtra(toggled ? PI : 0);
			}
		}
	}

	smoother.Update(step);
}


// Graphics

void Game::drawInventoryPanel() {
	ButtonManager::addButton(LEFT, LEFT + 0.5f, 1, -1,
	                         nullptr, 0, BS_PANEL, "", false, false, false);

	ButtonManager::addToggleButton(LEFT + 0.3f, 1 - 0.05f, smoother.X, toggle);

	Text::addText(LEFT + 0.05f, 0.8f, "Straight obstacle", BAHNSCHRIFT, 0.1f * 0.6f, TEXT_INACTIVE);

	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.7f, 0.62f,
	                         addObstacleCallback,
	                         0x100 + ST_STATIC,
	                         BS_BASIC,
	                         "Static");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.6f, 0.52f,
	                         addObstacleCallback,
	                         0x100 + ST_POS,
	                         BS_BASIC,
	                         "Position");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.5f, 0.42f,
	                         addObstacleCallback,
	                         0x100 + ST_POS_OSC,
	                         BS_BASIC,
	                         "Oscillator");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.4f, 0.32f,
	                         addObstacleCallback,
	                         0x100 + ST_ANG,
	                         BS_BASIC,
	                         "Angle");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.3f, 0.22f,
	                         addObstacleCallback,
	                         0x100 + ST_ANG_VEL,
	                         BS_BASIC,
	                         "Spinner");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.2f, 0.12f,
	                         addObstacleCallback,
	                         0x100 + ST_ANG_OSC,
	                         BS_BASIC,
	                         "Angle osc");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, 0.1f, 0.02f,
	                         addObstacleCallback,
	                         0x300 + ST_STATIC,
	                         BS_BASIC,
	                         "GOAL");

	Text::addText(LEFT + 0.05f, 0, "Curved obstacle", BAHNSCHRIFT, 0.1f * 0.6f, TEXT_INACTIVE);

	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.1f, -0.18f,
	                         addObstacleCallback,
	                         ST_STATIC,
	                         BS_BASIC,
	                         "Static");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.2f, -0.28f,
	                         addObstacleCallback,
	                         ST_POS,
	                         BS_BASIC,
	                         "Position");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.3f, -0.38f,
	                         addObstacleCallback,
	                         ST_POS_OSC,
	                         BS_BASIC,
	                         "Oscillator");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.4f, -0.48f,
	                         addObstacleCallback,
	                         ST_ANG,
	                         BS_BASIC,
	                         "Angle");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.5f, -0.58f,
	                         addObstacleCallback,
	                         ST_ANG_VEL,
	                         BS_BASIC,
	                         "Spinner");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.6f, -0.68f,
	                         addObstacleCallback,
	                         ST_ANG_OSC,
	                         BS_BASIC,
	                         "Angle osc");
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.25f, -0.7f, -0.78f,
	                         addObstacleCallback,
	                         0x200 + ST_STATIC,
	                         BS_BASIC,
	                         "GOAL");

	std::string resetViewText = "Reset view";
	ButtonManager::addButton(LEFT + 0.05f, LEFT + 0.05f + Text::calculateWidth(resetViewText, BAHNSCHRIFT, 0.1f * 0.6f) + 0.07f, -0.85f, -0.95f,
	                         resetEditorView,
	                         0,
	                         BS_GREY,
	                         resetViewText);
}

void Game::drawPropertiesPanel() {
	ButtonManager::addButton(RIGHT - 0.7f, RIGHT, 1, -1,
	                         nullptr, 0, BS_PANEL, "", false, false, false);

	std::ostringstream ss;
	ss << std::fixed << std::setprecision(2);

	if (focus < 0) {
		ss << level.arenaWidth;
		Text::addText(RIGHT - 0.65f, 0.95f - 0.08f * 0.2f, "Arena width :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.95f, 0.87f,
		                           ss.str(), TR_FLOAT, PROPERTY_ARENA_WIDTH, changePropertyInput, updatePropertyInput);

		ss.str("");
		ss.clear();
		ss << level.arenaHeight;
		Text::addText(RIGHT - 0.65f, 0.85f - 0.08f * 0.2f, "Arena height :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.85f, 0.77f,
		                           ss.str(), TR_FLOAT, PROPERTY_ARENA_HEIGHT, changePropertyInput, updatePropertyInput);

		ss.str("");
		ss.clear();
		ss << level.transitionTime;
		Text::addText(RIGHT - 0.65f, 0.75f - 0.08f * 0.2f, "Transition time :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.75f, 0.67f,
		                           ss.str(), TR_FLOAT, PROPERTY_TRANSITION_TIME, changePropertyInput, updatePropertyInput);
	} else if (focus == MAX_OBSTACLES) {
		ss << level.ballPos.y;
		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i] && obstacles[i].getPos().y != level.ballPos.y) {
				ss.str("");
				break;
			}
		}
		Text::addText(RIGHT - 0.65f, 0.95f - 0.08f * 0.2f, "X :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.95f, 0.87f,
		                           ss.str(), TR_FLOAT, PROPERTY_POS_X, changePropertyInput, updatePropertyInput);

		ss.str("");
		ss.clear();
		ss << level.ballPos.z;
		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i] && obstacles[i].getPos().z != level.ballPos.z) {
				ss.str("");
				break;
			}
		}
		Text::addText(RIGHT - 0.65f, 0.85f - 0.08f * 0.2f, "Y :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.85f, 0.77f,
		                           ss.str(), TR_FLOAT, PROPERTY_POS_Y, changePropertyInput, updatePropertyInput);
	} else {
		ss << obstacles[focus].getPos().y;
		if (selection[MAX_OBSTACLES] && level.ballPos.y != obstacles[focus].getPos().y)
			ss.str("");
		else {
			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && obstacles[i].getPos().y != obstacles[focus].getPos().y) {
					ss.str("");
					break;
				}
			}
		}
		Text::addText(RIGHT - 0.65f, 0.95f - 0.08f * 0.2f, "X :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.95f, 0.87f,
		                           ss.str(), TR_FLOAT, PROPERTY_POS_X, changePropertyInput, updatePropertyInput, obstacles[focus].getStateType() == ST_POS);
		if (obstacles[focus].getStateType() == ST_POS_OSC)
			ButtonManager::addButton(RIGHT - 0.06f - SEMI_STATEFUL_BLOB_RADIUS * 2, RIGHT - 0.06f, 0.94f, 0.94f - SEMI_STATEFUL_BLOB_RADIUS * 2,
			                         nullptr, 0, BS_SEMI_STATEFUL_BLOB, "", false, true, false);

		ss.str("");
		ss.clear();
		ss << obstacles[focus].getPos().z;
		if (selection[MAX_OBSTACLES] && level.ballPos.z != obstacles[focus].getPos().z)
			ss.str("");
		else {
			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && obstacles[i].getPos().z != obstacles[focus].getPos().z) {
					ss.str("");
					break;
				}
			}
		}
		Text::addText(RIGHT - 0.65f, 0.85f - 0.08f * 0.2f, "Y :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.85f, 0.77f,
		                           ss.str(), TR_FLOAT, PROPERTY_POS_Y, changePropertyInput, updatePropertyInput, obstacles[focus].getStateType() == ST_POS);
		if (obstacles[focus].getStateType() == ST_POS_OSC)
			ButtonManager::addButton(RIGHT - 0.06f - SEMI_STATEFUL_BLOB_RADIUS * 2, RIGHT - 0.06f, 0.84f, 0.84f - SEMI_STATEFUL_BLOB_RADIUS * 2,
			                         nullptr, 0, BS_SEMI_STATEFUL_BLOB, "", false, true, false);

		const float angle = obstacles[focus].getAngle();
		float displayAngle = angleToDisplay(angle);
		if (obstacles[focus].getStateType() != ST_ANG && obstacles[focus].getStateType() != ST_ANG_OSC)
			displayAngle = wrapDisplayAngle(displayAngle);
		ss.str("");
		ss.clear();
		ss << displayAngle;
		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				float otherAngle = obstacles[i].getAngle();
				float otherDisplayAngle = angleToDisplay(otherAngle);
				if (obstacles[i].getStateType() != ST_ANG && obstacles[focus].getStateType() != ST_ANG_OSC)
					otherDisplayAngle = wrapDisplayAngle(otherDisplayAngle);
				if (otherDisplayAngle != displayAngle) {
					ss.str("");
					break;
				}
			}
		}
		Text::addText(RIGHT - 0.65f, 0.75f - 0.08f * 0.2f, "Angle :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.75f, 0.67f,
		                           ss.str(), TR_FLOAT, PROPERTY_ANGLE, changePropertyInput, updatePropertyInput, obstacles[focus].getStateType() == ST_ANG);
		if (obstacles[focus].getStateType() == ST_ANG_OSC)
			ButtonManager::addButton(RIGHT - 0.06f - SEMI_STATEFUL_BLOB_RADIUS * 2, RIGHT - 0.06f, 0.74f, 0.74f - SEMI_STATEFUL_BLOB_RADIUS * 2,
			                         nullptr, 0, BS_SEMI_STATEFUL_BLOB, "", false, true, false);

		ss.str("");
		ss.clear();
		ss << obstacles[focus].getMinorRadius();
		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i] && obstacles[i].getMinorRadius() != obstacles[focus].getMinorRadius()) {
				ss.str("");
				break;
			}
		}
		Text::addText(RIGHT - 0.65f, 0.65f - 0.08f * 0.2f, "Minor radius :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
		TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.65f, 0.57f,
		                           ss.str(), TR_FLOAT, PROPERTY_MINOR_RADIUS, changePropertyInput, updatePropertyInput);

		switch (obstacles[focus].getStateType()) {
		case ST_POS_OSC:
		case ST_ANG_OSC: {
			const float angFreq = lerp(obstacles[focus].getStateA().x, obstacles[focus].getStateB().x, smoother.X);
			const float opm = angFreq * 30 / PI;
			ss.str("");
			ss.clear();
			ss << opm;
			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && (obstacles[i].getStateType() == ST_POS_OSC || obstacles[i].getStateType() == ST_ANG_OSC) && lerp(obstacles[i].getStateA().x, obstacles[i].getStateB().x, smoother.X) != angFreq) {
					ss.str("");
					break;
				}
			}
			Text::addText(RIGHT - 0.65f, 0.55f - 0.08f * 0.2f, "Oscillations/min :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
			TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.55f, 0.47f,
			                           ss.str(), TR_FLOAT, PROPERTY_OPM, changePropertyInput, updatePropertyInput, true);
		} break;
		case ST_ANG_VEL: {
			const float angVel = lerp(obstacles[focus].getStateA().x, obstacles[focus].getStateB().x, smoother.X);
			const float rpm = angVel * 30 / PI;
			ss.str("");
			ss.clear();
			ss << rpm;
			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && obstacles[i].getStateType() == ST_ANG_VEL && lerp(obstacles[i].getStateA().x, obstacles[i].getStateB().x, smoother.X) != angVel) {
					ss.str("");
					break;
				}
			}
			Text::addText(RIGHT - 0.65f, 0.55f - 0.08f * 0.2f, "Revolutions/min :", BAHNSCHRIFT, 0.08f * 0.6f, TEXT_INACTIVE);
			TextBoxManager::addTextBox(RIGHT - 0.35f, RIGHT - 0.05f, 0.55f, 0.47f,
			                           ss.str(), TR_FLOAT, PROPERTY_RPM, changePropertyInput, updatePropertyInput, true);
		}
		}
	}

	ButtonManager::addButton(RIGHT - 0.12f - Text::calculateWidth("Save as", BAHNSCHRIFT, 0.1f * 0.6f), RIGHT - 0.05f, -0.55f, -0.65f,
	                         requestDialogue,
	                         DLG_SAVE_AS,
	                         BS_GREEN,
	                         "Save as");

	ButtonManager::addButton(RIGHT - 0.12f - Text::calculateWidth("Save", BAHNSCHRIFT, 0.1f * 0.6f), RIGHT - 0.05f, -0.7f, -0.8f,
	                         saveLevel,
	                         0,
	                         BS_GREEN,
	                         "Save");
}

void Game::drawDialogue() {
	if (dialogue == DLG_NONE) return;

	ButtonManager::addButton(LEFT, RIGHT, TOP, BOTTOM,
	                         requestDialogue,
	                         DLG_NONE,
	                         BS_INVISIBLE,
	                         "",
	                         true, false, false);

	switch (dialogue) {
#ifdef WINDOWS_VERSION
	case DLG_CONFIRM_EXIT:
		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f), fminf(RIGHT - 0.05f, 0.5f), 0.3f, -0.3f,
		                         nullptr, 0,
		                         BS_GREY_BACKING,
		                         "",
		                         false, false, false);

		Text::addText(-Text::calculateWidth("Unsaved changes will be lost", BAHNSCHRIFT, 0.08f) * 0.5f, 0.2f, "Unsaved changes will be lost", BAHNSCHRIFT, 0.08f, BLACK);

		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f) + 0.05f, fmaxf(LEFT + 0.05f, -0.5f) + 0.12f + Text::calculateWidth("Exit anyway", BAHNSCHRIFT, 0.1f * 0.6f), -0.15f, -0.25f,
		                         confirmExit, 0,
		                         BS_RED,
		                         "Exit anyway");

		ButtonManager::addButton(fminf(RIGHT - 0.05f, 0.5f) - 0.12f - Text::calculateWidth("Cancel", BAHNSCHRIFT, 0.1f * 0.6f), fminf(RIGHT - 0.05f, 0.5f) - 0.05f, -0.15f, -0.25f,
		                         requestDialogue, DLG_NONE,
		                         BS_GREEN,
		                         "Cancel");
		break;
#endif
	case DLG_CONFIRM_SWITCH:
		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f), fminf(RIGHT - 0.05f, 0.5f), 0.3f, -0.3f,
		                         nullptr, 0,
		                         BS_GREY_BACKING,
		                         "",
		                         false, false, false);

		Text::addText(-Text::calculateWidth("Unsaved changes will be lost", BAHNSCHRIFT, 0.08f) * 0.5f, 0.2f, "Unsaved changes will be lost", BAHNSCHRIFT, 0.08f, BLACK);

		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f) + 0.05f, fmaxf(LEFT + 0.05f, -0.5f) + 0.12f + Text::calculateWidth("Change level anyway", BAHNSCHRIFT, 0.1f * 0.6f), -0.15f, -0.25f,
		                         selectPendingLevel, 0,
		                         BS_RED,
		                         "Change level anyway");

		ButtonManager::addButton(fminf(RIGHT - 0.05f, 0.5f) - 0.12f - Text::calculateWidth("Cancel", BAHNSCHRIFT, 0.1f * 0.6f), fminf(RIGHT - 0.05f, 0.5f) - 0.05f, -0.15f, -0.25f,
		                         requestDialogue, DLG_NONE,
		                         BS_GREEN,
		                         "Cancel");

		break;
	case DLG_CHOOSE_LEVEL: {
		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.85f), fminf(RIGHT - 0.05f, 0.85f), 0.8f, -0.8f,
		                         nullptr, 0,
		                         BS_GREY_BACKING,
		                         "",
		                         false, false, false);

		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.85f) + 0.05f, fmaxf(LEFT + 0.05f, -0.85f) + 0.12f + Text::calculateWidth("Cancel", BAHNSCHRIFT, 0.1f * 0.6f), -0.65f, -0.75f,
		                         requestDialogue, DLG_NONE,
		                         BS_RED,
		                         "Cancel");

		std::vector<std::string> levelList = getLevelList();
		std::string* name;
		for (int i = 0; i < levelList.size(); i++) {
			name = &levelList[i];
			ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.85f) + 0.05f, fminf(RIGHT - 0.05f, 0.85f) - 0.05f, 0.75f - 0.12f * (float)i, 0.75f - 0.12f * (float)i - 0.1f,
			                         selectLevelCallback, i,
			                         BS_GREY,
			                         *name);
		}
	} break;
	case DLG_SAVE_AS:
		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f), fminf(RIGHT - 0.05f, 0.5f), 0.3f, -0.3f,
		                         nullptr, 0,
		                         BS_GREY_BACKING,
		                         "",
		                         false, false, false);

		Text::addText(-Text::calculateWidth("Save as", BAHNSCHRIFT, 0.1f) * 0.5f, 0.2f, "Save as", BAHNSCHRIFT, 0.1f, BLACK);
		TextBoxManager::addTextBox(fmaxf(LEFT + 0.05f, -0.4f), fminf(RIGHT - 0.05f, 0.4f), 0.05f, -0.05f,
		                           saveAsName, TR_FILE_NAME, PROPERTY_LEVEL_NAME, changePropertyInput, updatePropertyInput);

		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f) + 0.05f, fmaxf(LEFT + 0.05f, -0.5f) + 0.12f + Text::calculateWidth("Cancel", BAHNSCHRIFT, 0.1f * 0.6f), -0.15f, -0.25f,
		                         requestDialogue, DLG_NONE,
		                         BS_RED,
		                         "Cancel");

		ButtonManager::addButton(fminf(RIGHT - 0.05f, 0.5f) - 0.12f - Text::calculateWidth("Save", BAHNSCHRIFT, 0.1f * 0.6f), fminf(RIGHT - 0.05f, 0.5f) - 0.05f, -0.15f, -0.25f,
		                         requestDialogue, DLG_CONFIRM_OVERWRITE,
		                         BS_GREEN,
		                         "Save");

		if (justOpenedSaveAsDialogue) {
			TextBoxManager::focusTextBoxByProperty(PROPERTY_LEVEL_NAME);
			justOpenedSaveAsDialogue = false;
		}
		break;
	case DLG_CONFIRM_OVERWRITE:
		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f), fminf(RIGHT - 0.05f, 0.5f), 0.3f, -0.3f,
		                         nullptr, 0,
		                         BS_GREY_BACKING,
		                         "",
		                         false, false, false);

		Text::addText(-Text::calculateWidth("Overwrite existing level?", BAHNSCHRIFT, 0.08f) * 0.5f, 0.2f, "Overwrite existing level?", BAHNSCHRIFT, 0.08f, BLACK);

		ButtonManager::addButton(fmaxf(LEFT + 0.05f, -0.5f) + 0.05f, fmaxf(LEFT + 0.05f, -0.5f) + 0.12f + Text::calculateWidth("Overwrite", BAHNSCHRIFT, 0.1f * 0.6f), -0.15f, -0.25f,
		                         saveAs, 0,
		                         BS_RED,
		                         "Overwrite");

		ButtonManager::addButton(fminf(RIGHT - 0.05f, 0.5f) - 0.12f - Text::calculateWidth("Cancel", BAHNSCHRIFT, 0.1f * 0.6f), fminf(RIGHT - 0.05f, 0.5f) - 0.05f, -0.15f, -0.25f,
		                         requestDialogue, DLG_SAVE_AS,
		                         BS_GREEN,
		                         "Cancel");

		break;
	}
}

void Game::render() {
	STATE = lerp(STATE_A, STATE_B, smoother.X);
	STATE_HOVER = STATE * 0.8f;
	STATE_ACTIVE = STATE * 0.6f;
	STATE_INSTANT = smoother.X < 0.5f ? STATE_A : STATE_B;

	// Check to see if the surface has changed size. This is _necessary_ to do every frame when
	// using immersive mode as you'll get no other notification that your renderable area has
	// changed.
	updateRenderArea();

	buildOrthographicMatrix(&projMat, HALF_HEIGHT, WINDOW_WIDTH / WINDOW_HEIGHT, arenaWidth + viewDist * 2, -arenaWidth - viewDist * 2);
	// For perspective use: buildProjectionMatrix(&projMat, 2 * tanf(FOV * 0.5f), 0.2f, 1000, { 0, 0 }, (int) WINDOW_WIDTH, (int) WINDOW_HEIGHT);
	updateView();
	buildViewRotationMatrix(&viewRotMat, viewDir);
	buildViewMatrix(&viewMat, viewRotMat, viewPos);

	// clear the color & depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_ERROR();

	vec3 up(0, 0, 1);
	vec3 view_up = viewRotMat * up;

	vec3 sunDir(2, 2, 3);
	sunDir.unit();
	vec3 view_sunDir = viewRotMat * sunDir;

	objShader->activate();

	glUniformMatrix4fv(uProjectionLoc, 1, false, (float*)&projMat);
	CHECK_ERROR();
	glUniform3fv(uUpDirectionLoc, 1, (float*)&view_up);
	CHECK_ERROR();
	glUniform3fv(uSunDirectionLoc, 1, (float*)&view_sunDir);
	CHECK_ERROR();

	if (inEditor) {
		drawPlane(arenaBackground);

		outlineShader->activate();
		drawObstacleDomains();

		objShader->activate();
		drawObstacles();
		glDepthFunc(GL_ALWAYS);
		drawBall(ball);

		outlineShader->activate();

		if (smoother.X == 0 || smoother.X == 1) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);

			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && i != focus) {
					if (action != ACTION_NONE && limiting[i])
						glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&WARNING_VEC4);
					else
						glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&SELECTED_VEC4);

					drawObstacleOutline(obstacles[i]);
				}
			}

			if (focus >= 0 && focus < MAX_OBSTACLES && selection[focus]) {
				if (action != ACTION_NONE && limiting[focus])
					glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&WARNING_VEC4);
				else
					glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&FOCUSED_VEC4);
				drawObstacleOutline(obstacles[focus]);
			}

			if (selection[MAX_OBSTACLES]) {
				ballOutline.pos.x = -ballOutline.radius;
				buildScaledWorldMatrix(&worldMat, ballOutline.rot, ballOutline.pos, ballOutline.scale);
				mat4 projectionMatrix = worldMat * viewMat * projMat;
				glUniformMatrix4fv(uProjectionFullLoc, 1, false, (float*)&projectionMatrix);
				CHECK_ERROR();

				bool intersecting = checkBallObstacleCollision(&ball, true) >= 0;

				if (intersecting || (action != ACTION_NONE && limiting[MAX_OBSTACLES]))
					glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&WARNING_VEC4);
				else if (MAX_OBSTACLES == focus)
					glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&FOCUSED_VEC4);
				else
					glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&SELECTED_VEC4);

				outlineShader->drawObject(ballModel, ballOutline.texture);
			}
		}
	} else {
		drawPlane(arenaBackground);

		drawBall(ball);

		glEnable(GL_DEPTH_TEST);
		drawObstacles();
	}
	glDisable(GL_DEPTH_TEST);

	Text::clearText();

	ButtonManager::clearButtons();
	ButtonManager::activateShader();
	ButtonManager::updateProjectionMatrix();

	TextBoxManager::clearTextBoxes();

	if (inEditor) {
		if (action == ACTION_SELECT) {
			vec2 tl = pixelsToYNorm(selectBoxTL.x, selectBoxTL.y, WINDOW_WIDTH, WINDOW_HEIGHT);
			vec2 br = pixelsToYNorm(selectBoxBR.x, selectBoxBR.y, WINDOW_WIDTH, WINDOW_HEIGHT);
			float OWX, OWY;
			float width = br.x - tl.x;
			float height = tl.y - br.y;
			OWX = width >= OUTLINE_WIDTH * 2 ? OUTLINE_WIDTH : width * 0.5f;
			OWY = height >= OUTLINE_WIDTH * 2 ? OUTLINE_WIDTH : height * 0.5f;
			ButtonManager::addButton(tl.x, br.x, tl.y, br.y, nullptr, 0, BS_SELECT, "", false, false, false);
		}

		if (smoother.X == 0 || smoother.X == 1) {
			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i]) {
					Obstacle* g = &obstacles[i];

					if (g->getIsStraight()) {
						const vec3 middle = (g->getPos() + g->getRot() * ((g->getStart() + g->getEnd()) * 0.5f) - viewPos) / HALF_HEIGHT;
						bool activeMiddle = actionMod == ACTION_MOD_SLIDE;
						ButtonManager::addButton(middle.y - BLOB_RADIUS, middle.y + BLOB_RADIUS, middle.z + BLOB_RADIUS, middle.z - BLOB_RADIUS,
						                         blobPressed, (ACTION_MOD_SLIDE << 16) + i,
						                         activeMiddle ? BS_OB_BLOB_CENTRAL_ACTIVE : BS_OB_BLOB_CENTRAL,
						                         "", true, false, !activeMiddle);
					} else {
						const vec3 middle = (g->getPos() + g->getRot() * vec3(0, 0, g->getMajorRadius()) - viewPos) / HALF_HEIGHT;
						bool activeMiddle = actionMod == ACTION_MOD_MAJOR;
						ButtonManager::addButton(middle.y - BLOB_RADIUS, middle.y + BLOB_RADIUS, middle.z + BLOB_RADIUS, middle.z - BLOB_RADIUS,
						                         blobPressed, (ACTION_MOD_MAJOR << 16) + i,
						                         activeMiddle ? BS_OB_BLOB_CENTRAL_ACTIVE : BS_OB_BLOB_CENTRAL,
						                         "", true, false, !activeMiddle);
					}
					const vec3 start = (g->getPos() + g->getRot() * g->getStart() - viewPos) / HALF_HEIGHT;
					bool activeStart = (actionMod == ACTION_MOD_START || actionMod == ACTION_MOD_START_MIRRORED || actionMod == ACTION_MOD_END_MIRRORED) && obstacles[focus].getIsStraight() == g->getIsStraight();
					ButtonManager::addButton(start.y - BLOB_RADIUS, start.y + BLOB_RADIUS, start.z + BLOB_RADIUS, start.z - BLOB_RADIUS,
					                         blobPressed, (ACTION_MOD_START << 16) + i,
					                         activeStart ? BS_OB_BLOB_TERMINAL_ACTIVE : BS_OB_BLOB_TERMINAL,
					                         "", true, false, !activeStart);
					const vec3 end = (g->getPos() + g->getRot() * g->getEnd() - viewPos) / HALF_HEIGHT;
					bool activeEnd = (actionMod == ACTION_MOD_END || actionMod == ACTION_MOD_START_MIRRORED || actionMod == ACTION_MOD_END_MIRRORED) && obstacles[focus].getIsStraight() == g->getIsStraight();
					ButtonManager::addButton(end.y - BLOB_RADIUS, end.y + BLOB_RADIUS, end.z + BLOB_RADIUS, end.z - BLOB_RADIUS,
					                         blobPressed, (ACTION_MOD_END << 16) + i,
					                         activeEnd ? BS_OB_BLOB_TERMINAL_ACTIVE : BS_OB_BLOB_TERMINAL,
					                         "", true, false, !activeEnd);

					if (i != focus) {
						const vec3 centre = (g->getPos() - viewPos) / HALF_HEIGHT;
						ButtonManager::addButton(centre.y - OUTLINE_WIDTH * 2, centre.y + OUTLINE_WIDTH * 2, centre.z + OUTLINE_WIDTH * 2, centre.z - OUTLINE_WIDTH * 2,
						                         nullptr, 0,
						                         BS_PIVOT, "", false, true);
					}
				}
			}
			if (focus >= 0 && focus < MAX_OBSTACLES && selection[focus]) {
				const vec3 centre = (obstacles[focus].getPos() - viewPos) / HALF_HEIGHT;
				ButtonManager::addButton(centre.y - OUTLINE_WIDTH * 2, centre.y + OUTLINE_WIDTH * 2, centre.z + OUTLINE_WIDTH * 2, centre.z - OUTLINE_WIDTH * 2,
				                         nullptr, 0,
				                         BS_PIVOT_FOCUSED, "", false, true);
			}
		}

		drawInventoryPanel();
		drawPropertiesPanel();
	} else {
		ButtonManager::addButton(RIGHT - 0.28f, RIGHT - 0.05f, 0.95f, 0.85f,
		                         restartLevel,
		                         0,
		                         BS_GREY,
		                         "Restart");

		if (levelComplete)
			ButtonManager::addButton(-0.4f, 0.4f, 0.1f, -0.1f,
			                         restartLevel,
			                         0,
			                         BS_GREEN,
			                         "Level Complete!");
	}

	glEnable(GL_BLEND);

	std::string name = level.name;
	if (!changesAreSaved())
		name += "*";
	ButtonManager::addButton(RIGHT - 0.12f - Text::calculateWidth(name, BAHNSCHRIFT, 0.1f * 0.6f), RIGHT - 0.05f, -0.85f, -0.95f,
	                         requestDialogue,
	                         DLG_CHOOSE_LEVEL,
	                         BS_GREY,
	                         name);

	ButtonManager::markEndOfBatch();
	Text::markEndOfBatch();

	drawDialogue();

	Text::activateShader();
	Text::updateProjectionMatrix();
	char fpsText[10];
	snprintf(fpsText, 10, "%.1f", fps());
	Text::addText(LEFT + 0.05f, 0.95f, fpsText, COURIER_NEW, 0.06f, WHITE);

	for (int i = 0; i < 2; i++) { // 2 = number of batches
		ButtonManager::activateShader();
		ButtonManager::drawButtons(i);
		Text::activateShader();
		Text::drawText(i);
	}

	glDisable(GL_BLEND);

	Line::projMat = viewMat * projMat;
	Line::activateShader();
	//glLineWidth(10.0f);
	Line::drawLines();

#ifdef WINDOWS_VERSION
	Cursor::activateShader();
	Cursor::updateProjectionMatrix();
	Cursor::pos = pixelsToYNorm(pointerPos, WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_BLEND);
	updateCursor();
	Cursor::drawCursor();
	glDisable(GL_BLEND);
#endif

	glUseProgram(0);

	// Present the rendered image. This is an implicit glFlush.
#ifdef WINDOWS_VERSION
	glfwSwapBuffers(window);
#else
	eglSwapBuffers(eglDisplay, eglSurface);
#endif
}

void Game::drawBall(const Ball& b) {
	drawObject(ballModel, b.texture, b.pos, b.rot, b.scale);
}

void Game::drawPlane(const Plane& p) {
	drawObject(planeModel, p.texture, p.pos, p.rot, p.scale);
}

void Game::drawObject(const Model* model, const TextureAsset* texture, const vec3& pos, const mat3& rot, const vec3& scale) {
	buildScaledWorldMatrix(&worldMat, rot, pos, scale);

	mat4 bodyToView = worldMat * viewMat;
	mat3 bodyToViewRot = viewRotMat * rot;

	glUniformMatrix3fv(uBodyToViewRotLoc, 1, true, (float*)&bodyToViewRot);
	CHECK_ERROR();
	glUniformMatrix4fv(uBodyToViewLoc, 1, false, (float*)&bodyToView);
	CHECK_ERROR();

	glUniform1f(uAlphaLoc, 1);
	CHECK_ERROR();

	objShader->drawObject(model, texture);
}

void Game::drawObstacles() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	for (short i = (short)obstacles.size() - 1; i >= 0; i--) {
		const Obstacle& g = obstacles[i];
		buildScaledWorldMatrix(&worldMat, g.getRot(), g.getPos(), vec3(1));

		mat4 bodyToView = worldMat * viewMat;
		mat3 bodyToViewRot = viewRotMat * g.getRot();

		glUniformMatrix3fv(uBodyToViewRotLoc, 1, true, (float*)&bodyToViewRot);
		CHECK_ERROR();
		glUniformMatrix4fv(uBodyToViewLoc, 1, false, (float*)&bodyToView);
		CHECK_ERROR();

		float alpha = 1;
		if (inEditor) {
			if (g.getStateType() == ST_POS_OSC || g.getStateType() == ST_ANG_OSC) {
				glEnable(GL_BLEND);
				alpha = 2.5f * fabsf(0.5f - smoother.X) - 0.2f; // Includes a small period where the obstacle is completely invisible
			}
		}
		glUniform1f(uAlphaLoc, alpha);
		CHECK_ERROR();

		objShader->drawObject(&g.obstacle, g.texture);
		glDisable(GL_BLEND);
	}
}

void Game::drawObstacleOutline(const Obstacle& g) {
	buildScaledWorldMatrix(&worldMat, g.getRot(), g.getPos(), vec3(1));
	mat4 projectionMatrix = worldMat * viewMat * projMat;
	glUniformMatrix4fv(uProjectionFullLoc, 1, false, (float*)&projectionMatrix);
	CHECK_ERROR();

	objShader->drawObject(&g.outline, g.texture);
}

void Game::drawObstacleDomains() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	for (short i = 0; i < (short)obstacles.size(); i++) {
		const Obstacle& g = obstacles[i];
		if (selection[i] && g.getStateType() != ST_STATIC) {
			vec4 DOMAIN_VEC4 = g.getColor().toVec4();
			DOMAIN_VEC4.a = 0.35f;
			glUniform4fv(outlineShader->getUniformLocation("uOutlineColor"), 1, (float*)&DOMAIN_VEC4);
			CHECK_ERROR();

			mat4 projectionMatrix;
			const vec3 depth = vec3(((float)i - (float)obstacles.size()), 0, 0);

			if (g.getStateType() == ST_POS || g.getStateType() == ST_ANG) {
				vec3 posYZ;
				mat3 rot;

				if (smoother.X != 0) {
					if (g.getStateType() == ST_POS) {
						posYZ = g.getStateA();
						rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, g.getAngle());
					} else {
						posYZ = g.getPos();
						rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, g.getStateA().x);
					}
					buildScaledWorldMatrix(&worldMat, rot, posYZ + depth, vec3(0, 1, 1));
					projectionMatrix = worldMat * viewMat * projMat;
					glUniformMatrix4fv(uProjectionFullLoc, 1, false, (float*)&projectionMatrix);
					CHECK_ERROR();

					outlineShader->drawObject(&g.obstacle, g.texture);
				}

				if (smoother.X != 1) {
					if (g.getStateType() == ST_POS) {
						posYZ = g.getStateB();
						rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, g.getAngle());
					} else {
						posYZ = g.getPos();
						rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, g.getStateB().x);
					}
					buildScaledWorldMatrix(&worldMat, rot, posYZ + depth, vec3(0, 1, 1));
					projectionMatrix = worldMat * viewMat * projMat;
					glUniformMatrix4fv(uProjectionFullLoc, 1, false, (float*)&projectionMatrix);
					CHECK_ERROR();

					outlineShader->drawObject(&g.obstacle, g.texture);
				}
			}

			vec3 pos = depth + ((g.getStateType() == ST_POS || g.getStateType() == ST_POS_OSC) ? vec3() : g.getPos());
			buildScaledWorldMatrix(&worldMat, I_MAT3, pos, vec3(1)); // Alpha sorting
			projectionMatrix = worldMat * viewMat * projMat;
			glUniformMatrix4fv(uProjectionFullLoc, 1, false, (float*)&projectionMatrix);
			CHECK_ERROR();

			outlineShader->drawObject(&g.domain, g.texture);
		}
	}
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}


// Input handling

vec3 Game::getPointerWorldPos(float x, float y) {
	vec3 mousePos, mouseDir;
	vec2 posYNorm = pixelsToYNorm(x, y, WINDOW_WIDTH, WINDOW_HEIGHT);
	float
	ch = cosf(heading),
	sh = sinf(heading),
	cp = cosf(pitch),
	sp = sinf(pitch);

	vec3 offset;
	offset.x = posYNorm.x * HALF_HEIGHT * -sh + posYNorm.y * HALF_HEIGHT * -sp * ch;
	offset.y = posYNorm.x * HALF_HEIGHT * ch + posYNorm.y * HALF_HEIGHT * -sp * sh;
	offset.z = posYNorm.y * HALF_HEIGHT * cp;
	mousePos = viewPos + offset;
	mouseDir = viewDir * -1;
	/* For perspective use:
	vec2 posXNorm = pixelsToXNorm(x, y, WINDOW_WIDTH, WINDOW_HEIGHT);
	vec3 v = { posXNorm.x * tanf(FOV * 0.5f), posXNorm.y * tanf(FOV * 0.5f), 1 };
	mat3 rot;
	rot.SetTransposeOf(&viewRotMat);
	mouseDir = rot * v * -1;
	mousePos = viewPos;*/

	vec3 normal = {1, 0, 0};
	float d = dot(mousePos, normal) / dot(mouseDir, normal);
	return mousePos - mouseDir * d;
}

vec3 Game::getPointerWorldPos(const vec2& pos) { return getPointerWorldPos(pos.x, pos.y); }

std::vector<std::string> Game::getLevelList() {
	std::vector<std::string> levelList;
#ifdef WINDOWS_VERSION
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA((ASSETS_PATH + "levels/*.lvl").c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			std::string fileName = findFileData.cFileName;
			if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".lvl") {
				levelList.push_back(fileName.substr(0, fileName.length() - 4));
			}
		} while (FindNextFileA(hFind, &findFileData) != 0);
		FindClose(hFind);
	}
#else
	AAssetDir* dir = AAssetManager_openDir(androidApp->activity->assetManager, "levels");
	if (dir != nullptr) {
		const char* fName = AAssetDir_getNextFileName(dir);
		while (fName != nullptr) {
			std::string fileName = fName;
			if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".lvl")
				levelList.push_back(fileName.substr(0, fileName.length() - 4));
			fName = AAssetDir_getNextFileName(dir);
		}
		AAssetDir_close(dir);
	}
#endif
	return levelList;
}

void Game::updateArena() {
	arenaWidth = level.arenaWidth * ball.radius;
	arenaHeight = level.arenaHeight * ball.radius;
	const vec3 scale = {arenaWidth, arenaWidth, arenaHeight};

	vec3 pos = {0};
	arenaBounds[0].scale = scale;
	arenaBounds[0].setPos(pos);
	pos = {0, 0, arenaHeight};
	arenaBounds[1].scale = scale;
	arenaBounds[1].setPos(pos);
	pos = {0, -arenaWidth * 0.5f, arenaHeight * 0.5f};
	arenaBounds[2].scale = scale;
	arenaBounds[2].setPos(pos);
	pos = {0, arenaWidth * 0.5f, arenaHeight * 0.5f};
	arenaBounds[3].scale = scale;
	arenaBounds[3].setPos(pos);
	pos = {-arenaWidth * 0.5f, 0, arenaHeight * 0.5f};
	arenaBackground.scale = scale;
	arenaBackground.setPos(pos);
}

void Game::loadLevel(const Level& newLevel, bool resetView) {
	level = newLevel;
	levelComplete = false;

	ball = Ball(level.ballType, inEditor);
	resetBall();

	updateArena();

	if (resetView) {
		baseViewDist = fmaxf(arenaWidth, arenaHeight) * 2.0f;
		resetEditorView();
	} else
		updateHalfHeight();
		updateHalfHeight();

	obstacles.clear();
	obstacles.reserve(level.obstacleDefinitions.size());
	for (ObstacleDefinition& d: level.obstacleDefinitions) {
		obstacles.emplace_back(&d, ball.radius);
		obstacles.back().createAllModels(inEditor);
	}

	if (toggled)
		smoother.SetPosition(1, 0);
	else
		smoother.SetPosition(0, 0);

	moveEditorObstacles();
}

std::string Game::levelString;
bool resetUndoBufferFlag = false;

void Game::selectLevel(const std::string& name) {
	Level newLevel;
	bool loaded;

	if (name.empty())
		loaded = Level::importLevel(&newLevel, levelString, level.name);
	else {
#ifdef WINDOWS_VERSION
		std::ifstream ifs((ASSETS_PATH + "levels/" + name + ".lvl").c_str());
		if (ifs.is_open()) {
			std::string str(std::istreambuf_iterator<char>{ifs}, {});
			ifs.close();
			loaded = Level::importLevel(&newLevel, str, name);
			levelString = str;
		} else
			return;
#else
		AAsset* file = AAssetManager_open(
		androidApp->activity->assetManager,
		("levels/" + name + ".lvl").c_str(),
		AASSET_MODE_BUFFER);
		if (file) {
			const off_t fileLength = AAsset_getLength(file);
			std::string str;
			str.resize(fileLength);
			AAsset_read(file, (void*)str.c_str(), fileLength);
			AAsset_close(file);
			loaded = Level::importLevel(&newLevel, str, name);
		} else
			return;
#endif
	}

	if (!loaded)
		return;

	toggled = false;

	if (!name.empty())
		bufferIndex = savedIndex = 0;

	if (inEditor) {
		if (!name.empty() || resetUndoBufferFlag) {
			memset(selection, false, sizeof(selection));
			focus = -1;
			undoBuffer[0].set(newLevel, selection, focus);

			bufferIndex = lastBufferIndex = selectionBufferIndex = 0;
		}
		resetUndoBufferFlag = false;

		action = ACTION_NONE;
		actionMod = ACTION_MOD_NONE;
		blobWasPressed = false;
	} else if (!name.empty())
		resetUndoBufferFlag = true; // Reset undo buffer upon entering editor

	loadLevel(newLevel);
}

std::string Game::pendingLevel;

void Game::selectPendingLevel(int _) {
	selectLevel(pendingLevel);
	requestDialogue(DLG_NONE);
}

void Game::selectLevelCallback(int levelIndex) {
	pendingLevel = getLevelList()[levelIndex];
	requestDialogue(DLG_CONFIRM_SWITCH);
}

short Game::savedIndex;
void Game::saveLevel(int _) {
#ifdef WINDOWS_VERSION
	std::string data = std::move(level.exportLevel());

	std::ofstream ofs((ASSETS_PATH + "levels/" + level.name + ".lvl").c_str());
	if (ofs.is_open()) {
		ofs << data;
		ofs.close();
		savedIndex = bufferIndex;
	}
#endif
}

#ifdef WINDOWS_VERSION

WindowInfo windowInfo;

int arrowKeys[] = {GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_KP_4, GLFW_KEY_KP_6, GLFW_KEY_KP_8, GLFW_KEY_KP_2};

bool Game::allArrowKeysUp(int exceptKey) {
	for (int key: arrowKeys)
		if (glfwGetKey(window, key) == GLFW_PRESS && exceptKey != key)
			return false;
	return true;
}

vec3 keyContribution(int key) {
	switch (key) {
	case GLFW_KEY_KP_4:
		[[fallthrough]];
	case GLFW_KEY_LEFT:
		return {0, -1, 0};
	case GLFW_KEY_KP_6:
		[[fallthrough]];
	case GLFW_KEY_RIGHT:
		return {0, 1, 0};
	case GLFW_KEY_KP_8:
		[[fallthrough]];
	case GLFW_KEY_UP:
		return {0, 0, 1};
	case GLFW_KEY_KP_2:
		[[fallthrough]];
	case GLFW_KEY_DOWN:
		return {0, 0, -1};
	default:
		return {0, 0, 0};
	}
} 

void Game::moveObjectsWithArrowKey(int key, int keyAction) {
	if (keyAction == GLFW_RELEASE) {
		if (allArrowKeysUp())
			finishAction();
		return;
	}

	vec3 moveDir = vec3(0);

	if (keyAction == GLFW_PRESS)
		moveDir = keyContribution(key);
	else {
		for (int k: arrowKeys)
			if (glfwGetKey(window, k) == GLFW_PRESS)
				moveDir += keyContribution(k);
	}

	float moveAmount;
	if (shiftDown && ctrlDown)
		moveAmount = 0.01f;
	else if (shiftDown)
		moveAmount = 0.1f;
	else if (ctrlDown)
		moveAmount = 5.0f;
	else
		moveAmount = 1.0f;

	if (keyAction == GLFW_PRESS && allArrowKeysUp(key)) {
		action = ACTION_MOVE;
		actionMod = ACTION_MOD_UNIT;
		startAction();
	}

	actionVector += moveDir * moveAmount;

	updateAction();
}

void Game::handleKeyInput(GLFWwindow* window, int key, int scancode, int keyAction, int mods) {
	if (ctrlDown != (bool)(mods & GLFW_MOD_CONTROL) || shiftDown != (bool)(mods & GLFW_MOD_SHIFT) || altDown != (bool)(mods & GLFW_MOD_ALT)) {
		ctrlDown = mods & GLFW_MOD_CONTROL;
		shiftDown = mods & GLFW_MOD_SHIFT;

		if (altDown != (bool)(mods & GLFW_MOD_ALT)) {
			altDown = mods & GLFW_MOD_ALT;
			actionIsStateless = altDown; // Only update this if altDown has changed
			                             // This ensures that if an action has been set as stateless by the program (e.g. when pasting obstacles), it is not overridden by the alt key state
		} else
			altDown = mods & GLFW_MOD_ALT;


		if (inEditor && action != ACTION_NONE) {
			switch (action) {
			case ACTION_SELECT:
				if (shiftDown)
					actionMod = ACTION_MOD_SUBTRACT;
				else if (ctrlDown)
					actionMod = ACTION_MOD_ADD;
				else
					actionMod = ACTION_MOD_REPLACE;
				changeAction();
				break;
			case ACTION_MOVE:
				[[fallthrough]];
			case ACTION_ROTATE:
				[[fallthrough]];
			case ACTION_SCALE:
				changeAction(); // Actions affected by actionIsStateless (altDown)
				break;
			case ACTION_SHAPE:
				if (actionMod == ACTION_MOD_START && ctrlDown)
					actionMod = ACTION_MOD_START_MIRRORED;
				else if (actionMod == ACTION_MOD_END && ctrlDown)
					actionMod = ACTION_MOD_END_MIRRORED;
				else if ((actionMod == ACTION_MOD_START_MIRRORED && !ctrlDown))
					actionMod = ACTION_MOD_START;
				else if ((actionMod == ACTION_MOD_END_MIRRORED && !ctrlDown))
					actionMod = ACTION_MOD_END;
				else
					break;
				changeAction();
				break;
			}
		}
	}

	if (keyAction == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			if (shiftDown) {
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				return;
			}
			break;
		case GLFW_KEY_F5:
			if (altDown)
				reloadShaders();
			else
				restartLevel(0);
			return;
		case GLFW_KEY_F11:
			if (glfwGetWindowMonitor(window))
				glfwSetWindowMonitor(window, nullptr, windowInfo.posX, windowInfo.posY, windowInfo.width, windowInfo.height, windowInfo.refreshRate);
			else {
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwGetWindowPos(window, &windowInfo.posX, &windowInfo.posY);
				glfwGetWindowSize(window, &windowInfo.width, &windowInfo.height);
				windowInfo.refreshRate = mode->refreshRate;
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			}
			return;
		case GLFW_KEY_TAB:
			if (inEditor) {
				levelString = std::move(level.exportLevel());
				TextBoxManager::focusedBoxIndex = -1;
				if (TextBoxManager::anythingChanged)
					finishAction();
			}
			inEditor = !inEditor;
			selectLevel();
			return;
		case GLFW_KEY_SPACE:
			if (!inEditor || inEditor && action == ACTION_NONE) {
				if (dialogue != DLG_NONE) break;
				TextBoxManager::focusedBoxIndex = -1;
				if (TextBoxManager::anythingChanged)
					finishAction();
				toggle();
			}
			return;
		case GLFW_KEY_P: {
			bool pause = true;
		}
			return;
		}
	}

	const bool NUM_LOCK = mods & GLFW_MOD_NUM_LOCK;

	if (TextBoxManager::focusedBoxIndex >= 0 && (keyAction == GLFW_PRESS || keyAction == GLFW_REPEAT)) {
		switch (key) {
		case GLFW_KEY_A:
			if (ctrlDown && keyAction == GLFW_PRESS) {
				TextBoxManager::selected = true;
				TextBoxManager::cursorPos = (short)TextBoxManager::text.length();
			}
			break;
		case GLFW_KEY_C:
			if (ctrlDown && TextBoxManager::selected && keyAction == GLFW_PRESS)
				glfwSetClipboardString(window, TextBoxManager::text.c_str());
			break;
		case GLFW_KEY_V:
			if (ctrlDown && keyAction == GLFW_PRESS) {
				const char* clipText = glfwGetClipboardString(window);
				if (clipText)
					TextBoxManager::typed(clipText);
			}
			break;
		case GLFW_KEY_ESCAPE:
			if (keyAction == GLFW_PRESS) {
				TextBoxManager::focusedBoxIndex = -1;
				if (TextBoxManager::anythingChanged)
					cancelAction();
				if (dialogue == DLG_SAVE_AS)
					requestDialogue(DLG_NONE);
			}
			break;
		case GLFW_KEY_ENTER:
			if (keyAction == GLFW_PRESS) {
				TextBoxManager::focusedBoxIndex = -1;
				if (dialogue == DLG_SAVE_AS) {
					requestDialogue(DLG_CONFIRM_OVERWRITE);
				} else if (TextBoxManager::anythingChanged)
					finishAction();
			}
			break;
		case GLFW_KEY_BACKSPACE:
			if (TextBoxManager::selected) {
				TextBoxManager::text.clear();
				TextBoxManager::selected = false;
				TextBoxManager::cursorPos = 0;
			} else if (TextBoxManager::cursorPos > 0) {
				TextBoxManager::text.erase((size_t)(TextBoxManager::cursorPos - 1), 1);
				TextBoxManager::cursorPos--;
				updatePropertyInput(TextBoxManager::getCurrentProperty());
			}
			break;
		case GLFW_KEY_DELETE:
			if (TextBoxManager::selected) {
				TextBoxManager::text.clear();
				TextBoxManager::selected = false;
				TextBoxManager::cursorPos = 0;
			} else if (TextBoxManager::cursorPos < (short)TextBoxManager::text.length()) {
				TextBoxManager::text.erase(TextBoxManager::cursorPos, 1);
				updatePropertyInput(TextBoxManager::getCurrentProperty());
			}
			break;
		case GLFW_KEY_KP_4:
			if (NUM_LOCK) break;
			[[fallthrough]];
		case GLFW_KEY_LEFT:
			if (TextBoxManager::cursorPos > 0 || keyAction == GLFW_PRESS)
				TextBoxManager::start_ms = now_ms();

			if (TextBoxManager::selected) {
				TextBoxManager::selected = false;
				TextBoxManager::cursorPos = 0;
			} else if (TextBoxManager::cursorPos > 0)
				TextBoxManager::cursorPos--;
			break;
		case GLFW_KEY_KP_6:
			if (NUM_LOCK) break;
			[[fallthrough]];
		case GLFW_KEY_RIGHT:
			if (TextBoxManager::cursorPos < (short)TextBoxManager::text.length() || keyAction == GLFW_PRESS)
				TextBoxManager::start_ms = now_ms();
					
			if (TextBoxManager::selected) {
				TextBoxManager::selected = false;
				TextBoxManager::cursorPos = (short)TextBoxManager::text.length();
			} else if (TextBoxManager::cursorPos < (short)TextBoxManager::text.length())
				TextBoxManager::cursorPos++;
			break;
		case GLFW_KEY_KP_8:
			if (NUM_LOCK) break;
			[[fallthrough]];
		case GLFW_KEY_UP:
			[[fallthrough]];
		case GLFW_KEY_KP_7:
			if (NUM_LOCK) break;
			[[fallthrough]];
		case GLFW_KEY_HOME:
			if (keyAction == GLFW_PRESS) {
				TextBoxManager::cursorPos = 0;
				TextBoxManager::start_ms = now_ms();
				TextBoxManager::selected = false;
			}
			break;
		case GLFW_KEY_KP_2:
			if (NUM_LOCK) break;
			[[fallthrough]];
		case GLFW_KEY_DOWN:
			[[fallthrough]];
		case GLFW_KEY_KP_1:
			if (NUM_LOCK) break;
			[[fallthrough]];
		case GLFW_KEY_END:
			if (keyAction == GLFW_PRESS) {
				TextBoxManager::cursorPos = (short)TextBoxManager::text.length();
				TextBoxManager::start_ms = now_ms();
				TextBoxManager::selected = false;
			}
			break;
		}

		return;
	}

	switch (key) {
	case GLFW_KEY_ESCAPE:
		if (keyAction == GLFW_PRESS) {
			if (action != ACTION_NONE)
				cancelAction();
			else
				requestDialogue(DLG_NONE);
		}
		break;
	case GLFW_KEY_F5:
		if (keyAction == GLFW_PRESS) {
			if (altDown)
				reloadShaders();
			else
				restartLevel(0);
		}
		break;
	case GLFW_KEY_F11:
		if (keyAction == GLFW_PRESS) {
			if (glfwGetWindowMonitor(window))
				glfwSetWindowMonitor(window, nullptr, windowInfo.posX, windowInfo.posY, windowInfo.width, windowInfo.height, windowInfo.refreshRate);
			else {
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwGetWindowPos(window, &windowInfo.posX, &windowInfo.posY);
				glfwGetWindowSize(window, &windowInfo.width, &windowInfo.height);
				windowInfo.refreshRate = mode->refreshRate;
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			}
		}
		break;
	case GLFW_KEY_DELETE:
	case GLFW_KEY_BACKSPACE:
		if (inEditor && keyAction == GLFW_PRESS) {
			if (action != ACTION_NONE) {
				bool selectionCopy[MAX_OBSTACLES] = {};
				memcpy(selectionCopy, selection, MAX_OBSTACLES);
				cancelAction();
				memcpy(selection, selectionCopy, MAX_OBSTACLES);
			}

			deleteObstacles();
		}
		break;
	case GLFW_KEY_A:
		if (inEditor && ctrlDown && keyAction == GLFW_PRESS) {
			for (short i = 0; i < obstacles.size(); i++)
				selection[i] = !shiftDown;
			selection[MAX_OBSTACLES] = !shiftDown;

			if (shiftDown)
				focus = -1;
			else if (focus < 0)
				focus = MAX_OBSTACLES;
			finishAction();
		}
		break;
	case GLFW_KEY_C:
		if (inEditor && action == ACTION_NONE && ctrlDown && keyAction == GLFW_PRESS)
			copyObstacles();
		break;
	case GLFW_KEY_D:
		if (inEditor && keyAction == GLFW_PRESS) {
			if (action == ACTION_NONE) {
				if (ctrlDown)
					duplicateObstacles();
			} else if (action == ACTION_SCALE) {
				switch (actionMod) {
				case ACTION_MOD_INDIVIDUAL:
				case ACTION_MOD_GROUP:
					actionMod = ACTION_MOD_MINOR;
					break;
				case ACTION_MOD_MINOR:
					actionMod = ACTION_MOD_MAJOR;
					break;
				case ACTION_MOD_MAJOR:
					actionMod = ACTION_MOD_INDIVIDUAL;
					break;
				}
				changeAction();
			}
		}
		break;
	case GLFW_KEY_R:
		if (inEditor && keyAction == GLFW_PRESS) {
			switch (action) {
			case ACTION_NONE:
				action = ACTION_ROTATE;
				actionMod = ACTION_MOD_INDIVIDUAL;
				startAction();
				break;
			case ACTION_ROTATE:
				if (actionMod == ACTION_MOD_INDIVIDUAL) {
					actionMod = ACTION_MOD_GROUP;
					changeAction();
				} else {
					actionMod = ACTION_MOD_INDIVIDUAL;
					changeAction();
				}
				break;
			default:
				action = ACTION_ROTATE;
				actionMod = ACTION_MOD_INDIVIDUAL;
				changeAction();
				break;
			}
		}
		break;
	case GLFW_KEY_S:
		if (inEditor && keyAction == GLFW_PRESS) {
			if (ctrlDown) {
				if (action == ACTION_NONE)
					if (shiftDown)
						requestDialogue(DLG_SAVE_AS);
					else
						saveLevel();
			} else {
				switch (action) {
				case ACTION_NONE:
					action = ACTION_SCALE;
					actionMod = ACTION_MOD_INDIVIDUAL;
					startAction();
					break;
				case ACTION_SCALE:
					if (actionMod == ACTION_MOD_INDIVIDUAL) {
						actionMod = ACTION_MOD_GROUP;
						changeAction();
					} else {
						actionMod = ACTION_MOD_INDIVIDUAL;
						changeAction();
					}
					break;
				default:
					action = ACTION_SCALE;
					actionMod = ACTION_MOD_INDIVIDUAL;
					changeAction();
					break;
				}
			}
		} else
			slowMotion = !slowMotion;
		break;
	case GLFW_KEY_T:
		if (inEditor && keyAction == GLFW_PRESS) {
			switch (action) {
			case ACTION_NONE:
				action = ACTION_MOVE;
				actionMod = ACTION_MOD_FREE;
				startAction();
				break;
			case ACTION_MOVE:
				break;
			default:
				action = ACTION_MOVE;
				actionMod = ACTION_MOD_FREE;
				changeAction();
				break;
			}
		}
		break;
	case GLFW_KEY_V:
		if (inEditor && action == ACTION_NONE && ctrlDown && keyAction == GLFW_PRESS)
			pasteObstacles();
		break;
	case GLFW_KEY_X:
		if (inEditor && keyAction == GLFW_PRESS) {
			if (action == ACTION_NONE) {
				if (ctrlDown)
					cutObstacles();
			} else if (action == ACTION_MOVE) {
				switch (actionMod) {
				case ACTION_MOD_FREE:
				case ACTION_MOD_GLOBAL_Y:
					actionMod = ACTION_MOD_GLOBAL_X;
					break;
				case ACTION_MOD_GLOBAL_X:
				case ACTION_MOD_LOCAL_Y:
					actionMod = ACTION_MOD_LOCAL_X;
					break;
				case ACTION_MOD_LOCAL_X:
					actionMod = ACTION_MOD_FREE;
					break;
				}

				changeAction();
			}
		}
		break;
	case GLFW_KEY_Y:
		if (inEditor && (keyAction == GLFW_PRESS || keyAction == GLFW_REPEAT)) {
			if (action == ACTION_NONE && ctrlDown)
				redo();
			else if (action == ACTION_MOVE) {
				switch (actionMod) {
				case ACTION_MOD_FREE:
				case ACTION_MOD_GLOBAL_X:
					actionMod = ACTION_MOD_GLOBAL_Y;
					break;
				case ACTION_MOD_GLOBAL_Y:
				case ACTION_MOD_LOCAL_X:
					actionMod = ACTION_MOD_LOCAL_Y;
					break;
				case ACTION_MOD_LOCAL_Y:
					actionMod = ACTION_MOD_FREE;
					break;
				}

				changeAction();
			}
		}
		break;
	case GLFW_KEY_Z:
		if (inEditor && ctrlDown && (keyAction == GLFW_PRESS || keyAction == GLFW_REPEAT)) {
			if (action == ACTION_NONE) {
				if (shiftDown)
					redo();
				else
					undo();
			} else if (!shiftDown)
				cancelAction();
		}
		break;
	}

	for (int i = 0; i < 8; i++)
		if (key == arrowKeys[i] && !(NUM_LOCK && (i > 3)))
			moveObjectsWithArrowKey(key, keyAction);
}

void Game::handleCharInput(GLFWwindow* window, unsigned int c) {
	if (TextBoxManager::focusedBoxIndex >= 0 && c < CHAR_MAX)
		TextBoxManager::typed((char)c);
}

void Game::handleCursorPosInput(GLFWwindow* window, double xpos, double ypos) {
	if (inEditor && middleMouseDown)
		moveViewOrigin((float)xpos - pointerPos.x, (float)ypos - pointerPos.y);

	pointerPos = {(float)xpos, (float)ypos};

	if (inEditor) {
		if (action == ACTION_NONE)
			pointerBall.pos = getPointerWorldPos(pointerPos);
		else
			updateAction();

		pointerBallWide.pos = pointerBall.pos;
	}

	ButtonManager::pointerPos = pixelsToYNorm((float)xpos, (float)ypos, WINDOW_WIDTH, WINDOW_HEIGHT);
	TextBoxManager::pointerPosX = ButtonManager::pointerPos.x;
}

void Game::handleCursorEnterEvent(GLFWwindow* window, int entered) {
	if (entered)
		Cursor::visible = true;
	else {
		Cursor::visible = false;
		handleCursorPosInput(window, -1, -1);
	}
}

void Game::handleMouseButtonInput(GLFWwindow* window, int mouseButton, int mouseButtonAction, int mods) {
	switch (mouseButton) {
	case GLFW_MOUSE_BUTTON_LEFT:
		if (mouseButtonAction == GLFW_PRESS) {
			leftMouseDown = true;

			int oldFocusedBoxIndex = TextBoxManager::focusedBoxIndex;

			bool buttonPressed = ButtonManager::pressed();

			if (inEditor) {
				if (oldFocusedBoxIndex >= 0 && !(buttonPressed && TextBoxManager::focusedBoxIndex >= 0)) {
					TextBoxManager::focusedBoxIndex = -1;
					if (TextBoxManager::anythingChanged)
						finishAction();
				}

				if ((!buttonPressed && action == ACTION_NONE) || (buttonPressed && action == ACTION_SHAPE))
					startAction();
				else if (action != ACTION_NONE) {
					if (rightMouseDown)
						cancelAction();
					else
						finishAction();
				}
			}
		} else if (mouseButtonAction == GLFW_RELEASE && leftMouseDown) {
			leftMouseDown = false;
			ButtonManager::released();
			if (inEditor && action != ACTION_NONE)
				finishAction();
		}
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (mouseButtonAction == GLFW_PRESS) {
			rightMouseDown = true;
			if (inEditor) {
				if (TextBoxManager::focusedBoxIndex >= 0) {
					TextBoxManager::focusedBoxIndex = -1;
					if (TextBoxManager::anythingChanged)
						cancelAction();
				}

				if (action == ACTION_NONE)
					startAction();
				else
					cancelAction();
			}
		} else if (mouseButtonAction == GLFW_RELEASE && rightMouseDown) {
			rightMouseDown = false;
			if (inEditor && action != ACTION_NONE)
				finishAction();
		}
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		if (mouseButtonAction == GLFW_PRESS)
			middleMouseDown = true;
		else if (mouseButtonAction == GLFW_RELEASE)
			middleMouseDown = false;
		break;
	}
}

void Game::handleScrollInput(GLFWwindow* window, double xoffset, double yoffset) {
	if (!inEditor && viewLocked)
		return;

	float mult = 1.0f;

	if (yoffset > 0)
		mult = 0.9f; // Zoom in
	else if (yoffset < 0)
		mult = 1.1111111111f; // Zoom out

	mult = std::clamp(viewDist * mult, baseViewDist * 0.1f, baseViewDist * 10.0f) / viewDist;

	viewDist *= mult;

	viewOrigin = viewOrigin * mult + getPointerWorldPos(pointerPos) * (1 - mult);

	updateHalfHeight();

	if (inEditor)
		updateAllOutlines();
}

void Game::updateCursor() {
	if (ButtonManager::focusedButtonIndex >= 0) {
		if (ButtonManager::buttons[ButtonManager::focusedButtonIndex].affectsCursor)
			Cursor::tex = Cursor::handTex;
		else
			Cursor::tex = Cursor::arrowTex;
		Cursor::angle = 0;
	} else {
		int i = focus;
		if (action == ACTION_SHAPE && actionMod == ACTION_MOD_MINOR)
			i += checkBallObstacleCollision(&pointerBallWide, false, false, true) * (int)obstacles.size();
		else {
			i = checkBallObstacleCollision(&pointerBallWide, true, true);
		}

		if (i >= 0 && action == ACTION_NONE || (action == ACTION_SHAPE && actionMod == ACTION_MOD_MINOR)) {
			Cursor::tex = Cursor::resizeTex;
			if (i < obstacles.size()) {
				if (obstacles[i].getIsStraight()) {
					Cursor::angle = obstacles[i].getAngle();
				} else {
					vec3 diff = pointerBallWide.pos - obstacles[i].getPos();
					Cursor::angle = PI * 0.5f + atanf(diff.z / diff.y);
				}
			} else {
				if (i >= obstacles.size() * 2) { // End
					i -= (int)obstacles.size() * 2;
					vec3 diff = pointerBallWide.pos - (obstacles[i].getPos() + obstacles[i].getRot() * obstacles[i].getEnd());
					Cursor::angle = PI * 0.5f + atanf(diff.z / diff.y);
				} else { // Start
					i -= (int)obstacles.size();
					vec3 diff = pointerBallWide.pos - (obstacles[i].getPos() + obstacles[i].getRot() * obstacles[i].getStart());
					Cursor::angle = PI * 0.5f + atanf(diff.z / diff.y);
				}
			}
		} else if (action == ACTION_ROTATE || action == ACTION_SCALE) {
			Cursor::tex = Cursor::resizeTex;
			vec3 focusPos = focus == MAX_OBSTACLES ? ball.pos : obstacles[focus].getPos();
			vec3 diff = pointerBallWide.pos - focusPos;
			Cursor::angle = (action == ACTION_SCALE ? PI * 0.5f : 0) + atanf(diff.z / diff.y);
		} else {
			Cursor::tex = Cursor::arrowTex;
			Cursor::angle = 0;
		}
	}
}

#else

vec2 prevPos;
vec2 tapPos;
bool downOnButton;
bool pinch;
float prevPinchDistSq;
vec2 prevPinchPos;

void Game::handleInput() {
	// handle all queued inputs
	auto* inputBuffer = android_app_swap_input_buffers(androidApp);
	if (!inputBuffer) {
		// no inputs yet.
		return;
	}

	// handle motion events (motionEventsCounts can be 0).
	for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
		GameActivityMotionEvent& motionEvent = inputBuffer->motionEvents[i];
		int androidAction = motionEvent.action;

		// Find the pointer index, mask and bit shift to turn it into a readable value.
		int pointerIndex = (androidAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

		// get the x and y position of this event if it is not ACTION_MOVE.
		GameActivityPointerAxes& pointer = motionEvent.pointers[pointerIndex];
		vec2 pos = {GameActivityPointerAxes_getX(&pointer), GameActivityPointerAxes_getY(&pointer)};

		// determine the androidAction type and process the event accordingly.
		switch (androidAction & AMOTION_EVENT_ACTION_MASK) {
		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			if (motionEvent.pointerCount == 2) {
				pinch = true;
				prevPinchDistSq = (pos - prevPos).lengthSq();
				prevPinchPos = (prevPos + pos) * 0.5f;
			} else {
				prevPos = tapPos = pos;
				ButtonManager::pointerPos = pixelsToYNorm(pos, WINDOW_WIDTH, WINDOW_HEIGHT);
				downOnButton = ButtonManager::pressed();
			}
			break;
		case AMOTION_EVENT_ACTION_CANCEL:
			// treat the CANCEL as an UP event: doing nothing in the app, except
			// removing the pointer from the cache if pointers are locally saved.
			// code pass through on purpose.
		case AMOTION_EVENT_ACTION_UP:
		case AMOTION_EVENT_ACTION_POINTER_UP:
			if (motionEvent.pointerCount == 2 && pinch) {
				pinch = false;
				if (pointerIndex == 0)
					prevPos = {GameActivityPointerAxes_getX(&motionEvent.pointers[1]), GameActivityPointerAxes_getY(&motionEvent.pointers[1])};
			} else {
				ButtonManager::released();
				float minDim = fminf(WINDOW_WIDTH, WINDOW_HEIGHT);
				if (!downOnButton && (pos - tapPos).lengthSq() < minDim * minDim * 0.000025) toggle(); // Counts as a tap if finger has moved less than 5% of smallest window dimension
			}
			break;
		case AMOTION_EVENT_ACTION_MOVE:
			// There is no pointer index for ACTION_MOVE, only a snapshot of
			// all active pointers; app needs to cache previous active pointers
			// to figure out which ones are actually moved.
			pointer = motionEvent.pointers[0];
			pos = {GameActivityPointerAxes_getX(&pointer), GameActivityPointerAxes_getY(&pointer)};
			if (pinch) {
				pointer = motionEvent.pointers[1];
				vec2 pos2 = {GameActivityPointerAxes_getX(&pointer), GameActivityPointerAxes_getY(&pointer)};

				float distSq = (pos - pos2).lengthSq();
				float scale = sqrtf(prevPinchDistSq / distSq);
				prevPinchDistSq = distSq;
				viewDist *= scale;

				vec2 pinchPos = (pos + pos2) * 0.5f;
				vec2 delta = pinchPos - prevPinchPos;
				prevPinchPos = pinchPos;
				moveViewOrigin(delta.x, delta.y);

				prevPos = pos;
			} else {
				if (ButtonManager::isPressed)
					ButtonManager::pointerPos = pixelsToYNorm(pos, WINDOW_WIDTH, WINDOW_HEIGHT);
				else {
					rotateView(pos - prevPos);
					prevPos = pos;
				}
			}
			break;
		default:
			break;
		}
	}
	// clear the motion input count in this buffer for main thread to re-use.
	android_app_clear_motion_events(inputBuffer);

	// handle input key events.
	for (int i = 0; i < inputBuffer->keyEventsCount; i++) {
		GameActivityKeyEvent& keyEvent = inputBuffer->keyEvents[i];
		switch (keyEvent.action) {
		case AKEY_EVENT_ACTION_DOWN:
			break;
		case AKEY_EVENT_ACTION_UP:
			if (keyEvent.keyCode == AKEYCODE_BACK) {
				//exit(EXIT_SUCCESS);
				GameActivity_finish(androidApp->activity);
			}
			break;
		}
	}
	// clear the key input count too.
	android_app_clear_key_events(inputBuffer);
}

#endif


short Game::focus;
void Game::findNewFocus() {
	for (short i = MAX_OBSTACLES; i >= 0; i--) {
		if (selection[i]) {
			focus = i;
			return;
		}
	}
	focus = -1;
}
bool Game::selection[] = {false};
void Game::selectObject(short index) {
	focus = index;
	if (!ctrlDown && !selection[index]) // Deselect all other objects
		memset(selection, false, sizeof(selection));
	selection[index] = true;
}
void Game::deselectObject(short index) {
	selection[index] = false;
	if (focus == index)
		findNewFocus();
}

bool Game::limiting[] = {false};

bool Game::ctrlDown, Game::shiftDown, Game::altDown;
bool Game::leftMouseDown, Game::middleMouseDown, Game::rightMouseDown;

byte Game::action;
byte Game::actionMod;
bool Game::actionIsStateless;
vec2 Game::actionStartPointerPos;
vec3 Game::actionStartPointerWorldPos;
vec3 Game::actionVector;
short Game::pressedObIndex;
float Game::initialPointerObDist;

static float getPointerObDist(const Obstacle* g, const vec3& pointerPos, byte section) { // Section: 0 = middle, 1 = start, 2 = end
	if (section == 1) {                                                                  // Start
		vec3 absStart = g->getPos() + g->getRot() * g->getStart();
		return (pointerPos - absStart).length();
	} else if (section == 2) { // End
		vec3 absEnd = g->getPos() + g->getRot() * g->getEnd();
		return (pointerPos - absEnd).length();
	} else { // Middle
		if (g->getIsStraight()) {
			PlaneDefinition plane = g->getTopPlane();
			float dist = dot(plane.normal, pointerPos) - plane.dotProduct + g->getMinorRadius();
			return fabsf(dist);
		} else {
			float toCentre = (pointerPos - g->getPos()).length();
			return fabsf(toCentre - g->getMajorRadius());
		}
	}
}

void Game::startAction() {
	actionStartPointerPos = pointerPos;
	actionStartPointerWorldPos = pointerBall.pos;
	actionVector.set(0);

	memset(limiting, false, sizeof(limiting));

	// Triggered by pointer down, so action not yet known - depends on pointer position & button
	if (action == ACTION_NONE) {
		bool pressedOnRim = false;

		if ((ball.pos - actionStartPointerWorldPos).lengthSq() < 1) // Pointer down on ball
			pressedObIndex = MAX_OBSTACLES;
		else { // Check if pointer down on obstacle
			short rimIndex = checkBallObstacleCollision(&pointerBallWide, true, true);
			if (rimIndex >= 0) {
				pressedObIndex = (short)(rimIndex % obstacles.size());
				pressedOnRim = true;
				initialPointerObDist = getPointerObDist(&obstacles[pressedObIndex], actionStartPointerWorldPos, rimIndex / (int)obstacles.size());
			} else
				pressedObIndex = checkBallObstacleCollision(&pointerBall, true);
		}

		if (pressedObIndex >= 0) { // Pointer down on object
			if (shiftDown) {
				deselectObject(pressedObIndex);
				finishAction();
				return;
			}
			selectObject(pressedObIndex);

			if (leftMouseDown) {
				if (pressedOnRim) { // Mouse down on edge of obstacle
					action = ACTION_SHAPE;
					actionMod = ACTION_MOD_MINOR;
				} else {
					action = ACTION_MOVE;
					actionMod = ACTION_MOD_FREE;
					actionIsStateless = altDown;
				}
			} else if (rightMouseDown) {
				action = ACTION_ROTATE;
				actionMod = ACTION_MOD_INDIVIDUAL;
			}
		} else // Pointer down on empty space
			action = ACTION_SELECT;

	} else {
		if (action == ACTION_SHAPE) {
			if (shiftDown) {
				deselectObject(pressedObIndex); // Index will always be valid since a blob was pressed
				finishAction();
				return;
			}
			selectObject(pressedObIndex);
		} else if (focus < 0) { // Trying to start an action with nothing selected
			action = ACTION_NONE;
			actionMod = ACTION_MOD_NONE;
			return;
		}
	}

	if (actionMod == ACTION_MOD_NONE) {
		if (action == ACTION_SELECT) {
			if (shiftDown)
				actionMod = ACTION_MOD_SUBTRACT;
			else if (ctrlDown)
				actionMod = ACTION_MOD_ADD;
			else
				actionMod = ACTION_MOD_REPLACE;
		}
	} else if (ctrlDown) {
		if (actionMod == ACTION_MOD_START)
			actionMod = ACTION_MOD_START_MIRRORED;
		else if (actionMod == ACTION_MOD_END)
			actionMod = ACTION_MOD_END_MIRRORED;
	}

	switch (action) {
	case ACTION_SELECT:
		focus = -1;
		selectBoxTL = selectBoxBR = pointerPos;
		boxSelect();
		break;
	case ACTION_SHAPE:
		if (actionMod == ACTION_MOD_MAJOR)
			initialPointerObDist = (actionStartPointerWorldPos - obstacles[focus].getPos()).length();
		break;
	}
}

void Game::finishAction() {
	if (action == ACTION_SELECT && (focus < 0 || !selection[focus]))
		findNewFocus();

	action = ACTION_NONE;
	actionMod = ACTION_MOD_NONE;
	actionIsStateless = altDown;
	blobWasPressed = false;

	if (undoBuffer[bufferIndex].level == level) { // The level itself did not change
		const SelectionState& prevSelectionState = undoBuffer[bufferIndex].selectionStates[selectionBufferIndex];
		bool selectionChanged = prevSelectionState.focus != focus;
		for (int i = 0; i < MAX_OBSTACLES + 1; i++)
			if (selection[i] != prevSelectionState.selection[i]) {
				selectionChanged = true;
				break;
			}
		if (selectionChanged) {
			if (selectionBufferIndex == MAX_SELECTIONS - 1) { // Shunt everything down a place
				for (int i = 0; i < MAX_SELECTIONS - 1; i++)
					undoBuffer[bufferIndex].selectionStates[i] = undoBuffer[bufferIndex].selectionStates[i + 1];
			} else
				selectionBufferIndex++;

			undoBuffer[bufferIndex].selectionStates[selectionBufferIndex].set(selection, focus);
			undoBuffer[bufferIndex].lastSelectionBufferIndex = selectionBufferIndex;
		}
	} else {
		if (bufferIndex == MAX_UNDO - 1) { // Shunt everything down a place
			savedIndex--;
			for (int i = 0; i < MAX_UNDO - 1; i++)
				undoBuffer[i] = undoBuffer[i + 1];
		} else
			bufferIndex++;

		undoBuffer[bufferIndex].set(level, selection, focus);
		lastBufferIndex = bufferIndex;
		selectionBufferIndex = 0;
	}
}

float getAngleDiff(const vec3& oldPos, const vec3& newPos, const vec3& pivot) {
	vec3 oldPosDiff = oldPos - pivot;
	vec3 newPosDiff = newPos - pivot;
	if (!(oldPosDiff.y == 0 && oldPosDiff.z == 0) && !(newPosDiff.y == 0 && newPosDiff.z == 0)) {
		float oldAng = atan2f(oldPosDiff.z, oldPosDiff.y);
		float newAng = atan2f(newPosDiff.z, newPosDiff.y);
		float angDiff = newAng - oldAng;
		if (angDiff < -PI) angDiff += PI * 2;
		else if (angDiff > PI)
			angDiff -= PI * 2;
		return angDiff;
	}
	return 0;
}

float limitRotationByPos(float rotation, const vec3& pivot, const vec3& pos, float minX, float maxX, float minY, float maxY, byte* valPos) {
	const float rotRadiusSq = (pos - pivot).lengthSq();
	mat3 rot;
	rot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, rotation);
	vec3 proposedPos = pivot + rot * (pos - pivot);

	float xLimitedRotation, yLimitedRotation;

	byte xValPos, yValPos;
	xValPos = yValPos = VP_INSIDE;

	if (proposedPos.y == minX || proposedPos.y == maxX)
		xValPos = VP_BOUNDARY;
	else if (proposedPos.y < minX) {
		float x = minX;
		float xRelative = x - pivot.y;
		float yRelative = sqrtf(rotRadiusSq - xRelative * xRelative);
		float y = pivot.z + (rotation < 0 ? -yRelative : yRelative);
		xLimitedRotation = getAngleDiff(pos, {0, x, y}, pivot);
		xValPos = VP_OUTSIDE;
	} else if (proposedPos.y > maxX) {
		float x = maxX;
		float xRelative = x - pivot.y;
		float yRelative = sqrtf(rotRadiusSq - xRelative * xRelative);
		float y = pivot.z + (rotation > 0 ? -yRelative : yRelative);
		xLimitedRotation = getAngleDiff(pos, {0, x, y}, pivot);
		xValPos = VP_OUTSIDE;
	}

	if (proposedPos.z == minY || proposedPos.z == maxY)
		yValPos = VP_BOUNDARY;
	else if (proposedPos.z < minY) {
		float y = minY;
		float yRelative = y - pivot.z;
		float xRelative = sqrtf(rotRadiusSq - yRelative * yRelative);
		float x = pivot.y + (rotation > 0 ? -xRelative : xRelative);
		yLimitedRotation = getAngleDiff(pos, {0, x, y}, pivot);
		yValPos = VP_OUTSIDE;
	} else if (proposedPos.z > maxY) {
		float y = maxY;
		float yRelative = y - pivot.z;
		float xRelative = sqrtf(rotRadiusSq - yRelative * yRelative);
		float x = pivot.y + (rotation < 0 ? -xRelative : xRelative);
		yLimitedRotation = getAngleDiff(pos, {0, x, y}, pivot);
		yValPos = VP_OUTSIDE;
	}

	*valPos = std::max(xValPos, yValPos);

	if (xValPos == VP_OUTSIDE)
		return xLimitedRotation;
	else if (yValPos == VP_OUTSIDE)
		return yLimitedRotation;
	else
		return rotation;
}

float limitScaleFactorByPos(float scaleFactor, const vec3& centre, const vec3& pos, float minX, float maxX, float minY, float maxY, byte* valPos) {
	vec3 posOffset = pos - centre;

	float xLimit = posOffset.y < 0 ? minX : maxX;
	float yLimit = posOffset.z < 0 ? minY : maxY;
	float scaleFactorLimit = fminf((xLimit - centre.y) / posOffset.y, (yLimit - centre.z) / posOffset.z);

	if (scaleFactor == scaleFactorLimit) {
		*valPos = VP_BOUNDARY;
		return scaleFactor;
	} else if (scaleFactor > scaleFactorLimit) {
		*valPos = VP_OUTSIDE;
		return scaleFactorLimit;
	} else {
		*valPos = VP_INSIDE;
		return scaleFactor;
	}
}

void Game::updateAction() {
	vec3 newPos = getPointerWorldPos(pointerPos);

	memset(limiting, false, sizeof(limiting));
	byte valPos;

	const float MINIMUM_BALL_POS_X = 1.0f - level.arenaWidth * 0.5f;
	const float MAXIMUM_BALL_POS_X = -MINIMUM_BALL_POS_X;
	const float MINIMUM_BALL_POS_Y = 1.0f;
	const float MAXIMUM_BALL_POS_Y = level.arenaHeight - 1.0f;

	switch (action) {
	case ACTION_SELECT:
		boxSelect();
		break;
	case ACTION_MOVE: {
		const vec3 xAxis = {0, 1, 0};
		const vec3 yAxis = {0, 0, 1};

		const vec3 rawTranslation = actionMod == ACTION_MOD_UNIT ? actionVector : newPos - actionStartPointerWorldPos;
		vec3 limitedTranslation = rawTranslation;

		vec3 focusedDir;
		if (actionMod == ACTION_MOD_LOCAL_X) {
			if (focus == MAX_OBSTACLES)
				focusedDir = xAxis;
			else
				focusedDir = obstacles[focus].getRot() * xAxis;
		} else if (actionMod == ACTION_MOD_LOCAL_Y) {
			if (focus == MAX_OBSTACLES)
				focusedDir = yAxis;
			else
				focusedDir = obstacles[focus].getRot() * yAxis;
		}
		float limitedTranslationFactor = dot(focusedDir, rawTranslation); // For local axis translations

		if (actionMod == ACTION_MOD_GLOBAL_X)
			limitedTranslation.z = 0;
		else if (actionMod == ACTION_MOD_GLOBAL_Y)
			limitedTranslation.y = 0;

		if (selection[MAX_OBSTACLES]) {
			switch (actionMod) {
			case ACTION_MOD_UNIT:
				[[fallthrough]];
			case ACTION_MOD_FREE: {
				byte vp1;
				limitedTranslation.y = clamp(limitedTranslation.y, MINIMUM_BALL_POS_X - undoBuffer[bufferIndex].level.ballPos.y, MAXIMUM_BALL_POS_X - undoBuffer[bufferIndex].level.ballPos.y, &vp1);
				byte vp2;
				limitedTranslation.z = clamp(limitedTranslation.z, MINIMUM_BALL_POS_Y - undoBuffer[bufferIndex].level.ballPos.z, MAXIMUM_BALL_POS_Y - undoBuffer[bufferIndex].level.ballPos.z, &vp2);
				valPos = std::max(vp1, vp2);
			} break;
			case ACTION_MOD_GLOBAL_X:
				limitedTranslation.y = clamp(limitedTranslation.y, MINIMUM_BALL_POS_X - undoBuffer[bufferIndex].level.ballPos.y, MAXIMUM_BALL_POS_X - undoBuffer[bufferIndex].level.ballPos.y, &valPos);
				break;
			case ACTION_MOD_GLOBAL_Y:
				limitedTranslation.z = clamp(limitedTranslation.z, MINIMUM_BALL_POS_Y - undoBuffer[bufferIndex].level.ballPos.z, MAXIMUM_BALL_POS_Y - undoBuffer[bufferIndex].level.ballPos.z, &valPos);
				break;
			case ACTION_MOD_LOCAL_X:
				limitedTranslationFactor = clamp(limitedTranslationFactor, MINIMUM_BALL_POS_X - undoBuffer[bufferIndex].level.ballPos.y, MAXIMUM_BALL_POS_X - undoBuffer[bufferIndex].level.ballPos.y, &valPos);
				break;
			case ACTION_MOD_LOCAL_Y:
				limitedTranslationFactor = clamp(limitedTranslationFactor, MINIMUM_BALL_POS_Y - undoBuffer[bufferIndex].level.ballPos.z, MAXIMUM_BALL_POS_Y - undoBuffer[bufferIndex].level.ballPos.z, &valPos);
				break;
			}

			if (valPos > VP_INSIDE) // Only have to do single check here since this is first
				limiting[MAX_OBSTACLES] = true;
		}

		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				vec3 dir;
				if (actionMod == ACTION_MOD_LOCAL_X)
					dir = obstacles[i].getRot() * xAxis;
				else if (actionMod == ACTION_MOD_LOCAL_Y)
					dir = obstacles[i].getRot() * yAxis;

				float minDX, maxDX, minDY, maxDY;
				if (actionIsStateless && (obstacles[i].getStateType() == ST_POS || obstacles[i].getStateType() == ST_POS_OSC)) { // Find limits for both positions
					minDX = MINIMUM_POS_X - fminf(undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA.y, undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB.y);
					maxDX = MAXIMUM_POS_X - fmaxf(undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA.y, undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB.y);
					minDY = MINIMUM_POS_Y - fminf(undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA.z, undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB.z);
					maxDY = MAXIMUM_POS_Y - fmaxf(undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA.z, undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB.z);
				} else { // Find limits for the only position
					float xPos, yPos;
					if (obstacles[i].getStateType() == ST_POS || obstacles[i].getStateType() == ST_POS_OSC) {
						xPos = (toggled ? undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB : undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA).y;
						yPos = (toggled ? undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB : undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA).z;
					} else {
						xPos = undoBuffer[bufferIndex].level.obstacleDefinitions[i].initPos.y;
						yPos = undoBuffer[bufferIndex].level.obstacleDefinitions[i].initPos.z;
					}
					minDX = MINIMUM_POS_X - xPos;
					maxDX = MAXIMUM_POS_X - xPos;
					minDY = MINIMUM_POS_Y - yPos;
					maxDY = MAXIMUM_POS_Y - yPos;
				}

				switch (actionMod) {
				case ACTION_MOD_UNIT:
					[[fallthrough]];
				case ACTION_MOD_FREE: {
					byte vp1;
					limitedTranslation.y = clamp(limitedTranslation.y, minDX, maxDX, &vp1);
					byte vp2;
					limitedTranslation.z = clamp(limitedTranslation.z, minDY, maxDY, &vp2);
					valPos = std::max(vp1, vp2);
				} break;
				case ACTION_MOD_GLOBAL_X:
					limitedTranslation.y = clamp(limitedTranslation.y, minDX, maxDX, &valPos);
					break;
				case ACTION_MOD_GLOBAL_Y:
					limitedTranslation.z = clamp(limitedTranslation.z, minDY, maxDY, &valPos);
					break;
				case ACTION_MOD_LOCAL_X:
					[[fallthrough]];
				case ACTION_MOD_LOCAL_Y:
					float minFactor = fmaxf((dir.y < 0 ? maxDX : minDX) / dir.y, (dir.z < 0 ? maxDY : minDY) / dir.z);
					float maxFactor = fminf((dir.y < 0 ? minDX : maxDX) / dir.y, (dir.z < 0 ? minDY : maxDY) / dir.z);
					limitedTranslationFactor = clamp(limitedTranslationFactor, minFactor, maxFactor, &valPos);
					break;
				}

				if (valPos > VP_INSIDE) {
					if (valPos == VP_OUTSIDE)
						memset(limiting, false, sizeof(limiting));
					limiting[i] = true;
				}
			}
		}

		if (selection[MAX_OBSTACLES]) {
			switch (actionMod) {
			case ACTION_MOD_UNIT:
				[[fallthrough]];
			case ACTION_MOD_FREE:
				[[fallthrough]];
			case ACTION_MOD_GLOBAL_X:
				[[fallthrough]];
			case ACTION_MOD_GLOBAL_Y:
				moveBallByFull(limitedTranslation);
				break;
			case ACTION_MOD_LOCAL_X:
				moveBallByFull(xAxis * limitedTranslationFactor);
				break;
			case ACTION_MOD_LOCAL_Y:
				moveBallByFull(yAxis * limitedTranslationFactor);
				break;
			}
		}
		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				if (actionMod == ACTION_MOD_LOCAL_X)
					limitedTranslation = obstacles[i].getRot() * xAxis * limitedTranslationFactor;
				else if (actionMod == ACTION_MOD_LOCAL_Y)
					limitedTranslation = obstacles[i].getRot() * yAxis * limitedTranslationFactor;

				moveObstacleByFull(i, limitedTranslation);
			}
		}
	} break;
	case ACTION_ROTATE: {
		const vec3 pivot = focus == MAX_OBSTACLES ? ball.pos : obstacles[focus].getPos();
		const float rawRotation = getAngleDiff(pointerBall.pos, newPos, pivot);
		float limitedRotation = rawRotation;

		if (selection[MAX_OBSTACLES] && actionMod == ACTION_MOD_GROUP) {
			limitedRotation = limitRotationByPos(limitedRotation, pivot, ball.pos, MINIMUM_BALL_POS_X, MAXIMUM_BALL_POS_X, MINIMUM_BALL_POS_Y, MAXIMUM_BALL_POS_Y, &valPos);

			if (valPos > VP_INSIDE) // Only have to do single check here since this is first
				limiting[MAX_OBSTACLES] = true;
		}

		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				valPos = VP_INSIDE;

				if (obstacles[i].getStateType() == ST_ANG || obstacles[i].getStateType() == ST_ANG_OSC) { // Only these state types have angle limits
					if (actionIsStateless) {
						if (obstacles[i].getStateType() == ST_ANG) {
							byte vp1;
							limitedRotation = clamp(obstacles[i].getStateA().x + limitedRotation, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp1) - obstacles[i].getStateA().x;
							byte vp2;
							limitedRotation = clamp(obstacles[i].getStateB().x + limitedRotation, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp2) - obstacles[i].getStateB().x;
							valPos = std::max(vp1, vp2);
						} else if (obstacles[i].getStateType() == ST_ANG_OSC) {
							byte vp1;
							limitedRotation = clamp(obstacles[i].getStateA().y + limitedRotation, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp1) - obstacles[i].getStateA().y;
							byte vp2;
							limitedRotation = clamp(obstacles[i].getStateB().y + limitedRotation, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp2) - obstacles[i].getStateB().y;
							valPos = std::max(vp1, vp2);
						}
					} else
						limitedRotation = clamp(limitedRotation, MINIMUM_ANGLE - obstacles[i].getAngle(), MAXIMUM_ANGLE - obstacles[i].getAngle(), &valPos);
				}

				if (actionMod == ACTION_MOD_GROUP) {
					if (actionIsStateless && obstacles[i].getStateType() == ST_POS || obstacles[i].getStateType() == ST_POS_OSC) {
						vec3 stateAPos = obstacles[i].getStateA();
						stateAPos.x = 0;
						byte vp1;
						limitedRotation = limitRotationByPos(limitedRotation, pivot, stateAPos, MINIMUM_POS_X, MAXIMUM_POS_X, MINIMUM_POS_Y, MAXIMUM_POS_Y, &vp1);
						vec3 stateBPos = obstacles[i].getStateB();
						stateBPos.x = 0;
						byte vp2;
						limitedRotation = limitRotationByPos(limitedRotation, pivot, stateBPos, MINIMUM_POS_X, MAXIMUM_POS_X, MINIMUM_POS_Y, MAXIMUM_POS_Y, &vp2);
						valPos = std::max(vp1, vp2);
					} else {
						byte vp;
						limitedRotation = limitRotationByPos(limitedRotation, pivot, obstacles[i].getPos(), MINIMUM_POS_X, MAXIMUM_POS_X, MINIMUM_POS_Y, MAXIMUM_POS_Y, &vp);
						valPos = std::max(valPos, vp);
					}
				}

				if (valPos > VP_INSIDE) {
					if (valPos == VP_OUTSIDE)
						memset(limiting, false, sizeof(limiting));
					limiting[i] = true;
				}
			}
		}

		mat3 limitedRot;
		limitedRot.R_VecAndAngle(OBSTACLE_ROTATION_AXIS, limitedRotation);

		if (selection[MAX_OBSTACLES] && actionMod == ACTION_MOD_GROUP)
			moveBallTo(pivot + limitedRot * (ball.pos - pivot));

		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				if (actionMod == ACTION_MOD_GROUP) {
					if (obstacles[i].getStateType() == ST_POS || obstacles[i].getStateType() == ST_POS_OSC) {
						if (!toggled || actionIsStateless) {
							vec3 stateAPos = obstacles[i].getStateA();
							stateAPos.x = 0;
							obstacles[i].setStateA(pivot + limitedRot * (stateAPos - pivot));
							obstacles[i].setInitPos({0, obstacles[i].getStateA().y, obstacles[i].getStateA().z});
						}
						if (toggled || actionIsStateless) {
							vec3 stateBPos = obstacles[i].getStateB();
							stateBPos.x = 0;
							obstacles[i].setStateB(pivot + limitedRot * (stateBPos - pivot));
						}
						vec3 currentState = toggled ? obstacles[i].getStateB() : obstacles[i].getStateA();
						obstacles[i].setPos({0, currentState.y, currentState.z});
					} else {
						vec3 translatedPos = pivot + limitedRot * (obstacles[i].getInitPos() - pivot);
						obstacles[i].setPos(translatedPos);
						obstacles[i].setInitPos(translatedPos);
					}
				}

				rotateObstacleBy(i, limitedRotation);

				if (obstacles[i].getStateType() != ST_STATIC && obstacles[i].getStateType() != ST_ANG_VEL)
					obstacles[i].createDomainModel();
			}
		}
	} break;
	case ACTION_SCALE: {
		vec3 centre = focus == MAX_OBSTACLES ? ball.pos : obstacles[focus].getPos();
		const float rawScaleFactor = sqrtf((newPos - centre).lengthSq() / (actionStartPointerWorldPos - centre).lengthSq());
		float limitedScaleFactor = rawScaleFactor;

		valPos = VP_INSIDE;

		if (selection[MAX_OBSTACLES] && actionMod == ACTION_MOD_GROUP) {
			limitedScaleFactor = limitScaleFactorByPos(limitedScaleFactor, centre, undoBuffer[bufferIndex].level.ballPos, MINIMUM_BALL_POS_X, MAXIMUM_BALL_POS_X, MINIMUM_BALL_POS_Y, MAXIMUM_BALL_POS_Y, &valPos);

			if (valPos > VP_INSIDE) // Only have to do single check here since this is first
				limiting[MAX_OBSTACLES] = true;
		}

		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				switch (actionMod) {
				case ACTION_MOD_GROUP:
					if (obstacles[i].getStateType() == ST_POS || obstacles[i].getStateType() == ST_POS_OSC) {
						byte vp1 = VP_INSIDE;
						if (!toggled || actionIsStateless) {
							vec3 stateAPos = undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA;
							stateAPos.x = 0;
							limitedScaleFactor = limitScaleFactorByPos(limitedScaleFactor, centre, stateAPos, MINIMUM_POS_X, MAXIMUM_POS_X, MINIMUM_POS_Y, MAXIMUM_POS_Y, &vp1);
						}
						byte vp2 = VP_INSIDE;
						if (toggled || actionIsStateless) {
							vec3 stateBPos = undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB;
							stateBPos.x = 0;
							limitedScaleFactor = limitScaleFactorByPos(limitedScaleFactor, centre, stateBPos, MINIMUM_POS_X, MAXIMUM_POS_X, MINIMUM_POS_Y, MAXIMUM_POS_Y, &vp2);
						}
						valPos = std::max(vp1, vp2);
					} else
						limitedScaleFactor = limitScaleFactorByPos(limitedScaleFactor, centre, undoBuffer[bufferIndex].level.obstacleDefinitions[i].initPos, MINIMUM_POS_X, MAXIMUM_POS_X, MINIMUM_POS_Y, MAXIMUM_POS_Y, &valPos);
					[[fallthrough]];
				case ACTION_MOD_INDIVIDUAL: {
					float minorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius;
					byte vp1;
					limitedScaleFactor = clamp(limitedScaleFactor * minorRadius, MINIMUM_MINOR_RADIUS, MAXIMUM_MINOR_RADIUS, &vp1) / minorRadius;
					float majorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].majorRadius;
					byte vp2;
					limitedScaleFactor = clamp(limitedScaleFactor * majorRadius, 0.0f, MAXIMUM_MAJOR_RADIUS, &vp2) / majorRadius;
					valPos = std::max(valPos, std::max(vp1, vp2));
				} break;
				case ACTION_MOD_MINOR: {
					float minorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius;
					float maxMinorRadius = obstacles[i].getIsStraight() ? MAXIMUM_MINOR_RADIUS : fminf(MAXIMUM_MINOR_RADIUS, obstacles[i].getMajorRadius());
					limitedScaleFactor = clamp(limitedScaleFactor * minorRadius, MINIMUM_MINOR_RADIUS, maxMinorRadius, &valPos) / minorRadius;
				} break;
				case ACTION_MOD_MAJOR: {
					float minorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius;
					float majorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].majorRadius;
					float minMajorRadius = obstacles[i].getIsStraight() ? 0 : minorRadius;
					limitedScaleFactor = clamp(limitedScaleFactor * majorRadius, minMajorRadius, MAXIMUM_MAJOR_RADIUS, &valPos) / majorRadius;
				} break;
				}

				if (valPos > VP_INSIDE) {
					if (valPos == VP_OUTSIDE)
						memset(limiting, false, sizeof(limiting));
					limiting[i] = true;
				}
			}
		}

		if (selection[MAX_OBSTACLES] && actionMod == ACTION_MOD_GROUP)
			moveBallTo(centre + (undoBuffer[bufferIndex].level.ballPos - centre) * limitedScaleFactor);

		for (short i = 0; i < (short)obstacles.size(); i++) {
			if (selection[i]) {
				switch (actionMod) {
				case ACTION_MOD_GROUP:
					if (obstacles[i].getStateType() == ST_POS || obstacles[i].getStateType() == ST_POS_OSC) {
						if (!toggled || actionIsStateless) {
							vec3 stateAPos = undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateA;
							stateAPos.x = 0;
							obstacles[i].setStateA(centre + (stateAPos - centre) * limitedScaleFactor);
							obstacles[i].setInitPos({0, obstacles[i].getStateA().y, obstacles[i].getStateA().z});
						}
						if (toggled || actionIsStateless) {
							vec3 stateBPos = undoBuffer[bufferIndex].level.obstacleDefinitions[i].stateB;
							stateBPos.x = 0;
							obstacles[i].setStateB(centre + (stateBPos - centre) * limitedScaleFactor);
						}
						vec3 currentState = toggled ? obstacles[i].getStateB() : obstacles[i].getStateA();
						obstacles[i].setPos({0, currentState.y, currentState.z});
					} else {
						vec3 translatedPos = centre + (undoBuffer[bufferIndex].level.obstacleDefinitions[i].initPos - centre) * limitedScaleFactor;
						obstacles[i].setPos(translatedPos);
						obstacles[i].setInitPos(translatedPos);
					}
					[[fallthrough]];
				case ACTION_MOD_INDIVIDUAL:
					obstacles[i].setMinorRadius(undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius * limitedScaleFactor);
					obstacles[i].setMajorRadius(undoBuffer[bufferIndex].level.obstacleDefinitions[i].majorRadius * limitedScaleFactor);
					obstacles[i].setStart(undoBuffer[bufferIndex].level.obstacleDefinitions[i].start * limitedScaleFactor);
					obstacles[i].setEnd(undoBuffer[bufferIndex].level.obstacleDefinitions[i].end * limitedScaleFactor);
					break;
				case ACTION_MOD_MINOR:
				    obstacles[i].setMinorRadius(undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius * limitedScaleFactor);
				    break;
				case ACTION_MOD_MAJOR:
				    obstacles[i].setMajorRadius(undoBuffer[bufferIndex].level.obstacleDefinitions[i].majorRadius * limitedScaleFactor);
				    obstacles[i].setStart(undoBuffer[bufferIndex].level.obstacleDefinitions[i].start * limitedScaleFactor);
				    obstacles[i].setEnd(undoBuffer[bufferIndex].level.obstacleDefinitions[i].end * limitedScaleFactor);
					break;
				}

				obstacles[i].createAllModels();
			}
		}
	} break;
	case ACTION_SHAPE: {
		if (actionMod == ACTION_MOD_MINOR) {
			int section = checkBallObstacleCollision(&pointerBallWide, false, false, true);
			float pointerObDist = getPointerObDist(&obstacles[focus], newPos, section);
			float rawExtension = pointerObDist - initialPointerObDist;

			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i]) {
					float minorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius;
					if (obstacles[i].getIsStraight())
						rawExtension = clamp(minorRadius + rawExtension, MINIMUM_MINOR_RADIUS, MAXIMUM_MINOR_RADIUS, &valPos) - minorRadius;
					else
						rawExtension = clamp(minorRadius + rawExtension, MINIMUM_MINOR_RADIUS, fminf(obstacles[i].getMajorRadius(), MAXIMUM_MINOR_RADIUS), &valPos) - minorRadius;

					if (valPos > VP_INSIDE) {
						if (valPos == VP_OUTSIDE)
							memset(limiting, false, sizeof(limiting));
						limiting[i] = true;
					}
				}
			}

			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i]) {
					obstacles[i].setMinorRadius(fmaxf(MINIMUM_MINOR_RADIUS, undoBuffer[bufferIndex].level.obstacleDefinitions[i].minorRadius + rawExtension));

					obstacles[i].createAllModels();
				}
			}

			break;
		}

		if (obstacles[focus].getIsStraight()) {
			const vec3 focusedDir = obstacles[focus].getRot() * vec3(0, 1, 0);
			const float rawExtension = dot(newPos - actionStartPointerWorldPos, focusedDir);
			float limitedExtension = rawExtension;

			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && obstacles[i].getIsStraight()) {
					float oldStartY = undoBuffer[bufferIndex].level.obstacleDefinitions[i].start.y;
					byte vp1 = VP_INSIDE;
					switch (actionMod) {
					case ACTION_MOD_START:
						[[fallthrough]];
					case ACTION_MOD_START_MIRRORED:
						limitedExtension = clamp(oldStartY + limitedExtension, 0.0f, MAXIMUM_MAJOR_RADIUS, &vp1) - oldStartY;
						break;
					case ACTION_MOD_END_MIRRORED:
						limitedExtension = oldStartY - clamp(oldStartY - limitedExtension, 0.0f, MAXIMUM_MAJOR_RADIUS, &vp1);
						break;
					}

					float oldEndY = undoBuffer[bufferIndex].level.obstacleDefinitions[i].end.y;
					byte vp2 = VP_INSIDE;
					switch (actionMod) {
					case ACTION_MOD_END:
						[[fallthrough]];
					case ACTION_MOD_END_MIRRORED:
						limitedExtension = clamp(oldEndY + limitedExtension, -MAXIMUM_MAJOR_RADIUS, 0.0f, &vp2) - oldEndY;
						break;
					case ACTION_MOD_START_MIRRORED:
						limitedExtension = oldEndY - clamp(oldEndY - limitedExtension, -MAXIMUM_MAJOR_RADIUS, 0.0f, &vp2);
						break;
					}

					if (actionMod == ACTION_MOD_SLIDE) {
						limitedExtension = clamp(oldStartY + limitedExtension, 0.0f, MAXIMUM_MAJOR_RADIUS, &vp1) - oldStartY;
						limitedExtension = clamp(oldEndY + limitedExtension, -MAXIMUM_MAJOR_RADIUS, 0.0f, &vp2) - oldEndY;
					}

					valPos = std::max(vp1, vp2);

					if (valPos > VP_INSIDE) {
						if (valPos == VP_OUTSIDE)
							memset(limiting, false, sizeof(limiting));
						limiting[i] = true;
					}
				}
			}

			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && obstacles[i].getIsStraight()) {
					float oldStartY = undoBuffer[bufferIndex].level.obstacleDefinitions[i].start.y;
					switch (actionMod) {
					case ACTION_MOD_START:
						[[fallthrough]];
					case ACTION_MOD_START_MIRRORED:
						obstacles[i].setStartY(oldStartY + limitedExtension);
						break;
					case ACTION_MOD_END_MIRRORED:
						obstacles[i].setStartY(oldStartY - limitedExtension);
						break;
					}

					float oldEndY = undoBuffer[bufferIndex].level.obstacleDefinitions[i].end.y;
					switch (actionMod) {
					case ACTION_MOD_END:
						[[fallthrough]];
					case ACTION_MOD_END_MIRRORED:
						obstacles[i].setEndY(oldEndY + limitedExtension);
						break;
					case ACTION_MOD_START_MIRRORED:
						obstacles[i].setEndY(oldEndY - limitedExtension);
						break;
					}

					if (actionMod == ACTION_MOD_SLIDE) {
						obstacles[i].setStartY(oldStartY + limitedExtension);
						obstacles[i].setEndY(oldEndY + limitedExtension);
					}

					obstacles[i].setMajorRadius(sqrtf(fmaxf(obstacles[i].getStart().lengthSq(), obstacles[i].getEnd().lengthSq())));

					obstacles[i].createAllModels();
				}
			}
		} else {
			if (actionMod == ACTION_MOD_MAJOR) {
				float rawExpansion = (newPos - obstacles[focus].getPos()).length() - initialPointerObDist;
				float limitedExpansion = rawExpansion;

				for (short i = 0; i < (short)obstacles.size(); i++) {
					if (selection[i] && !obstacles[i].getIsStraight()) {
						float oldMajorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].majorRadius;
						limitedExpansion = clamp(oldMajorRadius + limitedExpansion, obstacles[i].getMinorRadius(), MAXIMUM_MAJOR_RADIUS, &valPos) - oldMajorRadius;

						if (valPos > VP_INSIDE) {
							if (valPos == VP_OUTSIDE)
								memset(limiting, false, sizeof(limiting));
							limiting[i] = true;
						}
					}
				}

				for (short i = 0; i < (short)obstacles.size(); i++) {
					if (selection[i] && !obstacles[i].getIsStraight()) {
						float oldMajorRadius = undoBuffer[bufferIndex].level.obstacleDefinitions[i].majorRadius;
						obstacles[i].setMajorRadius(oldMajorRadius + limitedExpansion);
						obstacles[i].setStart(obstacles[i].getStart() / obstacles[i].getStart().length() * obstacles[i].getMajorRadius());
						vec3 start = obstacles[i].getStart();
						obstacles[i].setEnd({start.x, -start.y, start.z});

						obstacles[i].createAllModels();
					}
				}
			} else {
				const float rawAngle = getAngleDiff(pointerBall.pos, newPos, obstacles[focus].getPos());
				float limitedAngle = rawAngle;
				if (actionMod == ACTION_MOD_START || actionMod == ACTION_MOD_END)
					limitedAngle *= 0.5f;

				for (short i = 0; i < (short)obstacles.size(); i++) {
					if (selection[i] && !obstacles[i].getIsStraight()) {
						valPos = VP_INSIDE;

						if (actionMod == ACTION_MOD_START || actionMod == ACTION_MOD_END) {
							if (obstacles[i].getStateType() == ST_ANG) {
								byte vp1;
								limitedAngle = clamp(obstacles[i].getStateA().x + limitedAngle, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp1) - obstacles[i].getStateA().x;
								byte vp2;
								limitedAngle = clamp(obstacles[i].getStateB().x + limitedAngle, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp2) - obstacles[i].getStateB().x;
								valPos = std::max(vp1, vp2);
							} else if (obstacles[i].getStateType() == ST_ANG_OSC) {
								byte vp1;
								limitedAngle = clamp(obstacles[i].getStateA().y + limitedAngle, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp1) - obstacles[i].getStateA().y;
								byte vp2;
								limitedAngle = clamp(obstacles[i].getStateB().y + limitedAngle, MINIMUM_ANGLE, MAXIMUM_ANGLE, &vp2) - obstacles[i].getStateB().y;
								valPos = std::max(vp1, vp2);
							}
						}

						byte vp;
						switch (actionMod) {
							case ACTION_MOD_START:
								[[fallthrough]];
						    case ACTION_MOD_START_MIRRORED:
							    limitedAngle = clamp(limitedAngle - obstacles[i].getHalfArcAngle(), -PI, 0.0f, &vp) + obstacles[i].getHalfArcAngle();
						    break;
						    case ACTION_MOD_END:
							    [[fallthrough]];
						    case ACTION_MOD_END_MIRRORED:
							    limitedAngle = clamp(limitedAngle + obstacles[i].getHalfArcAngle(), 0.0f, PI, &vp) - obstacles[i].getHalfArcAngle();
						    break;
						}

						valPos = std::max(valPos, vp);

						if (valPos > VP_INSIDE) {
							if (valPos == VP_OUTSIDE)
								memset(limiting, false, sizeof(limiting));
							limiting[i] = true;
						}
					}
				}

				bool temp = actionIsStateless;
				actionIsStateless = true;

				for (short i = 0; i < (short)obstacles.size(); i++) {
					if (selection[i] && !obstacles[i].getIsStraight()) {
						float halfArcAngle;

						switch (actionMod) {
						case ACTION_MOD_START:
							rotateObstacleBy(i, limitedAngle);
							[[fallthrough]];
						case ACTION_MOD_START_MIRRORED: {
							halfArcAngle = obstacles[i].getHalfArcAngle() - limitedAngle;
						} break;
						case ACTION_MOD_END:
							rotateObstacleBy(i, limitedAngle);
							[[fallthrough]];
						case ACTION_MOD_END_MIRRORED: {
							halfArcAngle = obstacles[i].getHalfArcAngle() + limitedAngle;
						} break;
						}

						obstacles[i].setHalfArcAngle(halfArcAngle);

						vec3 start = vec3(0, sinf(halfArcAngle), cosf(halfArcAngle)) * obstacles[i].getMajorRadius();
						obstacles[i].setStart(start);
						obstacles[i].setEnd({start.x, -start.y, start.z});

						obstacles[i].createAllModels();
					}
				}

				actionIsStateless = temp;
			}
		}
	} break;
	}

	pointerBall.pos = newPos;
}

void Game::changeAction() {
	blobWasPressed = false;
	loadLevel(undoBuffer[bufferIndex].level, false);
	pointerBall.pos = actionStartPointerWorldPos;
	updateAction();
}

void Game::cancelAction() {
	action = ACTION_NONE;
	actionMod = ACTION_MOD_NONE;
	actionIsStateless = altDown;
	blobWasPressed = false;

	arenaWidth = level.arenaWidth;
	arenaHeight = level.arenaHeight;

	loadLevel(undoBuffer[bufferIndex].level, false);
	SelectionState* ss = &undoBuffer[bufferIndex].selectionStates[selectionBufferIndex];
	memcpy(selection, ss->selection, sizeof(selection));
	focus = undoBuffer[bufferIndex].selectionStates[selectionBufferIndex].focus;

	if (arenaWidth != level.arenaWidth || arenaHeight != level.arenaHeight)
		resetEditorView(); // For cancelling changes to arena size
}

vec2 Game::selectBoxTL, Game::selectBoxBR;

static vec2 quadraticFormula(float a, float b, float c) {
	float d = b * b - 4 * a * c;
	if (d < 0) return {NAN, NAN};
	float sqrtD = sqrt(d);
	return vec2(-b + sqrtD, -b - sqrtD) * 0.5f / a;
}

void Game::boxSelect() {
	selectBoxTL = actionStartPointerPos;
	selectBoxBR = pointerPos;
	if (selectBoxTL.x > selectBoxBR.x) std::swap(selectBoxTL.x, selectBoxBR.x);
	if (selectBoxTL.y > selectBoxBR.y) std::swap(selectBoxTL.y, selectBoxBR.y);
	vec3 tl = getPointerWorldPos(selectBoxTL);
	vec3 br = getPointerWorldPos(selectBoxBR);

	bool ballSelected = pressedObIndex == MAX_OBSTACLES;

	if (ball.pos.y > tl.y && ball.pos.y < br.y && ball.pos.z > br.z && ball.pos.z < tl.z)
		ballSelected = true;

	float rSq = ball.radius * ball.radius;
	vec2 roots;

	if (!ballSelected) {
		float xMl = ball.pos.y - tl.y;
		float xMlSq = xMl * xMl;
		if (rSq - xMlSq > 0) {
			roots = quadraticFormula(1, -2 * ball.pos.z, ball.pos.z * ball.pos.z - rSq + xMlSq);
			if ((roots.x > br.z && roots.x < tl.z) || (roots.y > br.z && roots.y < tl.z))
				ballSelected = true;
		}
	}

	if (!ballSelected) {
		float xMr = ball.pos.y - br.y;
		float xMrSq = xMr * xMr;
		if (rSq - xMrSq > 0) {
			roots = quadraticFormula(1, -2 * ball.pos.z, ball.pos.z * ball.pos.z - rSq + xMrSq);
			if ((roots.x > br.z && roots.x < tl.z) || (roots.y > br.z && roots.y < tl.z))
				ballSelected = true;
		}
	}

	if (!ballSelected) {
		float yMt = ball.pos.z - tl.z;
		float yMtSq = yMt * yMt;
		if (rSq - yMtSq > 0) {
			roots = quadraticFormula(1, -2 * ball.pos.y, ball.pos.y * ball.pos.y - rSq + yMtSq);
			if ((roots.x > tl.y && roots.x < br.y) || (roots.y > tl.y && roots.y < br.y))
				ballSelected = true;
		}
	}

	if (!ballSelected) {
		float yMb = ball.pos.z - br.z;
		float yMbSq = yMb * yMb;
		if (rSq - yMbSq > 0) {
			roots = quadraticFormula(1, -2 * ball.pos.y, ball.pos.y * ball.pos.y - rSq + yMbSq);
			if ((roots.x > tl.y && roots.x < br.y) || (roots.y > tl.y && roots.y < br.y))
				ballSelected = true;
		}
	}

	switch (actionMod) {
	case ACTION_MOD_REPLACE:
		selection[MAX_OBSTACLES] = ballSelected;
		break;
	case ACTION_MOD_ADD:
		selection[MAX_OBSTACLES] = ballSelected || undoBuffer[bufferIndex].selectionStates[selectionBufferIndex].selection[MAX_OBSTACLES];
		break;
	case ACTION_MOD_SUBTRACT:
		selection[MAX_OBSTACLES] = !ballSelected && undoBuffer[bufferIndex].selectionStates[selectionBufferIndex].selection[MAX_OBSTACLES];
		break;
	}

	for (short i = 0; i < (short)obstacles.size(); i++) {
		Obstacle* g = &obstacles[i];

		bool obSelected = pressedObIndex == i;

		const vec3 absStart = g->getPos() + g->getRot() * g->getStart();

		if (absStart.y > tl.y && absStart.y < br.y && absStart.z > br.z && absStart.z < tl.z)
			obSelected = true;

		const vec3 absEnd = g->getPos() + g->getRot() * g->getEnd();

		if (absEnd.y > tl.y && absEnd.y < br.y && absEnd.z > br.z && absEnd.z < tl.z)
			obSelected = true;

		rSq = g->getMinorRadius() * g->getMinorRadius();

		// Start

		vec3 pos = absStart;

		bool checkEnd = true;

	checkCapIntersection:

		if (!obSelected) {
			float xMl = pos.y - tl.y;
			float xMlSq = xMl * xMl;
			if (rSq - xMlSq > 0) {
				roots = quadraticFormula(1, -2 * pos.z, pos.z * pos.z - rSq + xMlSq);
				if ((roots.x > br.z && roots.x < tl.z) || (roots.y > br.z && roots.y < tl.z))
					obSelected = true;
			}
		}

		if (!obSelected) {
			float xMr = pos.y - br.y;
			float xMrSq = xMr * xMr;
			if (rSq - xMrSq > 0) {
				roots = quadraticFormula(1, -2 * pos.z, pos.z * pos.z - rSq + xMrSq);
				if ((roots.x > br.z && roots.x < tl.z) || (roots.y > br.z && roots.y < tl.z))
					obSelected = true;
			}
		}

		if (!obSelected) {
			float yMt = pos.z - tl.z;
			float yMtSq = yMt * yMt;
			if (rSq - yMtSq > 0) {
				roots = quadraticFormula(1, -2 * pos.y, pos.y * pos.y - rSq + yMtSq);
				if ((roots.x > tl.y && roots.x < br.y) || (roots.y > tl.y && roots.y < br.y))
					obSelected = true;
			}
		}

		if (!obSelected) {
			float yMb = pos.z - br.z;
			float yMbSq = yMb * yMb;
			if (rSq - yMbSq > 0) {
				roots = quadraticFormula(1, -2 * pos.y, pos.y * pos.y - rSq + yMbSq);
				if ((roots.x > tl.y && roots.x < br.y) || (roots.y > tl.y && roots.y < br.y))
					obSelected = true;
			}
		}

		// End

		if (checkEnd) {
			checkEnd = false;
			pos = absEnd;
			goto checkCapIntersection;
		}

		if (g->getIsStraight()) {
			PlaneDefinition d = g->getTopPlane();

			vec3 offset = d.normal * g->getMinorRadius();
			vec3 lineStart = absStart + offset;
			vec3 lineEnd = absEnd + offset;

			bool checkBottomLine = true;

		checkLineIntersection:

			if (d.normal.z != 0) {
				float xDiffInv = 1 / (lineStart.y - lineEnd.y);

				if (!obSelected && tl.y > fminf(lineStart.y, lineEnd.y) && tl.y < fmaxf(lineStart.y, lineEnd.y)) {
					roots.y = xDiffInv * ((lineStart.z - lineEnd.z) * tl.y + lineStart.y * lineEnd.z - lineEnd.y * lineStart.z);
					if (roots.y < tl.z && roots.y > br.z)
						obSelected = true;
				}

				if (!obSelected && br.y > fminf(lineStart.y, lineEnd.y) && br.y < fmaxf(lineStart.y, lineEnd.y)) {
					roots.y = xDiffInv * ((lineStart.z - lineEnd.z) * br.y + lineStart.y * lineEnd.z - lineEnd.y * lineStart.z);
					if (roots.y < tl.z && roots.y > br.z)
						obSelected = true;
				}
			}

			if (d.normal.y != 0) {
				float yDiffInv = 1 / (lineStart.z - lineEnd.z);

				if (!obSelected && tl.z > fminf(lineStart.z, lineEnd.z) && tl.z < fmaxf(lineStart.z, lineEnd.z)) {
					roots.x = yDiffInv * ((lineStart.y - lineEnd.y) * tl.z + lineStart.z * lineEnd.y - lineEnd.z * lineStart.y);
					if (roots.x < br.y && roots.x > tl.y)
						obSelected = true;
				}

				if (!obSelected && br.z > fminf(lineStart.z, lineEnd.z) && br.z < fmaxf(lineStart.z, lineEnd.z)) {
					roots.x = yDiffInv * ((lineStart.y - lineEnd.y) * br.z + lineStart.z * lineEnd.y - lineEnd.z * lineStart.y);
					if (roots.x < br.y && roots.x > tl.y)
						obSelected = true;
				}
			}

			if (checkBottomLine) {
				checkBottomLine = false;
				lineStart = absStart - offset;
				lineEnd = absEnd - offset;
				goto checkLineIntersection;
			}
		} else {
			// Outer curve

			float r = g->getMajorRadius() + g->getMinorRadius();
			rSq = r * r;

			bool checkInnerArc = true;

		checkArcIntersection:

			if (!obSelected) {
				float xMl = g->getPos().y - tl.y;
				float xMlSq = xMl * xMl;
				if (rSq - xMlSq > 0) {
					roots = quadraticFormula(1, -2 * g->getPos().z, g->getPos().z * g->getPos().z - rSq + xMlSq);
					bool root1valid = (roots.x > br.z && roots.x < tl.z);
					bool root2valid = (roots.y > br.z && roots.y < tl.z);
					if (root1valid || root2valid) {
						float startAng = atan2f(absStart.z - g->getPos().z, absStart.y - g->getPos().y);
						float endAng = atan2f(absEnd.z - g->getPos().z, absEnd.y - g->getPos().y) - startAng;
						float root1Ang = atan2f(roots.x - g->getPos().z, tl.y - g->getPos().y) - startAng;
						float root2Ang = atan2f(roots.y - g->getPos().z, tl.y - g->getPos().y) - startAng;
						if (endAng < 0) endAng += PI * 2;
						if (root1Ang < 0) root1Ang += PI * 2;
						if (root2Ang < 0) root2Ang += PI * 2;
						startAng = 0;

						if ((root1valid && root1Ang > startAng && root1Ang < endAng) || (root2valid && root2Ang > startAng && root2Ang < endAng) || (g->getStart().y == 0 && g->getStart().z < 0))
							obSelected = true;
					}
				}
			}

			if (!obSelected) {
				float xMr = g->getPos().y - br.y;
				float xMrSq = xMr * xMr;
				if (rSq - xMrSq > 0) {
					roots = quadraticFormula(1, -2 * g->getPos().z, g->getPos().z * g->getPos().z - rSq + xMrSq);
					bool root1valid = (roots.x > br.z && roots.x < tl.z);
					bool root2valid = (roots.y > br.z && roots.y < tl.z);
					if (root1valid || root2valid) {
						float startAng = atan2f(absStart.z - g->getPos().z, absStart.y - g->getPos().y);
						float endAng = atan2f(absEnd.z - g->getPos().z, absEnd.y - g->getPos().y) - startAng;
						float root1Ang = atan2f(roots.x - g->getPos().z, br.y - g->getPos().y) - startAng;
						float root2Ang = atan2f(roots.y - g->getPos().z, br.y - g->getPos().y) - startAng;
						if (endAng < 0) endAng += PI * 2;
						if (root1Ang < 0) root1Ang += PI * 2;
						if (root2Ang < 0) root2Ang += PI * 2;
						startAng = 0;

						if ((root1valid && root1Ang > startAng && root1Ang < endAng) || (root2valid && root2Ang > startAng && root2Ang < endAng) || (g->getStart().y == 0 && g->getStart().z < 0))
							obSelected = true;
					}
				}
			}

			if (!obSelected) {
				float yMt = g->getPos().z - tl.z;
				float yMtSq = yMt * yMt;
				if (rSq - yMtSq > 0) {
					roots = quadraticFormula(1, -2 * g->getPos().y, g->getPos().y * g->getPos().y - rSq + yMtSq);
					bool root1valid = (roots.x > tl.y && roots.x < br.y);
					bool root2valid = (roots.y > tl.y && roots.y < br.y);
					if (root1valid || root2valid) {
						float startAng = atan2f(absStart.z - g->getPos().z, absStart.y - g->getPos().y);
						float endAng = atan2f(absEnd.z - g->getPos().z, absEnd.y - g->getPos().y) - startAng;
						float root1Ang = atan2f(tl.z - g->getPos().z, roots.x - g->getPos().y) - startAng;
						float root2Ang = atan2f(tl.z - g->getPos().z, roots.y - g->getPos().y) - startAng;
						if (endAng < 0) endAng += PI * 2;
						if (root1Ang < 0) root1Ang += PI * 2;
						if (root2Ang < 0) root2Ang += PI * 2;
						startAng = 0;

						if ((root1valid && root1Ang > startAng && root1Ang < endAng) || (root2valid && root2Ang > startAng && root2Ang < endAng) || (g->getStart().y == 0 && g->getStart().z < 0))
							obSelected = true;
					}
				}
			}

			if (!obSelected) {
				float yMb = g->getPos().z - br.z;
				float yMbSq = yMb * yMb;
				if (rSq - yMbSq > 0) {
					roots = quadraticFormula(1, -2 * g->getPos().y, g->getPos().y * g->getPos().y - rSq + yMbSq);
					bool root1valid = (roots.x > tl.y && roots.x < br.y);
					bool root2valid = (roots.y > tl.y && roots.y < br.y);
					if (root1valid || root2valid) {
						float startAng = atan2f(absStart.z - g->getPos().z, absStart.y - g->getPos().y);
						float endAng = atan2f(absEnd.z - g->getPos().z, absEnd.y - g->getPos().y) - startAng;
						float root1Ang = atan2f(br.z - g->getPos().z, roots.x - g->getPos().y) - startAng;
						float root2Ang = atan2f(br.z - g->getPos().z, roots.y - g->getPos().y) - startAng;
						if (endAng < 0) endAng += PI * 2;
						if (root1Ang < 0) root1Ang += PI * 2;
						if (root2Ang < 0) root2Ang += PI * 2;
						startAng = 0;

						if ((root1valid && root1Ang > startAng && root1Ang < endAng) || (root2valid && root2Ang > startAng && root2Ang < endAng) || (g->getStart().y == 0 && g->getStart().z < 0))
							obSelected = true;
					}
				}
			}

			// Inner curve

			if (checkInnerArc) {
				checkInnerArc = false;
				r = g->getMajorRadius() - g->getMinorRadius();
				rSq = r * r;
				goto checkArcIntersection;
			}
		}

		switch (actionMod) {
		case ACTION_MOD_REPLACE:
			selection[i] = obSelected;
			break;
		case ACTION_MOD_ADD:
			selection[i] = obSelected || undoBuffer[bufferIndex].selectionStates[selectionBufferIndex].selection[i];
			break;
		case ACTION_MOD_SUBTRACT:
			selection[i] = !obSelected && undoBuffer[bufferIndex].selectionStates[selectionBufferIndex].selection[i];
			break;
		}
	}
}

bool Game::blobWasPressed;
void Game::blobPressed(int blobActionMod) {
	if (action == ACTION_NONE) {
		action = ACTION_SHAPE;
		pressedObIndex = blobActionMod & 0xFFFF;
		actionMod = blobActionMod >> 16;
		blobWasPressed = true;
	}
}

void Game::changePropertyInput() { // Called whenever text box focus changes
	if (TextBoxManager::anythingChanged)
		finishAction();

	TextBoxManager::anythingChanged = false;
}

void Game::updatePropertyInput(byte property) {
	if (property == PROPERTY_LEVEL_NAME) {
		saveAsName = std::string(TextBoxManager::text.begin(), TextBoxManager::text.end());
		return;
	}

	bool valueSet = false;
	float value, oldValue;
	if (property > FLOAT_PROPERTY_START && property < FLOAT_PROPERTY_END) {
		size_t numProcessed;
		try {
			value = std::stof(TextBoxManager::text, &numProcessed);
			value = round(value * 100) * 0.01f;
			valueSet = true;
			if (numProcessed != TextBoxManager::text.length()) return;
			oldValue = std::stof(TextBoxManager::getCurrentValue(), &numProcessed);
			oldValue = round(oldValue * 100) * 0.01f;
			if (numProcessed == TextBoxManager::getCurrentValue().length() && value == oldValue) return; // The parsed value did not change
		} catch (const std::exception&) {
			if (!valueSet)
				return;
		}
	}

	TextBoxManager::anythingChanged = true;

	if (focus < 0) { // Level settings
		switch (property) {
		case PROPERTY_ARENA_WIDTH:
			level.arenaWidth = std::clamp(value, MINIMUM_ARENA_SIZE, MAXIMUM_ARENA_SIZE);
			updateArena();
			resetEditorView();
			break;
		case PROPERTY_ARENA_HEIGHT:
			level.arenaHeight = std::clamp(value, MINIMUM_ARENA_SIZE, MAXIMUM_ARENA_SIZE);
			updateArena();
			resetEditorView();
			break;
		case PROPERTY_TRANSITION_TIME:
			level.transitionTime = std::clamp(value, MINIMUM_TRANSITION_TIME, MAXIMUM_TRANSITION_TIME);
			break;
		}
	} else if (focus == MAX_OBSTACLES) { // Ball settings
		switch (property) {
		case PROPERTY_POS_X:
			moveBallTo({0, value, ball.pos.z});
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i])
					moveObstacleTo(i, {0, value, obstacles[i].getPos().z});
			break;
		case PROPERTY_POS_Y:
			moveBallTo({0, ball.pos.y, value});
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i])
					moveObstacleTo(i, {0, obstacles[i].getPos().y, value});
			break;
		}
	} else { // Obstacle settings
		switch (property) {
		case PROPERTY_POS_X:
			if (selection[MAX_OBSTACLES]) moveBallTo({0, value, ball.pos.z});
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i])
					moveObstacleTo(i, {0, value, obstacles[i].getPos().z});
			break;
		case PROPERTY_POS_Y:
			if (selection[MAX_OBSTACLES]) moveBallTo({0, ball.pos.y, value});
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i])
					moveObstacleTo(i, {0, obstacles[i].getPos().y, value});
			break;
		case PROPERTY_ANGLE:
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i])
					rotateObstacleTo(i, displayToAngle(value));
			break;
		case PROPERTY_MINOR_RADIUS:
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i]) {
					obstacles[i].setMinorRadius(std::clamp(value, MINIMUM_MINOR_RADIUS, obstacles[i].getIsStraight() ? INFINITY : obstacles[i].getMajorRadius()));

					obstacles[i].createAllModels();
				}
			break;
		case PROPERTY_OPM:
			for (short i = 0; i < (short)obstacles.size(); i++) {
				if (selection[i] && (obstacles[i].getStateType() == ST_POS_OSC || obstacles[i].getStateType() == ST_ANG_OSC)) {
					if (toggled)
						obstacles[i].setStateBX(PI / 30 * std::clamp(value, MINIMUM_OPM, MAXIMUM_OPM));
					else
						obstacles[i].setStateAX(PI / 30 * std::clamp(value, MINIMUM_OPM, MAXIMUM_OPM));
					obstacles[i].setExtra(obstacles[focus].getExtra());
				}
			}
			break;
		case PROPERTY_RPM:
			for (short i = 0; i < (short)obstacles.size(); i++)
				if (selection[i] && obstacles[i].getStateType() == ST_ANG_VEL) {
					if (toggled)
						obstacles[i].setStateBX(PI / 30 * std::clamp(value, MINIMUM_RPM, MAXIMUM_RPM));
					else
						obstacles[i].setStateAX(PI / 30 * std::clamp(value, MINIMUM_RPM, MAXIMUM_RPM));
					obstacles[i].setAngle(wrapAngle(obstacles[focus].getAngle() - obstacles[focus].getInitAngle() + obstacles[i].getInitAngle()));
				}
			break;
		}
	}
}

void Game::moveBallByFull(const vec3& vector) {
	moveBallTo(undoBuffer[bufferIndex].level.ballPos + vector);
}
void Game::moveBallBy(const vec3& vector) {
	moveBallTo(ball.pos + vector);
}
void Game::moveBallTo(const vec3& pos) {
	setBallPos(pos);
	level.ballPos = pos;
}

void Game::moveObstacleByFull(short index, const vec3& vector) {
	if (obstacles[index].getStateType() == ST_POS || obstacles[index].getStateType() == ST_POS_OSC) {
		if (toggled || actionIsStateless)
			obstacles[index].setStateB(undoBuffer[bufferIndex].level.obstacleDefinitions[index].stateB + vector);
		if (!toggled || actionIsStateless) {
			obstacles[index].setStateA(undoBuffer[bufferIndex].level.obstacleDefinitions[index].stateA + vector);
			obstacles[index].setInitPos({0, obstacles[index].getStateA().y, obstacles[index].getStateA().z});
		}
		vec3 currentState = toggled ? obstacles[index].getStateB() : obstacles[index].getStateA();
		obstacles[index].setPos({0, currentState.y, currentState.z});

		obstacles[index].createDomainModel();
	} else {
		vec3 translatedPos = undoBuffer[bufferIndex].level.obstacleDefinitions[index].initPos + vector;
		obstacles[index].setPos(translatedPos);
		obstacles[index].setInitPos(translatedPos);
	}
}
void Game::moveObstacleTo(short index, const vec3& pos) { // TODO: remove this function
	obstacles[index].setPos(pos);
	if (obstacles[index].getStateType() == ST_POS || obstacles[index].getStateType() == ST_POS_OSC) {
		if (toggled)
			obstacles[index].setStateB({obstacles[index].getStateB().x, pos.y, pos.z});
		else {
			obstacles[index].setStateA({obstacles[index].getStateA().x, pos.y, pos.z});
			obstacles[index].setInitPos(pos);
		}

		obstacles[index].createDomainModel();
	} else
		obstacles[index].setInitPos(pos);
}

void Game::rotateObstacleBy(short index, float angle) {
	if (obstacles[index].getStateType() == ST_ANG) {
		if (toggled || actionIsStateless)
			obstacles[index].setStateBX(obstacles[index].getStateB().x + angle);
		if (!toggled || actionIsStateless) {
			obstacles[index].setStateAX(obstacles[index].getStateA().x + angle);
			obstacles[index].setInitAngle(obstacles[index].getStateA().x);
		}
		vec3 currentState = toggled ? obstacles[index].getStateB() : obstacles[index].getStateA();
		obstacles[index].setAngle(currentState.x);
	} else if (obstacles[index].getStateType() == ST_ANG_OSC) {
		if (toggled || actionIsStateless)
			obstacles[index].setStateBY(obstacles[index].getStateB().y + angle);
		if (!toggled || actionIsStateless) {
			obstacles[index].setStateAY(obstacles[index].getStateA().y + angle);
			obstacles[index].setInitAngle(obstacles[index].getStateA().y);
		}
		vec3 currentState = toggled ? obstacles[index].getStateB() : obstacles[index].getStateA();
		obstacles[index].setAngle(currentState.y);
	} else {
		angle = wrapAngle(obstacles[index].getInitAngle() + angle);
		obstacles[index].setAngle(angle);
		obstacles[index].setInitAngle(angle);
	}
}
void Game::rotateObstacleTo(short index, float angle) { // TODO: remove this function
	if (obstacles[index].getStateType() != ST_ANG && obstacles[index].getStateType() != ST_ANG_OSC)
		angle = wrapAngle(angle);

	angle = std::clamp(angle, MINIMUM_ANGLE, MAXIMUM_ANGLE);

	obstacles[index].setAngle(angle);

	if (obstacles[index].getStateType() == ST_ANG) {
		if (toggled)
			obstacles[index].setStateBX(angle);
		else {
			obstacles[index].setStateAX(angle);
			obstacles[index].setInitAngle(angle);
		}
	} else if (obstacles[index].getStateType() == ST_ANG_OSC) {
		if (toggled)
			obstacles[index].setStateBY(angle);
		else {
			obstacles[index].setStateAY(angle);
			obstacles[index].setInitAngle(angle);
		}
	} else
		obstacles[index].setInitAngle(angle);

	if (obstacles[index].getStateType() != ST_STATIC)
		obstacles[index].createDomainModel();
}

std::vector<ObstacleDefinition> Game::clipboard;

void Game::addObstacle(bool isStraight, bool isGoal, byte stateType) {
	if (action == ACTION_NONE && obstacles.size() < MAX_OBSTACLES) {
		vec3 start = {0, 3, 0};
		vec3 end = {0, -3, 0};
		vec3 pos = {0, 0, arenaHeight * 0.5f};
		if (focus >= 0) {
			if (focus == MAX_OBSTACLES)
				pos = ball.pos;
			else
				pos = obstacles[focus].getPos();
		}
		vec3 stateA, stateB;
		col color;

		if (isGoal)
			color = SOFT_GREEN;
		else
			switch (stateType) {
			case ST_STATIC:
				color = WHITE;
				stateA = stateB = {0};
				break;
			case ST_POS:
				color = SOFT_BLUE;
				stateA = stateB = pos;
				break;
			case ST_POS_OSC:
				color = SOFT_CYAN;
				stateA = pos + vec3(PI * 0.5f, 0, 0);
				stateB = pos;
				break;
			case ST_ANG:
				color = SOFT_RED;
				stateA = stateB = {0};
				break;
			case ST_ANG_VEL:
				color = SOFT_MAGENTA;
				stateA = {0};
				stateB = {PI, 0, 0};
				break;
			case ST_ANG_OSC:
				color = SOFT_YELLOW;
				stateA = {0};
				stateB = {PI * 0.5f, 0, 0};
				break;
			}

		focus = (short)obstacles.size();
		level.obstacleDefinitions.emplace_back(pos, 0.0f, isStraight, start, end, 0.8f, stateType, stateA, stateB, isGoal);
		obstacles.emplace_back(&level.obstacleDefinitions[focus], 1.0f);

		obstacles[focus].createAllModels();

		memset(selection, false, sizeof(selection));
		selection[focus] = true;
		finishAction();
	}
}

void Game::addObstacleCallback(int data) {
	addObstacle(data & 0x100, data & 0x200, data & 0xff);
}

void Game::deleteObstacles() {
	short oldNumObs = (short)obstacles.size();
	for (short i = oldNumObs - 1; i >= 0; i--) {
		if (selection[i]) {
			level.obstacleDefinitions.erase(level.obstacleDefinitions.begin() + i);
			obstacles.erase(obstacles.begin() + i);
		}
	}
	for (short i = 0; i < oldNumObs; i++)
		selection[i] = false;
	if (focus < MAX_OBSTACLES)
		focus = selection[MAX_OBSTACLES] ? MAX_OBSTACLES : -1;
	finishAction();
}

void Game::duplicateObstacles() {
	short numSelected = 0;
	for (short i = 0; i < (short)obstacles.size(); i++)
		if (selection[i]) numSelected++;
	if (obstacles.size() + numSelected > MAX_OBSTACLES || numSelected == 0)
		return;

	selection[MAX_OBSTACLES] = false;
	const short originalSize = (short)obstacles.size();
	level.obstacleDefinitions.reserve((size_t)(originalSize + numSelected));
	obstacles.reserve((size_t)(originalSize + numSelected));
	for (short i = 0; i < originalSize; i++) {
		if (selection[i]) {
			selection[i] = false;
			const short newIndex = (short)obstacles.size();
			selection[newIndex] = true;
			if (i == focus)
				focus = newIndex;
			level.obstacleDefinitions.emplace_back(level.obstacleDefinitions[i]);
			obstacles.emplace_back(&level.obstacleDefinitions.back(), 1.0f);
			obstacles.back().createAllModels();
		}
	}

	if (focus == MAX_OBSTACLES)
		focus = (short)(originalSize + numSelected - 1);

	finishAction();

	action = ACTION_MOVE;
	actionMod = ACTION_MOD_FREE;
	actionIsStateless = true;
	startAction();
}

void Game::copyObstacles() {
	short numSelected = 0;
	for (short i = 0; i < (short)level.obstacleDefinitions.size(); i++)
		if (selection[i]) numSelected++;
	if (numSelected == 0)
		return;

	clipboard.clear();

	for (short i = 0; i < (short)level.obstacleDefinitions.size(); i++)
		if (selection[i])
			clipboard.push_back(level.obstacleDefinitions[i]);
}

void Game::cutObstacles() {
	copyObstacles();
	deleteObstacles();
}

void Game::pasteObstacles() {
	if (!clipboard.empty() && obstacles.size() + clipboard.size() <= MAX_OBSTACLES) {
		memset(selection, false, sizeof(selection));

		short newObstaclesStartIndex = (short)obstacles.size();

		for (ObstacleDefinition& d: clipboard) {
			const short newIndex = (short)obstacles.size();
			level.obstacleDefinitions.emplace_back(d);
			obstacles.emplace_back(&level.obstacleDefinitions.back(), 1.0f);
			obstacles.back().createAllModels();
			selection[newIndex] = true;
		}

		moveEditorObstacles(); // Update pos of all obstacles so we can calculate centre point

		float minX = MINIMUM_POS_X;
		float maxX = MAXIMUM_POS_X;
		float minY = MINIMUM_POS_Y;
		float maxY = MAXIMUM_POS_Y;

		for (short i = newObstaclesStartIndex; i < obstacles.size(); i++) {
			minX = fmaxf(minX, obstacles[i].getPos().y);
			maxX = fminf(maxX, obstacles[i].getPos().y);
			minY = fmaxf(minY, obstacles[i].getPos().z);
			maxY = fminf(maxY, obstacles[i].getPos().z);
		}
		// Override the actual pointer position, so the obstacles 'teleport' to the mouse position
		pointerBall.pos = vec3(0, (minX + maxX) * 0.5f, (minY + maxY) * 0.5f);

		focus = (short)(obstacles.size() - 1);
		finishAction();

		action = ACTION_MOVE;
		actionMod = ACTION_MOD_FREE;
		actionIsStateless = true;

		startAction();

		updateAction();
	}
}

void Game::updateBallOutline() {
	float factor = 1 + OUTLINE_WIDTH * HALF_HEIGHT * 2;
	ballOutline.radius = ball.radius * factor;
	ballOutline.scale.set(factor);
}

void Game::updateAllOutlines() {
	updateBallOutline();
	for (Obstacle& g: obstacles)
		g.createOutlineModel();
}

vec2 Game::pointerPos;
Ball Game::pointerBall = Ball();
Ball Game::pointerBallWide = Ball();

short Game::bufferIndex;
short Game::lastBufferIndex;
short Game::selectionBufferIndex;
UndoNode Game::undoBuffer[MAX_UNDO];

void Game::undo() {
	if (selectionBufferIndex == 0) {
		if (bufferIndex == 0)
			return; // No previous state

		bufferIndex--;
		selectionBufferIndex = undoBuffer[bufferIndex].lastSelectionBufferIndex;
		loadLevel(undoBuffer[bufferIndex].level, false);
		SelectionState* ss = &undoBuffer[bufferIndex].selectionStates[selectionBufferIndex];
		memcpy(selection, ss->selection, sizeof(selection));
		focus = ss->focus;
	} else { // Undoing selection only
		selectionBufferIndex--;
		SelectionState* ss = &undoBuffer[bufferIndex].selectionStates[selectionBufferIndex];
		memcpy(selection, ss->selection, sizeof(selection));
		focus = ss->focus;
	}
}

void Game::redo() {
	if (selectionBufferIndex == undoBuffer[bufferIndex].lastSelectionBufferIndex) {
		if (bufferIndex == lastBufferIndex)
			return; // No next state

		bufferIndex++;
		selectionBufferIndex = 0;
		loadLevel(undoBuffer[bufferIndex].level, false);
		SelectionState* ss = &undoBuffer[bufferIndex].selectionStates[0];
		memcpy(selection, ss->selection, sizeof(selection));
		focus = ss->focus;
	} else {
		selectionBufferIndex++;
		SelectionState* ss = &undoBuffer[bufferIndex].selectionStates[selectionBufferIndex];
		memcpy(selection, ss->selection, sizeof(selection));
		focus = ss->focus;
	}
}