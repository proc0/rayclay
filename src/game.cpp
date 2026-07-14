#include "game.hpp"

#include "type.hpp"

#include "raylib.h"

void Game::load() {
    updateTitle();
}

void Game::renderNull() const {

}

void Game::renderMain() const {

}

void Game::renderGame() const {

}

void Game::renderTitle() const {
    ClearBackground(GOLD);
    DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);
    DrawText(titleHint, titleHintX, titleHintY, titleHintFontSize, RAYWHITE);
}

GameState Game::updateNull(State::App appState, InputEvent inputEvent){
    return gameState;
}

GameState Game::updateMain(State::App appState, InputEvent inputEvent){
    return gameState;
}

GameState Game::updateGame(State::App appState, InputEvent inputEvent){

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

void Game::transition(State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            update = &Game::updateMain;
            render = &Game::renderMain;
            break;
        case State::Screen::GAME:
            update = &Game::updateGame;
            render = &Game::renderGame;
            break;
        default:
            update = &Game::updateNull;
    };
}

void Game::resize(int height, int width) {
    updateTitle();
}

void Game::unload(){

}
