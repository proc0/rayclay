#pragma once

#include "index.h"
#include "type.hpp"
#include "window.hpp"
#include "text.hpp"

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
        .score = 0,
        .state = State::Game::START
    };
    GameState defaultState = {
        .score = 0,
        .state = State::Game::START
    };
    State::Game state = State::Game::START;

    const Window& window;
    TimerId gameTimerId;

    bool paused;

public:
    Game(const Window& window): window(window) {}
    ~Game() = default;

    void (Game::*render)() const = &Game::renderUnit;
    GameState (Game::*update)(InputEvent, WorldState) = &Game::updateUnit;
    
    void load();
    void start();
    void reset();
    
    void renderUnit() const {};
    void renderGame() const;
    void renderMain() const;
    void renderTitle() const;

    GameState updateUnit(InputEvent, WorldState) { return defaultState; };
    GameState updateMain(InputEvent, WorldState);
    GameState updateGame(InputEvent, WorldState);
    void updateTitle();

    void transition(State::App, State::Screen);
    void resize(int height, int width) override;
    void unload();
};
