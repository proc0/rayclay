#pragma once

#include "type.hpp"
#include "timer.hpp"
#include "input.hpp"
#include "window.hpp"
#include "world.hpp"
#include "game.hpp"
#include "logo.hpp"

#include "interface.hpp"
// #include "surface.hpp"
// #include "clay.h"
// #include "brick.h"
// #include <cstdint>

class App : Layer {
    Font fonts[2];

	Input input;
    Timer timer;

    Camera2D camera = {
        .offset = { 0.0f, 0.0f },
        .target = { 0.0f, 0.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    RenderTexture2D target;
    Rectangle targetSource;
    Rectangle targetDestination;

    Clay_Arena clayArena;

	Window window = Window(camera, timer);
    // Surface surface = Surface(window);
    Interface interface = Interface(window);

	World world = World(window);
    Game game = Game(window);
    Logo logo = Logo(window);
    
    // Brick_ElementId buttonId;
    // Brick_ElementId buttonId2;
    // Brick_ElementId buttonId3;
    // Brick_ElementId buttonGroupId1;

	State::App state = State::App::LOAD;
    State::Screen screen = State::Screen::INTRO;

public:
	App() {}
    ~App() = default;

    void load();
    void loadTarget();
    void start();
    
    void render(Clay_RenderCommandArray& renderCommands) const;
    Clay_RenderCommandArray update();
    
    void runIntro();
    static void intro(void* self);
    static void run(void* self);
    static const char* unload(int eventType, const void *reserved, void *self);
    
    void resize(int width, int height) override;
};
