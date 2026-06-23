#pragma once

#include "screen.hpp"
#include "types.hpp"

#include "clay.h" // IWYU pragma: export

#include <raylib.h>

#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) Color({ .r = static_cast<unsigned char>(roundf(color.r)), .g = static_cast<unsigned char>(roundf(color.g)), .b = static_cast<unsigned char>(roundf(color.b)), .a = static_cast<unsigned char>(roundf(color.a)) })
#define RAYLIB_COLOR_TO_CLAY_COLOR(color) Clay_Color({ static_cast<float>(roundf(color.r)), static_cast<float>(roundf(color.g)), static_cast<float>(roundf(color.b)), static_cast<float>(roundf(color.a)) })
#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) Clay_Vector2({ .x = vector.x, .y = vector.y })

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

// TODO: this should be a member but the render method should be const
// needs to somehow read this to start/end shaderMode for overlay
// static inline bool overlayEnabled = false;

// A MALLOC'd buffer, that we keep modifying inorder to save from so many Malloc and Free Calls.
// Call Clay_Raylib_Close() to free
static inline char *temp_render_buffer;
static inline int temp_render_buffer_len;

class Display : public ScreenListener {
    Shader overlayShader;
    Camera Raylib_camera;
    Font fonts[2];
    Texture2D profilePicture;
    Texture2D parchmentTexture;
    Texture2D monkTexture;
    Clay_Arena arena = {};
    ScrollbarData scrollbarData = {0};
    
    uint32_t buttonHoverId = 0;
    Clay_SizingAxis sidebarWidth = CLAY_SIZING_PERCENT(0.2f);

    const Screen& screen;

    int colorLoc;

public:
    bool showOverlay;
    uint32_t activeTabId = 0;

    Display(const Screen& screen): screen(screen) {};
    ~Display() = default;

    void load();
    void render(Clay_RenderCommandArray& renderCommands) const;
    void initOverlay();
    void setColorOverlay(Color) const;
    void disableColorOverlay() const;
    void layout();
    void update(const InputEvent& inputEvent);
    void buttonSimple(const Clay_ElementId& id, const Clay_String& buttonText);
    void buttonTab(const Clay_ElementId& id, const Clay_String& buttonText);
    static void handleError(Clay_ErrorData);
    void onScreenResize(int width, int height);
    void unload();
};
