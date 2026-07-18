#include "widget.hpp"

// #include "clay.h"
#include "type.hpp"
#include "style.hpp"

#include <cstdint>
#include <cstring>

// the initialState is set to a targetState for the transition, and the transition properties are set in the element
Clay_TransitionData FadeSlide(Clay_TransitionData initialState, Clay_TransitionProperty properties) {
    Clay_TransitionData targetState = initialState;
    // small slide-in effect 
    if (properties & CLAY_TRANSITION_PROPERTY_POSITION) {
        targetState.boundingBox.y = targetState.boundingBox.y - 10.0f;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR) {
        targetState.backgroundColor.a = 0.0f;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_BORDER_COLOR) {
        targetState.borderColor.b = 0.0f;
    }
    return targetState;
}

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
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    }) {
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
    	Clay_OnHover(handleClayHover, this);
        CLAY_TEXT(button.label, STYLE_TEXT_CENTERED);
    }
}

void Widget::layoutButtonSecondary(const BUTTON_ID id) {
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
        .backgroundColor = Clay_Hovered() ? SURFACE_COLOR_SECONDARY : CLAY_BLANK,
        .border = { 
            .color = SURFACE_COLOR_BG, 
            .width = CLAY_BORDER_OUTSIDE(1) 
        },
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    }) {
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(handleClayHover, this);
        CLAY_TEXT(button.label, STYLE_TEXT_CENTERED);
    }
}

