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

    screen.listen(&world);
    screen.listen(&display);
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

void App::render(Clay_RenderCommandArray& renderCommands) const {
	BeginDrawing();
        ClearBackground(BLANK);
		world.render();
		game.render();
        display.render(renderCommands);
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

Clay_RenderCommandArray App::update() {
    timer.update();

    InputEvent inputEvent = input.update();
    screen.update(inputEvent);

	game.update();
	world.update();

    bool isMouseDown = inputEvent.id == Event::Input::PRIMARY_DOWN;
    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mousePosition, isMouseDown && !scrollbarData.mouseDown);

    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;
    Clay_UpdateScrollContainers(true, Clay_Vector2({ mouseWheelX, mouseWheelY }), GetFrameTime());

    if (isMouseDown && !scrollbarData.mouseDown && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar")))) {
        Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("MainContent")));
        scrollbarData.clickOrigin = mousePosition;
        scrollbarData.positionOrigin = *scrollContainerData.scrollPosition;
        scrollbarData.mouseDown = true;
    } else if (scrollbarData.mouseDown) {
        Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("MainContent")));
        if (scrollContainerData.contentDimensions.height > 0) {
            Clay_Vector2 ratio = Clay_Vector2({
                scrollContainerData.contentDimensions.width / scrollContainerData.scrollContainerDimensions.width,
                scrollContainerData.contentDimensions.height / scrollContainerData.scrollContainerDimensions.height,
            });

            if (scrollContainerData.config.vertical) {
                scrollContainerData.scrollPosition->y = scrollbarData.positionOrigin.y + (scrollbarData.clickOrigin.y - mousePosition.y) * ratio.y;
            }
            
            if (scrollContainerData.config.horizontal) {
                scrollContainerData.scrollPosition->x = scrollbarData.positionOrigin.x + (scrollbarData.clickOrigin.x - mousePosition.x) * ratio.x;
            }
        }
    }

    Clay_BeginLayout();

    display.update();
     
    // // Root container
    // CLAY(CLAY_ID("MainContent"), {
    //     .layout = { 
    //         .sizing = { CLAY_SIZING_GROW(1), CLAY_SIZING_GROW(1) },
    //         .padding = CLAY_PADDING_ALL(16),
    //         .childGap = 16 
    //     },
    //     .backgroundColor = {250, 250, 255, 0}
    // }) {
    //     // A nested child
    //     CLAY(CLAY_ID("Sidebar"), {
    //         .layout = { 
    //             .sizing = { 
    //                 .width = CLAY_SIZING_FIXED(screen.width()*0.3f), 
    //                 .height = CLAY_SIZING_FIXED(screen.height()*0.8f) 
    //             } 
    //         },
    //         .backgroundColor = { 200, 200, 200, 255 }
    //     }) {
    //         CLAY_TEXT(CLAY_STRING("I'm an inline floating container."), CLAY_TEXT_CONFIG({ .textColor = {0,0,0,255}, .fontSize = 24 }));
    //     }
    // }
     
    Clay_RenderCommandArray renderCommands = Clay_EndLayout(GetFrameTime());

#ifndef __EMSCRIPTEN__
    if (WindowShouldClose()) {
        state = State::App::END;
    }
#endif

    return renderCommands;
}

const char* App::unload(int eventType, const void *reserved, void *self) {
    App* app = static_cast<App*>(self);

    app->display.unload();

	app->world.unload();
	app->game.unload();

    CloseAudioDevice();
    CloseWindow();

    delete app;

    return nullptr;
}
