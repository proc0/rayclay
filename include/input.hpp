#pragma once

#include "type.hpp"

#include "raylib.h"

class Input {
    int currentGesture = GESTURE_NONE;
    int lastGesture = GESTURE_NONE;

public:
	InputEvent update();
    bool updateAnyKey();
};
