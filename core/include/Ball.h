#ifndef BALL_H
#define BALL_H

#include "Texture.h"
#include "Mesh.h"

enum {
	BASKETBALL,
	FOOTBALL,
	PING_PONG,
	MARBLE
};

const std::string ballString[] = {"basketball", "football", "ping-pong", "marble"};

class EditorBall {
public:
	explicit EditorBall(byte ballType);

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }

private:
	Texture* texture;

	bool selected = false;
};

#endif // BALL_H
