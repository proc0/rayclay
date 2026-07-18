#pragma once

#include "type.hpp"
#include "window.hpp"
#include "widget.hpp"

// silencing minor warnings due to C/C++ differences
// -Wpedantic for one anonymous struct in Clay_LayoutElement
extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "clay.h"
#pragma GCC diagnostic pop
}

#include "raylib.h"

#include <string>

class Surface : public Layer {
    Font fonts[3];
    // TODO: move this into a Display class
    // along with building the components
    // for the HUD
    Texture2D textureArrowUp;
    Texture2D textureArrowRight;
    Texture2D textureArrowDown;
    Texture2D textureArrowLeft;
    Texture2D textureBlueTile;
    std::string formatScore = "Score";
    std::string formatTotalTime = "Time";

    Camera Raylib_camera;
    Shader overlayShader;

    Clay_Arena arena = {};

    const Window& window;
    // TODO: move this to Display
    int gameScore;
    State::Game currentGameState = State::Game::START;
    // overlay shader variable
    int overlayColorLocation;

public:
    Widget widget;

    Event::Surface lastEvent = Event::Surface::NO_EVENT;
    Event::Surface surfaceEvent = Event::Surface::NO_EVENT;

    Surface(const Window& window): window(window) {};
    ~Surface() = default;

    void (Surface::*render)(Clay_RenderCommandArray& renderCommands) const = &Surface::renderUnit;
    void (Surface::*layoutMenu)() = &Surface::layoutMenuUnit;
    void (Surface::*layoutDisplay)() = &Surface::layoutDisplayUnit;
    Action::Surface (Surface::*update)(const InputEvent&) = &Surface::updateUnit;

    void load();
    void loadOverlay();

    void renderUnit(Clay_RenderCommandArray& renderCommands) const {};
    void renderRaylib(Clay_RenderCommandArray& renderCommands) const;

    void layoutMenuUnit() {};
    void layoutMenuMain();
    void layoutMenuPause();
    void layoutTutorial();
    void layoutOptions();
    void layoutWinLose();

    // TODO: this would also move to Display
    void layoutDisplayUnit() {};
    void layoutDisplayGame();
    void updateDisplay(const GameState);
    
    Action::Surface updateUnit(const InputEvent&) { return Action::Surface::DO_NOTHING; };
    Action::Surface updateMenu(const InputEvent&);
    Action::Surface updateOptions(const InputEvent&);

    void beginEvent(Event::Surface);
    void clearEvent();
    bool hasEvent() const;

    static void handleError(Clay_ErrorData);
    void resize(int width, int height);
    void transition(State::App, State::Screen);
    void unload();
};

// forward declare functions coming in from Widget
Clay_TransitionData FadeSlide(Clay_TransitionData initialState, Clay_TransitionProperty properties);
