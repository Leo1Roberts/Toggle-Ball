#ifndef FPS_OVERLAY_H
#define FPS_OVERLAY_H

#include "ui/UIText.h"


class FPSOverlay : public UIText {
public:
	FPSOverlay();

private:
	void doUpdate(microseconds dt) override;

	microseconds timeAccumulator = 0;
	int frameCount = 0;
};


#endif // FPS_OVERLAY_H
