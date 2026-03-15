#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "Level.h"
#include "Smoother.h"
#include <string>
#include <vector>
#include "Model.h"
#include "TextureAsset.h"
#include "VectorMatrix.h"

#define CHECK_ERROR() CheckGLError()

struct android_app;

void CheckGLError();

struct SelectionState {
	bool selection[MAX_OBSTACLES + 1]{};
	short focus;

	SelectionState() : focus(0) {
		memset(selection, 0, sizeof(selection));
	}

	SelectionState(const bool* s, short f) {
		set(s, f);
	}

	void set(const bool* s, short f) {
		memcpy(selection, s, sizeof(selection));
		focus = f;
	}
};

const byte MAX_SELECTIONS = 100;

struct UndoNode {
	Level level;
	SelectionState selectionStates[MAX_SELECTIONS];
	short lastSelectionBufferIndex;

	UndoNode() : level(Level()), lastSelectionBufferIndex(0) {
		for (SelectionState& s: selectionStates)
			s = SelectionState();
	}

	void set(const Level& l, const bool* s, short f) {
		level = l;
		selectionStates[0] = SelectionState(s, f);
		lastSelectionBufferIndex = 0;
	}
};

class Game {
public:
#ifdef WINDOWS_VERSION
	static void createGame(GLFWwindow* win) {
		window = win;
		initGame();
	}

	static void handleKeyInput(GLFWwindow* window, int key, int scancode, int keyAction, int mods);
	static void handleCharInput(GLFWwindow* window, unsigned int c);
	static void handleCursorPosInput(GLFWwindow* window, double xpos, double ypos);
	static void handleCursorEnterEvent(GLFWwindow* window, int entered);
	static void handleMouseButtonInput(GLFWwindow* window, int mouseButton, int mouseButtonAction, int mods);
	static void handleScrollInput(GLFWwindow* window, double xoffset, double yoffset);

	static bool allArrowKeysUp(int exceptKey = GLFW_KEY_UNKNOWN);
	static void moveObjectsWithArrowKey(int key, int keyAction);

	static void updateCursor();
#else

	static void createGame(android_app* pApp) {
		androidApp = pApp;
		initGame();
	}

	/*!
	 * Handles input from the android_app.
	 *
	 * Note: this will clear the input queue
	 */
	static void handleInput();

#endif

	static void deleteGame();

	static void update();

private:
#ifdef WINDOWS_VERSION
	static GLFWwindow* window;
#else
	static android_app* androidApp;
	static EGLDisplay eglDisplay;
	static EGLSurface eglSurface;
	static EGLContext eglContext;
#endif

	static bool inEditor;
	static bool toggled;
	static bool levelComplete;
	static bool viewLocked;

	static Smoother smoother;

	static Level level;
	static std::vector<Obstacle> obstacles;
	static float arenaWidth, arenaHeight;
	static Plane arenaBounds[4];
	static Plane arenaBackground;
	static Ball ball;
	static Ball ballOutline;
	static inline void setBallPos(const vec3& pos);

	static std::vector<std::string> getLevelList();

	static void drawInventoryPanel();
	static void drawPropertiesPanel();
	static void drawDialogue();

	static void render();

	static void ballObstacleCollision(Ball* pBall, Obstacle* g, const vec3& normal, float separation);

	static short checkBallObstacleCollision(Ball* pBall, bool getIndex, bool onlyRim = false, bool onlySection = false);

	static void physicsUpdate();

	static void updateEditorObstacles();

	static void setupObjShaders();

	static void reloadShaders();

	static void initGame();

	static void initFonts();

	static void updateHalfHeight();

	static void updateRenderArea();

	static void resetBall();

	static void restartLevel(int _);


	static int dialogue;
	enum {
		DLG_NONE,
		DLG_CONFIRM_EXIT,
		DLG_CHOOSE_LEVEL,
		DLG_CONFIRM_SWITCH,
		DLG_SAVE_AS,
		DLG_CONFIRM_OVERWRITE,
		DLG_NUM
	};
	static void requestDialogue(int d);

#ifdef WINDOWS_VERSION
	static void confirmExit(int _ = 0);
#endif

	static bool justOpenedSaveAsDialogue;
	static std::string saveAsName;
	static void saveAs(int _ = 0);


	static void createObjects();

