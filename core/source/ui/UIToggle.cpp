#include "ui/UIToggle.h"


UIToggle::UIToggle(bool initialState, const ToggleStyle& toggleStyle) :
	UIPanel(initialState ? toggleStyle.normalTrackOn : toggleStyle.normalTrackOff),
	toggleStyle(toggleStyle),
	state(initialState),
	handlePosition(initialState) {
	handleNode = addChild<UIPanel>(toggleStyle.handle);
	handleNode->setHitTestable(false);
	updateVisuals();
}


void UIToggle::setState(bool newState, bool animate) {
	if (state == newState) return;
    
	state = newState;
	if (!animate && !valueProvider) {
		handlePosition = state;
		updateVisuals();
	}
}


float UIToggle::getTrackWidth() const {
	return std::max(0.f, getAbsoluteBounds().width()
	                     - handleNode->getAbsoluteBounds().width()
	                     - getLayout().padding.x * 2.f);
}


UIResponse UIToggle::processEvent(const Event& event) {
    if (auto* pointer = std::get_if<PointerEvent>(&event)) {
        switch (pointer->action) {
        case PointerAction::Down:
            if (pointer->button == PointerButton::Primary) {
            	pressed = true;
            	updateVisuals();
            	return UIResponse::Consumed;
            } break;
        case PointerAction::Up:
            if (pointer->button == PointerButton::Primary) {
            	if (pressed && !dragging) {
            		pressed = false;
            		setState(!state, true);
            		if (onToggleCallback)
            			onToggleCallback(state);
            	}
            	pressed = false;
            	updateVisuals();
            	dragging = false;
            	return UIResponse::Consumed;
            } break;
        case PointerAction::StartDrag:
        	if (pointer->button == PointerButton::Primary) {
        		if (pressed) {
        			dragging = true;
        			dragStartX = pointer->position.x;
        			dragStartHandlePosition = handlePosition;
        		}
        		return UIResponse::Consumed;
        	} break;
        case PointerAction::Drag:
            if (dragging && !valueProvider) { // Don't allow drag editing if externally driven
                float trackWidth = getTrackWidth();
                if (trackWidth > 0.f) {
                    float deltaX = pointer->position.x - dragStartX;
                    handlePosition = std::clamp(dragStartHandlePosition + (deltaX / trackWidth), 0.f, 1.f);
                    updateVisuals();
                }
            }
            return UIResponse::ConsumedNeedsHoverUpdate;
        case PointerAction::FinishDrag:
        case PointerAction::CancelDrag:
            if (dragging) {
                dragging = false;
                pressed = false;

                bool newState = (handlePosition >= 0.5f);
                if (newState != state) {
                    state = newState;
                    if (onToggleCallback)
                        onToggleCallback(state);
                }
            }
            return UIResponse::Consumed;
        default:;
        }
    } else if (auto* key = std::get_if<KeyEvent>(&event)) {
        if (key->action == KeyAction::Down && key->chord.code == KeyCode::Enter) {
            setState(!state, true);
            if (onToggleCallback)
            	onToggleCallback(state);
            return UIResponse::Consumed;
        }
    }
    
    return UIResponse::Ignored;
}


void UIToggle::onPointerEntered() {
	hovered = true;
	updateVisuals();
}
void UIToggle::onPointerExited() {
	hovered = false;
	updateVisuals();
}


void UIToggle::doUpdate(microseconds dt) {
    float targetValue = state;
    float currentHandlePosition = handlePosition;

    if (valueProvider)
        handlePosition = std::clamp(valueProvider(), 0.f, 1.f);
    else if (!dragging && handlePosition != targetValue) {
        float animationSpeed = 8.f * toSeconds(dt);
        
        if (targetValue > handlePosition)
            handlePosition = std::min(handlePosition + animationSpeed, targetValue);
        else
            handlePosition = std::max(handlePosition - animationSpeed, targetValue);
    }

    if (currentHandlePosition != handlePosition)
        updateVisuals();
}


void UIToggle::updateVisuals() {
	if (visualState != ButtonState::Disabled) {
		if (pressed && hovered)
			visualState = ButtonState::Pressed;
		else if (hovered)
			visualState = ButtonState::Hovered;
		else
			visualState = ButtonState::Normal;
	}

	PanelStyle trackOffStyle, trackOnStyle;

	switch (visualState) {
	case ButtonState::Normal:
		trackOffStyle = toggleStyle.normalTrackOff;
		trackOnStyle  = toggleStyle.normalTrackOn;
		break;
	case ButtonState::Hovered:
	case ButtonState::Pressed:
		trackOffStyle = toggleStyle.hoveredTrackOff;
		trackOnStyle  = toggleStyle.hoveredTrackOn;
		break;
	case ButtonState::Disabled:
		trackOffStyle = toggleStyle.disabledTrackOff;
		trackOnStyle  = toggleStyle.disabledTrackOn;
		break;
	}

	panelStyle = PanelStyle::mix(trackOffStyle, trackOnStyle, handlePosition);

	auto handleLayout = handleNode->getLayout();
	handleLayout.anchor = Anchor::CentreLeft;
	handleLayout.offset = {getTrackWidth() * handlePosition, 0.f};

	handleNode->setLayout(handleLayout);
}