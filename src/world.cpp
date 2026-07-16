#include "world.hpp"

#include "index.h"
#include "raylib.h"
#include "type.hpp"
// #include "config.hpp"

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

void World::updateUnit(InputEvent){

}

void World::updateMain(InputEvent){

}

void World::updateGame(InputEvent inputEvent){

    if(IsKeyPressed(KEY_SPACE)){
        PlaySound(splat);
    }

    switch(inputEvent.id) {
        case Event::Input::MOVE_UP:
            TraceLog(LOG_INFO, "MOVE UP");
            break;
        case Event::Input::MOVE_DOWN:
            TraceLog(LOG_INFO, "MOVE DOWN");
            break;
        case Event::Input::MOVE_RIGHT:
            TraceLog(LOG_INFO, "MOVE RIGHT");
            break;
        case Event::Input::MOVE_LEFT:
            TraceLog(LOG_INFO, "MOVE LEFT");
            break;
        default:
            break;
    }

}

void World::transition(State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            update = &World::updateMain;
            render = &World::renderMain;
            break;
        case State::Screen::GAME:
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