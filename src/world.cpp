#include "world.hpp"

#include "index.h"
#include "raylib.h"
#include "type.hpp"

void World::load(){
    splat = LoadSound(PATH_ASSET(URI_SOUND_SPLAT));
}

void World::renderMain() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, DARKBLUE, DARKGRAY);
}

void World::renderGame() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, BLUE, GREEN);
}

void World::renderHold() const {
    renderGame();
    const char* pausedText = TextFormat("PAUSED");
    float pausedTextWidth = MeasureText(pausedText, 200);
    DrawText(pausedText, window.halfWidthf-pausedTextWidth*0.5f, window.halfHeightf-100.0f, 200, RAYWHITE);
}

WorldState World::updateMain(InputEvent, Action::Surface){
    return { .reachedGoal = false };
}

WorldState World::updateHold(InputEvent inputEvent, Action::Surface action){
    return { .reachedGoal = false };
}

WorldState World::updateGame(InputEvent inputEvent, Action::Surface action){

    if (inputEvent.id == Event::Input::MOVE_UP || action == Action::Surface::MOVE_UP ) {
            TraceLog(LOG_INFO, "MOVE UP");
            dummyGoalTracker++;
    } else if (inputEvent.id == Event::Input::MOVE_RIGHT || action == Action::Surface::MOVE_RIGHT ) {
            TraceLog(LOG_INFO, "MOVE RIGHT");

    } else if (inputEvent.id == Event::Input::MOVE_DOWN || action == Action::Surface::MOVE_DOWN ) {
            TraceLog(LOG_INFO, "MOVE DOWN");
            dummyGoalTracker--;

    } else if (inputEvent.id == Event::Input::MOVE_LEFT || action == Action::Surface::MOVE_LEFT ) {
            TraceLog(LOG_INFO, "MOVE LEFT");

    }

    if (dummyGoalTracker >= 3) {
        PlaySound(splat);
        dummyGoalTracker = 0;
        return { .reachedGoal = true };
    } else if (dummyGoalTracker < -2) {
        dummyGoalTracker = 0;
        return { .failedGoal = true };
    }

    return { .reachedGoal = false };
}

void World::transition(State::App appState, State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            update = &World::updateMain;
            render = &World::renderMain;
            break;
        case State::Screen::GAME:
            if (appState == State::App::HOLD) {
                update = &World::updateHold;
                render = &World::renderHold;
            } else {
                update = &World::updateGame;
                render = &World::renderGame;
            }
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