void Widget::layoutTab(const BUTTON_ID id) {
	const Button& button = getButton(id);
    bool isActive = getActiveTab() == button.id;
    CLAY(button.clayId, { 
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
            },
            .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(isActive ? 10 : 6)),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_BOTTOM },
        }, 
        // Clay_Hovered only works inside the paramaters or Clay declaration body
        // .backgroundColor = Clay_Hovered() ? WIDGET_COLOR_BUTTON_BG_HL : WIDGET_COLOR_BUTTON_BG,
        .border = { 
            .color = WIDGET_COLOR_BORDER, 
            .width = { 0, 0, 0, isActive } 
        },
    }) {
        onButtonHover(id, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(handleClayHover, this);
        if (isActive) {
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
Clay_ElementId Widget::initScrollBox(const std::string& stringId) {
    scrollState.id = CLAY_ID("ScrollBar");
    // TODO: refactor into something that builds Clay Strings
    Clay_String clayString = CLAY__INIT(Clay_String){ .isStaticallyAllocated = true, .length = static_cast<int32_t>(stringId.length()), .chars = stringId.c_str() };
    Clay_ElementId containerId = CLAY_SID(clayString);
    scrollState.parentId = containerId;

    // TODO: figure out how to pass/configure proxyId
    // to work with layouts in Surface so you can scroll backgrounds in other containers
    // scrollState.proxyId = proxyId;

    return containerId; 
}

// TODO: use the scrollState within Widget directly without needing to pass in parentId and scrollId, since Ids are needed as a proxy to pass in userData
// this however would require updateScrollbar to take in a ScrollState (to decouple), and that would be unecessary unless there is a list of ScrollStates
// for multiple scrollbars. 
void Widget::updateScrollBox(InputEvent inputEvent, const Clay_Vector2& mousePosition) {

    Clay_UpdateScrollContainers(true, Clay_Vector2({ inputEvent.mouseWheel.x*2.0f, inputEvent.mouseWheel.y*2.0f }), GetFrameTime());

    Clay_ScrollContainerData container = Clay_GetScrollContainerData(scrollState.parentId);
    // WARNING: crashes without this check!
    if(container.scrollPosition) {
        // update the vertical scroll movement for mouse wheel, and mouse grab (content drag)
        scrollState.scrollY = container.scrollPosition->y - scrollState.positionOrigin.y;
    }

    if (inputEvent.id == Event::Input::PRIMARY_UP) {
        scrollState.isPrimaryDown = false;
        return;
    }

    if (inputEvent.id == Event::Input::PRIMARY_DOWN && !scrollState.isPrimaryDown && Clay_PointerOver(scrollState.id)) {

        scrollState.clickOrigin = mousePosition;
        scrollState.positionOrigin = *container.scrollPosition;
        scrollState.isPrimaryDown = true;

    } else if (scrollState.isPrimaryDown) {
        // TODO: fix pulling the content too far up or too far down causing the scrollbar to go beyond the content
        // and if there is an image background, it offsets it too much and causes glitching
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
            CLAY_AUTO_ID({
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

// TODO: this would recieve a ScrollStateId,
// and then use those Ids for declaring the container and scrollbar
void Widget::BeginScrollBox() {
    // Clay__OpenElement();
    // Clay__ConfigureOpenElement((Clay_ElementDeclaration) {
    //   .id = CLAY_ID("Container"),
    //   .backgroundColor = { 255, 200, 200, 255 }
    // });
    // ...children declared here
    // Clay__CloseElement();
    Clay__OpenElementWithId(scrollState.parentId);
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
            .layout = { 
                .padding = CLAY_PADDING_ALL(32), 
                .childGap = 12, 
                .layoutDirection = CLAY_TOP_TO_BOTTOM 
            },
            .clip = { 
                .vertical = true, 
                .childOffset = Clay_GetScrollOffset()
            },
        });
}

// TODO: this would receive a ScrollStateId, pointing to
// one of many ScrollStates, or it would know which one is open
// and close it automatically
void Widget::EndScrollBox() {
  layoutScrollBar(scrollState.parentId, scrollState.id);
  Clay__CloseElement();

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

Action::Surface Widget::getButtonAction() const {
	return currentButtonAction;
}

BUTTON_ID Widget::getButtonHovered() const {
	return currentButtonHovered;
}

bool Widget::onButtonHover(BUTTON_ID id, bool isHovered) {
    // gets called on every frame with every button
    // compare the cached buttons with the current button hovered
    // and move the current hovered to the last hovered
	if (isHovered && !buttonHovers[id] && currentButtonHovered != id) {
        // mark the button as hovered
		buttonHovers[id] = 1;
		buttonHovers[lastButtonHovered] = 0;
		lastButtonHovered = currentButtonHovered;
		currentButtonHovered = id;

		return true;
	} else if (isHovered && currentButtonHovered == id && lastButtonHovered != currentButtonHovered) {
        // this allows for one frame of propagation of when the button was hovered
        // it can be queried to know the frame right after the button hovered
        lastButtonHovered = currentButtonHovered;
    } else if (!isHovered && currentButtonHovered == id) {
        // blur the current button
		buttonHovers[id] = 0;
		currentButtonHovered = BUTTON_ID::NIL;
	} else if (!isHovered && lastButtonHovered == id) {
        // this allows for one frame of propagation of the blur
        lastButtonHovered = BUTTON_ID::NIL;
    } 

    // return whether the button was hovered or not
    // to allow immediate query on the hover
	return false;
}

bool Widget::onButtonJustHovered() const {
	return currentButtonHovered != BUTTON_ID::NIL && lastButtonHovered != currentButtonHovered;
}

bool Widget::onButtonJustBlurred() const {
	return currentButtonHovered == BUTTON_ID::NIL && lastButtonHovered != BUTTON_ID::NIL;
}

void Widget::triggerButtonAction(const char* elementId) {
    BUTTON_ID currentId = BUTTON_ID::NIL;
	for (auto& button : buttons) {
		if (strcmp(button.clayId.stringId.chars, elementId) == 0) {
			currentButtonAction = button.action;
            currentId = button.id;
            break;
		}
	}

    for (auto& tabId : tabButtonIds) {
        if (currentId == tabId) {
            activeTab = currentId;
            break;
        }
    }
}

BUTTON_ID Widget::getActiveTab() const {
    return activeTab;
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
