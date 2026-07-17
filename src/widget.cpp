#include "widget.hpp"

#include "clay.h"
#include "type.hpp"
#include "style.hpp"

#include <cstdint>
#include <cstring>

void handleClayHover(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Widget* widget = static_cast<Widget*>(userData);
    
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    	widget->triggerButtonAction(elementId.stringId.chars);
    }
}

void Widget::layoutButtonTexture(const BUTTON_ID id, Texture2D* textureData) {
    const Button& button = getButton(id);
    CLAY(button.clayId, { 
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_FIXED(static_cast<float>(textureData->width)),
                .height = CLAY_SIZING_FIXED(static_cast<float>(textureData->height))
            },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
        }, 
        .image = { .imageData = textureData },
    }) {
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(handleClayHover, this);
    }
}

void Widget::layoutButton(const BUTTON_ID id) {
	const Button& button = getButton(id);
    CLAY(button.clayId, { 
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
        }, 
        // Clay_Hovered only works inside the paramaters or Clay declaration body
        .backgroundColor = Clay_Hovered() ? WIDGET_COLOR_BUTTON_BG_HL : WIDGET_COLOR_BUTTON_BG,
        .border = { 
            .color = WIDGET_COLOR_BORDER, 
            .width = CLAY_BORDER_OUTSIDE(1) 
        },
    }) {
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
    	Clay_OnHover(handleClayHover, this);
        CLAY_TEXT(button.label, STYLE_TEXT_CENTERED);
    }
}

void Widget::layoutTab(const BUTTON_ID id, bool active) {
	const Button& button = getButton(id);
    CLAY(button.clayId, { 
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
            },
            .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(active ? 10 : 6)),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_BOTTOM },
        }, 
        // Clay_Hovered only works inside the paramaters or Clay declaration body
        // .backgroundColor = Clay_Hovered() ? WIDGET_COLOR_BUTTON_BG_HL : WIDGET_COLOR_BUTTON_BG,
        .border = { 
            .color = WIDGET_COLOR_BORDER, 
            .width = { 0, 0, 0, active } 
        },
    }) {
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(handleClayHover, this);
        if (active) {
            CLAY_TEXT(button.label, STYLE_TEXT_DEFAULT);
        } else {
            CLAY_TEXT(button.label, STYLE_TEXT_DISABLED);
        }
    }
}

void Widget::layoutLabel(const std::string& label) {
    Clay_String clayString = CLAY__INIT(Clay_String){ .isStaticallyAllocated = true, .length = static_cast<int32_t>(label.length()), .chars = label.c_str() };
    CLAY_TEXT(clayString, STYLE_TEXT_BANNER);
}

// TODO: create a vector of ScrollState, and return and Id. This Id is then passed into the update and layout
// methods of scrollbar, to look up the different Ids.
void Widget::initScrollbar(Clay_ElementId parentId, Clay_ElementId scrollbarId, Clay_ElementId proxyId) {
    scrollState.id = scrollbarId;
    scrollState.parentId = parentId;
    scrollState.proxyId = proxyId;
}

