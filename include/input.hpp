#pragma once

#include "types.hpp"

#include <raylib.h>

class Input {
    int currentGesture = GESTURE_NONE;
    int lastGesture = GESTURE_NONE;

public:
	InputEvent update();
};
