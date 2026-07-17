#include "input.hpp"

#include "type.hpp"

#include "raylib.h"

InputEvent Input::update() {

	Vector2 mousePosition = GetMousePosition();

    lastGesture = currentGesture;
    currentGesture = GetGestureDetected();
    Vector2 touchPosition = GetTouchPosition(0);

    Vector2 position = currentGesture != GESTURE_NONE ? touchPosition : mousePosition;
    Vector2 mouseWheelDelta = GetMouseWheelMoveV();

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || currentGesture == GESTURE_TAP) {
		return {
			.id = Event::Input::PRIMARY,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) || currentGesture == GESTURE_HOLD || currentGesture == GESTURE_DRAG) {
		return {
			.id = Event::Input::PRIMARY_DOWN,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || (currentGesture == GESTURE_NONE && lastGesture != GESTURE_NONE)) {
		return {
			.id = Event::Input::PRIMARY_UP,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || currentGesture == GESTURE_DOUBLETAP) {
		return {
			.id = Event::Input::SECONDARY,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		return {
			.id = Event::Input::SECONDARY_DOWN,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
		return {
			.id = Event::Input::SECONDARY_UP,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
		return {
			.id = Event::Input::TERTIARY,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
		return {
			.id = Event::Input::TERTIARY_DOWN,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
		return {
			.id = Event::Input::TERTIARY_UP,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	}

	// TODO: figure out how to send both key pressed and mouse inputs simultaneously
	// and whether raylibe /glfw supports this
	if (IsKeyPressed(KEY_W)) {
		return {
			.id = Event::Input::MOVE_UP,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsKeyPressed(KEY_D)) {
		return {
			.id = Event::Input::MOVE_RIGHT,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsKeyPressed(KEY_S)) {
		return {
			.id = Event::Input::MOVE_DOWN,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsKeyPressed(KEY_A)) {
		return {
			.id = Event::Input::MOVE_LEFT,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (IsKeyPressed(KEY_ESCAPE)) { 
		return {
			.id = Event::Input::KEY_ESCAPE,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	} else if (GetKeyPressed() != 0) {
		return {
			.id = Event::Input::KEY_OTHER,
			.position = position,
			.mouseWheel = mouseWheelDelta,
		};
	}

	return {
		.id = Event::Input::IDLE,
		.position = position,
		.mouseWheel = mouseWheelDelta,
	};
}

bool Input::updateAnyKey() {
	InputEvent event = update();

	return event.id == Event::Input::PRIMARY_UP || event.id == Event::Input::SECONDARY_UP || event.id == Event::Input::KEY_ESCAPE || event.id == Event::Input::KEY_OTHER;
}
