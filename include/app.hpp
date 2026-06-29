#pragma once

#include "types.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "screen.hpp"
#include "game.hpp"
#include "world.hpp"

#include "display.hpp"

#include "clay.h"

class App : ScreenListener {
	Input input;
    Timer timer;

    Camera2D camera = {
        .offset = { 0.0f, 0.0f },
        .target = { 0.0f, 0.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };
    RenderTexture2D target;  // Render texture to render our game

	Screen screen = Screen(camera);
    Display display = Display(screen);
    // std::unique_ptr<Display> display = std::make_unique<Display>(screen);
	World world = World(screen);
    Game game = Game(screen);

    void (Display::*displayRender)(Clay_RenderCommandArray& renderCommands) const = &Display::render;
    Action::Display (Display::*displayUpdate)(const InputEvent& inputEvent) = &Display::update;
    void (Display::*displayLayout)(GameState) = &Display::layoutMainMenu;

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
    void start();
    Clay_RenderCommandArray update();
    static const char* unload(int eventType, const void *reserved, void *self);
    void onScreenResize(int width, int height);
};
