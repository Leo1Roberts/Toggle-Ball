#ifndef FPS_OVERLAY_H
#define FPS_OVERLAY_H

#include "UINode.h"


class UIText;

class FPSOverlay : public UINode {
public:
	FPSOverlay();

private:
	void doUpdate(microseconds dt) override;

	UIText* labelNode = nullptr;
	microseconds timeAccumulator = 0;
	int frameCount = 0;
};


#endif // FPS_OVERLAY_H
