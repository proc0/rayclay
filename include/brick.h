// VERSION: 0.1

/* 
    USAGE: Define BRICK_IMPLEMENTATION in exactly ONE file, then include brick.h:
    
    #define BRICK_IMPLEMENTATION
    #include "brick.h"
    
    Other files can include either brick.h or clay.h for types and utility functions.


    DESC: Clay is immediate mode. Every frame, the UI is declared from scratch, laid out, rendered, and discarded. Brick is the stateful layer on top. It remembers which buttons exist, what their IDs are, and what their interaction state was last frame. It turns Clay's stateless per-frame declarations into a system where the user can say "did button 47 get clicked?" in a natural way.
*/

#ifndef BRICK_HEADER
#define BRICK_HEADER

#ifdef BRICK_IMPLEMENTATION
#define CLAY_IMPLEMENTATION
#endif
#include "clay.h"

// Default Settings 
// ---------------------------------------------------------------

#define BRICK_MAX_BUTTONS 128
#define BRICK_MAX_SCROLLBOXES 32
#define BRICK_MAX_ELEMENTS (BRICK_MAX_BUTTONS + BRICK_MAX_SCROLLBOXES)

// Default Styles 
// ---------------------------------------------------------------

#define STYLE_TEXT_CENTERED CLAY_TEXT_CONFIG({ .textColor = Clay_Color({ 200, 200, 200, 255 }), .fontSize = 24, .textAlignment = CLAY_TEXT_ALIGN_CENTER })

// Public Types 
// ---------------------------------------------------------------

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
} Brick_Button;

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
    // internal brick id
    uint32_t id;
    // event state for the element
    Brick_EventType eventType;
} Brick_Event;

// events array container
typedef struct Brick_EventArray {
    int32_t length;
    Brick_Event* data;
} Brick_EventArray;

#ifdef __cplusplus
}
#endif

// Function Forward Declarations ---------------------------------
// Public API functions ------------------------------------------

Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index);

#endif /* BRICK_HEADER */

// IMPLEMENTATION 
// ---------------------------------------------------------------

#ifdef BRICK_IMPLEMENTATION
#undef BRICK_IMPLEMENTATION

Brick_Event Brick_Event_DEFAULT = CLAY__DEFAULT_STRUCT;
Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_Event_DEFAULT;
}    

typedef struct Brick_ButtonArray {
    int32_t length;
    Brick_Button* data;
} Brick_ButtonArray;

Brick_Button Brick_Button_DEFAULT = CLAY__DEFAULT_STRUCT;
Brick_Button* Brick_ButtonArray_Get(Brick_ButtonArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_Button_DEFAULT;
}    

typedef struct Brick__Elements {
    Brick_ButtonArray buttons;
} Brick__Elements;

// Global Context
// --------------------------

// Clay context
Clay_Arena g_clay_arena = CLAY__DEFAULT_STRUCT;

// Window state also holds pointer state
Brick_Window g_window = CLAY__DEFAULT_STRUCT;

// Main element state arrays
Brick_Button g_buttons[BRICK_MAX_BUTTONS];
Brick__Elements g_elements = {
    .buttons = {
        .length = 0,
        .data = g_buttons
    },
};
// keeps track of total elements created
size_t g_element_count = 0;

// event array passed back to user to handle events
Brick_Event g_events[BRICK_MAX_ELEMENTS];

// Button internals
// ---------------------------------------------------------------

void Brick_OnButtonHover(uint32_t id, bool isHovered) {
    Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, id);
    // gets called on every frame with every button
    // compare the cached buttons with the current button hovered
    // and move the current hovered to the last hovered
    if (isHovered && !button->hovered && g_window.hoveredId != id) {
        // mark the button as hovered
        button->hovered = true;

        Brick_Button* lastButton = Brick_ButtonArray_Get(&g_elements.buttons, g_window.lastHoveredId);
        lastButton->hovered = false;
        // NOTE: resetting lastHovered blur here prevents fast hover changes
        // that prevent the blur from being reset in the last conditional
        lastButton->blurred = false;

        g_window.lastHoveredId = g_window.hoveredId;
        g_window.hoveredId = id;

    } else if (isHovered && g_window.hoveredId == id && g_window.lastHoveredId != id) {
        // this allows for one frame of propagation of when the button was hovered
        // it can be queried to know the frame right after the button hovered
        g_window.lastHoveredId = id;
    } else if (!isHovered && g_window.hoveredId == id) {
        // blur the current button
        button->hovered = false;
        button->blurred = true;
        g_window.hoveredId = 0;
    } else if (!isHovered && g_window.lastHoveredId == id) {
        // this allows for one frame of propagation of the blur
        g_window.lastHoveredId = 0;
        button->blurred = false;
    }
}

