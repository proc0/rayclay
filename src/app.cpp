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
#include <chrono>

void App::load() {
#if DEBUG == 0
    SetTraceLogLevel(LOG_NONE);
#endif

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROJECT_NAME);
    InitAudioDevice();
    
    SetExitKey(KEY_NULL);

	screen.load();
    display.load();

	game.load();
	world.load();

    screen.enlist(this);
    screen.enlist(&display);
    screen.enlist(&world);
    screen.enlist(&game);

    // Render texture to draw, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screen.width(), screen.height());
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}

void App::renderLogo() const {
    const char* logoName = "proc0";
    int logoFontSize = 108;
    float logoSize = MeasureText(logoName, logoFontSize);
    int logoX = static_cast<int>(screen.halfWidth())-logoSize/2;
    int logoY = static_cast<int>(screen.halfHeight())-logoFontSize/2;

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
    int titleX = static_cast<int>(screen.halfWidth())-titleSize/2;
    int titleY = static_cast<int>(screen.halfHeight())-titleFontSize/2;

    const char* subtitle = "Press any key";
    int subtitleFontSize = 32;
    float subtitleSize = MeasureText(subtitle, subtitleFontSize);
    int subtitleX = static_cast<int>(screen.halfWidth())-subtitleSize/2;
    int subtitleY = static_cast<int>(screen.height()-screen.height()/4)-subtitleFontSize/2;
    BeginDrawing();
        ClearBackground(BLANK);
        DrawText(gameTitle, titleX, titleY, titleFontSize, RAYWHITE);
        DrawText(subtitle, subtitleX, subtitleY, subtitleFontSize, RAYWHITE);
    EndDrawing();
}

void App::intro(void* self) {
    App* app = static_cast<App*>(self);
    app->timer.update();

    app->screen.update({ .id = Event::Input::IDLE, .position = Vector2({}) });
    
    if (app->appScreen == State::AppScreen::INTRO) {
        if(app->input.updateAnyKey() || app->timer.isEmpty()) {
            app->appScreen = State::AppScreen::TITLE;
        }

        app->renderLogo();
    } else if (app->appScreen == State::AppScreen::TITLE) {
        app->renderTitle();

        if(app->input.updateAnyKey()) {
            app->state = State::App::RUN;
            app->appScreen = State::AppScreen::MAIN;

#ifdef __EMSCRIPTEN__
            // cancel the main loop before setting it to run
            emscripten_cancel_main_loop();
            emscripten_set_main_loop_arg(app->run, app, 0, 0);
#endif
        }
    }

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        app->state = State::App::HALT;
    }
#endif
}

void App::render(Clay_RenderCommandArray&& renderCommands) const {
    BeginTextureMode(target);
        ClearBackground(BLANK);
        
        (world.*worldRender)();
        (game.*game.render)();
    EndTextureMode();

	BeginDrawing();
        ClearBackground(BLANK);

        DrawTexturePro(target.texture, { 0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height) }, 
            { 0, 0, static_cast<float>(target.texture.width), static_cast<float>(target.texture.height) }, Vector2({}), 0.0f, WHITE);
        
        (display.*displayRender)(std::move(renderCommands));
	EndDrawing();
}

void App::run(void* self) {
    App* app = static_cast<App*>(self);

    // Clay_RenderCommandArray renderCommands = app->update();
    app->render(app->update());
}

void App::start() {
    // set intro timer
    timer.schedule(std::chrono::milliseconds(INTRO_TIME_MS), nullptr);
	
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
    timer.update();

    InputEvent inputEvent = input.update();
    screen.update(inputEvent);

    // TODO: create multiple display.update<Type> depending on the situation
    // to switch in here. Pause/Menu screen has an update, in-game UI update?
    Action::Display displayAction = (display.*displayUpdate)(inputEvent);

    if (appScreen == State::AppScreen::MAIN && displayAction == Action::Display::NEW_GAME) {
        state = State::App::RUN;
        appScreen = State::AppScreen::GAME;
        
        worldRender = &World::render;
        worldUpdate = &World::update;

        // gameRender = &Game::render;
        // gameUpdate = &Game::update;
        game.changeState(appScreen);

        displayLayout = &Display::layoutHUD;
        displayUpdate = &Display::updateNull;
        displayRender = &Display::render;

    }

    if(appScreen == State::AppScreen::GAME && inputEvent.id == Event::Input::KEY_ESCAPE){
        if(state == State::App::PAUSE) {
            state = State::App::RUN;

            displayLayout = &Display::layoutHUD;
            displayUpdate = &Display::updateNull;
            displayRender = &Display::render;

        } else if (state == State::App::RUN) {
            state = State::App::PAUSE;

            displayLayout = &Display::layoutPauseMenu;
            displayUpdate = &Display::update;
            displayRender = &Display::render;
        }
    }

    if (state == State::App::PAUSE) {
        if (displayAction == Action::Display::RESUME_GAME) {
            state = State::App::RUN;
            displayLayout = &Display::layoutHUD;
            displayUpdate = &Display::updateNull;
            displayRender = &Display::render;            
        } else if (displayAction == Action::Display::MAIN_MENU) {
            //TODO: refactor into display.showConfirmation(<confirmationType>)
            // and do a switch case in Display to toggle whatever is needed
            display.showReturnMainMenuConfirmation = true;
        } else if (displayAction == Action::Display::CONFIRM_RETURN_MAIN) {
            display.showReturnMainMenuConfirmation = false;
            state = State::App::RUN;
            appScreen = State::AppScreen::MAIN;
        
            worldRender = &World::renderMain;
            worldUpdate = &World::updateMain;

            // gameRender = &Game::renderMain;
            // gameUpdate = &Game::updateMain;
            game.changeState(appScreen);

            displayLayout = &Display::layoutMainMenu;
            displayUpdate = &Display::update;
            displayRender = &Display::render;
        } else if (displayAction == Action::Display::CANCEL_RETURN_MAIN) {
            display.showReturnMainMenuConfirmation = false;
        }
    }

    if (displayAction == Action::Display::QUIT_APP) {
        state = State::App::HALT;
        return Clay_RenderCommandArray({ 0, 0, nullptr });
    }

    GameState gameState = GameState{0};
    if (appScreen == State::AppScreen::GAME) {        
    	gameState = (game.*game.update)(state, inputEvent);
    	world.update();
    }

    Clay_BeginLayout();
    (display.*displayLayout)(gameState);
    Clay_RenderCommandArray renderCommands = Clay_EndLayout(GetFrameTime());

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
    target = LoadRenderTexture(screen.width(), screen.height());
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}
