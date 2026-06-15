#pragma once

#include "types.hpp"

// #ifdef __EMSCRIPTEN__
// #include <emscripten.h>
// #endif
// #include <chrono>
// #include <raylib.h>

// #include "config.h"
// #include "display.hpp"

// // initial settings
// #define SCREEN_WIDTH 1280
// #define SCREEN_HEIGHT 720
// #define TARGET_FPS 120

// #ifdef __EMSCRIPTEN__
// // padding for web components
// // NOTE: canvas width should not be 100% in CSS
// #define WINDOW_PAD 0
// #endif
// // resize debounce, prevents too many calls for Web
// static inline constexpr auto RESIZE_COOLDOWN = std::chrono::milliseconds(300);

// class Game {
//     World world;
//     Display display;

//     std::chrono::steady_clock::time_point lastResize = std::chrono::steady_clock::now();
//     enum State {
//         BEGIN, // initialization
//         START, // intro and menu screen
//         PLAY,  
//         READY, // level transition
//         PAUSE, 
//         OVER,  // game over
//         WIN,   // win conditions met
//         END    // breaks loop
//     };
//     State state = BEGIN;
    
//     public:
//     int screenWidth = SCREEN_WIDTH;
//     int screenHeight = SCREEN_HEIGHT;
    
//     Game() {};
//     ~Game() = default;

//     const bool isRunning() const;
//     void load();
//     static void loop(void* self);
//     void render() const;
//     void resize();
//     void run();
//     void unload();
//     void update();
// };

class Game {
    
    State::Game state = State::Game::STALL;

public:
    void load();
    void render() const;
    void resize();
    void unload();
    void update();
};
