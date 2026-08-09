#ifndef EDITOR_BALL_H
#define EDITOR_BALL_H

#include "ball/BallDescriptor.h"
#include "SelectBox.h"
#include "Settings.h"


class EditorBall {
public:
	EditorBall() = default;

	explicit EditorBall(BallDescriptor* descriptor) :
		descriptor(descriptor),
		texture(getBallTexture(descriptor->type)) {}

	~EditorBall() = default;

	EditorBall(const EditorBall& other) = delete;
	EditorBall& operator=(const EditorBall&) = delete;
	EditorBall(EditorBall&&) = default;
	EditorBall& operator=(EditorBall&&) = default;

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }
	void setSelected(bool select) { selected = select; }
	[[nodiscard]] bool isInSelectBox(SelectBox selectBox) const {
		return selectBox.touchesCircle(descriptor->initialPosition, 1.f);
	}

	void updateOutlineRadius(float uiToWorldScale) {
		outlineRadius = 1.f + Settings::Sizes.outlineWidth * uiToWorldScale;
	}
	[[nodiscard]] float getOutlineRadius() const { return outlineRadius; }

	void translateBy(glm::vec2 vector, const BallDescriptor* base) const {
		descriptor->initialPosition = base->initialPosition + vector;
	}
	void rotateBy(glm::mat2 rotationMatrix, glm::vec2 pivot, const BallDescriptor* base) const {
		descriptor->initialPosition = pivot + rotationMatrix * (base->initialPosition - pivot);
	}
	void scaleBy(float factor, glm::vec2 pivot, bool local, const BallDescriptor* base) const {
		descriptor->initialPosition = local ? base->initialPosition : pivot + factor * (base->initialPosition - pivot);
	}

	[[nodiscard]] const Texture* getTexture() const { return texture; }

	BallDescriptor* descriptor{};

private:
	Texture* texture{};

	bool selected = false;

	float outlineRadius = 1.f;
};


#endif // EDITOR_BALL_H
