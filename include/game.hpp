#pragma once

#include "types.hpp"
// #include "defaults.hpp"
#include "window.hpp"

// #include "raylib.h"

class Game : public Layer {
    const char* title = "GAME TITLE";
    int titleFontSize = 128;
    float titleX;
    float titleY;

    const char* titleHint = "Press any key";
    int titleHintFontSize = 32;
    float titleHintX;
    float titleHintY;

    GameState gameState = {
        .state = State::Game::START
    };
    State::Game state = State::Game::START;

    const Window& window;

public:
    Game(const Window& window): window(window) {}
    ~Game() = default;

    void (Game::*render)() const = &Game::renderNull;
    GameState (Game::*update)(State::App, InputEvent) = &Game::updateNull;
    
    void load();
    void loadRaylibLogo();

    void renderNull() const;
    void renderGame() const;
    void renderMain() const;
    void renderTitle() const;

    GameState updateNull(State::App, InputEvent);
    GameState updateMain(State::App, InputEvent);
    GameState updateGame(State::App, InputEvent);
    void updateTitle();

    void transition(State::Screen);
    void resize(int height, int width) override;
    void unload();
};
