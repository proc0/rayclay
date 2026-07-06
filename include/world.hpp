#pragma once

#include "window.hpp"
#include "types.hpp"

#include <raylib.h>

class World : public Layer {
    Sound splat;
    const Window& window;

public:
    World(const Window& window): window(window) {};
    ~World() = default;
    
    void (World::*render)() const = &World::renderUnit;
    void (World::*update)() = &World::updateUnit;

    void load();
    
    void renderUnit() const;
    void renderMain() const;
    void renderGame() const;

    void updateUnit();
    void updateMain();
    void updateGame();
    
    void resize(int width, int height) override;
    void transition(State::AppScreen);
    void unload();
};
