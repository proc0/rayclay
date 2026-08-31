// VERSION: 0.1

/* 
    USAGE: Define BRICK_IMPLEMENTATION in exactly ONE file, then include brick.h:
    
    #define BRICK_IMPLEMENTATION
    #include "brick.h"
    
    Other files can include either brick.h or clay.h for types and utility functions.


    DESC: Clay is immediate mode. Every frame, the UI is declared from scratch, laid out, rendered, and discarded. Brick is the stateful layer on top. It remembers which buttons exist, what their IDs are, and what their interaction state was last frame. It turns Clay's stateless per-frame declarations into a system where the user can say "did button 47 get clicked?" in a natural way.

    OBJ:
    1. add basic UI state, i.e. button hover or not, button is toggled, panel is visible or hidden
    2. separate content from layout, i.e. declar button and labels in one place, use them in the layout later
    3. uses 'embeded scope constructs' like Raylib to match this C style of API
    4. provide sensible defaults where possible, windows grow, have some padding, etc
    5. provide a global config that overrides defaults, change colors, adjust padding
    6. add responsive reactive behavior to the window, i.e. resize on event window resizing and adjust layout dynamically
    7. provide an easy way to localize, i.e. through Brick_TextEx or TextPro or a Brick_LocalizedText, that would stand in for normal strings and can be globally configured
*/

// TODO: double check cleanup of global state in Destroy
// TODO: separate element ids from array indices, create a global incremental id, and getters and setters
#ifdef BRICK_IMPLEMENTATION
#define CLAY_IMPLEMENTATION
#endif
#include "clay.h"

#ifndef BRICK_HEADER
#define BRICK_HEADER


// Default Settings 
// ---------------------------------------------------------------

#define BRICK_MAX_BUTTONS 256
#define BRICK_MAX_BUTTON_GROUP_SIZE 16
#define BRICK_MAX_BUTTON_GROUPS 16
#define BRICK_MAX_SCROLLBOXES 32
#define BRICK_MAX_ELEMENTS (BRICK_MAX_BUTTONS + BRICK_MAX_SCROLLBOXES)

// Default Styles 
// ---------------------------------------------------------------
#define CLAY_BLANK Clay_Color({ 0, 0, 0, 0 })
#define CLAY_WHITE Clay_Color({ 255.0f, 255.0f, 255.0f, 255.0f })
#define CLAY_BLACK Clay_Color({ 0, 0, 0, 255.0f })

#define SURFACE_COLOR_BG Clay_Color({ 140, 140, 140, 255 })
#define SURFACE_COLOR_FG Clay_Color({ 200, 200, 200, 255 })
#define SURFACE_COLOR_HL Clay_Color({ 235, 235, 235, 255 })
#define SURFACE_COLOR_SECONDARY Clay_Color({ 80, 80, 80, 255 })

#define SURFACE_COLOR_MENU_BG Clay_Color({ 0, 0, 0, 180 })
#define SURFACE_COLOR_ACCENT_BG Clay_Color({ 140, 140, 140, 255 })
#define SURFACE_COLOR_ACCENT_BORDER Clay_Color({ 80, 80, 80, 255 })
#define SURFACE_COLOR_ACCENT_RED Clay_Color({ 230, 40, 45, 255 })
#define SURFACE_COLOR_ACCENT_GOLD Clay_Color({ 216, 238, 10, 255 })

#define WIDGET_COLOR_BUTTON_BG Clay_Color({ 80, 80, 80, 255 })
#define WIDGET_COLOR_BUTTON_BG_HL Clay_Color({ 90, 90, 90, 255 })
#define WIDGET_COLOR_BORDER Clay_Color({ 200, 200, 200, 255 })

#define WIDGET_COLOR_SCROLLBAR Clay_Color({80, 80, 80, 255})
#define WIDGET_COLOR_SCROLLBAR_HL Clay_Color({140, 140, 140, 255})

#define STYLE_TEXT_DEFAULT CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_FG, .fontId = 1, .fontSize = 24 })

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

typedef CLAY_PACKED_ENUM {
    BRICK_ELEMENT_TYPE_BUTTON,
    BRICK_ELEMENT_TYPE_PANEL,
    BRICK_ELEMENT_TYPE_BUTTON_GROUP,
} Brick_ElementType;

typedef struct Brick_ElementId {
    uint32_t index;
    Brick_ElementType type;
} Brick_ElementId;

