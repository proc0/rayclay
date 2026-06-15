#pragma once

#include "screen.hpp"
#include "types.hpp"

#include <raylib.h>

class World : public ScreenListener {
    int count_ = 0;
    Sound splat;
    const Screen& screen;

public:
    World(const Screen& screen): screen(screen) {};
    ~World() = default;
    
    int count();
    void load();
    void render() const;
    void update();
    void unload();
    void onScreenResize(int width, int height);
};
