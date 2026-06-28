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
    // 1. Query minimum memory required for default element limits
    // uint64_t totalMemorySize = Clay_MinMemorySize();
     
    // // 2. Allocate memory (malloc, stack, or custom allocator)
    // void* memory = malloc(totalMemorySize);
     
    // // 3. Create arena [clay.h:2150-2158]
    // Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, memory);
     
    // // 4. Initialize Clay [clay.h:2186-2188]
    // Clay_Initialize(arena, Clay_Dimensions({ static_cast<float>(screen.width()), static_cast<float>(screen.height()) }), Clay_ErrorHandler({ Clay__ErrorHandlerFunctionDefault }));

    // fonts[0] = LoadFontEx(PATH_ASSET("Roboto-Regular.ttf"), 48, 0, 400);
    // SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    // fonts[1] = LoadFontEx(PATH_ASSET("Roboto-Regular.ttf"), 32, 0, 400);
    // SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);
    // Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

	game.load();
	world.load();

    screen.listen(this);
    screen.listen(&world);
    screen.listen(&display);

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

    const char* raylibName = "raylib";
    constexpr int raylibLogoFontSize = 40;
    int raylibLogoTextSize = MeasureText(raylibName, raylibLogoFontSize);
    constexpr int raylibLogoSize = 200;
    constexpr int raylibLogoBorder = 16;
    int raylibLogoX = static_cast<int>(screen.halfWidth())-raylibLogoSize/2;
    int raylibLogoY = static_cast<int>(screen.height())-raylibLogoSize-20;
    constexpr int raylibLogoInnerSize = raylibLogoSize - 2*raylibLogoBorder;
    int raylibLogoInnerX = raylibLogoX + raylibLogoBorder;
    int raylibLogoInnerY = raylibLogoY + raylibLogoBorder;
    int raylibLogoTextX = raylibLogoX + raylibLogoSize - raylibLogoTextSize - 2*raylibLogoBorder;
    int raylibLogoTextY = raylibLogoY + raylibLogoSize - raylibLogoFontSize - static_cast<int>(1.5f*raylibLogoBorder);
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle(raylibLogoX, raylibLogoY, raylibLogoSize, raylibLogoSize, BLACK);
        DrawRectangle(raylibLogoInnerX, raylibLogoInnerY, raylibLogoInnerSize, raylibLogoInnerSize, RAYWHITE);
        DrawText(raylibName, raylibLogoTextX, raylibLogoTextY, raylibLogoFontSize, BLACK);
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
        ClearBackground(GRAY);
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

void App::render(Clay_RenderCommandArray& renderCommands) const {
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        
        // TODO: Draw your game screen here
        world.render();

        DrawRectangle(70, 90, 200, 200, BLACK);
        DrawRectangle(70 + 16, 90 + 16, 200 - 32, 200 - 32, RAYWHITE);
        DrawText("raylib", 70 + 200 - MeasureText("raylib", 40) - 32, 90 + 200 - 40 - 24, 40, BLACK);

        game.render();
        // if ((frameCounter/20)%2) DrawText("are you ready?", 160, 500, 50, BLACK);
        
        // DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 16, BLACK);
        
    EndTextureMode();

	BeginDrawing();
        ClearBackground(RAYWHITE);

        // display.render(renderCommands);
        DrawTexturePro(target.texture, { 0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height) }, 
            { 0, 0, static_cast<float>(target.texture.width), static_cast<float>(target.texture.height) }, Vector2({}), 0.0f, WHITE);
        
        (display.*displayRender)(renderCommands);
	EndDrawing();
}

void App::run(void* self) {
    App* app = static_cast<App*>(self);

    Clay_RenderCommandArray renderCommands = app->update();
    app->render(renderCommands);
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

    if (displayAction != Action::Display::DO_NOTHING) {        
        if (appScreen == State::AppScreen::GAME && displayAction == Action::Display::MAIN_MENU) {
            state = State::App::RUN;
            displayLayout = &Display::layoutMainMenu;
            displayUpdate = &Display::update;
            displayRender = &Display::render;
            appScreen = State::AppScreen::MAIN;
        } else if (appScreen == State::AppScreen::MAIN && displayAction == Action::Display::NEW_GAME) {
            displayLayout = &Display::layoutPauseMenu;
            displayUpdate = &Display::updateNull;
            displayRender = &Display::renderNull;
            appScreen = State::AppScreen::GAME;
        }
    }

    if(inputEvent.id == Event::Input::KEY_ESCAPE){
        if(state == State::App::PAUSE) {
            state = State::App::RUN;
            displayUpdate = &Display::updateNull;
            displayRender = &Display::renderNull;
        } else if (state == State::App::RUN) {
            state = State::App::PAUSE;
            displayUpdate = &Display::update;
            displayRender = &Display::render;
        }
    }

    if (state == State::App::PAUSE) {
        if (displayAction == Action::Display::RESUME_GAME) {
            state = State::App::RUN;
            displayUpdate = &Display::updateNull;
            displayRender = &Display::renderNull;            
        }
    }

    if (displayAction == Action::Display::QUIT_APP) {
        state = State::App::HALT;
// #ifdef __EMSCRIPTEN__
//         emscripten_cancel_main_loop();
// #endif
        return Clay_RenderCommandArray({ 0, 0, nullptr });
    }

    if (appScreen == State::AppScreen::GAME) {        
    	game.update();
    	world.update();
    }

    Clay_BeginLayout();
    (display.*displayLayout)();
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

    delete app;

    return nullptr;
}

void App::onScreenResize(int width, int height) {
    TraceLog(LOG_INFO, "APP RESIZE");
    UnloadRenderTexture(target);
    target = LoadRenderTexture(screen.width(), screen.height());
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}