typedef struct {
    Clay_ElementId clayId;
    Clay_String label;
    Brick_ElementId id;
    bool hovered;
    bool clicked;
    bool pressed;
    bool blurred;
    bool released;
    bool toggled;
} Brick_Button;

typedef struct {
    size_t length;
    uint32_t ids[BRICK_MAX_BUTTON_GROUP_SIZE];
} Brick_ElementGroup;

//TODO: rename HOVER to HOVER_START, HOVER, HOVER_END
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
    // Triggers when the button is switched on
    // BRICK_EVENT_TOGGLED_ON,
    // Triggers when the button is switched off
    // BRICK_EVENT_TOGGLED_OFF
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

typedef struct Brick_ButtonGroupArray {
    size_t length;
    Brick_ElementGroup* data;
} Brick_ButtonGroupArray;

Brick_ElementGroup Brick_ElementGroup_DEFAULT = CLAY__DEFAULT_STRUCT;
Brick_ElementGroup* Brick_ButtonGroupArray_Get(Brick_ButtonGroupArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_ElementGroup_DEFAULT;
}

typedef struct Brick_Elements {
    Brick_ButtonArray buttons;
    Brick_ButtonGroupArray buttonGroups;
} Brick_Elements;

// Global Context
// --------------------------

// Clay context
Clay_Arena g_clay_arena = CLAY__DEFAULT_STRUCT;

// Window state also holds pointer state
Brick_Window g_window = CLAY__DEFAULT_STRUCT;

// Main element state arrays
Brick_Button g_buttons[BRICK_MAX_BUTTONS];
Brick_ElementGroup g_button_groups[BRICK_MAX_BUTTON_GROUPS];

Brick_Elements g_elements = {
    .buttons = {
        .length = 0,
        .data = g_buttons
    },
    .buttonGroups = {
        .length = 0,
        .data = g_button_groups
    },
};
// keeps track of total elements created
size_t g_element_count = 0;

// event array passed back to user to handle events
Brick_Event g_events[BRICK_MAX_ELEMENTS];

// Button internals
// ---------------------------------------------------------------

void Brick_OnButtonHover(uint32_t idx, bool isHovering) {
    // NOTE: gets called on every frame with every button
    Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, idx);
    
    if (isHovering) {
        if (g_window.hoveredId != idx && g_window.lastHoveredId != idx) {
            g_window.hoveredId = idx;
            button->hovered = true;
        } else if (g_window.hoveredId == idx && g_window.lastHoveredId != idx) {
            g_window.lastHoveredId = idx;
        } 
        // TODO: quick test this invalid states
        // else if (g_window.hoveredId != id && g_window.lastHoveredId == id) { }
    } else {
        if (g_window.hoveredId == idx && g_window.lastHoveredId == idx) {
            g_window.hoveredId = 0;
            button->hovered = false;
            button->blurred = true;
        // } else if (g_window.hoveredId == id && g_window.lastHoveredId != id) {
        } else if (g_window.hoveredId != idx && g_window.lastHoveredId == idx) {
            g_window.lastHoveredId = 0;
            button->blurred = false;
        }
    }
    // Compares the cached pointer state with the current button hovered.
    // WARN: Conditional order is very sensitive.
    // 1. pointer enters button area - HOVER
    // 2. pointer is still in button area - HOVERING
    // 3. pointer leaves button area - BLUR
    // 4. clear pointer cache and button state after #3
    // if (!isHovering && g_window.hoveredId == id) {
    //     // 3. BLUR: blur the current button
    //     button->hovered = false;
    //     button->blurred = true;
    //     g_window.hoveredId = 0;
    // } else if (!isHovering && g_window.lastHoveredId == id) {
    //     // 4. Clear BLUR. This allows for one frame of propagation of the blur
    //     g_window.lastHoveredId = 0;
    //     button->blurred = false;
    // } else if (isHovering && !button->hovered && g_window.hoveredId != id) {
    //     // 1. HOVER: mark button as hovered
    //     button->hovered = true;
    //     // clear the state from the last button
    //     Brick_Button* lastButton = Brick_ButtonArray_Get(&g_elements.buttons, g_window.lastHoveredId);
    //     lastButton->hovered = false;
    //     // NOTE: resetting lastHovered blur here in case pointer moves
    //     // between buttons very quickly and did not clear properly
    //     lastButton->blurred = false;
    //     // save whatever is currently hovering as the last hover
    //     g_window.lastHoveredId = g_window.hoveredId;
    //     // cache the button id on the global window
    //     g_window.hoveredId = id;
    // } else if (isHovering && g_window.hoveredId == id && g_window.lastHoveredId != id) {
    //     // 2. HOVERING: this allows for one frame of propagation when the button was hovered
    //     // and is queried to know the frame right after the button hovered
    //     g_window.lastHoveredId = id;
    // }
}

