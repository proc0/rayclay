#pragma once

#include "type.hpp"

#include "clay.h"

#include <array>
#include <vector>

// TODO: make consistent button ids and their actions in type header
namespace WidgetId {
	// WARNING: used for indices to Widget.buttons
	// needs to be the same order as BUTTONS macro
	enum ButtonId {
		NIL,
		NEW_GAME,
		RESUME,
		MOVE_UP,
		MOVE_RIGHT,
		MOVE_DOWN,
		MOVE_LEFT,
		RETURN,
		RELOAD,
		RESTART,
		SAVE_GAME,
		OPTIONS,
		CONFIRM_RETURN,
		CONFIRM_TUTORIAL,
		CANCEL_RETURN,
		OPTIONS_GAME,
		OPTIONS_AUDIO,
		OPTIONS_INPUTS,
		CONFIRM_OPTIONS,
		CANCEL_OPTIONS,
		QUIT
	};
}

using BUTTON_ID = WidgetId::ButtonId;

// menu buttons per screen
#define BUTTONS_MENU_MAIN const std::array<BUTTON_ID, 4> buttonsMenuMain{ \
		BUTTON_ID::NEW_GAME,\
		BUTTON_ID::RELOAD,\
		BUTTON_ID::OPTIONS,\
		BUTTON_ID::QUIT,\
	};

#define BUTTONS_MENU_PAUSE const std::array<BUTTON_ID, 6> buttonsMenuPause{ \
		BUTTON_ID::RESUME,\
		BUTTON_ID::SAVE_GAME,\
		BUTTON_ID::RELOAD,\
		BUTTON_ID::OPTIONS,\
		BUTTON_ID::RETURN,\
		BUTTON_ID::QUIT,\
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
BUTTON(NIL, 		   "ButtonNil", 		DO_NOTHING,			   "Nothing") \
BUTTON(NEW_GAME, 	   "ButtonNewGame", 	NEW_GAME,			  "New Game") \
BUTTON(RESUME, 		   "ButtonGameResume",  RESUME_GAME,			"Resume") \
BUTTON(MOVE_UP, 	   "HUDMoveUp",  		MOVE_UP,					"Up") \
BUTTON(MOVE_RIGHT, 	   "HUDMoveRight",  	MOVE_RIGHT,				 "Right") \
BUTTON(MOVE_DOWN, 	   "HUDMoveDown",  		MOVE_DOWN,				  "Down") \
BUTTON(MOVE_LEFT, 	   "HUDMoveLeft",  		MOVE_LEFT,				  "Left") \
BUTTON(RETURN, 		   "ButtonMainMenu", 	MAIN_MENU, "Return to Main Menu") \
BUTTON(RELOAD, 		   "ButtonGameLoad", 	LOAD_GAME,			 "Load Game") \
BUTTON(RESTART, 	   "ButtonGameRestart", RESTART,			   "Restart") \
BUTTON(SAVE_GAME, 	   "ButtonGameSave", 	SAVE_GAME,			 "Save Game") \
BUTTON(OPTIONS, 	   "ButtonOptions", 	OPTIONS, 			   "Options") \
BUTTON(CONFIRM_RETURN, "ButtonConfirmReturn", CONFIRM_RETURN,	       "Yes") \
BUTTON(CONFIRM_TUTORIAL, "ButtonConfirmTutorial", CONFIRM_TUTORIAL,  "Begin") \
BUTTON(CANCEL_RETURN,   "ButtonCancelReturn", CANCEL_RETURN, 	    	"No") \
BUTTON(OPTIONS_GAME,    "TabOptionsGame", 		CHANGE_OPTIONS_GAME,   "Game") \
BUTTON(OPTIONS_AUDIO, 	"TabOptionsAudio", 		CHANGE_OPTIONS_AUDIO,	 "Audio") \
BUTTON(OPTIONS_INPUTS,  "TabOptionsInputs", 	CHANGE_OPTIONS_INPUTS, "Inputs") \
BUTTON(CONFIRM_OPTIONS, "ButtonConfirmOptions", CONFIRM_OPTIONS,	  "Save") \
BUTTON(CANCEL_OPTIONS,  "ButtonCancelOptions",  CANCEL_OPTIONS, 	"Cancel") \
BUTTON(QUIT, 		   "ButtonQuit", 			QUIT_APP, 	          "Quit")

struct ScrollState {
	Clay_ElementId id;
	Clay_ElementId parentId;
	Clay_ElementId proxyId;
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    float scrollY;
    bool isPrimaryDown;
};

class Widget {
	const std::vector<Button> buttons = {
#define BUTTON(BID, CID, ACTION, LABEL) Button({ BUTTON_ID::BID, CLAY_ID(CID), Action::Surface::ACTION, CLAY_STRING(LABEL) }),
BUTTONS
#undef BUTTON
	};
	std::vector<char> buttonHovers;


	Action::Surface currentButtonAction = Action::Surface::DO_NOTHING;
	BUTTON_ID lastButtonHovered = BUTTON_ID::NIL;
	BUTTON_ID currentButtonHovered = BUTTON_ID::NIL;

public:
	ScrollState scrollState = {0};

	BUTTONS_MENU_MAIN
	BUTTONS_MENU_PAUSE

	Widget() : buttonHovers(buttons.size(), 0) {};
	~Widget() = default;

	void initScrollbar(Clay_ElementId parentId, Clay_ElementId scrollbarId, Clay_ElementId proxyId);
	void updateScrollbar(InputEvent, const Clay_Vector2& mousePosition, const Clay_ElementId& parentId, Clay_ElementId scrollbarId);

	const Button& getButton(WidgetId::ButtonId) const;
	const Button& getButton(Action::Surface) const;
	const Action::Surface getButtonAction() const;
	const BUTTON_ID getButtonHovered() const;
	bool onButtonHover(BUTTON_ID id, bool isHovered);
	bool onButtonJustHovered() const;
	bool onButtonJustBlurred() const;
	void triggerButtonAction(const char* elementId);
	void clearButtonAction();
	Action::Surface consumeButtonAction();

	void layoutLabel(const std::string& label);
	void layoutButton(const BUTTON_ID);
	void layoutTab(const BUTTON_ID, bool active);
	void layoutButtonTexture(const BUTTON_ID, Texture2D* buttonTexture);
	void layoutScrollBar(const Clay_ElementId& parentId, Clay_ElementId scrollbarId);
};

#undef BUTTONS
#endif

constexpr float INV255 = 1.0f/255.0f;

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) Clay_Vector2({ .x = vector.x, .y = vector.y })
#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) Rectangle({ .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height })
// NOTE: if color conversion is off, check Clay color representation type, it might need to be rounded. Clay uses 0.0f-255.0f, Raylib is unsighed char.
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) Color({ .r = static_cast<unsigned char>(roundf(color.r)), .g = static_cast<unsigned char>(roundf(color.g)), .b = static_cast<unsigned char>(roundf(color.b)), .a = static_cast<unsigned char>(roundf(color.a)) })
#define RAYLIB_COLOR_TO_CLAY_COLOR(color) Clay_Color({ static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b), static_cast<float>(color.a) })

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

struct DisplayButtonContext {
    void* self;
    Action::Surface action;
};
