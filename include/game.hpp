#pragma once

#include "types.hpp"
#include "defaults.hpp"
#include "screen.hpp"

#include <raylib.h>
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

class Game : public ScreenListener {
    const char* raylibName = "raylib";
    const int raylibLogoFontSize = 40;
    const float raylibLogoSize = 200;
    const int raylibLogoBorder = 16;
    const float raylibLogoInnerSize = raylibLogoSize - 2.0f*raylibLogoBorder;
    Vector2 raylibLogoDir = { static_cast<float>(GetRandomValue(-100, 100))/100.0f, static_cast<float>(GetRandomValue(-100, 100))/100.0f };
    Vector2 raylibLogoPos = { static_cast<float>(GetRandomValue(0, SCREEN_WIDTH-raylibLogoSize)), static_cast<float>(GetRandomValue(0, SCREEN_HEIGHT-raylibLogoSize)) };
    Rectangle raylibLogoOuterRec;
    Rectangle raylibLogoInnerRec;
    Vector2 raylibLogoTextPos;

    GameState gameState = GameState{0};
    const Screen& screen;
    State::Game state = State::Game::START;

public:
    Game(const Screen& screen): screen(screen) {}
    ~Game() = default;

    void load();
    void loadRaylibLogo();

    void renderNull() const;
    void (Game::*render)() const = &Game::renderNull;
    void renderGame() const;
    void renderMain() const;
    void renderRaylibLogo() const;

    GameState updateNull(State::App, InputEvent);
    GameState (Game::*update)(State::App, InputEvent) = &Game::updateNull;
    GameState updateMain(State::App, InputEvent);
    GameState updateGame(State::App, InputEvent);
    void updateRaylibLogo();

    void changeState(State::AppScreen);
    void resize(int height, int width);
    void unload();
};
