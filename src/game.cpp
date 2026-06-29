#include "game.hpp"

#include "types.hpp"
#include <raylib.h>

void Game::load() {
    loadRaylibLogo();
}

void Game::updateRaylibLogo() {
    raylibLogoPos.x += raylibLogoDir.x;
    raylibLogoPos.y += raylibLogoDir.y;

    if (raylibLogoPos.x < 0 || raylibLogoPos.x + 200 > screen.width()) {
        raylibLogoDir.x *= -1.0f;
    } 

    if (raylibLogoPos.y < 0 || raylibLogoPos.y + 200 > screen.height()) {
        raylibLogoDir.y *= -1.0f;
    }

    float raylibLogoInnerX = raylibLogoPos.x + raylibLogoBorder;
    float raylibLogoInnerY = raylibLogoPos.y + raylibLogoBorder;
    
    int raylibLogoTextSize = MeasureText(raylibName, raylibLogoFontSize);
    float raylibLogoTextX = raylibLogoPos.x + raylibLogoSize - raylibLogoTextSize - 2.0f*raylibLogoBorder;
    float raylibLogoTextY = raylibLogoPos.y + raylibLogoSize - raylibLogoFontSize - 1.5f*raylibLogoBorder;

    raylibLogoOuterRec = { raylibLogoPos.x, raylibLogoPos.y, raylibLogoSize, raylibLogoSize };
    raylibLogoInnerRec = { raylibLogoInnerX, raylibLogoInnerY, raylibLogoInnerSize, raylibLogoInnerSize };
    raylibLogoTextPos = { raylibLogoTextX, raylibLogoTextY };
}

void Game::loadRaylibLogo() {

    float raylibLogoX = static_cast<float>(screen.halfWidth())-raylibLogoSize/2.0f;
    float raylibLogoY = static_cast<float>(screen.height()-raylibLogoSize-20);
    float raylibLogoInnerX = raylibLogoX + raylibLogoBorder;
    float raylibLogoInnerY = raylibLogoY + raylibLogoBorder;

    int raylibLogoTextSize = MeasureText(raylibName, raylibLogoFontSize);
    float raylibLogoTextX = raylibLogoX + raylibLogoSize - raylibLogoTextSize - 2.0f*raylibLogoBorder;
    float raylibLogoTextY = raylibLogoY + raylibLogoSize - raylibLogoFontSize - 1.5f*raylibLogoBorder;

    raylibLogoOuterRec = { raylibLogoX, raylibLogoY, raylibLogoSize, raylibLogoSize };
    raylibLogoInnerRec = { raylibLogoInnerX, raylibLogoInnerY, raylibLogoInnerSize, raylibLogoInnerSize };
    raylibLogoTextPos = { raylibLogoTextX, raylibLogoTextY };
}

void Game::update(State::App appState, InputEvent inputEvent){
    // if(!IsWindowFullscreen() && IsWindowResized()){
    //     auto now = std::chrono::steady_clock::now();
    //     if (now - lastResize > RESIZE_COOLDOWN) {
    //         resize();
    //     }
    // }

    // toggle pause menu
    // if(IsKeyPressed(KEY_ESCAPE)){
    if(appState == State::App::PAUSE) {
        state = State::Game::HOLD;
        return;
    } else if (state == State::Game::HOLD) {
        state = State::Game::PLAY;
    }
        
    //     if(state == State::Game::PLAY) {
    //         state = State::Game::PAUSE;
    //         ShowCursor();
    //         return;
    //     }
    // }

    // run background stuff and UI to begin
    // if(state == State::Game::OVER || state == State::Game::END || state == State::Game::START) {
    //     // listen for play button here
    //     // display.update(world);
    //     return;
    // }

    if(state == State::Game::PLAY){
        updateRaylibLogo();

        if (inputEvent.id == Event::Input::PRIMARY_UP && INTERSECTS(inputEvent.position, raylibLogoOuterRec)) {
            TraceLog(LOG_INFO, "LOGO CLICK");
        }
    }

    if(state == State::Game::START){
        // press any key screen (needed to load sound for web, as it needs user input to load audio)
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || GetKeyPressed() != 0){
            state = State::Game::PLAY;
        }
        return;
    }
}

void Game::renderRaylibLogo() const {
    DrawRectangleRec(raylibLogoOuterRec, BLACK);
    DrawRectangleRec(raylibLogoInnerRec, RAYWHITE);
    DrawText(raylibName, raylibLogoTextPos.x, raylibLogoTextPos.y, raylibLogoFontSize, BLACK);
}

void Game::render() const {

    renderRaylibLogo();
    // BeginDrawing();
        // ClearBackground(BLACK);
        // world.render();
        // display.render();

        // if(state == PLAY){
        //     DrawCircleV(GetMousePosition(), 40, YELLOW);
        // }
    // EndDrawing();
}

void Game::unload(){
    // world.unload();
    // display.unload();
}

// void Game::loop(void* self) {
//     Game* game = static_cast<Game*>(self);
//     #ifdef __EMSCRIPTEN__
//     if (!game->isRunning()) return;
//     #endif
//     game->update();
//     game->render();
// }

// void Game::run() {
//     resize();
//     #ifdef __EMSCRIPTEN__
//         // no target FPS (3rd param) for performance
//         emscripten_set_main_loop_arg(loop, this, 0, 1);
//     #else
//         SetTargetFPS(TARGET_FPS);
//         while (!WindowShouldClose() && isRunning()) {
//             loop(this);
//         }
//     #endif
// }

// const bool Game::isRunning() const {
//     return state != END;
// }

// #if __EMSCRIPTEN__
// EM_JS(int, getWindowWidth, (), {
//     return window.document.querySelector('canvas').clientWidth;
// });

// EM_JS(int, getWindowHeight, (), {
//     return window.document.querySelector('canvas').clientHeight;
// });
// #endif

// void Game::resize() {
//     #if __EMSCRIPTEN__
//         // add padding to fit other web elements 
//         int width = getWindowWidth() - WINDOW_PAD;
//         int height = getWindowHeight() - WINDOW_PAD;
//     #else
//         int width = GetScreenWidth();
//         int height = GetScreenHeight();
//     #endif

//     if(screenHeight != height || screenWidth != width || state == BEGIN){
//         screenWidth = width; 
//         display.screenWidth = width;
//         world.screenWidth = width;
//         screenHeight = height;
//         display.screenHeight = height;
//         world.screenHeight = height;
//     #if __EMSCRIPTEN__
//         SetWindowSize(screenWidth, screenHeight);
//     #endif
//     }

//     lastResize = std::chrono::steady_clock::now();
//     TraceLog(LOG_INFO, "Window resized %dx%d", screenWidth, screenHeight);
// }
