#pragma once

#include "index.h"
#include "type.hpp"
#include "window.hpp"

class Game : public Layer {
    const char* title = PROJECT_NAME;
    int titleFontSize = 128;
    float titleX;
    float titleY;

    const char* titleHint = TITLE_HINT;
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
    GameState (Game::*update)(InputEvent) = &Game::updateNull;
    
    void load();

    void renderNull() const;
    void renderGame() const;
    void renderMain() const;
    void renderTitle() const;

    GameState updateNull(InputEvent);
    GameState updateMain(InputEvent);
    GameState updateGame(InputEvent);
    void updateTitle();

    void transition(State::Screen);
    void resize(int height, int width) override;
    void unload();
};