// TODO: use the scrollState within Widget directly without needing to pass in parentId and scrollId, since Ids are needed as a proxy to pass in userData
// this however would require updateScrollbar to take in a ScrollState (to decouple), and that would be unecessary unless there is a list of ScrollStates
// for multiple scrollbars. 
void Widget::updateScrollbar(InputEvent inputEvent, const Clay_Vector2& mousePosition, const Clay_ElementId& parentId, Clay_ElementId scrollbarId) {

    Clay_UpdateScrollContainers(true, Clay_Vector2({ inputEvent.mouseWheelDelta.x*2.0f, inputEvent.mouseWheelDelta.y*2.0f }), GetFrameTime());

    Clay_ScrollContainerData container = Clay_GetScrollContainerData(parentId);
    // WARNING: crashes without this check!
    if(container.scrollPosition) {
        // update the vertical scroll movement for mouse wheel, and mouse grab (content drag)
        scrollState.scrollY = container.scrollPosition->y - scrollState.positionOrigin.y;
    }

    if (inputEvent.id == Event::Input::PRIMARY_UP) {
        scrollState.isPrimaryDown = false;
        return;
    }

    if (inputEvent.id == Event::Input::PRIMARY_DOWN && !scrollState.isPrimaryDown && Clay_PointerOver(scrollbarId)) {

        scrollState.clickOrigin = mousePosition;
        scrollState.positionOrigin = *container.scrollPosition;
        scrollState.isPrimaryDown = true;

    } else if (scrollState.isPrimaryDown) {

        if (container.contentDimensions.height > 0) {
            Clay_Vector2 ratio = Clay_Vector2({
                container.contentDimensions.width / container.scrollContainerDimensions.width,
                container.contentDimensions.height / container.scrollContainerDimensions.height,
            });

            if (container.config.vertical) {
                container.scrollPosition->y = scrollState.positionOrigin.y + (scrollState.clickOrigin.y - mousePosition.y) * ratio.y;
            }
            
            if (container.config.horizontal) {
                container.scrollPosition->x = scrollState.positionOrigin.x + (scrollState.clickOrigin.x - mousePosition.x) * ratio.x;
            }
        }
    }


}

// TODO: when scrollState becomes a list, pass in the Id instead and get parent and scrollbar from there.
void Widget::layoutScrollBar(const Clay_ElementId& parentId, Clay_ElementId scrollbarId) {
    Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(parentId);
    if (scrollContainerData.found && scrollContainerData.scrollContainerDimensions.height < scrollContainerData.contentDimensions.height) {
        CLAY(scrollbarId, {
            .floating = {
                .offset = { 
                	.y = -(scrollContainerData.scrollPosition->y / scrollContainerData.contentDimensions.height) * scrollContainerData.scrollContainerDimensions.height 
                },
                .parentId = parentId.id,
                .zIndex = 2,
                .attachPoints = { 
                	.element = CLAY_ATTACH_POINT_RIGHT_TOP, 
                	.parent = CLAY_ATTACH_POINT_RIGHT_TOP 
                },
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
            }
        }) {
            CLAY(CLAY_ID("ScrollbarBarButton"), {
                .layout = { 
                	.sizing = { 
                		CLAY_SIZING_FIXED(12), 
                		CLAY_SIZING_FIXED((scrollContainerData.scrollContainerDimensions.height / scrollContainerData.contentDimensions.height) * scrollContainerData.scrollContainerDimensions.height) 
                	}
                },
                .backgroundColor = Clay_Hovered() || scrollState.isPrimaryDown ? WIDGET_COLOR_SCROLLBAR_HL : WIDGET_COLOR_SCROLLBAR,
            });
        }
    }
}

const Button& Widget::getButton(WidgetId::ButtonId id) const {
	return buttons.at(id);
};

const Button& Widget::getButton(Action::Surface action) const {
	for (auto& button : buttons) {
		if (button.action == action) {
			return button;
		}
	}

	return buttons.at(0);
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
	} else if (isHovered && currentButtonHovered == id && lastButtonHovered != currentButtonHovered) {
        lastButtonHovered = currentButtonHovered;
    } else if (!isHovered && currentButtonHovered == id) {
		buttonHovers[id] = 0;
		currentButtonHovered = BUTTON_ID::NIL;
		return false;
	} else if (!isHovered && lastButtonHovered == id) {
        lastButtonHovered = BUTTON_ID::NIL;
        return false;
    } 

	return false;
}

bool Widget::onButtonJustHovered() const {
	return currentButtonHovered != BUTTON_ID::NIL && lastButtonHovered != currentButtonHovered;
}

bool Widget::onButtonJustBlurred() const {
	return currentButtonHovered == BUTTON_ID::NIL && lastButtonHovered != BUTTON_ID::NIL;
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
