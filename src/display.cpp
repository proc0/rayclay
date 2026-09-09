#include "display.hpp"

#include "brick.h"
#include "index.h"

#include "raylib.h"

void Display::load() {

    textureArrowUp = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_UP));
    textureArrowRight = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_RIGHT));
    textureArrowDown = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_DOWN));
    textureArrowLeft = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_LEFT));

    bid_arrowUp = Brick_CreateImageButton(static_cast<float>(textureArrowUp.width), static_cast<float>(textureArrowUp.height), &textureArrowUp);
    bid_arrowRight = Brick_CreateImageButton(static_cast<float>(textureArrowRight.width), static_cast<float>(textureArrowRight.height), &textureArrowRight);
    bid_arrowDown = Brick_CreateImageButton(static_cast<float>(textureArrowDown.width), static_cast<float>(textureArrowDown.height), &textureArrowDown);
    bid_arrowLeft = Brick_CreateImageButton(static_cast<float>(textureArrowLeft.width), static_cast<float>(textureArrowLeft.height), &textureArrowLeft);

}

Action::Display Display::update() {
    Action::Display action = Action::Display::NONE;

    if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_arrowUp)) {
        TraceLog(LOG_INFO, "UP UP UP");
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_arrowRight)) {
        TraceLog(LOG_INFO, "RIGHT");
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_arrowDown)) {
        TraceLog(LOG_INFO, "DOWN");
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_arrowLeft)) {
        TraceLog(LOG_INFO, "LEFT");
    }

    return action;
}

void Display::layoutGame() {
    Brick_BeginFloatingPanel();
        Brick_BeginHorizontalStack();
    		Brick_BeginVerticalStack();
	        	Brick_LayoutImageButton(bid_arrowLeft);
        	Brick_EndHorizontalStack();
    		Brick_BeginVerticalStack();
		        Brick_LayoutImageButton(bid_arrowUp);
		        Brick_LayoutImageButton(bid_arrowDown);
    		Brick_EndVerticalStack();
    		Brick_BeginVerticalStack();
	        	Brick_LayoutImageButton(bid_arrowRight);
    		Brick_EndVerticalStack();
        Brick_EndHorizontalStack();
    Brick_EndFloatingPanel();
}

void Display::render() const {
	
}

void Display::transition(State::App state, State::Screen screen) {
    switch(screen) {
    case State::Screen::MAIN:
        layout = &Display::layoutUnit;
    break;
    case State::Screen::GAME:
        layout = state == State::App::HOLD ? &Display::layoutUnit : &Display::layoutGame;
    break;
    default: 
        layout = &Display::layoutUnit;
    }
}

void Display::unload() {
    UnloadTexture(textureArrowUp);
    UnloadTexture(textureArrowRight);
    UnloadTexture(textureArrowDown);
    UnloadTexture(textureArrowLeft);
}