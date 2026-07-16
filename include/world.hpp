#pragma once

#include "config.hpp"
#include "type.hpp"
#include "window.hpp"

#include "raylib.h"

class World : public Layer {
    Sound splat;
    const Window& window;

    Vector2 rlLogoDir = { static_cast<float>(GetRandomValue(-100, 100))/100.0f, static_cast<float>(GetRandomValue(-100, 100))/100.0f };
    Vector2 rlLogoPos = { static_cast<float>(GetRandomValue(0, SCREEN_WIDTH-RAYLIB_LOGO_SIZE)), static_cast<float>(GetRandomValue(0, SCREEN_HEIGHT-RAYLIB_LOGO_SIZE)) };

    int dummyGoalTracker = 0;

public:
    World(const Window& window): window(window) {};
    ~World() = default;
    
    void (World::*render)() const = &World::renderUnit;
    WorldState (World::*update)(InputEvent, Action::Surface) = &World::updateUnit;

    void load();
    
    void renderUnit() const {};
    void renderMain() const;
    void renderGame() const;
    void renderHold() const;

    WorldState updateUnit(InputEvent, Action::Surface) { return { .reachedGoal = false }; };
    WorldState updateMain(InputEvent, Action::Surface);
    WorldState updateGame(InputEvent, Action::Surface);
    WorldState updateHold(InputEvent, Action::Surface);
    
    void resize(int width, int height) override;
    void transition(State::App, State::Screen);
    void unload();
};