void Brick_HandleClayHover(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    
    for (uint32_t i = 1; i < g_elements.buttons.length; i++) {
        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, i);
        
        if(button->clayId.id == elementId.id) {
            switch(pointerData.state) {
            case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
                // printf("CLICK %d\n", elementId.id);
                button->clicked = true;
                break;
            case CLAY_POINTER_DATA_PRESSED:
                button->pressed = true;
                break;
            case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
                button->clicked = false;
                button->pressed = false;
                button->released = true;
                break;
            case CLAY_POINTER_DATA_RELEASED:
                // NOTE: This is just the same as hover, Clay triggers this if pointer 
                // is on the button not pressing, and after pressing
                break;
            default: break;
            }
            break;
        }
    }
}


// Public API
// ----------------------------------

void Brick_HandleError(Clay_ErrorData errorData);

uint32_t Brick_CreateButton(const char* label);

// initializes Clay first, then Brick
void Brick_Initialize(float width, float height, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void *fontData), void *fontData) {
    printf("Initializing Brick");
    // cache window dimensions
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
    // 5. Set the MeasureText function along with pointer to fonts
    Clay_SetMeasureTextFunction(measureTextFunction, fontData);

    // seed button array at index 0
    // for safety and as a neutral value
    Brick_CreateButton("BRICK");
}

// simple wrapper around Clay_BeginLayout
void Brick_BeginLayout(void) {
    Clay_BeginLayout();
}

// simple wrapper around Clay_EndLayout which returns render commands
Clay_RenderCommandArray Brick_EndLayout(float deltaTime) {
    return Clay_EndLayout(deltaTime);
}
// cleans up Brick then Clay
void Brick_Destroy(void) {
    if(g_clay_arena.memory) free(g_clay_arena.memory);
}

// Brick elements Add<Element> initializes the element and is to be called once
uint32_t Brick_CreateButton(const char* label) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(label), 
        .chars = label 
    };
    Clay_ElementId buttonId = CLAY_SID(clayString);

    uint32_t index = g_elements.buttons.length;
    g_buttons[index] = (Brick_Button){
        .clayId = buttonId,
        .label = clayString,
        .id = index,
        .hovered = false,
        .clicked = false,
        .pressed = false,
        .blurred = false,
        .released = false,
    };
    g_elements.buttons.length++;
    g_element_count++;

    return index;
}

bool Brick_PointerJustHovered() {
    return g_window.hoveredId != 0 && g_window.lastHoveredId != g_window.hoveredId;
}

bool Brick_PointerJustBlurred() {
    return g_window.hoveredId == 0 && g_window.lastHoveredId != 0;
}

// Brick only function that will handle any potential updates of elements per frame
Brick_EventArray Brick_PollEvents(Brick_PointerData pointerData) {

    Clay_SetPointerState(Clay_Vector2({ .x = pointerData.x, .y = pointerData.y }), pointerData.pressed);
    
    Brick_EventArray events = {
        .length = 0,
        .data = g_events
    };

    for (uint32_t i = 1; i < g_element_count; i++) {
        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, i);

        if(button->clicked && !button->pressed) {
            // click only lasts one frame
            // button->clicked = false;
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_PRESS
            };
            events.length++;
        } 
        else if (button->pressed) { 
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_PRESSING
            };
            events.length++;
        } 
        else if(button->released) {
            button->released = false;
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_RELEASE
            };
            events.length++;
        }
        else if(button->hovered) {
            g_events[events.length] = {
                .id = i,
                .eventType = Brick_PointerJustHovered() ? BRICK_EVENT_HOVER : BRICK_EVENT_HOVERING
            };
            events.length++;
        } 
        else if(button->blurred) {
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_BLUR
            };
            events.length++;
        } 

    }

    return events;
}

// Layout<element> is to be called within CLAY macros which are also encapsulated in other Brick elements
void Brick_LayoutButton(uint32_t id) {
    if (id > g_elements.buttons.length) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, id);

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
void Brick_BeginLayoutPanel(void) {
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

void Brick_EndLayoutPanel(void) {
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
    
    // NOTE: this handles Clay exceed element count
    // TODO: decide to initialize Clay only or Brick & Clay, and how this would work
    // with Brick element max count
    // if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
    //     Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    //     Brick_Initialize(g_window.width, g_window.height);
    // } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
    //     Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    //     Brick_Initialize(g_window.width, g_window.height);
    // }
}

#endif /* BRICK_IMPLEMENTATION */

/*
LICENSE
zlib/libpng license

Copyright (c) 2026 Alan Vincenzi

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/
