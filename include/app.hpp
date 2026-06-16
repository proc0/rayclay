#pragma once

#include "clay.h"

#include "types.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "screen.hpp"
#include "game.hpp"
#include "world.hpp"

class App {
	Game game;
	Input input;
    Timer timer;
    Font fonts[2];

    Camera2D camera = {
        .offset = { 0.0f, 0.0f },
        .target = { 0.0f, 0.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };
	Screen screen = Screen(camera);
	World world = World(screen);
	
	State::App state = State::App::NIL;

public:
	App() {};
    ~App() = default;

    void load();
    void logo() const;
    static void intro(void* self);
    void render(Clay_RenderCommandArray renderCommands);
    static void run(void* self);
    void start();
    Clay_RenderCommandArray update();
    static const char* unload(int eventType, const void *reserved, void *self);
};
