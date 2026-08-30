#ifndef OPERATION_H
#define OPERATION_H

#include "utilities/Camera.h"
#include "editor/GizmoRenderer.h"
#include "io/Event.h"
#include "system/Cursor.h"


class EditorObstacle;
class EditorScene;


enum class TriggerType { Pointer, TriggerKey, ActionKey };

enum class OperationStatus { Running, Committed, Cancelled };

struct OperationResponse {
	bool consumedEvent = false;
	OperationStatus status = OperationStatus::Running;
};

class Operation : public IGizmoProvider, public ICursorProvider {
public:
	~Operation() override = default;

	[[nodiscard]] OperationResponse processEvent(const Event& event);

	[[nodiscard]] virtual std::vector<BindingHint> getBindingHints() const { return {}; }
	virtual void createUI(UINode& container) {}
	virtual bool updateUI() { return true; }

	[[nodiscard]] bool start(byte mods) {
		if (canStart()) {
			applyModifiers(mods);
			applyOperation();
			return true;
		}
		return false;
	}
	virtual void finish() {}

	virtual void cancel() const = 0;
	virtual void commit() const = 0;

	[[nodiscard]] static std::optional<int> getTopObstacleIndex(const std::vector<EditorObstacle>& obstacles, const std::function<bool(const EditorObstacle&)>& includePredicate);

	void onQuickSettingsChanged() { applyOperation(); }

	const TriggerType trigger;

protected:
	Operation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition)
		: trigger(trigger), scene(scene), camera(camera), initialPlanarPosition(initialPlanarPosition), pointerPlanarPosition(initialPlanarPosition) {}
	Operation(const Operation&) = default;

	[[nodiscard]] virtual OperationResponse doProcessEvent(const Event& event) = 0;

	virtual void applyOperation() = 0;

	EditorScene& scene;
	const Camera& camera;

	const glm::vec2 initialPlanarPosition;
	glm::vec2 pointerPlanarPosition;

private:
	virtual void applyModifiers(byte mods) = 0;

	[[nodiscard]] virtual bool canStart() const { return true; }
};


#endif // OPERATION_H
