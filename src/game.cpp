#include "game.hpp"

#include "type.hpp"

#include "raylib.h"

void Game::load() {
    updateTitle();
}

void Game::start() {
    state = State::Game::PLAY;
    gameState.state = state;
    gameTimerId = window.timer.startWatch();
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
    DrawRectangleGradientV(0, 0, window.width, window.height, DARKBLUE, ORANGE);
    DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);
    DrawText(titleHint, titleHintX, titleHintY, titleHintFontSize, RAYWHITE);
}

GameState Game::updateMain(InputEvent, WorldState){
    return gameState;
}

GameState Game::updateGame(InputEvent inputEvent, WorldState worldState){
    if (state != State::Game::PLAY) return gameState;
    
    if (paused) {
        state = State::Game::PAUSE;
        gameState.state = state;
        return gameState;
    }

    if (worldState.reachedGoal) {
        gameState.score++;
    }

    if (worldState.failedGoal) {
        gameState.score--;
    }
 
    if (gameState.score > 2) {
        state = State::Game::WIN;
        gameState.state = state;
        window.timer.stopWatch(gameTimerId);
        gameState.totalTimeId = gameTimerId;
    } else if (gameState.score < -2) {
        state = State::Game::OVER;
        gameState.state = state;
        window.timer.stopWatch(gameTimerId);
        gameState.totalTimeId = gameTimerId;
    }

    return gameState;
}

void Game::updateTitle() {
    float titleTextSize = MeasureText(title, titleFontSize);
    titleX = window.halfWidth - titleTextSize*0.5f;
    titleY = window.halfHeight - titleFontSize;

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
