#pragma once

#include "type.hpp"
#include "window.hpp"
#include "widget.hpp"

#include "clay.h" 
#include "raylib.h"



// constexpr std::pair<std::string_view, Action::Surface> blah = std::make_pair("blah", Action::Surface::NEW_GAME);

// NOTE: make this WIDGETS / WIDGET, abstract to any pair of IDs / Actions
// add it to type.hpp in a Widget class, to be a member of Surface
// i.e. surface.widget.buttons.at(BUTTON_NEW_GAME)
// where BUTTON_NEW_GAME would be the string to the map in Widget.buttons
// that way it can be used for modal windows, or anything else, and is next to Action enum
// or it could be in its own widget.hpp and it's only for Surface to import
// #ifndef BUTTONS
// #define BUTTONS 
// BUTTON("ButtonGameNew",            NEW_GAME)
// BUTTON("ButtonGameResume",      RESUME_GAME)
// BUTTON("ButtonMainMenu",          MAIN_MENU)
// BUTTON("ButtonGameLoad",          LOAD_GAME)
// BUTTON("ButtonOptions",             OPTIONS)
// BUTTON("ButtonConfirm", CONFIRM_RETURN_MAIN)
// BUTTON("ButtonCancel",   CANCEL_RETURN_MAIN)
// BUTTON("ButtonQuit",               QUIT_APP)

// const std::unordered_map<std::string, Action::Surface> buttonActions = {
// #define BUTTON(ID, ACTION) { NAME, Action::Surcae::ACTION },
// BUTTONS
// #undef BUTTON
// };

// #undef BUTTONS


// TODO: this should be a member but the render method should be const
// needs to somehow read this to start/end shaderMode for overlay
// static inline bool overlayEnabled = false;

// A MALLOC'd buffer, that we keep modifying inorder to save from so many Malloc and Free Calls.
// Call Clay_Raylib_Close() to free


class Surface : public Layer {
    Shader overlayShader;
    Camera Raylib_camera;
    Font fonts[2];
    // Texture2D profilePicture;
    // Texture2D parchmentTexture;
    // Texture2D monkTexture;
    Clay_Arena arena = {};
    ScrollbarData scrollbarData = {0};

    uint32_t buttonHoverId = 0;
    // Clay_SizingAxis sidebarWidth = CLAY_SIZING_PERCENT(0.2f);

    const Window& window;
    int colorLoc;

public:
    Widget widget;
//     const std::unordered_map<std::string, Action::Surface> buttonActions = {
// #define BUTTON(ID, ACTION) { ID, Action::Surface::ACTION },
// BUTTONS
// #undef BUTTON
    // };
    // const std::unordered_map<std::string, Action::Surface> buttonActions = {
    //     { "ButtonGameNew", Action::Surface::NEW_GAME },
    //     { "ButtonGameResume", Action::Surface::RESUME_GAME },
    //     { "ButtonMainMenu", Action::Surface::MAIN_MENU },
    //     { "ButtonGameLoad", Action::Surface::LOAD_GAME },
    //     { "ButtonOptions", Action::Surface::OPTIONS },
    //     { "ButtonConfirm", Action::Surface::CONFIRM_RETURN_MAIN },
    //     { "ButtonCancel", Action::Surface::CANCEL_RETURN_MAIN },
    //     { "ButtonQuit", Action::Surface::QUIT_APP }
    // };
    // Action::Surface buttonAction = Action::Surface::DO_NOTHING;
    Event::Surface surfaceEvent = Event::Surface::NO_EVENT;
    // bool showOverlay = 0;
    // bool showReturnMainMenuConfirmation = 0;
    uint32_t activeTabId = 0;

    Surface(const Window& window): window(window) {};
    ~Surface() = default;

    void load();
    void initOverlay();
    void setColorOverlay(Color) const;
    void disableColorOverlay() const;

    void renderNull(Clay_RenderCommandArray& renderCommands) const;
    void (Surface::*render)(Clay_RenderCommandArray& renderCommands) const = &Surface::renderNull;
    void renderRaylib(Clay_RenderCommandArray& renderCommands) const;

    // void layout(GameState);
    void menuNull();
    void (Surface::*menu)() = &Surface::menuNull;
    void menuMain();
    void menuPause();

    void displayUnit(GameState);
    void (Surface::*display)(GameState) = &Surface::displayUnit;
    void displayGame(GameState);
    
    Action::Surface updateNull(const InputEvent& inputEvent);
    Action::Surface (Surface::*update)(const InputEvent& inputEvent) = &Surface::updateNull;
    Action::Surface updateMenu(const InputEvent& inputEvent);

    // void buttonSimple(const Clay_ElementId& id, const Clay_String& buttonText);
    void buttonTab(const Clay_ElementId& id, const Clay_String& buttonText);
    static void handleError(Clay_ErrorData);
    void resize(int width, int height);
    void beginEvent(Event::Surface);
    void clearEvent();
    void transition(State::App, State::Screen);
    void unload();
};

// #undef BUTTONS
// #endif
