#include "widget.hpp"

#include "clay.h"

#include <cstring>

void onButtonClick(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Widget* widget = static_cast<Widget*>(userData);
    
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    	widget->triggerButtonAction(elementId.stringId.chars);
    }
}

void Widget::layoutButton(const BUTTON_ID id, const Clay_ElementId& elementId, const Clay_String& buttonText) {
    CLAY(elementId, { 
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
        }, 
        .backgroundColor = Clay_Hovered() ? SURFACE_BUTTON_COLOR_BG_HL : SURFACE_BUTTON_COLOR_BG,
        .border = { 
            .color = Clay_Color({ 220, 220, 220, 255 }), 
            .width = CLAY_BORDER_OUTSIDE(1) 
        },
    }) {
        Clay_Color textColor = SURFACE_BUTTON_COLOR_FG;
        // Clay_Hovered only works inside the CLAY declaration body
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
    	Clay_OnHover(onButtonClick, this);
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontSize = 24 }));
    }
}

void Widget::layoutTab(const BUTTON_ID id, const Clay_ElementId& elementId, const Clay_String& label) {
    CLAY(elementId, { 
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER } 
        }, 
        .backgroundColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE),
    }) {
        onButtonHover(id, Clay_Hovered());
        Clay_OnHover(onButtonClick, this);
        CLAY_TEXT(label, CLAY_TEXT_CONFIG({ .textColor = { 255, 255, 255, 255 }, .fontSize = 24 }));
    }
}

const Button& Widget::getButton(WidgetId::ButtonId id) const {
	return buttons.at(id);
};

const Action::Surface Widget::getButtonAction() const {
	return currentButtonAction;
}

const BUTTON_ID Widget::getButtonHovered() const {
	return currentButtonHovered;
}

bool Widget::onButtonHover(BUTTON_ID id, bool isHovered) {

	if (isHovered && !buttonHovers.at(id) && currentButtonHovered != id) {
		buttonHovers[id] = 1;
		buttonHovers[lastButtonHovered] = 0;
		lastButtonHovered = currentButtonHovered;
		currentButtonHovered = id;
		return true;
	} else if (!isHovered && currentButtonHovered == id) {
		buttonHovers[id] = 0;
		currentButtonHovered = BUTTON_ID::NIL;
		return false;
	} else if (isHovered && currentButtonHovered == id && lastButtonHovered != currentButtonHovered) {
		lastButtonHovered = currentButtonHovered;
	}

	return false;
}

bool Widget::onButtonJustHovered() const {
	return currentButtonHovered != BUTTON_ID::NIL && lastButtonHovered != currentButtonHovered;
}

bool Widget::onButtonJustBlurred() const {
	return currentButtonHovered == BUTTON_ID::NIL && lastButtonHovered != currentButtonHovered;
}

void Widget::triggerButtonAction(const char* elementId) {
	for (auto& button : buttons) {
		if (strcmp(button.clayId.stringId.chars, elementId) == 0) {
			currentButtonAction = button.action;
		}
	}
}

void Widget::clearButtonAction() {
	currentButtonAction = Action::Surface::DO_NOTHING;
}

Action::Surface Widget::consumeButtonAction() {
	if (!currentButtonAction) return currentButtonAction;

	Action::Surface buttonAction = currentButtonAction;
	clearButtonAction();

	return buttonAction;
}