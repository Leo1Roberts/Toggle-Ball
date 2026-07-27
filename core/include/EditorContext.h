#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H


class Camera;
class EditorScene;

struct EditorContext {
	EditorScene* scene;
	Camera* camera;
};


#endif // EDITOR_CONTEXT_H
