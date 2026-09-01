#include "app.hpp"

#include "index.h"
#include "config.hpp"
#include "type.hpp"

#include "raylib.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "clay_renderer.h"

#define BRICK_IMPLEMENTATION
#include "brick.h"

// #define CLAY_IMPLEMENTATION
// #include "clay.h"

void App::load() {
#if DEBUG == 0
    SetTraceLogLevel(LOG_NONE);
#endif

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROJECT_NAME);
    InitAudioDevice();
    
    SetExitKey(KEY_NULL);

	window.load();
    // surface.load();
	world.load();
	game.load();
    logo.load();

    window.enlist(this);
    // window.enlist(&surface);
    window.enlist(&world);
    window.enlist(&game);
    window.enlist(&logo);

    loadTarget();

    fonts[0] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_MEDIUM), 48, 0, 400);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    fonts[1] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_REGULAR), 32, 0, 400);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);

    Brick_Initialize(window.widthf, window.heightf, Raylib_MeasureText, fonts);
    // // 1. Query minimum memory required for default element limits
    // uint64_t memorySize = Clay_MinMemorySize();
    // // 2. Allocate memory (malloc, stack, or custom allocator)
    // void* memory = malloc(memorySize);
    // // 3. Create arena [clay.h:2150-2158]
    // clayArena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    // // 4. Initialize Clay [clay.h:2186-2188]
    // Clay_Initialize(clayArena, Clay_Dimensions({ window.widthf, window.heightf }), Clay_ErrorHandler({ .errorHandlerFunction = nullptr, .userData = nullptr }));

    LoadOverlay();

    buttonId = Brick_CreateButton("HELLO");
    buttonId2 = Brick_CreateButton("HELLO 2");
    buttonId3 = Brick_CreateButton("HELLO 3");
    Brick_ElementId buttonGroup1[3] = { buttonId, buttonId2, buttonId3 };
    buttonGroupId1 = Brick_GroupButtons(buttonGroup1, 3);
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

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        state = State::App::HALT;
    }
