// VERSION: 0.1

/*
    NOTE: In order to use this library you must define
    the following macro in exactly one file, _before_ including brick.h:

    #define BRICK_IMPLEMENTATION
    #include "brick.h"

    DESC: Clay is immediate mode. Every frame, the UI is declared from scratch, laid out, rendered, and discarded. Brick is the stateful layer on top. It remembers which buttons exist, what their IDs are, and what their interaction state was last frame. It turns Clay's stateless per-frame declarations into a system where the user can say "did button 47 get clicked?" in a natural way.
*/


#ifndef BRICK_HEADER
#define BRICK_HEADER


#include <cstdint>
#ifdef BRICK_IMPLEMENTATION
#define CLAY_IMPLEMENTATION
#endif

#include "clay.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
// #include <stdio.h>
// #include <string.h>

#define BRICK_MAX_ELEMENTS 256

// -----------------------------------------
// PUBLIC TYPES ----------------------------
// -----------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float width;
    float height;
    uint32_t hoveredId;
    uint32_t lastHoveredId;
} Brick_Window;

typedef struct {
    float x;
    float y;
    bool pressed;
} Brick_PointerData;

typedef struct {
    Clay_ElementId clayId;
    Clay_String label;
    uint32_t id;
    bool hovered;
    bool clicked;
    bool pressed;
    bool blurred;
    bool released;
} Brick_Element;

// Different event types triggered by element interactions
typedef CLAY_PACKED_ENUM {
    // This event should be skipped.
    BRICK_EVENT_NONE,
    // Triggers when the element is hovered (first frame only)
    BRICK_EVENT_HOVER,
    // Triggers when the element is being hovered
    BRICK_EVENT_HOVERING,
    // Triggers on the frame hovering stops
    BRICK_EVENT_BLUR,
    // Triggers when the element is pressed with the primary input (first frame only)
    BRICK_EVENT_PRESS,
    // Triggers while the element is being pressed on across frames
    BRICK_EVENT_PRESSING,
    // Triggers on the exact frame (or delayed by one) the press was released
    BRICK_EVENT_RELEASE,
} Brick_EventType;

typedef struct Brick_Event {
    // The id of this element
    uint32_t id;
    // The event that this element is in
    Brick_EventType eventType;
} Brick_Event;

// A sized array of events
typedef struct Brick_EventArray {
    // The underlying max capacity of the array, not necessarily all initialized.
    int32_t capacity;
    // The number of initialized elements in this array. Used for loops and iteration.
    int32_t length;
    // A pointer to the first element in the internal array.
    Brick_Event* internalArray;
} Brick_EventArray;

#ifdef __cplusplus
}
#endif

#endif /* BRICK_HEADER */

// -----------------------------------------
// IMPLEMENTATION --------------------------
// -----------------------------------------

#ifdef BRICK_IMPLEMENTATION
#undef BRICK_IMPLEMENTATION

#define STYLE_TEXT_CENTERED CLAY_TEXT_CONFIG({ .textColor = Clay_Color({ 200, 200, 200, 255 }), .fontSize = 24, .textAlignment = CLAY_TEXT_ALIGN_CENTER })

// Clay Global Arena
static Clay_Arena g_clay_arena = {0};

// Brick state variables
Brick_Window  g_window = {0};

Brick_Element g_elements[BRICK_MAX_ELEMENTS];
size_t        g_element_index = 0;

/* ---- Per-frame: reset at start of frame, read after PollEvents ---- */
Brick_Event   g_events[BRICK_MAX_ELEMENTS];   /* max one event per element per frame */
// size_t        g_event_index = 0;

// Helper functions
// --------------------------

static bool Brick_OnButtonHover(uint32_t id, bool isHovered) {
    // gets called on every frame with every button
    // compare the cached buttons with the current button hovered
    // and move the current hovered to the last hovered
    if (isHovered && !g_elements[id].hovered && g_window.hoveredId != id) {
        // mark the button as hovered
        g_elements[id].hovered = true;
        g_elements[g_window.lastHoveredId].hovered = false;
        g_window.lastHoveredId = g_window.hoveredId;
        g_window.hoveredId = id;

        return true;
    } else if (isHovered && g_window.hoveredId == id && g_window.lastHoveredId != g_window.hoveredId) {
        // this allows for one frame of propagation of when the button was hovered
        // it can be queried to know the frame right after the button hovered
        g_window.lastHoveredId = g_window.hoveredId;
    } else if (!isHovered && g_window.hoveredId == id) {
        // blur the current button
        g_elements[id].hovered = false;
        g_elements[id].blurred = true;
        g_window.hoveredId = 0;
    } else if (!isHovered && g_window.lastHoveredId == id) {
        // this allows for one frame of propagation of the blur
        g_window.lastHoveredId = 0;
        g_elements[id].blurred = false;
    } 

    // return whether the button was hovered or not
    // to allow immediate query on the hover
    return false;
}

