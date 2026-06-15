#include "app.hpp"

#include "config.h"
#include "defaults.hpp"
#include "types.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif
#include <raylib.h>
#include <chrono>

void App::load() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROJECT_NAME);
    InitAudioDevice();
    
    SetExitKey(KEY_NULL);

	screen.load();
	game.load();
	world.load();

    screen.listen(&world);
}

void App::logo() const {
    BeginDrawing();
        ClearBackground(RED);
    EndDrawing();
}

void App::intro(void* self) {
    App* app = static_cast<App*>(self);

    InputEvent inputEvent = app->input.update();
    app->timer.update();

    app->logo();
    
    if(inputEvent.id != Event::Input::IDLE || app->timer.isEmpty()) {
        app->state = State::App::RUN;
#ifdef __EMSCRIPTEN__
        // cancel the main loop before setting it to run
        emscripten_cancel_main_loop();
        emscripten_set_main_loop_arg(app->run, app, 0, 0);
#endif
    }

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        app->state = State::App::END;
    }
#endif
}

void App::render() const {
	BeginDrawing();
		world.render();
		game.render();
	EndDrawing();
}

void App::run(void* self) {
    App* app = static_cast<App*>(self);

    app->update();
    app->render();
}

void App::start() {
    // set intro timer
    timer.schedule(std::chrono::milliseconds(INTRO_TIME_MS), nullptr);
	
#ifdef __EMSCRIPTEN__
    // no target FPS (3rd param) to allow browser to optimize frame rate
    // set simulate infinite loop (4th param) to 0 to let the rest of the function execute (on Web)
    emscripten_set_main_loop_arg(intro, this, 0, 0);
    // set the main loop directly to run here if any performance issues on Web
    // and set the App state to Run before running
    // emscripten_set_main_loop_arg(run, this, 0, 0);
    emscripten_set_beforeunload_callback(this, unload);
#else
    SetTargetFPS(TARGET_FPS);

    while (state == State::App::NIL) {
        intro(this);
    }

    while (state == State::App::RUN) {
        run(this);
    }
#endif
}

void App::update() {
    timer.update();

    InputEvent inputEvent = input.update();
    screen.update(inputEvent);

	game.update();
	world.update();

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        state = State::App::END;
    }
#endif
}

const char* App::unload(int eventType, const void *reserved, void *self) {
    App* app = static_cast<App*>(self);

	app->world.unload();
	app->game.unload();

    CloseAudioDevice();
    CloseWindow();

    delete app;

    return nullptr;
}
