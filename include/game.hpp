#pragma once

#include "types.hpp"
// #include "defaults.hpp"
#include "window.hpp"

// #include "raylib.h"

class Game : public Layer {
    // const char* raylibName = "raylib";
    // const int raylibLogoFontSize = 40;
    // const float raylibLogoSize = 200;
    // const int raylibLogoBorder = 16;
    // const float raylibLogoInnerSize = raylibLogoSize - 2.0f*raylibLogoBorder;
    // Vector2 raylibLogoDir = { static_cast<float>(GetRandomValue(-100, 100))/100.0f, static_cast<float>(GetRandomValue(-100, 100))/100.0f };
    // Vector2 raylibLogoPos = { static_cast<float>(GetRandomValue(0, SCREEN_WIDTH-raylibLogoSize)), static_cast<float>(GetRandomValue(0, SCREEN_HEIGHT-raylibLogoSize)) };
    // Rectangle raylibLogoOuterRec;
    // Rectangle raylibLogoInnerRec;
    // Vector2 raylibLogoTextPos;

    GameState gameState = {
        .state = State::Game::START
    };
    State::Game state = State::Game::START;

    const Window& window;

public:
    Game(const Window& window): window(window) {}
    ~Game() = default;

    void load();
    void loadRaylibLogo();

    void renderNull() const;
    void (Game::*render)() const = &Game::renderNull;
    void renderGame() const;
    void renderMain() const;
    // void renderRaylibLogo() const;

    GameState updateNull(State::App, InputEvent);
    GameState (Game::*update)(State::App, InputEvent) = &Game::updateNull;
    GameState updateMain(State::App, InputEvent);
    GameState updateGame(State::App, InputEvent);
    // void updateRaylibLogo();

    void transition(State::Screen);
    void resize(int height, int width) override;
    void unload();
};