static void Brick_HandleClayHover(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    
    for (uint32_t i = 0; i < BRICK_MAX_ELEMENTS; i++) {
        if(g_elements[i].clayId.id == elementId.id) {
            switch(pointerData.state) {
            case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
                // printf("CLICK %d\n", elementId.id);
                g_elements[i].clicked = true;
                break;
            case CLAY_POINTER_DATA_PRESSED:
                g_elements[i].pressed = true;
                break;
            case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
                g_elements[i].clicked = false;
                g_elements[i].pressed = false;
                g_elements[i].released = true;
                break;
            case CLAY_POINTER_DATA_RELEASED:
                // g_elements[i].released = true;
                break;
            default: break;
            }
            break;
        }
    }
}


// Public API
// ----------------------------------
static void Brick_HandleError(Clay_ErrorData errorData);
static uint32_t Brick_CreateButton(const char* label);

// initializes Clay first, then Brick
static void Brick_Initialize(float width, float height, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void *fontData), void *fontData) {
    printf("Initializing Brick");

    g_window.width = width;
    g_window.height = height;
    // 1. Query minimum memory required for default element limits
    uint64_t memorySize = Clay_MinMemorySize();
    // 2. Allocate memory (malloc, stack, or custom allocator)
    void* memory = malloc(memorySize);
    // 3. Create arena [clay.h:2150-2158]
    g_clay_arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    // 4. Initialize Clay [clay.h:2186-2188]
    Clay_Initialize(g_clay_arena, Clay_Dimensions({ width, height }), Clay_ErrorHandler({ .errorHandlerFunction = Brick_HandleError, .userData = nullptr }));

    Clay_SetMeasureTextFunction(measureTextFunction, fontData);

    Brick_CreateButton("DUMMY");
}

// simple wrapper around Clay_BeginLayout
static void Brick_BeginLayout(void) {
    Clay_BeginLayout();
}

static bool Brick_PointerJustHovered() {
    return g_window.hoveredId != 0 && g_window.lastHoveredId != g_window.hoveredId;
}

static bool Brick_PointerJustBlurred() {
    return g_window.hoveredId == 0 && g_window.lastHoveredId != 0;
}

// Brick only function that will handle any potential updates of elements per frame
static Brick_EventArray Brick_PollEvents(Brick_PointerData pointerData) {

    Clay_SetPointerState(Clay_Vector2({ .x = pointerData.x, .y = pointerData.y }), pointerData.pressed);
    
    Brick_EventArray events = {
        .capacity = BRICK_MAX_ELEMENTS,
        .length = 0,
        .internalArray = g_events
    };

    for (uint32_t i = 0; i < g_element_index; i++) {

        if(g_elements[i].clicked && !g_elements[i].pressed) {
            // click only lasts one frame
            // g_elements[i].clicked = false;
            g_events[i] = {
                .id = g_elements[i].id,
                .eventType = BRICK_EVENT_PRESS
            };
            events.length++;
        } 
        else if (g_elements[i].pressed) { 
            g_events[i] = {
                .id = g_elements[i].id,
                .eventType = BRICK_EVENT_PRESSING
            };
            events.length++;
        } 
        else if(g_elements[i].released) {
            g_elements[i].released = false;
            g_events[i] = {
                .id = g_elements[i].id,
                .eventType = BRICK_EVENT_RELEASE
            };
            events.length++;
        }
        else if(g_elements[i].hovered) {
            g_events[i] = {
                .id = g_elements[i].id,
                .eventType = Brick_PointerJustHovered() ? BRICK_EVENT_HOVER : BRICK_EVENT_HOVERING
            };
            events.length++;
        } 
        else if(g_elements[i].blurred) {
            g_events[i] = {
                .id = g_elements[i].id,
                .eventType = BRICK_EVENT_BLUR
            };
            events.length++;
        } 

    }

    return events;
}


// simple wrapper around Clay_EndLayout which returns render commands
static Clay_RenderCommandArray Brick_EndLayout(float deltaTime) {
    return Clay_EndLayout(deltaTime);
}
// cleans up Brick then Clay
static void Brick_Destroy(void) {
    if(g_clay_arena.memory) free(g_clay_arena.memory);
}

