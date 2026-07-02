#pragma once

#include "screen.hpp"
#include "types.hpp"

#include <raylib.h>

class World : public Layer {
    int count_ = 0;
    Sound splat;
    const Screen& screen;

public:
    World(const Screen& screen): screen(screen) {};
    ~World() = default;
    
    int count();
    void load();
    void renderNull() const;
    void renderMain() const;
    void render() const;
    void updateNull();
    void updateMain();
    void update();
    void unload();
    void resize(int width, int height);
};