	static void drawBall(const Ball& b);
	static void drawPlane(const Plane& p);
	static void drawObject(const Model* model, const TextureAsset* texture, const vec3& pos, const mat3& rot, const vec3& scale);
	static void drawObstacles();
	static void drawObstacleOutline(const Obstacle& g);
	static void drawObstacleDomains();

	static void toggle(int _ = 0);

	static void resetEditorView(int _ = 0);

	static void rotateView(const vec2& dxy);

	static void rotateView(float dx, float dy);

	static void moveViewOrigin(float dx, float dy);

	static vec3 getPointerWorldPos(float x, float y);

	static vec3 getPointerWorldPos(const vec2& pos);

	static void updateArena();
	static void loadLevel(const Level& newLevel, bool resetView = true);
	static std::string levelString;
	static void selectLevel(const std::string& name = "");
	static std::string pendingLevel;
	static void selectPendingLevel(int _ = 0);
	static void selectLevelCallback(int levelIndex);
	static short savedIndex;
	static void saveLevel(int _ = 0);
	static inline bool changesAreSaved() { return savedIndex == bufferIndex; }

	static short focus;
	static void findNewFocus();
	static bool selection[MAX_OBSTACLES + 1]; // Ball selection state at end
	static void selectObject(short index);
	static void deselectObject(short index);

	static bool limiting[MAX_OBSTACLES + 1]; // Objects which have reached a limit

	static bool ctrlDown, shiftDown, altDown;
	static bool leftMouseDown, middleMouseDown, rightMouseDown;

	enum {
		ACTION_NONE,
		ACTION_SELECT,
		ACTION_MOVE,
		ACTION_ROTATE,
		ACTION_SCALE,
		ACTION_SHAPE,
		ACTION_NUM
	};
	enum {
		ACTION_MOD_NONE,

		// Select
		ACTION_MOD_REPLACE,
		ACTION_MOD_ADD,
		ACTION_MOD_SUBTRACT,

		// Move
		ACTION_MOD_FREE,
		ACTION_MOD_GLOBAL_X,
		ACTION_MOD_GLOBAL_Y,
		ACTION_MOD_LOCAL_X,
		ACTION_MOD_LOCAL_Y,
		ACTION_MOD_UNIT,

		// Rotate & scale
		ACTION_MOD_INDIVIDUAL,
		ACTION_MOD_GROUP,

		// Scale & shape
		ACTION_MOD_MAJOR,
		ACTION_MOD_MINOR,

		// Shape
		ACTION_MOD_START,
		ACTION_MOD_END,
		ACTION_MOD_START_MIRRORED,
		ACTION_MOD_END_MIRRORED,
		ACTION_MOD_SLIDE, // Straight only

		ACTION_MOD_NUM
	};

	static byte action;
	static byte actionMod;
	static bool actionIsStateless;
	static vec2 actionStartPointerPos;
	static vec3 actionStartPointerWorldPos;
	static vec3 actionVector;
	static short pressedObIndex;
	static float initialPointerObDist; // Used for radius adjustments

	static void startAction();

	static void finishAction();

	static void updateAction();

	static void changeAction();

	static void cancelAction();

	static vec2 selectBoxTL, selectBoxBR;
	static void boxSelect();

	static bool blobWasPressed;
	static void blobPressed(int blobActionMod);

	static void changePropertyInput();
	static void updatePropertyInput(byte property);

	static void moveBallByFull(const vec3& vector);
	static void moveBallBy(const vec3& vector);
	static void moveBallTo(const vec3& pos);

	static void moveObstacleByFull(short index, const vec3& vector);
	static void moveObstacleTo(short index, const vec3& vector); // TODO: remove

	static void rotateObstacleBy(short index, float angle);
	static void rotateObstacleTo(short index, float angle); // TODO: remove

	static void addObstacle(bool isStraight, bool isGoal, byte stateType);
	static void addObstacleCallback(int data);
	static void deleteObstacles();
	static void duplicateObstacles();

	static std::vector<ObstacleDefinition> clipboard;
	static void copyObstacles();
	static void cutObstacles();
	static void pasteObstacles();

	static inline void updateBallOutline();
	static void updateAllOutlines();

	static vec2 pointerPos;
	static Ball pointerBall;
	static Ball pointerBallWide;

	static const byte MAX_UNDO = 50;
	static short bufferIndex;
	static short lastBufferIndex;
	static short selectionBufferIndex;
	static UndoNode undoBuffer[];

	static void undo();
	static void redo();
};

#endif // GAME_H