// Brick elements Add<Element> initializes the element and is to be called once
static uint32_t Brick_CreateButton(const char* label) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(label), 
        .chars = label 
    };
    Clay_ElementId buttonId = CLAY_SID(clayString);

    uint32_t index = g_element_index;
    g_elements[g_element_index] = (Brick_Element){
        .clayId = buttonId,
        .label = clayString,
        .id = index,
        .hovered = false,
        .clicked = false,
        .pressed = false,
        .blurred = false,
        .released = false,
    };
    g_element_index++;

    return index;
}
// Layout<element> is to be called within CLAY macros which are also encapsulated in other Brick elements
static void Brick_LayoutButton(uint32_t id) {
    if (id > g_element_index) return;

    const Brick_Element* button = &g_elements[id];
    // if (button != nullptr) {
    //     printf("layout button %s", button->label);
    // }
    CLAY(button->clayId, { 
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
        }, 
        // Clay_Hovered only works inside the paramaters or Clay declaration body
        .backgroundColor = Clay_Hovered() ? Clay_Color({ 140, 140, 140, 255 }) : Clay_Color({ 216, 238, 10, 255 }),
        .border = { 
            .color = Clay_Color({ 140, 140, 140, 255 }), 
            .width = CLAY_BORDER_OUTSIDE(1) 
        }
        // .transition = {
        //     .handler = Clay_EaseOut,
        //     .duration = 0.3f,
        //     .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
        //     .enter = { .setInitialState = FadeSlide },
        //     // .exit = { .setFinalState = FadeSlide },
        // }
    }) {
        Brick_OnButtonHover(button->id, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHover, nullptr);
        CLAY_TEXT(button->label, STYLE_TEXT_CENTERED);
    }
}
// BeginLayout<element> and EndLayout<element> are the opening and close functions for container elements
static void Brick_BeginLayoutPanel(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(0.5f),
                .height = CLAY_SIZING_PERCENT(0.5f),
            },
            .padding = CLAY_PADDING_ALL(32), 
            .childGap = 12, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = Clay_Color({ 140, 140, 140, 255 }),
        .floating = { 
            .offset = {0, 0}, 
            .zIndex = 1, 
            .attachPoints = { 
                CLAY_ATTACH_POINT_CENTER_CENTER, 
                CLAY_ATTACH_POINT_CENTER_CENTER 
            }, 
            .attachTo = CLAY_ATTACH_TO_PARENT 
        },
    });
}

static void Brick_EndLayoutPanel(void) {
    Clay__CloseElement();
}

void Brick_HandleError(Clay_ErrorData errorData) {

    switch(errorData.errorType) {

        case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED:
            printf("CLAY ERROR: Text Measurement Function not provided.");
            break;
        // Clay attempted to allocate its internal data structures but ran out of space.
        // The arena passed to Clay_Initialize was created with a capacity smaller than that required by Clay_MinMemorySize().
        case CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Arena capacity exceeded.");
            break;
        // Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxElementCount().
        case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Elements capacity exceeded.");
            break;
        // Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxMeasureTextCacheWordCount().
        case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Text measurement capacity exceeded.");
            break;
        // Two elements were declared with exactly the same ID within one layout.
        case CLAY_ERROR_TYPE_DUPLICATE_ID:
            printf("CLAY ERROR: Duplicate ID.");
            break;
        // A floating element was declared using CLAY_ATTACH_TO_ELEMENT_ID and either an invalid .parentId was provided or no element with the provided .parentId was found.
        case CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND:
            printf("CLAY ERROR: Floating container parent not found.");
            break;
        // An element was declared that using CLAY_SIZING_PERCENT but the percentage value was over 1. Percentage values are expected to be in the 0-1 range.
        case CLAY_ERROR_TYPE_PERCENTAGE_OVER_1:
            printf("CLAY ERROR: Percentage over 1.");
            break;
        // Clay encountered an internal error. It would be wonderful if you could report this so we can fix it!
        case CLAY_ERROR_TYPE_INTERNAL_ERROR:
            printf("CLAY ERROR: Internal error.");
            break;
        // Clay__OpenElement was called more times than Clay__CloseElement, so there were still remaining open elements when the layout ended.
        case CLAY_ERROR_TYPE_UNBALANCED_OPEN_CLOSE:
            printf("CLAY ERROR: Unbalanced open-close.");
            break;
        case CLAY_ERROR_TYPE_HASH_MAP_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Hash map capacity exceeded.");
            break;
        default: break;
    }
    
    if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    }

    // Brick_Initialize(g_window.width, g_window.height);
}

#endif /* BRICK_IMPLEMENTATION */
