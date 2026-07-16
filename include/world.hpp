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

public:
    World(const Window& window): window(window) {};
    ~World() = default;
    
    void (World::*render)() const = &World::renderUnit;
    void (World::*update)(InputEvent) = &World::updateUnit;

    void load();
    
    void renderUnit() const;
    void renderMain() const;
    void renderGame() const;

    void updateUnit(InputEvent);
    void updateMain(InputEvent);
    void updateGame(InputEvent);
    
    void resize(int width, int height) override;
    void transition(State::Screen);
    void unload();
};
