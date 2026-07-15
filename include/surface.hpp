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
    ScrollbarData scrollbarData = {0};

    const Window& window;
    // overlay shader variable
    int overlayColorLocation;

public:
    Widget widget;

    Event::Surface surfaceEvent = Event::Surface::NO_EVENT;

    Surface(const Window& window): window(window) {};
    ~Surface() = default;

    void (Surface::*render)(Clay_RenderCommandArray& renderCommands) const = &Surface::renderUnit;
    void (Surface::*menu)() = &Surface::menuUnit;
    void (Surface::*display)(GameState) = &Surface::displayUnit;
    Action::Surface (Surface::*update)(const InputEvent& inputEvent) = &Surface::updateUnit;

    void load();
    void initOverlay();

    void renderUnit(Clay_RenderCommandArray& renderCommands) const {};
    void renderRaylib(Clay_RenderCommandArray& renderCommands) const;

    // void layout(GameState);
    void menuUnit() {};
    void menuMain();
    void menuPause();

    void displayUnit(GameState) {};
    void displayGame(GameState);
    
    Action::Surface updateUnit(const InputEvent& inputEvent) {
        return Action::Surface::DO_NOTHING;
    };
    Action::Surface updateMenu(const InputEvent& inputEvent);

    void beginEvent(Event::Surface);
    void clearEvent();

    static void handleError(Clay_ErrorData);
    void resize(int width, int height);
    void transition(State::App, State::Screen);
    void unload();
};
