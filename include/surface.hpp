#pragma once

#include "type.hpp"
#include "window.hpp"

#include "clay.h" 
#include "raylib.h"

#include <string>
#include <unordered_map>

#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) Color({ .r = static_cast<unsigned char>(roundf(color.r)), .g = static_cast<unsigned char>(roundf(color.g)), .b = static_cast<unsigned char>(roundf(color.b)), .a = static_cast<unsigned char>(roundf(color.a)) })
#define RAYLIB_COLOR_TO_CLAY_COLOR(color) Clay_Color({ static_cast<float>(roundf(color.r)), static_cast<float>(roundf(color.g)), static_cast<float>(roundf(color.b)), static_cast<float>(roundf(color.a)) })
#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) Clay_Vector2({ .x = vector.x, .y = vector.y })

#define SURFACE_BUTTON_COLOR_BG Clay_Color({ 0, 0, 0, 100 })
#define SURFACE_BUTTON_COLOR_BG_HL Clay_Color({ 0, 0, 0, 130 })
#define SURFACE_BUTTON_COLOR_FG Clay_Color({ 200, 200, 200, 255 })
#define SURFACE_BUTTON_COLOR_FG_HL Clay_Color({ 255, 255, 255, 255 })
#define SURFACE_MENU_COLOR_BG Clay_Color({ 0, 0, 0, 150 })

typedef enum
{
    CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL
} CustomLayoutElementType;

typedef struct
{
    Model model;
    float scale;
    Vector3 position;
    Matrix rotation;
} CustomLayoutElement_3DModel;

typedef struct
{
    CustomLayoutElementType type;
    union {
        CustomLayoutElement_3DModel model;
    } customData;
} CustomLayoutElement;

typedef struct
{
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    float scrollY;
    bool mouseDown;
} ScrollbarData;

typedef struct DisplayButtonContext {
    void* self;
    Action::Surface action;
} DisplayButtonContext;
// TODO: this should be a member but the render method should be const
// needs to somehow read this to start/end shaderMode for overlay
// static inline bool overlayEnabled = false;

// A MALLOC'd buffer, that we keep modifying inorder to save from so many Malloc and Free Calls.
// Call Clay_Raylib_Close() to free
static inline char *temp_render_buffer;
static inline int temp_render_buffer_len;

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
    const std::unordered_map<std::string, Action::Surface> buttonActions = {
        { "ButtonGameNew", Action::Surface::NEW_GAME },
        { "ButtonGameResume", Action::Surface::RESUME_GAME },
        { "ButtonMainMenu", Action::Surface::MAIN_MENU },
        { "ButtonGameLoad", Action::Surface::LOAD_GAME },
        { "ButtonOptions", Action::Surface::OPTIONS },
        { "ButtonConfirm", Action::Surface::CONFIRM_RETURN_MAIN },
        { "ButtonCancel", Action::Surface::CANCEL_RETURN_MAIN },
        { "ButtonQuit", Action::Surface::QUIT_APP }
    };

    Action::Surface buttonAction = Action::Surface::DO_NOTHING;
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

    void buttonSimple(const Clay_ElementId& id, const Clay_String& buttonText);
    void buttonTab(const Clay_ElementId& id, const Clay_String& buttonText);
    static void handleError(Clay_ErrorData);
    void resize(int width, int height);
    void beginEvent(Event::Surface);
    void clearEvent();
    void transition(State::App, State::Screen);
    void unload();
};