#endif

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
            // surface.transition(state, screen);
            world.transition(state, screen);
            game.transition(state, screen);

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
        // (surface.*surface.render)(renderCommands);
        RenderRaylib(fonts, renderCommands);
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

    // Action::Surface surfaceAction = (surface.*surface.update)(inputEvent);

    // // TODO: implement app member function pointer state transitions
    // // and transition self into different functions depending on screen
    // if(screen == State::Screen::GAME) {
    //     // Game screen input events
    //     //-----------------------------
    //     if(inputEvent.id == Event::Input::KEY_ESCAPE){
    //         if(state == State::App::HOLD) {
    //             TraceLog(LOG_INFO, "UNPAUSE");
    //             if (surface.hasEvent()) {
    //                 surface.clearEvent();
    //             } else {                    
    //                 state = State::App::RUN;

    //                 game.transition(state, screen);
    //                 world.transition(state, screen);
    //                 surface.transition(state, screen);
    //             }
    //         } else if (state == State::App::RUN) {
    //             TraceLog(LOG_INFO, "PAUSE");
    //             state = State::App::HOLD;

    //             game.transition(state, screen);
    //             world.transition(state, screen);
    //             surface.transition(state, screen);
    //         }
    //     }

    //     if(surfaceAction == Action::Surface::CONFIRM_TUTORIAL || surfaceAction == Action::Surface::RESTART) {
    //         TraceLog(LOG_INFO, "BEGIN GAME");
    //         state = State::App::RUN;

    //         surface.clearEvent();
    //         // reset any game state
    //         game.reset();
    //         game.start();
    //         game.transition(state, screen);
    //         world.transition(state, screen);
    //         surface.transition(state, screen);

    //     } else if (state == State::App::HOLD) {
    //         // pause button events
    //         if (surfaceAction == Action::Surface::RESUME_GAME) {
    //             TraceLog(LOG_INFO, "UNPAUSE");
    //             state = State::App::RUN;
            
    //             game.transition(state, screen);
    //             world.transition(state, screen);
    //             surface.transition(state, screen);

    //         } else if (surfaceAction == Action::Surface::MAIN_MENU) {
            
    //             surface.beginEvent(Event::Surface::SHOW_RETURN_MAIN_MENU_CONFIRMATION);
            
    //         } else if (surfaceAction == Action::Surface::CONFIRM_RETURN) {
    //             surface.clearEvent();
    //             state = State::App::RUN;
    //             screen = State::Screen::MAIN;

    //             world.transition(state, screen);
    //             game.transition(state, screen);
    //             surface.transition(state, screen);

    //         } else if (surfaceAction == Action::Surface::CANCEL_RETURN) {

    //             surface.clearEvent();
            
    //         } else if (surfaceAction == Action::Surface::OPTIONS) {

    //             surface.beginEvent(Event::Surface::SHOW_OPTIONS);
    //             surface.transition(state, screen);
            
    //         } else if (surfaceAction == Action::Surface::CONFIRM_OPTIONS) {
    //             TraceLog(LOG_INFO, "SAVE OPTIONS");

    //             surface.clearEvent();
    //             surface.transition(state, screen);

    //         } else if (surfaceAction == Action::Surface::CANCEL_OPTIONS) {
    //             TraceLog(LOG_INFO, "DISCARD OPTIONS");

    //             surface.clearEvent();
    //             surface.transition(state, screen);
            
    //         } else if (surfaceAction == Action::Surface::QUIT_APP) {
    //             state = State::App::HALT;
    //             return Clay_RenderCommandArray({ 0, 0, nullptr });
    //         }
    //     }
    // } else if (screen == State::Screen::MAIN) {
    //     // Main screen input events
    //     //-----------------------------
    //     if(surfaceAction == Action::Surface::NEW_GAME) {
    //         screen = State::Screen::GAME;
    //         // NOTE: app state is still on HOLD until confirm
    //         surface.beginEvent(Event::Surface::SHOW_TUTORIAL);

    //         game.transition(state, screen);
    //         // transition world to start showing in background
    //         world.transition(state, screen);
    //         surface.transition(state, screen);

    //     } else if (surfaceAction == Action::Surface::OPTIONS) {

    //         surface.beginEvent(Event::Surface::SHOW_OPTIONS);
    //         surface.transition(state, screen);
        
    //     } else if (surfaceAction == Action::Surface::CONFIRM_OPTIONS) {
            
    //         surface.clearEvent();
    //         surface.transition(state, screen);

    //     } else if (surfaceAction == Action::Surface::CANCEL_OPTIONS) {

    //         surface.clearEvent();
    //         surface.transition(state, screen);
        
    //     } else if (surfaceAction == Action::Surface::QUIT_APP) {
    //         state = State::App::HALT;
    //         return Clay_RenderCommandArray({ 0, 0, nullptr });
    //     }            
    // }

	// WorldState worldState = (world.*world.update)(inputEvent);
	// GameState gameState = (game.*game.update)(inputEvent, worldState);

    // surface.updateDisplay(gameState);

    // Clay_BeginLayout();
    // (surface.*surface.layoutDisplay)();
    // (surface.*surface.layoutMenu)();
    // Clay_RenderCommandArray renderCommands = Clay_EndLayout(GetFrameTime());
    // Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(inputEvent.position);
    // Clay_SetPointerState(mousePosition, inputEvent.id == Event::Input::PRIMARY || inputEvent.id == Event::Input::PRIMARY_DOWN);
    
    Brick_EventArray eventArray = Brick_PollEvents({ 
        .x = inputEvent.position.x, 
        .y = inputEvent.position.y, 
        .pressed = inputEvent.id == Event::Input::PRIMARY || inputEvent.id == Event::Input::PRIMARY_DOWN
    });
    // TraceLog(LOG_INFO, "EVENT NUM: %d", eventArray.length);
    
    for (int i=0; i<eventArray.length; i++) {
        Brick_Event* event = Brick_EventArray_Get(&eventArray, i);

        switch(event->eventType) {
        case BRICK_EVENT_PRESS:
            TraceLog(LOG_INFO, "CLICK FROM APP HOORAY");
        break;
        case BRICK_EVENT_PRESSING:
            // TraceLog(LOG_INFO, "CLICKING...");
        break;
        case BRICK_EVENT_RELEASE:
            // TraceLog(LOG_INFO, "RELEASE");
        break;
        case BRICK_EVENT_HOVER:
            // TraceLog(LOG_INFO, "HOVER");
        break;
        case BRICK_EVENT_HOVERING:
            // TraceLog(LOG_INFO, "HOVERING...");
        break;
        case BRICK_EVENT_CLEAR:
            TraceLog(LOG_INFO, "CLEAR ID:%d", event->id);
        break;
        default: break;
        }
    }

    if (Brick_PointerJustHovered()) {
        TraceLog(LOG_INFO, "RAYLIB: JUST HOVERED");
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (Brick_PointerJustCleared()) {
        TraceLog(LOG_INFO, "RAYLIB: JUST CLEARED");
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    Brick_BeginLayout();
    Brick_BeginLayoutPanel();
    // Brick_LayoutButton(buttonId);
    // Brick_LayoutButton(buttonId2);
    // Brick_LayoutButton(buttonId3);
    Brick_LayoutButtonGroup(buttonGroupId1);
    Brick_BeginLayoutTogglePanel(buttonId2);
        CLAY_TEXT(CLAY_STRING("HAHAAHHAHA"), STYLE_TEXT_DEFAULT);
    Brick_EndLayoutTogglePanel(buttonId2);
    Brick_EndLayoutPanel();
    Clay_RenderCommandArray renderCommands = Brick_EndLayout(GetFrameTime());

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
    // app->surface.unload();

    UnloadRenderTexture(app->target);

    Clay_Raylib_Close();
    for (auto& font : app->fonts) {
        UnloadFont(font);
    }
    Brick_Destroy();
    // if (app->clayArena.memory) free(app->clayArena.memory);

    CloseAudioDevice();
    CloseWindow();

    return nullptr;
}

