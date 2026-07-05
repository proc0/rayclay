#include "app.hpp"

#include "config.h"
#include "defaults.hpp"
#include "display.hpp"
#include "types.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif
#include <raylib.h>

void App::load() {
#if DEBUG == 0
    SetTraceLogLevel(LOG_NONE);
#endif

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROJECT_NAME);
    InitAudioDevice();
    
    SetExitKey(KEY_NULL);

	window.load();
    display.load();

	game.load();
	world.load();

    window.enlist(this);
    window.enlist(&display);
    window.enlist(&world);
    window.enlist(&game);

    // Render texture to draw, enables window scaling
    // NOTE: If window is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(window.width(), window.height());
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}

void App::renderLogo() const {
    const char* logoName = "proc0";
    int logoFontSize = 108;
    float logoSize = MeasureText(logoName, logoFontSize);
    int logoX = static_cast<int>(window.halfWidth())-logoSize/2;
    int logoY = static_cast<int>(window.halfHeight())-logoFontSize/2;

    BeginDrawing();
        ClearBackground(RAYWHITE);
        game.renderRaylibLogo();
        DrawText(logoName, logoX, logoY, logoFontSize, BLACK);
    EndDrawing();
}

void App::renderTitle() const {
    const char* gameTitle = "GAME TITLE";
    int titleFontSize = 128;
    float titleSize = MeasureText(gameTitle, titleFontSize);
    int titleX = static_cast<int>(window.halfWidth())-titleSize/2;
    int titleY = static_cast<int>(window.halfHeight())-titleFontSize/2;

    const char* subtitle = "Press any key";
    int subtitleFontSize = 32;
    float subtitleSize = MeasureText(subtitle, subtitleFontSize);
    int subtitleX = static_cast<int>(window.halfWidth())-subtitleSize/2;
    int subtitleY = static_cast<int>(window.height()-window.height()/4)-subtitleFontSize/2;
    BeginDrawing();
        ClearBackground(BLANK);
        DrawText(gameTitle, titleX, titleY, titleFontSize, RAYWHITE);
        DrawText(subtitle, subtitleX, subtitleY, subtitleFontSize, RAYWHITE);
    EndDrawing();
}

void App::intro(void* self) {
    App* app = static_cast<App*>(self);
    app->runIntro();
}

void App::runIntro() {
    (timer.*timer.update)();

    window.update({ .id = Event::Input::IDLE, .position = Vector2({}) });
    
    if (appScreen == State::AppScreen::INTRO) {
        if(input.updateAnyKey() || timer.isEmpty()) {
            appScreen = State::AppScreen::TITLE;
        }

        renderLogo();
    } else if (appScreen == State::AppScreen::TITLE) {
        renderTitle();

        if(input.updateAnyKey()) {
            state = State::App::RUN;
            appScreen = State::AppScreen::MAIN;
            display.transition(state, appScreen);
            world.transition(appScreen);

#ifdef __EMSCRIPTEN__
            // cancel the main loop before setting it to run
            emscripten_cancel_main_loop();
            emscripten_set_main_loop_arg(run, app, 0, 0);
#endif
        }
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

        // TODO: memoize params in resize() 
        DrawTexturePro(target.texture, { 0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height) }, 
            { 0, 0, static_cast<float>(target.texture.width), static_cast<float>(target.texture.height) }, Vector2({}), 0.0f, WHITE);
        
        (display.*display.render)(renderCommands);
	EndDrawing();
}

void App::run(void* self) {
    App* app = static_cast<App*>(self);

    Clay_RenderCommandArray renderCommands = app->update();
    app->render(renderCommands);
}

void App::start() {
    // set intro timer
    timer.schedule(INTRO_TIME_MS, nullptr);
	
#ifdef __EMSCRIPTEN__
    // no target FPS (3rd param) to allow browser to optimize frame rate
    // set simulate infinite loop (4th param) to 0 to let the rest of the function execute (on Web)
    emscripten_set_main_loop_arg(intro, this, 0, 1);
    // set the main loop directly to run here if any performance issues on Web
    // and set the App state to Run before running
    // emscripten_set_main_loop_arg(run, this, 0, 0);
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

Clay_RenderCommandArray App::update() {
    (timer.*timer.update)();
    InputEvent inputEvent = input.update();
    window.update(inputEvent);

    Action::Display displayAction = (display.*display.update)(inputEvent);

    if(appScreen == State::AppScreen::GAME) {
        if(inputEvent.id == Event::Input::KEY_ESCAPE){
            if(state == State::App::PAUSE) {
                TraceLog(LOG_INFO, "UNPAUSE");
                state = State::App::RUN;

                display.transition(state, appScreen);

            } else if (state == State::App::RUN) {
                TraceLog(LOG_INFO, "PAUSE");
                state = State::App::PAUSE;

                display.transition(state, appScreen);
            }
        } else if (state == State::App::PAUSE) {

            if (displayAction == Action::Display::RESUME_GAME) {
                TraceLog(LOG_INFO, "UNPAUSE");
                state = State::App::RUN;
            
                display.transition(state, appScreen);          
            
            } else if (displayAction == Action::Display::MAIN_MENU) {
            
                display.beginEvent(Event::Display::SHOW_RETURN_MAIN_MENU_CONFIRMATION);
            
            } else if (displayAction == Action::Display::CONFIRM_RETURN_MAIN) {
                display.clearEvent();
                state = State::App::RUN;
                appScreen = State::AppScreen::MAIN;

                world.transition(appScreen);
                game.transition(appScreen);
                display.transition(state, appScreen);

            } else if (displayAction == Action::Display::CANCEL_RETURN_MAIN) {

                display.clearEvent();
            
            } else if (displayAction == Action::Display::QUIT_APP) {
                state = State::App::HALT;
                
                return Clay_RenderCommandArray({ 0, 0, nullptr });
            }
        }
    } 

    Clay_BeginLayout();
	GameState gameState = (game.*game.update)(state, inputEvent);
	(world.*world.update)();
    (display.*display.headsUp)(gameState);
    (display.*display.menu)();
    Clay_RenderCommandArray renderCommands = Clay_EndLayout(GetFrameTime());

    if (appScreen == State::AppScreen::MAIN) {

        if(displayAction == Action::Display::NEW_GAME) {
            state = State::App::RUN;
            appScreen = State::AppScreen::GAME;
            
            world.transition(appScreen);
            game.transition(appScreen);
            display.transition(state, appScreen);

        } else if (displayAction == Action::Display::QUIT_APP) {
            state = State::App::HALT;
        }            
    }

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        state = State::App::HALT;
    }
#endif

    return renderCommands;
}

const char* App::unload(int eventType, const void *reserved, void *self) {
    App* app = static_cast<App*>(self);
    
    UnloadRenderTexture(app->target);

    app->display.unload();

	app->world.unload();
	app->game.unload();

    CloseAudioDevice();
    CloseWindow();

    return nullptr;
}

void App::resize(int width, int height) {
    TraceLog(LOG_INFO, "APP RESIZE");
    if (appScreen == State::AppScreen::INTRO) {
        game.loadRaylibLogo();
    }
    
    UnloadRenderTexture(target);
    target = LoadRenderTexture(window.width(), window.height());
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}
