#include "world.hpp"

#include "config.h"
// #include "defaults.hpp"

#include <string>

int World::count() {
    return count_;
}

void World::load(){
    std::string pathAssets = DIR_ASSETS;
    const char* pathSoundSplat = pathAssets.append("/").append(URI_SOUND_SPLAT).c_str();

    splat = LoadSound(pathSoundSplat);
    count_ = 0;
}

void World::renderNull() const {

}

void World::renderMain() const {
    DrawRectangleGradientH(0, 0, window.width(), window.height(), GREEN, PINK);
}

void World::renderGame() const {
    DrawRectangleGradientH(0, 0, window.width(), window.height(), BLUE, ORANGE);
}

void World::updateNull(){

}

void World::updateMain(){

}

void World::updateGame(){

    if(IsKeyPressed(KEY_SPACE)){
        count_++;
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
            update = &World::updateNull;
            render = &World::renderNull;
    };
}

void World::unload(){
    UnloadSound(splat);
}

void World::resize(int width, int height) {
    TraceLog(LOG_INFO, "HELLO FROM WORLD RESIZE %i %i", width, height);
}