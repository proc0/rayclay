#include "widget.hpp"

void onButtonClick(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Widget* widget = static_cast<Widget*>(userData);
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        // std::string idStr(elementId.stringId.chars);
        // auto result = self->buttonActions.find(idStr.c_str());

        // Action::Surface action = Action::Surface::DO_NOTHING;
        // if (result != self->buttonActions.end()) {
        //     action = result->second;
        // } else {
        //     TraceLog(LOG_ERROR, "SURFACE ERROR: Button ID not found.");
        // }

        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    	widget->triggerAction(elementId.stringId.chars);
    }
}

void Widget::createButton(const Clay_ElementId& elementId, const Clay_String& buttonText) {
	// Clay_Color bgColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE);

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
        // if (Clay_Hovered() && buttonHoverId != elementId.id) {
        //     buttonHoverId = elementId.id;
        //     SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        //     textColor = SURFACE_BUTTON_COLOR_FG_HL;
        // }
    	Clay_OnHover(onButtonClick, this);
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontSize = 24 }));
    }
}