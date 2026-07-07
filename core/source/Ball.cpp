#include "main.h"
#include "Ball.h"

EditorBall::EditorBall(byte ballType) {
	switch (ballType) {
	case BASKETBALL:
		texture = Textures::basketball.get();
		break;
	case FOOTBALL:
		[[fallthrough]];
	case PING_PONG:
		[[fallthrough]];
	case MARBLE:
		[[fallthrough]];
	default:
		texture = Textures::white.get();
	}
}