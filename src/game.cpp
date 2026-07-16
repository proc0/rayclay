#include "game.hpp"

#include "index.h"
#include "type.hpp"

#include "raylib.h"

void Game::load() {
    updateTitle();
}

void Game::start() {
    state = State::Game::PLAY;
}

void Game::reset() {
    gameState.score = 0;
}

void Game::renderMain() const {
    DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);
}

void Game::renderGame() const {

}

void Game::renderTitle() const {
    ClearBackground(GOLD);
    DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);
    DrawText(titleHint, titleHintX, titleHintY, titleHintFontSize, RAYWHITE);
}

GameState Game::updateMain(InputEvent, WorldState){
    return gameState;
}

GameState Game::updateGame(InputEvent inputEvent, WorldState worldState){
    if (paused) return gameState;

    if (worldState.reachedGoal) {
        gameState.score++;
    }
    
    return gameState;
}

void Game::updateTitle() {
    float titleTextSize = MeasureText(title, titleFontSize);
    titleX = window.halfWidth - titleTextSize*0.5f;
    titleY = window.halfHeight - titleFontSize*0.5f;

    float titleHintTextSize = MeasureText(titleHint, titleHintFontSize);
    titleHintX = window.halfWidth - titleHintTextSize*0.5f;
    titleHintY = static_cast<int>(window.height - window.height*0.25f - titleHintFontSize*0.5f);
}

void Game::transition(State::App appState, State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            update = &Game::updateMain;
            render = &Game::renderMain;
            break;
        case State::Screen::GAME:
            paused = appState == State::App::HOLD;
            update = &Game::updateGame;
            render = &Game::renderGame;
            break;
        default:
            update = &Game::updateUnit;
            render = &Game::renderUnit;
    };
}

void Game::resize(int height, int width) {
    // TODO: only resize on Title screen
    updateTitle();
}

void Game::unload(){

}
