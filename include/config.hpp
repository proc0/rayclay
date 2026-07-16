#pragma once

#define TARGET_FPS 120
// screen defaults
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
// centimeters/pixel at 1280x720px
#define SCREEN_UNIT 1.0f
#define SCREEN_RESIZE_RATE 300

#define UNIT_RATIO (sqrtf(powf(static_cast<float>(SCREEN_WIDTH), 2.0f) + powf(static_cast<float>(SCREEN_HEIGHT), 2.0f)))

#define INTRO_TIME_MS 3000

#define RAYLIB_LOGO_SIZE 200