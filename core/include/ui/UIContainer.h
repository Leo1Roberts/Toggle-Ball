#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include "UINode.h"

class UIContainer : public UINode {
public:
	UIContainer() : UINode() {
		setHitTestable(false);
	}
};

#endif // UI_CONTAINER_H