void Brick_HandleClayHover(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    
    for (uint32_t i = 1; i < g_elements.buttons.length; i++) {
        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, i);
        
        if(button->clayId.id == elementId.id) {
            switch(pointerData.state) {
            case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
                // printf("CLICK %d\n", elementId.id);
                button->clicked = true;
                button->toggled = !button->toggled;
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

Brick_ElementId Brick_CreateButton(const char* label);
uint32_t Brick_GroupButtons(const uint32_t* buttonIds, size_t groupSize);

// initializes Clay first, then Brick
void Brick_Initialize(float width, float height, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void *fontData), void *fontData) {
    printf("Initializing Brick\n");
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
    // seed button groups, needs to bypass GroupButton
    // because it is used to check validity
    // g_button_groups[0] = {
    //     .length = 1,
    //     .ids = &Brick_ElementGroup_DEFAULT
    // }
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
Brick_ElementId Brick_CreateButton(const char* label) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(label), 
        .chars = label 
    };
    // Clay_ElementId buttonId = CLAY_SID(clayString);

    uint32_t index = g_elements.buttons.length;
    Brick_ElementId buttonId = (Brick_ElementId){
        .index = index,
        .type = BRICK_ELEMENT_TYPE_BUTTON,
    };
    g_buttons[index] = (Brick_Button){
        .clayId = CLAY_SID(clayString),
        .label = clayString,
        .id = buttonId,
        .hovered = false,
        .clicked = false,
        .pressed = false,
        .blurred = false,
        .released = false,
        .toggled = false,
    };
    g_elements.buttons.length++;
    g_element_count++;

    return buttonId;
}

uint32_t Brick_GroupButtons(const Brick_ElementId* buttonIds, size_t groupSize) {

    bool validGroup = true;
    Brick_ElementGroup group = CLAY__DEFAULT_STRUCT;

    for (uint32_t i = 0; i < groupSize; i++) {
        if (buttonIds[i].type != BRICK_ELEMENT_TYPE_BUTTON) {
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            validGroup = false;
            break;
        }

        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonIds[i].index);

        if (button->id.index <= 0 || button->id.index > g_elements.buttons.length) {
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            validGroup = false;
            break;
        }

        group.ids[i] = button->id.index;
        group.length++;
    }

    // TODO: exit or handle error instead of return 0 here (0 is valid group!)
    if (!validGroup) return 0;

    uint32_t index = g_elements.buttonGroups.length;
    g_button_groups[index] = group;
    g_elements.buttonGroups.length++;
    
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
            // button->blurred = false;
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_BLUR
            };
            events.length++;
        } 
        // else if(button->toggled) {
        //     // button->blurred = false;
        //     g_events[events.length] = {
        //         .id = i,
        //         .eventType = BRICK_EVENT_TOGGLED_ON
        //     };
        //     events.length++;
        // } 
        // else if(button->blurred) {
        //     // button->blurred = false;
        //     g_events[events.length] = {
        //         .id = i,
        //         .eventType = BRICK_EVENT_
        //     };
        //     events.length++;
        // } 


    }

    return events;
}

// Layout<element> is to be called within CLAY macros which are also encapsulated in other Brick elements
void Brick_LayoutButton(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);

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
        Brick_OnButtonHover(button->id.index, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHover, nullptr);
        CLAY_TEXT(button->label, STYLE_TEXT_CENTERED);
    }
}

void Brick_LayoutButtonGroup(int32_t groupId) {
    const Brick_ElementGroup* buttonGroup = Brick_ButtonGroupArray_Get(&g_elements.buttonGroups, groupId);

    for (uint32_t i = 0; i < buttonGroup->length; i++) {
        // TODO: create another internal interface to skip constructing brick_elementID
        Brick_LayoutButton((Brick_ElementId){ .index = buttonGroup->ids[i], .type = BRICK_ELEMENT_TYPE_BUTTON });
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

void Brick_BeginLayoutTogglePanel(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);

    if (button->toggled) {
        Brick_BeginLayoutPanel();
    }
}

void Brick_EndLayoutTogglePanel(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);
    
    if (button->toggled) {
        Clay__CloseElement();
    }
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
