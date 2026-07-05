#pragma once

#include "window.hpp"
#include "types.hpp"

#include <raylib.h>

class World : public Layer {
    int count_ = 0;
    Sound splat;
    const Window& window;

public:
    World(const Window& window): window(window) {};
    ~World() = default;
    
    int count();
    void load();
    
    void renderNull() const;
    void (World::*render)() const = &World::renderNull;
    void renderMain() const;
    void renderGame() const;

    void updateNull();
    void (World::*update)() = &World::updateNull;
    void updateMain();
    void updateGame();
    
    void transition(State::AppScreen);
    void unload();
    void resize(int width, int height);
};
