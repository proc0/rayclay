#include "game.hpp"

#include "types.hpp"
#include <raylib.h>

void Game::load() {
    loadRaylibLogo();
}

void Game::loadRaylibLogo() {

    float raylibLogoX = window.halfWidthf-raylibLogoSize*0.5f;
    float raylibLogoY = static_cast<float>(window.height-raylibLogoSize-20);
    float raylibLogoInnerX = raylibLogoX + raylibLogoBorder;
    float raylibLogoInnerY = raylibLogoY + raylibLogoBorder;

    int raylibLogoTextSize = MeasureText(raylibName, raylibLogoFontSize);
    float raylibLogoTextX = raylibLogoX + raylibLogoSize - raylibLogoTextSize - 2.0f*raylibLogoBorder;
    float raylibLogoTextY = raylibLogoY + raylibLogoSize - raylibLogoFontSize - 1.5f*raylibLogoBorder;

    raylibLogoOuterRec = { raylibLogoX, raylibLogoY, raylibLogoSize, raylibLogoSize };
    raylibLogoInnerRec = { raylibLogoInnerX, raylibLogoInnerY, raylibLogoInnerSize, raylibLogoInnerSize };
    raylibLogoTextPos = { raylibLogoTextX, raylibLogoTextY };
}

void Game::updateRaylibLogo() {
    raylibLogoPos.x += raylibLogoDir.x;
    raylibLogoPos.y += raylibLogoDir.y;

    bool hasBounced = false;
    if (raylibLogoPos.x < 0 || raylibLogoPos.x + raylibLogoOuterRec.width > window.width) {
        raylibLogoDir.x *= -1.0f;
        gameState.raylibLogoBounces++;
        hasBounced = true;
    } 

    if (raylibLogoPos.y < 0 || raylibLogoPos.y + raylibLogoOuterRec.width > window.height) {
        raylibLogoDir.y *= -1.0f;
        gameState.raylibLogoBounces++;
        if (hasBounced) {
            gameState.raylibLogoCorners++;
        }
    }

    float raylibLogoInnerX = raylibLogoPos.x + raylibLogoBorder;
    float raylibLogoInnerY = raylibLogoPos.y + raylibLogoBorder;
    
    int raylibLogoTextSize = MeasureText(raylibName, raylibLogoFontSize);
    float raylibLogoTextX = raylibLogoPos.x + raylibLogoSize - raylibLogoTextSize - 2.0f*raylibLogoBorder;
    float raylibLogoTextY = raylibLogoPos.y + raylibLogoSize - raylibLogoFontSize - 1.5f*raylibLogoBorder;

    raylibLogoOuterRec = { raylibLogoPos.x, raylibLogoPos.y, raylibLogoSize, raylibLogoSize };
    raylibLogoInnerRec = { raylibLogoInnerX, raylibLogoInnerY, raylibLogoInnerSize, raylibLogoInnerSize };
    raylibLogoTextPos = { raylibLogoTextX, raylibLogoTextY };
}

GameState Game::updateNull(State::App appState, InputEvent inputEvent){
    return gameState;
}

GameState Game::updateMain(State::App appState, InputEvent inputEvent){
    return gameState;
}

GameState Game::updateGame(State::App appState, InputEvent inputEvent){

    // toggle pause menu
    // if(IsKeyPressed(KEY_ESCAPE)){
    if(appState == State::App::PAUSE) {
        state = State::Game::HOLD;
        return gameState;
    } else if (state == State::Game::HOLD) {
        state = State::Game::PLAY;
    }
        
    //     if(state == State::Game::PLAY) {
    //         state = State::Game::PAUSE;
    //         ShowCursor();
    //         return;
    //     }
    // }

    // run background stuff and UI to begin
    // if(state == State::Game::OVER || state == State::Game::END || state == State::Game::START) {
    //     // listen for play button here
    //     // display.update(world);
    //     return;
    // }

    if(state == State::Game::PLAY){
        updateRaylibLogo();

        if (inputEvent.id == Event::Input::PRIMARY_UP && INTERSECTS(inputEvent.position, raylibLogoOuterRec)) {
            TraceLog(LOG_INFO, "LOGO CLICK");
            gameState.raylibLogoClicks++;
        }
    }

    if(state == State::Game::START){
        // press any key screen (needed to load sound for web, as it needs user input to load audio)
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || GetKeyPressed() != 0){
            state = State::Game::PLAY;
        }
        return gameState;
    }

    return gameState;
}

void Game::renderRaylibLogo() const {
    DrawRectangleRec(raylibLogoOuterRec, BLACK);
    DrawRectangleRec(raylibLogoInnerRec, RAYWHITE);
    DrawText(raylibName, raylibLogoTextPos.x, raylibLogoTextPos.y, raylibLogoFontSize, BLACK);
}

void Game::renderNull() const {

}

void Game::renderMain() const {

}

void Game::renderGame() const {
    renderRaylibLogo();
}

void Game::transition(State::AppScreen appScreen) {
    switch(appScreen) {
        case State::AppScreen::MAIN:
            update = &Game::updateMain;
            render = &Game::renderMain;
            break;
        case State::AppScreen::GAME:
            update = &Game::updateGame;
            render = &Game::renderGame;
            break;
        default:
            update = &Game::updateNull;
    };
}

void Game::resize(int height, int width) {
    // loadRaylibLogo();
}

void Game::unload(){

}
