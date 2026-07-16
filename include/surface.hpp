#pragma once

#include "type.hpp"
#include "window.hpp"
#include "widget.hpp"

#include "clay.h" 
#include "raylib.h"

class Surface : public Layer {
    Font fonts[2];
    Camera Raylib_camera;
    Shader overlayShader;

    Clay_Arena arena = {};


    const Window& window;
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
    void (Surface::*layoutDisplay)(GameState) = &Surface::layoutDisplayUnit;
    Action::Surface (Surface::*update)(const InputEvent& inputEvent) = &Surface::updateUnit;

    void load();
    void loadOverlay();

    void renderUnit(Clay_RenderCommandArray& renderCommands) const {};
    void renderRaylib(Clay_RenderCommandArray& renderCommands) const;

    void layoutMenuUnit() {};
    void layoutMenuMain();
    void layoutMenuPause();
    void layoutTutorial();
    void layoutOptions();
    void layoutDisplayUnit(GameState) {};
    void layoutDisplayGame(GameState);
    
    Action::Surface updateUnit(const InputEvent& inputEvent) { return Action::Surface::DO_NOTHING; };
    Action::Surface updateMenu(const InputEvent& inputEvent);

    void beginEvent(Event::Surface);
    void clearEvent();

    static void handleError(Clay_ErrorData);
    void resize(int width, int height);
    void transition(State::App, State::Screen);
    void unload();
};
