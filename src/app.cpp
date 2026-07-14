#include "app.hpp"

#include "index.h"
#include "config.hpp"
#include "type.hpp"

#include "raylib.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

void App::load() {
#if DEBUG == 0
    SetTraceLogLevel(LOG_NONE);
#endif

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROJECT_NAME);
    InitAudioDevice();
    
    SetExitKey(KEY_NULL);

	window.load();
    surface.load();
	world.load();
	game.load();
    logo.load();

    window.enlist(this);
    window.enlist(&surface);
    window.enlist(&world);
    window.enlist(&game);
    window.enlist(&logo);

    loadTarget();
}

void App::loadTarget() {
    // Render texture to draw, enables window scaling
    // NOTE: If window is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(window.width, window.height);
    targetSource = { 0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height) };
    targetDestination = { 0, 0, static_cast<float>(target.texture.width), static_cast<float>(target.texture.height) };
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}

void App::start() {
    // set intro timer
    timer.schedule(INTRO_TIME_MS, nullptr);
    
#ifdef __EMSCRIPTEN__
    // no target FPS (3rd param) to allow browser to optimize frame rate
    // set simulate infinite loop (4th param) to 0 to let the rest of the function execute
    // emscripten_set_main_loop_arg(run, this, 0, 0);
    // if any performance issues on Web, skip intro and set this to run
    emscripten_set_main_loop_arg(intro, this, 0, 1);
    // unload assets before quitting app on Web
    emscripten_set_beforeunload_callback(this, unload);
#else
    SetTargetFPS(TARGET_FPS);

    while (state != State::App::RUN && state != State::App::HALT) {
        intro(this);
    }

    while (state != State::App::HALT) {
        run(this);
    }
#endif
}

void App::intro(void* self) {
    App* app = static_cast<App*>(self);
    app->runIntro();
}

void App::runIntro() {
    (timer.*timer.update)();

    window.update(DEFAULT_INPUT);
    
    if (screen == State::Screen::INTRO) {
        if(input.updateAnyKey() || timer.isEmpty()) {
            screen = State::Screen::TITLE;
        }

        BeginDrawing();
        logo.render();
        EndDrawing();

    } else if (screen == State::Screen::TITLE) {
        if(input.updateAnyKey()) {
            state = State::App::RUN;
            screen = State::Screen::MAIN;
            surface.transition(state, screen);
            world.transition(screen);

#ifdef __EMSCRIPTEN__
            // cancel the main loop before setting it to run
            emscripten_cancel_main_loop();
            emscripten_set_main_loop_arg(run, this, 0, 0);
#endif
        }

        BeginDrawing();
        game.renderTitle();
        EndDrawing();
    }

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        state = State::App::HALT;
    }
#endif
}

void App::render(Clay_RenderCommandArray& renderCommands) const {
    BeginTextureMode(target);
        ClearBackground(BLANK);
        
        (world.*world.render)();
        (game.*game.render)();
    EndTextureMode();

	BeginDrawing();
        ClearBackground(BLANK);
        
        DrawTexturePro(target.texture, targetSource, targetDestination, Vector2({}), 0.0f, WHITE);

        (surface.*surface.render)(renderCommands);
	EndDrawing();
}

void App::run(void* self) {
    App* app = static_cast<App*>(self);

    Clay_RenderCommandArray renderCommands = app->update();
    app->render(renderCommands);
}


Clay_RenderCommandArray App::update() {

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        state = State::App::HALT;
        return Clay_RenderCommandArray({ 0, 0, nullptr });
    }
#endif

    (timer.*timer.update)();
    InputEvent inputEvent = input.update();
    window.update(inputEvent);

    Action::Surface surfaceAction = (surface.*surface.update)(inputEvent);

    if(screen == State::Screen::GAME) {
        if(inputEvent.id == Event::Input::KEY_ESCAPE){
            if(state == State::App::HOLD) {
                TraceLog(LOG_INFO, "UNPAUSE");
                state = State::App::RUN;

                surface.transition(state, screen);

            } else if (state == State::App::RUN) {
                TraceLog(LOG_INFO, "PAUSE");
                state = State::App::HOLD;

                surface.transition(state, screen);
            }
        } else if (state == State::App::HOLD) {

            if (surfaceAction == Action::Surface::RESUME_GAME) {
                TraceLog(LOG_INFO, "UNPAUSE");
                state = State::App::RUN;
            
                surface.transition(state, screen);          
            
            } else if (surfaceAction == Action::Surface::MAIN_MENU) {
            
                surface.beginEvent(Event::Surface::SHOW_RETURN_MAIN_MENU_CONFIRMATION);
            
            } else if (surfaceAction == Action::Surface::CONFIRM_RETURN_MAIN) {
                surface.clearEvent();
                state = State::App::RUN;
                screen = State::Screen::MAIN;

                world.transition(screen);
                game.transition(screen);
                surface.transition(state, screen);

            } else if (surfaceAction == Action::Surface::CANCEL_RETURN_MAIN) {

                surface.clearEvent();
            
            } else if (surfaceAction == Action::Surface::QUIT_APP) {
                state = State::App::HALT;
                return Clay_RenderCommandArray({ 0, 0, nullptr });
            }
        }
    } else if (screen == State::Screen::MAIN) {
        if(surfaceAction == Action::Surface::NEW_GAME) {
            state = State::App::RUN;
            screen = State::Screen::GAME;
            
            world.transition(screen);
            game.transition(screen);
            surface.transition(state, screen);

        } else if (surfaceAction == Action::Surface::QUIT_APP) {
            state = State::App::HALT;
            return Clay_RenderCommandArray({ 0, 0, nullptr });
        }            
    }

    Clay_BeginLayout();
	GameState gameState = (game.*game.update)(state, inputEvent);
	(world.*world.update)();
    (surface.*surface.display)(gameState);
    (surface.*surface.menu)();
    Clay_RenderCommandArray renderCommands = Clay_EndLayout(GetFrameTime());

    return renderCommands;
}

void App::resize(int width, int height) {    
    UnloadRenderTexture(target);
    loadTarget();
}

const char* App::unload(int eventType, const void *reserved, void *self) {
    App* app = static_cast<App*>(self);
    
	app->game.unload();
	app->world.unload();
    app->surface.unload();

    UnloadRenderTexture(app->target);

    CloseAudioDevice();
    CloseWindow();

    return nullptr;
}

