#pragma once

#include "type.hpp"
#include "window.hpp"
#include "widget.hpp"

#include "clay.h" 
#include "raylib.h"

#include <string>

class Surface : public Layer {
    Font fonts[2];
    // TODO: move this into a Display class
    // along with building the components
    // for the HUD
    Texture2D textureArrowUp;
    Texture2D textureArrowRight;
    Texture2D textureArrowDown;
    Texture2D textureArrowLeft;
    std::string formatScore = "Score";

    Camera Raylib_camera;
    Shader overlayShader;

    Clay_Arena arena = {};

    const Window& window;
    // TODO: move this to Display
    int gameScore;
    // overlay shader variable
    int overlayColorLocation;

public:
    Widget widget;

    Event::Surface lastEvent = Event::Surface::NO_EVENT;
    Event::Surface surfaceEvent = Event::Surface::NO_EVENT;
    BUTTON_ID activeOptionsTab = BUTTON_ID::OPTIONS_GAME;

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

    // TODO: this would also move to Display
    void layoutDisplayUnit() {};
    void layoutDisplayGame();
    void updateDisplay(const GameState);
    
    Action::Surface updateUnit(const InputEvent&) { return Action::Surface::DO_NOTHING; };
    Action::Surface updateMenu(const InputEvent&);
    Action::Surface updateOptions(const InputEvent&);

    void beginEvent(Event::Surface);
    void clearEvent();

    static void handleError(Clay_ErrorData);
    void resize(int width, int height);
    void transition(State::App, State::Screen);
    void unload();
};
