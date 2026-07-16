#include "widget.hpp"

#include "clay.h"
#include "type.hpp"

#include <cstring>

void onButtonClick(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Widget* widget = static_cast<Widget*>(userData);
    
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    	widget->triggerButtonAction(elementId.stringId.chars);
    }
}

void Widget::layoutButtonTexture(const BUTTON_ID id, const Clay_ElementId& elementId, Texture2D* buttonTexture) {
    CLAY(elementId, { 
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_FIXED(static_cast<float>(buttonTexture->width)),
                .height = CLAY_SIZING_FIXED(static_cast<float>(buttonTexture->height))
            },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
        }, 
        .image = { .imageData = buttonTexture },
    }) {
        // Clay_Hovered only works inside the CLAY declaration body
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(onButtonClick, this);
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
        .backgroundColor = Clay_Hovered() ? SURFACE_BUTTON_COLOR_BG_HL : SURFACE_BUTTON_COLOR_BG,
        .border = { 
            .color = Clay_Color({ 220, 220, 220, 255 }), 
            .width = CLAY_BORDER_OUTSIDE(1) 
        },
    }) {
        // Clay_Hovered only works inside the CLAY declaration body
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
    	Clay_OnHover(onButtonClick, this);
        CLAY_TEXT(button.label, CLAY_TEXT_CONFIG({ .textColor = SURFACE_BUTTON_COLOR_FG, .fontSize = 24 }));
    }
}

void Widget::layoutTab(const BUTTON_ID id) {
	const Button& button = getButton(id);
    CLAY(button.clayId, { 
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
        CLAY_TEXT(button.label, CLAY_TEXT_CONFIG({ .textColor = SURFACE_BUTTON_COLOR_FG, .fontSize = 24 }));
    }
}

void Widget::updateScrollbar(InputEvent inputEvent, const Clay_ElementId& parentId) {
    bool isMouseDown = inputEvent.id == Event::Input::PRIMARY_DOWN;

    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(inputEvent.position);
    Clay_SetPointerState(mousePosition, isMouseDown && !scrollbarData.mouseDown);

    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;
    Clay_UpdateScrollContainers(true, Clay_Vector2({ mouseWheelX, mouseWheelY }), GetFrameTime());

    if (inputEvent.id == Event::Input::PRIMARY_UP) {
        scrollbarData.mouseDown = false;
    }

    Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(parentId);
    // TODO: abstract Scrollbar ElementId
    if (isMouseDown && !scrollbarData.mouseDown && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar")))) {
        scrollbarData.clickOrigin = mousePosition;
        scrollbarData.positionOrigin = *scrollContainerData.scrollPosition;
        scrollbarData.mouseDown = true;

    } else if (scrollbarData.mouseDown) {
        // Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("TabContent")));
        if (scrollContainerData.contentDimensions.height > 0) {
            Clay_Vector2 ratio = Clay_Vector2({
                scrollContainerData.contentDimensions.width / scrollContainerData.scrollContainerDimensions.width,
                scrollContainerData.contentDimensions.height / scrollContainerData.scrollContainerDimensions.height,
            });

            if (scrollContainerData.config.vertical) {
                scrollContainerData.scrollPosition->y = scrollbarData.positionOrigin.y + (scrollbarData.clickOrigin.y - mousePosition.y) * ratio.y;
            }
            
            if (scrollContainerData.config.horizontal) {
                scrollContainerData.scrollPosition->x = scrollbarData.positionOrigin.x + (scrollbarData.clickOrigin.x - mousePosition.x) * ratio.x;
            }
        }
    }

    if(scrollContainerData.scrollPosition) {

        scrollbarData.scrollY = scrollContainerData.scrollPosition->y - scrollbarData.positionOrigin.y;
        // TraceLog(LOG_INFO, "scroll %f", scrollbarData.scrollY);
    }
}

void Widget::layoutScrollBar(const Clay_ElementId& parentId) {
    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(parentId);
    if (scrollData.found && scrollData.scrollContainerDimensions.height < scrollData.contentDimensions.height) {
        CLAY(CLAY_ID("ScrollBar"), {
            .floating = {
                .offset = { 
                	.y = -(scrollData.scrollPosition->y / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height 
                },
                // .parentId = Clay_GetElementId(CLAY_STRING("TabContent")).id,
                .parentId = parentId.id,
                .zIndex = 2,
                .attachPoints = { 
                	.element = CLAY_ATTACH_POINT_RIGHT_TOP, 
                	.parent = CLAY_ATTACH_POINT_RIGHT_TOP 
                },
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
            }
        }) {
            CLAY(CLAY_ID("ScrollBarButton"), {
                .layout = { 
                	.sizing = { 
                		CLAY_SIZING_FIXED(12), 
                		CLAY_SIZING_FIXED((scrollData.scrollContainerDimensions.height / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height) 
                	}
                },
                .backgroundColor = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar"))) ? Clay_Color({100, 100, 140, 150}) : Clay_Color({120, 120, 160, 150}),
                .cornerRadius = CLAY_CORNER_RADIUS(6),
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