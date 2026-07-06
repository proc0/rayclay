#pragma once

#include "types.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "window.hpp"
#include "game.hpp"
#include "world.hpp"

#include "surface.hpp"

#include "clay.h"

class App : Layer {
	Input input;
    Timer timer;

    Camera2D camera = {
        .offset = { 0.0f, 0.0f },
        .target = { 0.0f, 0.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    RenderTexture2D target;

	Window window = Window(camera, timer);
    Surface surface = Surface(window);

	World world = World(window);
    Game game = Game(window);

	State::App state = State::App::LOAD;
    State::AppScreen appScreen = State::AppScreen::INTRO;

public:
	App() {};
    ~App() = default;

    void load();
    void renderLogo() const;
    void renderTitle() const;
    static void intro(void* self);
    void render(Clay_RenderCommandArray& renderCommands) const;
    static void run(void* self);
    void runIntro();
    void start();
    Clay_RenderCommandArray update();
    static const char* unload(int eventType, const void *reserved, void *self);
    void resize(int width, int height) override;
};
