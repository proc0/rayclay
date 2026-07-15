#pragma once

#include "type.hpp"

#include "clay.h"

#include <array>
#include <cstring>
#include <vector>

#define SURFACE_BUTTON_COLOR_BG Clay_Color({ 0, 0, 0, 100 })
#define SURFACE_BUTTON_COLOR_BG_HL Clay_Color({ 0, 0, 0, 130 })
#define SURFACE_BUTTON_COLOR_FG Clay_Color({ 200, 200, 200, 255 })
#define SURFACE_BUTTON_COLOR_FG_HL Clay_Color({ 255, 255, 255, 255 })
#define SURFACE_MENU_COLOR_BG Clay_Color({ 0, 0, 0, 150 })

namespace WidgetId {
	// WARNING: used for indices to Widget.buttons
	// needs to be the same order as BUTTONS macro
	enum ButtonId {
		NIL,
		NEW,
		RESUME,
		RETURN,
		RELOAD,
		OPTIONS,
		CONFIRM,
		CANCEL,
		QUIT
	};
}

using BUTTON_ID = WidgetId::ButtonId;

// menu buttons per screen
#define BUTTONS_MENU_MAIN const std::array<BUTTON_ID, 4> buttonsMenuMain{ \
		BUTTON_ID::NEW, 	\
		BUTTON_ID::RELOAD, 	\
		BUTTON_ID::OPTIONS, \
		BUTTON_ID::QUIT, 	\
	};

#define BUTTONS_MENU_PAUSE const std::array<BUTTON_ID, 5> buttonsMenuPause{ \
		BUTTON_ID::RESUME, 	\
		BUTTON_ID::RELOAD, 	\
		BUTTON_ID::RETURN, 	\
		BUTTON_ID::OPTIONS, \
		BUTTON_ID::QUIT,	\
	};

struct Button {
	const BUTTON_ID id;
	const Clay_ElementId clayId;
	const Action::Surface action;
	const Clay_String label;
};

#ifndef BUTTONS
// WARNING: order should match WidgetId::ButtonId enum
#define BUTTONS \
BUTTON(NIL, 	"ButtonNil", 		DO_NOTHING,			   "Nothing") \
BUTTON(NEW, 	"ButtonNewGame", 	NEW_GAME,			  "New Game") \
BUTTON(RESUME, 	"ButtonGameResume", RESUME_GAME,			"Resume") \
BUTTON(RETURN, 	"ButtonMainMenu", 	MAIN_MENU, "Return to Main Menu") \
BUTTON(RELOAD, 	"ButtonGameLoad", 	LOAD_GAME,			 "Load Game") \
BUTTON(OPTIONS, "ButtonOptions", 	OPTIONS, 			   "Options") \
BUTTON(CONFIRM, "ButtonConfirm", 	CONFIRM_RETURN_MAIN,	   "Yes") \
BUTTON(CANCEL, 	"ButtonCancel", 	CANCEL_RETURN_MAIN, 	    "No") \
BUTTON(QUIT, 	"ButtonQuit", 		QUIT_APP, 	              "Quit")

class Widget {
	Action::Surface currentButtonAction = Action::Surface::DO_NOTHING;
	BUTTON_ID currentButtonHovered = BUTTON_ID::NIL;

	const std::vector<Button> buttons = {
#define BUTTON(BID, CID, ACTION, LABEL) Button({ BUTTON_ID::BID, CLAY_ID(CID), Action::Surface::ACTION, CLAY_STRING(LABEL) }),
BUTTONS
#undef BUTTON
	};

public:
	BUTTONS_MENU_MAIN
	BUTTONS_MENU_PAUSE

	Widget() {};
	~Widget() = default;

	const Button& getButton(WidgetId::ButtonId id) const {
		return buttons.at(id);
	};

	const Action::Surface getAction() const {
		return currentButtonAction;
	}

	const BUTTON_ID getButtonHovered() const {
		return currentButtonHovered;
	}

	void hoverButton(BUTTON_ID id) {
		currentButtonHovered = id;
	}

	void triggerAction(const char* elementId) {
		for (auto& button : buttons) {
			if (strcmp(button.clayId.stringId.chars, elementId) == 0) {
				currentButtonAction = button.action;
			}
		}
	}

	void clearAction() {
		currentButtonAction = Action::Surface::DO_NOTHING;
	}

	void createButton(const Clay_ElementId& clayId, const Clay_String& label);
};

#undef BUTTONS
#endif

#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) Color({ .r = static_cast<unsigned char>(roundf(color.r)), .g = static_cast<unsigned char>(roundf(color.g)), .b = static_cast<unsigned char>(roundf(color.b)), .a = static_cast<unsigned char>(roundf(color.a)) })
#define RAYLIB_COLOR_TO_CLAY_COLOR(color) Clay_Color({ static_cast<float>(roundf(color.r)), static_cast<float>(roundf(color.g)), static_cast<float>(roundf(color.b)), static_cast<float>(roundf(color.a)) })
#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) Clay_Vector2({ .x = vector.x, .y = vector.y })

enum CustomLayoutElementType {
    CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL
};

struct CustomLayoutElement_3DModel {
    Model model;
    float scale;
    Vector3 position;
    Matrix rotation;
};

struct CustomLayoutElement {
    CustomLayoutElementType type;
    union {
        CustomLayoutElement_3DModel model;
    } customData;
};

struct ScrollbarData {
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    float scrollY;
    bool mouseDown;
};

struct DisplayButtonContext {
    void* self;
    Action::Surface action;
};
