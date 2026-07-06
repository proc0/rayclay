#include "world.hpp"

#include "config.h"
// #include "defaults.hpp"

#include <string>

void World::load(){
    std::string pathAssets = DIR_ASSETS;
    const char* pathSoundSplat = pathAssets.append("/").append(URI_SOUND_SPLAT).c_str();

    splat = LoadSound(pathSoundSplat);
}

void World::renderUnit() const {

}

void World::renderMain() const {
    DrawRectangleGradientH(0, 0, window.width, window.height, GREEN, PINK);
}

void World::renderGame() const {
    DrawRectangleGradientH(0, 0, window.width, window.height, BLUE, ORANGE);
}

void World::updateUnit(){

}

void World::updateMain(){

}

void World::updateGame(){

    if(IsKeyPressed(KEY_SPACE)){
        PlaySound(splat);
    }
    
    if (IsKeyPressed(KEY_H)){
        if (IsCursorHidden()){
            ShowCursor();
        } else {
            HideCursor();
        }
    }
}

void World::transition(State::AppScreen appScreen) {
    switch(appScreen) {
        case State::AppScreen::MAIN:
            update = &World::updateMain;
            render = &World::renderMain;
            break;
        case State::AppScreen::GAME:
            update = &World::updateGame;
            render = &World::renderGame;
            break;
        default:
            update = &World::updateUnit;
            render = &World::renderUnit;
    };
}

void World::unload(){
    UnloadSound(splat);
}

void World::resize(int width, int height) {
    TraceLog(LOG_INFO, "HELLO FROM WORLD RESIZE %i %i", width, height);
}