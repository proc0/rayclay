#pragma once

#include <raylib.h>

#include "world.hpp"

class Display {
    int count = 0;

    public:
    int screenWidth;
    int screenHeight;

    Display(){};
    ~Display() = default;

    void load();
    void render() const;
    void update(World& world);
    void unload();
};
