#pragma once

#include <raymath.h>

#define TARGET_FPS 120
// screen defaults
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
// centimeters/pixel at 1280x720px
// to fit the table properly
#define SCREEN_UNIT 4.5f
#define SCREEN_RESIZE_RATE 300

#define UNIT_RATIO (sqrtf(powf(static_cast<float>(SCREEN_WIDTH), 2.0f) + powf(static_cast<float>(SCREEN_HEIGHT), 2.0f)))

#define INTRO_TIME_MS 8000